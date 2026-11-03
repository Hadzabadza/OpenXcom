/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/InventoryState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Screen.h"
#include "../Geoscape/GeoscapeEventState.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextEdit.h"
#include "../Interface/TextList.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Savegame/Base.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/Soldier.h"
#include "SoldierAssignmentState.h"
#include "SoldierInfoState.h"

#include <iomanip>
#include <sstream>

namespace OpenXcom
{
/**
 * Initializes all the elements in the Soldier Assignment screen.
 * @param base Pointer to the base that the assignment table has been called from.
 * @param assignment Pointer to the assignment ruleset.
 */
SoldierAssignmentState::SoldierAssignmentState(Base *base, RuleAssignment *assignment) :
	_base(base), _origSoldierOrder(*_base->getSoldiers()), _mainOffset(0), _ruleAssignment(*assignment),
	_minSoldiers(assignment->getMinSoldiers()), _maxSoldiers(assignment->getMaxSoldiers()),  _lmb(false),
	_activeStatColor(127), _inactiveStatColor(7), _outcomeCount(_ruleAssignment.getOutcomeNames().size()), bars(*new std::string(320, '|'))
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0, POPUP_BOTH);
	_btnOk = new TextButton(148, 16, 164, 176);
	_btnSendOff = new TextButton(148, 16, 8, 176);
	_txtMinimumSoldiers = new Text(32, 16, 220, 155);
	_txtSelectedSoldiers = new Text(96, 16, 249, 155);
	_txtTitle = new Text(200, 17, 5, 9);
	_txtName = new Text(114, 9, 148, 32);
	_txtContribution = new Text(144, 9, 148, 32);
	_txtDescriptionTitle = new Text(124, 9, 8, 32);
	_txtDescription = new Text(124, 120, 8, 40);
	_lstRelevantStatsTable = new TextList(288, 9, 5, 155);
	_lstSoldiers = new TextList(150, 104, 140, 40);
	_lstOutcomeDistribution = new TextList(308, 16, 8, 165);
	_cbxSortBy = new ComboBox(this, 120, 16, 192, 8, false);
	_interactiveWindow = new InteractiveSurface(320, 200, 0, 0);

	// Set palette
	setInterface("soldierAssignments");

	add(_window, "window", "soldierAssignments");
	add(_btnOk, "button", "soldierAssignments");
	add(_btnSendOff, "button", "soldierAssignments");
	add(_txtMinimumSoldiers, "text", "soldierAssignments");
	add(_txtSelectedSoldiers, "text", "soldierAssignments");
	add(_txtTitle, "text", "soldierAssignments");
	add(_txtName, "text", "soldierAssignments");
	add(_txtContribution, "text", "soldierAssignments");
	add(_txtDescriptionTitle, "text", "soldierAssignments");
	add(_txtDescription, "text", "soldierAssignments");
	add(_lstRelevantStatsTable, "list2", "soldierAssignments");
	add(_lstSoldiers, "list", "soldierAssignments");
	add(_cbxSortBy, "button", "soldierList");
	add(_lstOutcomeDistribution, "list", "soldierAssignments");
	add(_interactiveWindow, "interactiveWindow", "soldierAssignments");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "soldierAssignments");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&SoldierAssignmentState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&SoldierAssignmentState::btnOkClick, Options::keyCancel);

	_btnSendOff->setText(tr("STR_SEND_OFF").arg(""));
	_btnSendOff->onMouseClick((ActionHandler)&SoldierAssignmentState::btnSendOffClick);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr(_ruleAssignment.getName()));

	_txtName->setText(tr("STR_NAME_UC"));
	_txtMinimumSoldiers->setText(tr("STR_ASSIGNMENT_MIN_SOLDIERS_UC").arg(_minSoldiers));
	_txtSelectedSoldiers->setText(tr("STR_ASSIGNMENT_SELECTED_SOLDIERS_UC").arg(0).arg(_maxSoldiers));
	_txtContribution->setText(tr("STR_ASSIGNMENT_CONTRIBUTION_UC"));
	_txtContribution->setAlign(ALIGN_RIGHT);
	_txtDescriptionTitle->setAlign(ALIGN_CENTER);
	_txtDescriptionTitle->setText(tr("STR_ASSIGNMENT_SUMMARY_UC"));
	_txtDescription->setText(tr(_ruleAssignment.getDescription()));
	_txtDescription->setVerticalAlign(ALIGN_TOP);
	_txtDescription->setAlign(ALIGN_CENTER);
	_txtDescription->setWordWrap(true);
	_txtDescription->setColor(_txtDescription->getSecondaryColor());
	_txtDescription->setSecondaryColor(_activeStatColor);
	std::vector<std::string> sortOptions;
	sortOptions.push_back(tr("STR_ORIGINAL_ORDER"));
	for (std::string s : _ruleAssignment.getOutcomeNames())
	{
		sortOptions.push_back(tr(s));
	}
	_cbxSortBy->onChange((ActionHandler)&SoldierAssignmentState::cbxSortByChange);

	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(2);
	_lstSoldiers->onMousePress((ActionHandler)&SoldierAssignmentState::lstSoldiersMousePress);
	_lstSoldiers->onMouseOver((ActionHandler)&SoldierAssignmentState::lstSoldiersMultiselect);

	// Hack to make sure that mouse releases outside _lstSoldiers get registered.
	_interactiveWindow->onMouseRelease((ActionHandler)&SoldierAssignmentState::lstSoldiersMouseRelease);
;
	if (_game->getMod()->isManaFeatureEnabled())
	{
		_lstRelevantStatsTable->setColumnsVector(std::vector<size_t>(12, 18));
	}
	else
	{
		_lstRelevantStatsTable->setColumnsVector(std::vector<size_t>(11, 18));
	}
	_lstRelevantStatsTable->setAlign(ALIGN_CENTER);
}

/**
 *
 */
SoldierAssignmentState::~SoldierAssignmentState()
{

}

/**
 * Initializes the screen (fills the list).
 */
void SoldierAssignmentState::init()
{
	State::init();
	initSoldierList(_lstSoldiers->getScroll());
	initStatList();
	recalculateChances();
}

void SoldierAssignmentState::initSoldierList(size_t scrl)
{
	_lstSoldiers->clearList();

	_filteredListOfSoldiers.clear();
	_filteredIndicesOfSoldiers.clear();

	std::string selAction = "STR_SOLDIER_INFO";
	int offset = 0;
	if (selAction == "STR_SOLDIER_INFO")
	{
		// _lstSoldiers->setArrowColumn(188, ARROW_VERTICAL);

		// all soldiers in the base
		_filteredListOfSoldiers = *_base->getSoldiers();
	}
	else
	{
		offset = 20;
		// _lstSoldiers->setArrowColumn(-1, ARROW_VERTICAL);

		// filtered list of soldiers eligible for transformation
		RuleSoldierTransformation *transformationRule = _game->getMod()->getSoldierTransformation(selAction);
		if (transformationRule)
		{
			int idx = -1;
			for (auto* soldier : *_base->getSoldiers())
			{
				idx++;
				if (soldier->getCraft() && soldier->getCraft()->getStatus() == "STR_OUT")
				{
					// soldiers outside of the base are not eligible
					continue;
				}
				if (soldier->isEligibleForTransformation(transformationRule))
				{
					_filteredListOfSoldiers.push_back(soldier);
					_filteredIndicesOfSoldiers.push_back(idx);
				}
			}
			for (auto* deadMan : *_game->getSavedGame()->getDeadSoldiers())
			{
				if (deadMan->isEligibleForTransformation(transformationRule))
				{
					_filteredListOfSoldiers.push_back(deadMan);
					_filteredIndicesOfSoldiers.push_back(-1); // invalid
				}
			}
		}
	}

	fillContributionTable();

	std::vector<size_t> lstSoldierColumns(_outcomeCount+1,20);
	lstSoldierColumns[0] = 90;
	_lstSoldiers->setColumnsVector(lstSoldierColumns);

	BaseSumDailyRecovery recovery = _base->getSumRecoveryPerDay();
	unsigned int row = 0;
	for (const auto* soldier : _filteredListOfSoldiers)
	{
		std::string craftString = soldier->getCraftString(_game->getLanguage(), recovery);
		std::vector<std::string> lstSoldierContent(1+_outcomeCount, "");
		lstSoldierContent[0] = soldier->getName(true);
		for (int i=0; i<_outcomeCount; i++)
		{
			lstSoldierContent[1+i]=std::to_string(_contributionMap.at(soldier).at(i));
		}
		_lstSoldiers->addRowVectorString(lstSoldierContent);
		for (int i=0; i<_outcomeCount; i++)
		{
			_lstSoldiers->setCellColor(row, i+1, _ruleAssignment.getOutcomeColors().at(i));
		}
		_toggledRows.insert(std::pair(row,false));
		auto soldier_iter = std::find(_selectedSoldiers.begin(), _selectedSoldiers.end(), soldier);
		if (soldier_iter != _selectedSoldiers.end())
		{
			_lstSoldiers->setCellColor(row,0, _lstSoldiers->getSecondaryColor());
		}
		row++;
	}
	if (scrl)
		_lstSoldiers->scrollTo(scrl);
	_lstSoldiers->draw();
}

void SoldierAssignmentState::initStatList()
{
	_lstRelevantStatsTable->clearList();
	std::vector<std::string> stat_shorts(11, "");
	stat_shorts[0] = tr("STR_TIME_UNITS_ABBREVIATION");
	stat_shorts[1] = tr("STR_STAMINA_ABBREVIATION");
	stat_shorts[2] = tr("STR_HEALTH_ABBREVIATION");
	stat_shorts[3] = tr("STR_BRAVERY_ABBREVIATION");
	stat_shorts[4] = tr("STR_REACTIONS_ABBREVIATION");
	stat_shorts[5] = tr("STR_FIRING_ACCURACY_ABBREVIATION");
	stat_shorts[6] = tr("STR_THROWING_ACCURACY_ABBREVIATION");
	stat_shorts[7] = tr("STR_MELEE_ACCURACY_ABBREVIATION");
	stat_shorts[8] = tr("STR_STRENGTH_ABBREVIATION");
	stat_shorts[9] = tr("STR_PSIONIC_STRENGTH_ABBREVIATION");
	stat_shorts[10] = tr("STR_PSIONIC_SKILL_ABBREVIATION");
	if (_game->getMod()->isManaFeatureEnabled())
	{
		stat_shorts.push_back(tr("STR_MANA_ABBREVIATION"));
	}
	_lstRelevantStatsTable->addRowVectorString(stat_shorts);
	colorRelevantStatsTable(stat_shorts.size());
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void SoldierAssignmentState::btnOkClick(Action *)
{
	_game->popState();
}

/**
* Shows the Statistics screen.
*/
void SoldierAssignmentState::btnSendOffClick()
{
	if (_selectedSoldiers.size()>=_minSoldiers){
		const RuleEvent *resolution_event = _game->getMod()->getEvent(_resolutionsList.choose(), true);
		if (resolution_event)
		{
			_game->pushState(new GeoscapeEventState(*resolution_event));
		}
	}
}

void SoldierAssignmentState::toggleSoldier(int row)
{
	if (row >= 0 && _toggledRows.at(row) != true)
	{
		_toggledRows.at(row) = true;
		Soldier *_soldier = _filteredListOfSoldiers[row];
		auto soldier_iter = std::find(_selectedSoldiers.begin(), _selectedSoldiers.end(), _soldier);
		if (soldier_iter == _selectedSoldiers.end())
		{
			if (_selectedSoldiers.size()<_maxSoldiers)
			{
				_selectedSoldiers.push_back(_soldier);
				_lstSoldiers->setCellColor(row,0, _lstSoldiers->getSecondaryColor());
				recalculateChances();
			}
		}
		else
		{
			_selectedSoldiers.erase(soldier_iter);
			_lstSoldiers->setCellColor(row,0, _lstSoldiers->getColor());
			recalculateChances();
		}
		_txtSelectedSoldiers->setText(tr("STR_ASSIGNMENT_SELECTED_SOLDIERS_UC").arg(_selectedSoldiers.size()).arg(_maxSoldiers));
	}
}

/**
 * Invokes a singular soldier toggling on mouse press and prevents glitchy behaviour from duplicating with movement.
 * @param action Pointer to an action.
 */

void SoldierAssignmentState::lstSoldiersMousePress(Action *action)
{
	if (_game->isLeftClick(action, true))
	{
		toggleSoldier(_lstSoldiers->getSelectedRow());
		_lmb=true;
	}
}

/**
 * Shows the selected soldier's info if right click or stops toggling soldiers if left.
 * @param action Pointer to an action.
 */

void SoldierAssignmentState::lstSoldiersMouseRelease(Action *action)
{
	if (_game->isLeftClick(action, true))
	{
		for (auto& [key, value] : _toggledRows) value = false;
		_lmb=false;
	}
	else if (_game->isMiddleClick(action, true))
	{
		_game->pushState(new SoldierInfoState(_base, _lstSoldiers->getSelectedRow(), false));
	}
	else if (_game->isRightClick(action, true))
	{
		enterInventory();
	}
}

/**
 * Invokes soldier toggling on mouse movements for faster selection.
 * @param action Pointer to an action.
 */

void SoldierAssignmentState::lstSoldiersMultiselect()
{
	if (_lmb)
	{
		toggleSoldier(_lstSoldiers->getSelectedRow());
	}
}

/**
* Copypasted inventory screen from SoldiersState.
*/

void SoldierAssignmentState::enterInventory()
{
		// Copypasted from SoldiersState
		if (_base->getAvailableSoldiers(true, true) > 0)
		{
			SavedBattleGame *bgame = new SavedBattleGame(_game->getMod(), _game->getLanguage());
			_game->getSavedGame()->setBattleGame(bgame);
			bgame->setMissionType("STR_BASE_DEFENSE");

			if (_game->isCtrlPressed() && _game->isAltPressed())
			{
				_game->getSavedGame()->setDisableSoldierEquipment(true);
			}
			BattlescapeGenerator bgen = BattlescapeGenerator(_game);
			bgen.setBase(_base);
			bgen.runInventory(0);

			// pre-select the soldier under the mouse cursor (if possible)
			if (_availableOptions.empty())
			{
				size_t idx = _lstSoldiers->getSelectedRow();
				if (idx < _base->getSoldiers()->size())
				{
					int soldierId = _base->getSoldiers()->at(idx)->getId();
					for (auto* unit : *bgame->getUnits())
					{
						if (unit->getId() == soldierId)
						{
							bgame->setSelectedUnit(unit);
							break;
						}
					}
				}
			}

			_game->getScreen()->clear();
			_game->pushState(new InventoryState(false, 0, _base, true));
		}
}

int SoldierAssignmentState::countContribution(const Soldier *soldier, size_t outcome)
{
	const std::map<std::string, float>* ratios = &_ruleAssignment.getOutcomeStatsAndRatios().at(outcome);
	float contribution = 0;
	if (ratios->find("time units")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->tu) * ratios->at("timeUnits"); }
	if (ratios->find("stamina")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->stamina) * ratios->at("stamina"); }
	if (ratios->find("health")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->health) * ratios->at("health"); }
	if (ratios->find("bravery")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->bravery) * ratios->at("bravery"); }
	if (ratios->find("reactions")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->reactions) * ratios->at("reactions"); }
	if (ratios->find("firing")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->firing) * ratios->at("firing"); }
	if (ratios->find("throwing")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->throwing) * ratios->at("throwing"); }
	if (ratios->find("melee")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->melee) * ratios->at("melee"); }
	if (ratios->find("strength")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->strength) * ratios->at("strength"); }
	if (ratios->find("psi strength")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->psiStrength) * ratios->at("psi strength"); }
	if (ratios->find("psi skill")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->psiSkill) * ratios->at("psi skill"); }
	if (ratios->find("mana")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->mana) * ratios->at("mana"); }
	return floor(contribution);
}

void SoldierAssignmentState::fillContributionTable()
{
	for (Soldier * sol : _filteredListOfSoldiers)
	{
		std::vector soldierContribution(_outcomeCount, 0);
		for (int i=0; i<_outcomeCount; i++)
		{
			soldierContribution[i] = countContribution(sol, i);
		}
		_contributionMap.emplace(sol,soldierContribution);
	}
}

void SoldierAssignmentState::recalculateChances()
{
	int totalWeight=0;
	std::vector weights(_outcomeCount,0);
	for (int i=0; i<_outcomeCount; i++)
	{
		int currentWeight = _ruleAssignment.getOutcomeBaseWeights().at(i);
		for (auto sol:_selectedSoldiers)
		{
			currentWeight+=_contributionMap.at(sol).at(i);
		}
		// Log(LOG_DEBUG) << "currentWeight[" << i << "]: " << currentWeight;
		currentWeight = std::max(0, currentWeight);
		totalWeight+=currentWeight;
		weights[i]=currentWeight;
		_resolutionsList.set(_ruleAssignment.getResolutionTypes().at(i),currentWeight);
		// Log(LOG_DEBUG) << "weights[" << i << "]: " << weights[i];
	}
	std::vector<size_t> widths(_outcomeCount,0);;
	// Log(LOG_DEBUG) << "totalWeight: " << totalWeight;
	for (int i=0; i<_outcomeCount; i++)
	{
		float ratio = (float)weights[i]/(float)totalWeight;
		widths[i]=floor(304.*ratio);
		// Log(LOG_DEBUG) << "width[" << i << "]: " << widths[i];
	}
	resetOutcomeDistributionsList(widths);
}

/**
 * Helper for coloring the relevant stats table according to outcomes. If a stat affects a single outcome, it will get colored it in its color, if none - _inactiveStatColor, if multiple - _activeStatColor.
 * @param rowCount Amount of rows for stats (11 without mana and 12 with).
 */

void SoldierAssignmentState::colorRelevantStatsTable(int rowCount)
{
	std::vector statColoring(rowCount, _inactiveStatColor);
	const std::vector<std::map<std::string, float>> * outcomes = &_ruleAssignment.getOutcomeStatsAndRatios();
	for (int i=0; i<outcomes->size(); i++)
	{
		const std::map<std::string, float>* stats = &outcomes->at(i);
		if (stats->find("time units")!=stats->end())
		{
			if (statColoring[0]!=_inactiveStatColor) { statColoring[0]=_activeStatColor; }
			else { statColoring[0]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("stamina")!=stats->end())
		{
			if (statColoring[1]!=_inactiveStatColor) { statColoring[1]=_activeStatColor; }
			else { statColoring[1]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("health")!=stats->end())
		{
			if (statColoring[2]!=_inactiveStatColor) { statColoring[2]=_activeStatColor; }
			else { statColoring[2]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("bravery")!=stats->end())
		{
			if (statColoring[3]!=_inactiveStatColor) { statColoring[3]=_activeStatColor; }
			else { statColoring[3]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("reactions")!=stats->end())
		{
			if (statColoring[4]!=_inactiveStatColor) { statColoring[4]=_activeStatColor; }
			else { statColoring[4]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("firing")!=stats->end())
		{
			if (statColoring[5]!=_inactiveStatColor) { statColoring[5]=_activeStatColor; }
			else { statColoring[5]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("throwing")!=stats->end())
		{
			if (statColoring[6]!=_inactiveStatColor) { statColoring[6]=_activeStatColor; }
			else { statColoring[6]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("melee")!=stats->end())
		{
			if (statColoring[7]!=_inactiveStatColor) { statColoring[7]=_activeStatColor; }
			else { statColoring[7]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("strength")!=stats->end())
		{
			if (statColoring[8]!=_inactiveStatColor) { statColoring[8]=_activeStatColor; }
			else { statColoring[8]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("psi strength")!=stats->end())
		{
			if (statColoring[9]!=_inactiveStatColor) { statColoring[9]=_activeStatColor; }
			else { statColoring[9]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("psi skill")!=stats->end())
		{
			if (statColoring[10]!=_inactiveStatColor) { statColoring[10]=_activeStatColor; }
			else { statColoring[10]=_ruleAssignment.getOutcomeColors().at(i); }
		}
		if (stats->find("mana")!=stats->end())
		{
			// Will never be accessed if there is no mana in the ruleset, no need for other checks.
			if (statColoring[11]!=_inactiveStatColor) { statColoring[11]=_activeStatColor; }
			else { statColoring[11]=_ruleAssignment.getOutcomeColors().at(i); }
		}
	}
	_lstRelevantStatsTable->setColor(_inactiveStatColor);
	for (int i=0; i<statColoring.size(); i++) { _lstRelevantStatsTable->setCellColor(0,i,statColoring[i]); }
}

void SoldierAssignmentState::resetOutcomeDistributionsList(const std::vector<size_t> &widths)
{
	_lstOutcomeDistribution->clearList();
	_lstOutcomeDistribution->setColumnsVector(widths);
	_lstOutcomeDistribution->addRowVector(std::vector (_outcomeCount, bars.c_str()));
	for (int i=0; i<_outcomeCount; i++)
	{
		_lstOutcomeDistribution->setCellColor(0, i, _ruleAssignment.getOutcomeColors().at(i));
	}
}

void SoldierAssignmentState::cbxSortByChange()
{
	Log(LOG_DEBUG) << "selected[" << _cbxSortBy->getSelected();
	std::stable_sort(_filteredListOfSoldiers.begin(), _filteredListOfSoldiers.end(),
	[&](Soldier* a, Soldier* b) {
		return _contributionMap.at(a).at(_cbxSortBy->getSelected()-1) < _contributionMap.at(b).at(_cbxSortBy->getSelected()-1);
	});
	initSoldierList(_lstSoldiers->getScroll());
}

}