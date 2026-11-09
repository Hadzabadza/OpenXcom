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
#include "../Interface/ComboBox.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Savegame/Base.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Transfer.h"
#include "SoldierAssignmentState.h"
#include "SoldierInfoState.h"

#include <iomanip>
#include <numeric>

namespace OpenXcom
{
/**
 * Initializes all the elements in the Soldier Assignment screen.
 * @param base Pointer to the base that the assignment table has been called from.
 * @param assignment Pointer to the assignment ruleset.
 */
SoldierAssignmentState::SoldierAssignmentState(Base *base, RuleAssignment *assignment) :
	_base(base), _origSoldierOrder(*_base->getSoldiers()), _mainOffset(0), _ruleAssignment(*assignment),
	_minSoldiers(assignment->getMinSoldiers()), _maxSoldiers(assignment->getMaxSoldiers()),  _lmb(false), _toggleTo(false),
	_activeStatColor(0), _inactiveStatColor(7), _outcomeCount(_ruleAssignment.getOutcomeNames().size())
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0, POPUP_BOTH);
	_btnOk = new TextButton(148, 16, 164, 176);
	_btnSendOff = new TextButton(148, 16, 8, 176);
	_txtCalculationType = new Text(240, 10, 8, 145);
	_txtMinimumSoldiers = new Text(32, 8, 234, 155);
	_txtMaximumSoldiers = new Text(48, 8, 258, 155);
	_txtTitle = new Text(200, 17, 5, 9);
	_txtName = new Text(120, 9, 8, 32);
	_txtContribution = new Text(18*_outcomeCount, 9, 144, 32);
	_btnToggleDescription = new ToggleTextButton(12, 12, 300, 26);
	_txtDescriptionTitle = new Text(304, 9, 8, 32);
	_txtDescription = new Text(304, 120, 8, 40);
	_lstRelevantStatsTable = new TextList(288, 9, 5, 155);
	_lstSoldiers = new TextList(288, 104, 8, 40);
	_lstOutcomeDistribution = new TextList(308, 16, 8, 165);
	_cbxSortBy = new ComboBox(this, 120, 16, 192, 8, false);
	_interactiveWindow = new InteractiveSurface(320, 200, 0, 0);

	// Set palette
	setInterface("soldierAssignments");

	add(_window, "window", "soldierAssignments");
	add(_btnOk, "button", "soldierAssignments");
	add(_btnSendOff, "button", "soldierAssignments");
	add(_txtCalculationType, "text", "soldierAssignments");
	add(_txtMinimumSoldiers, "text", "soldierAssignments");
	add(_txtMaximumSoldiers, "text", "soldierAssignments");
	add(_txtTitle, "text", "soldierAssignments");
	add(_txtName, "text", "soldierAssignments");
	add(_txtContribution, "text", "soldierAssignments");
	add(_btnToggleDescription, "button", "soldierAssignments");
	add(_txtDescriptionTitle, "text", "soldierAssignments");
	add(_txtDescription, "text", "soldierAssignments");
	add(_lstRelevantStatsTable, "list2", "soldierAssignments");
	add(_lstSoldiers, "list", "soldierAssignments");
	add(_cbxSortBy, "button", "soldierAssignments");
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
	_calculationType = _ruleAssignment.getOutcomeCalculationTypes()[0];
	_maxSoldiersStr = _maxSoldiers == std::numeric_limits<int>::max() ? "∞" : std::to_string(_maxSoldiers);
	for (int i=1; i<_ruleAssignment.getOutcomeCalculationTypes().size(); i++)
	{
		if (_calculationType!= _ruleAssignment.getOutcomeCalculationTypes()[i])
		{
			_calculationType = "STR_ASSIGNMENT_OUTCOME_COUNT_TYPE_NOTICE";
			break;
		}
	}
	_txtCalculationType->setText(tr("STR_ASSIGNMENT_OUTCOME_COUNT_TYPE_UC").arg(tr(_calculationType)));
	_txtMinimumSoldiers->setText(tr("STR_ASSIGNMENT_MIN_SOLDIERS_UC").arg(_minSoldiers));
	_txtMaximumSoldiers->setText(tr("STR_ASSIGNMENT_MAX_SOLDIERS_UC").arg(0).arg(_maxSoldiersStr));
	_txtMaximumSoldiers->setAlign(ALIGN_RIGHT);
	_txtMaximumSoldiers->onMouseRelease((ActionHandler)&SoldierAssignmentState::txtMaxSoldiersMouseRelease);
	_txtContribution->setText(tr("STR_ASSIGNMENT_CONTRIBUTION_UC"));
	_txtContribution->setAlign(ALIGN_CENTER);
	_btnToggleDescription->setText("?");
	_btnToggleDescription->onMouseClick((ActionHandler)&SoldierAssignmentState::btnToggleDescriptionClick);
	_txtDescriptionTitle->setAlign(ALIGN_CENTER);
	_txtDescriptionTitle->setText(tr("STR_ASSIGNMENT_SUMMARY_UC"));
	_txtDescriptionTitle->setVisible(_btnToggleDescription->getPressed());
	_txtDescriptionTitle->setVisible(false);
	_txtDescription->setText(tr(_ruleAssignment.getDescription()));
	_txtDescription->setVerticalAlign(ALIGN_TOP);
	_txtDescription->setAlign(ALIGN_CENTER);
	_txtDescription->setWordWrap(true);
	_txtDescription->setColor(_txtDescription->getSecondaryColor());
	_txtDescription->setSecondaryColor(_activeStatColor);
	_txtDescription->setVisible(false);
	std::vector<std::string> sortOptions;
	sortOptions.push_back(tr("STR_ORIGINAL_ORDER"));
	for (const std::string & s : _ruleAssignment.getOutcomeNames())
		sortOptions.push_back(tr(s));
	_cbxSortBy->setOptions(sortOptions);
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
		_lstRelevantStatsTable->setColumnsVector(std::vector<size_t>(12, 18));
	else
		_lstRelevantStatsTable->setColumnsVector(std::vector<size_t>(11, 18));
	_lstRelevantStatsTable->setAlign(ALIGN_CENTER);
	_lstOutcomeDistribution->setBackground(_window);
	_lstOutcomeDistribution->setSelectable(true);
	_lstOutcomeDistribution->setSelectableColumns(true);
	_lstOutcomeDistribution->onMouseRelease((ActionHandler)&SoldierAssignmentState::lstSortByClick);

	initStatList();
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
	initSoldierList(_lstSoldiers->getScroll(), &_filteredListOfSoldiers);
	recalculateChances();
}

void SoldierAssignmentState::initSoldierList(size_t scrl, std::vector<Soldier *> * filteredList)
{
	_lstSoldiers->clearList();

	// Needs a check after other state screen in case player modifies the soldiers list.
	std::vector<Soldier *> newList = *_base->getSoldiers();
	filterWounded(&newList);
	if (_origSoldierOrder.size()!= newList.size())
	{
		_filteredListOfSoldiers.clear();
		resetSoldierSelection();
	}
	else
	{
		_filteredListOfSoldiers = *filteredList;
	}

	_origSoldierOrder = newList;

	if (_filteredListOfSoldiers.empty())
	{
		_filteredListOfSoldiers = _origSoldierOrder;
		fillContributionTable();
	}
	std::vector<size_t> lstSoldierColumns(_outcomeCount+1,18);
	lstSoldierColumns[0] = 136;
	_lstSoldiers->setColumnsVector(lstSoldierColumns);

	unsigned int row = 0;
	for (const auto* soldier : _filteredListOfSoldiers)
	{
		std::vector<std::string> lstSoldierContent(1+_outcomeCount, "");
		lstSoldierContent[0] = soldier->getName(true);
		for (int i=0; i<_outcomeCount; i++)
			lstSoldierContent[1+i]=std::to_string(_contributionMap[soldier][i]);
		_lstSoldiers->addRowVectorString(lstSoldierContent);
		for (int i=0; i<_outcomeCount; i++)
			_lstSoldiers->setCellColor(row, i+1, _ruleAssignment.getOutcomeColors()[i]);

		_affectedRows.insert(std::pair(row,false));
		auto soldier_iter = std::find(_selectedSoldiers.begin(), _selectedSoldiers.end(), soldier);
		if (soldier_iter != _selectedSoldiers.end())
			_lstSoldiers->setCellColor(row,0, _lstSoldiers->getSecondaryColor());
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
		stat_shorts.push_back(tr("STR_MANA_ABBREVIATION"));
	_lstRelevantStatsTable->addRowVectorString(stat_shorts);
	colorRelevantStatsTable();
}

/**
 * Returns to the previous screen.
 */
void SoldierAssignmentState::btnOkClick()
{
	_game->popState();
}

/**
* Shows the Statistics screen.
*/
void SoldierAssignmentState::btnSendOffClick()
{
	if (_selectedSoldiers.size()>=_minSoldiers){
		int outComeIndex = std::stoi(_resolutionsList.choose());
		int transferTime = _ruleAssignment.getOutcomeTransferTimes()[outComeIndex];
		int recoveryTime = _ruleAssignment.getOutcomeRecoveryTimes()[outComeIndex];
		for (auto sol : _selectedSoldiers)
		{
			if (recoveryTime > 0)
				sol->setWoundRecovery(recoveryTime);

			if (transferTime > 0)
			{
				// handle training (transfer rules)
				sol->setPsiTraining(false);
				if (sol->isInTraining())
					sol->setReturnToTrainingWhenHealed(true);
				sol->setTraining(false);

				auto transfer = new Transfer(transferTime);
				transfer->setSoldier(sol);
				_base->getTransfers()->push_back(transfer);
				auto it = find(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), sol);
				auto it2 = find(_filteredListOfSoldiers.begin(), _filteredListOfSoldiers.end(), sol);

				if (it != _base->getSoldiers()->end())
					_base->getSoldiers()->erase(it);
				if (it2 != _filteredListOfSoldiers.end())
					_filteredListOfSoldiers.erase(it2);
			}
		}
		resetSoldierSelection();
		initSoldierList(_lstSoldiers->getScroll(), &_filteredListOfSoldiers);
		const RuleEvent *resolution_event = _game->getMod()->getEvent(_ruleAssignment.getResolutionTypes()[outComeIndex], true);
		if (resolution_event)
			_game->pushState(new GeoscapeEventState(*resolution_event));
	}
}

void SoldierAssignmentState::toggleSoldier(int row)
{
	if (row >= 0 && _affectedRows[row] != true)
	{
		_affectedRows[row] = true;
		Soldier *_soldier = _filteredListOfSoldiers[row];
		auto soldier_iter = std::find(_selectedSoldiers.begin(), _selectedSoldiers.end(), _soldier);
		if (soldier_iter == _selectedSoldiers.end())
		{
			if (_selectedSoldiers.size()<_maxSoldiers && _toggleTo)
			{
				_selectedSoldiers.push_back(_soldier);
				_lstSoldiers->setCellColor(row,0, _lstSoldiers->getSecondaryColor());
				recalculateChances();
			}
		}
		else
		{
			if (!_toggleTo)
			{
				_selectedSoldiers.erase(soldier_iter);
				_lstSoldiers->setCellColor(row,0, _lstSoldiers->getColor());
				recalculateChances();
			}
		}
		_txtMaximumSoldiers->setText(tr("STR_ASSIGNMENT_MAX_SOLDIERS_UC").arg(_selectedSoldiers.size()).arg(_maxSoldiersStr));
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
		if (!_game->isCtrlPressed(true))
		{
			int row = _lstSoldiers->getSelectedRow();
			Soldier *_soldier = _filteredListOfSoldiers[row];
			auto soldier_iter = std::find(_selectedSoldiers.begin(), _selectedSoldiers.end(), _soldier);
			_toggleTo = soldier_iter == _selectedSoldiers.end();
			toggleSoldier(row);
			_lmb=true;
		}
	}
}

/**
 * Shows the selected soldier's info if right click or stops toggling soldiers if left.
 * @param action Pointer to an action.
 */

void SoldierAssignmentState::lstSoldiersMouseRelease(Action *action)
{
	unsigned int origId = origSoldierId(_lstSoldiers->getSelectedRow());
	if (_game->isLeftClick(action, true))
	{
		if (_game->isCtrlPressed(true) && _lstSoldiers->isMouseCursorOverMe(this))
		{
			txtMaxSoldiersMouseRelease(action);
		}
		else
		{
			for (auto& [key, value] : _affectedRows) value = false;
			_lmb=false;
		}
	}
	else if (_game->isMiddleClick(action, true) && _lstSoldiers->isMouseCursorOverMe(this) && origId!=4294967295)
	{
		_game->pushState(new SoldierInfoState(_base, origId, false));
	}
	else if (_game->isRightClick(action, true) && _lstSoldiers->isMouseCursorOverMe(this))
	{
		if (_game->isCtrlPressed(true))
			txtMaxSoldiersMouseRelease(action);
		else
			enterInventory(origId);
	}
}

void SoldierAssignmentState::txtMaxSoldiersMouseRelease(Action *action)
{
	size_t ceil=0;
	if (_game->isLeftClick(action, true))
		if (_minSoldiers==0)
			ceil = _maxSoldiers;
		else
			ceil = _minSoldiers;
	else if (_game->isRightClick(action, true))
		ceil = _maxSoldiers;
	if (_selectedSoldiers.empty())
	{
		for (int i=0; i<ceil; i++)
			if (_filteredListOfSoldiers.size() > i)
				_selectedSoldiers.push_back(_filteredListOfSoldiers[i]);
			else
				break;
		_txtMaximumSoldiers->setText(tr("STR_ASSIGNMENT_MAX_SOLDIERS_UC").arg(_selectedSoldiers.size()).arg(_maxSoldiersStr));
		recalculateChances();
	}
	else
		resetSoldierSelection();
	initSoldierList(_lstSoldiers->getScroll(), &_filteredListOfSoldiers);
}

/**
 * Toggles showing description or the soldiers table.
 */
void SoldierAssignmentState::btnToggleDescriptionClick()
{
	_txtName->setVisible(!_btnToggleDescription->getPressed());
	_txtContribution->setVisible(!_btnToggleDescription->getPressed());
	_lstSoldiers->setVisible(!_btnToggleDescription->getPressed());
	_txtDescription->setVisible(_btnToggleDescription->getPressed());
	_txtDescriptionTitle->setVisible(_btnToggleDescription->getPressed());
}

/**
 * Invokes soldier toggling on mouse movements for faster selection.
 * @param action Pointer to an action.
 */

void SoldierAssignmentState::lstSoldiersMultiselect()
{
	if (_lmb)
		toggleSoldier(_lstSoldiers->getSelectedRow());
}

/**
* Copypasted inventory screen from SoldiersState.
*/

void SoldierAssignmentState::enterInventory(size_t origId) const
{
		// Copypasted from SoldiersState
		if (_base->getAvailableSoldiers(true, true) > 0 && origId!=4294967295)
		{
			auto *bgame = new SavedBattleGame(_game->getMod(), _game->getLanguage());
			_game->getSavedGame()->setBattleGame(bgame);
			bgame->setMissionType("STR_BASE_DEFENSE");

			if (_game->isCtrlPressed() && _game->isAltPressed())
				_game->getSavedGame()->setDisableSoldierEquipment(true);
			BattlescapeGenerator bgen = BattlescapeGenerator(_game);
			bgen.setBase(_base);
			bgen.runInventory(nullptr);

			// pre-select the soldier under the mouse cursor (if possible)
			if (_availableOptions.empty())
			{
				if (origId < _base->getSoldiers()->size())
				{
					int soldierId = _base->getSoldiers()->at(origId)->getId();
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
			_game->pushState(new InventoryState(false, nullptr, _base, true));
		}
}

int SoldierAssignmentState::countContribution(const Soldier *soldier, const size_t outcome) const
{
	const std::map<std::string, float>* ratios = &_ruleAssignment.getOutcomeStatsAndRatios()[outcome];
	float contribution = 0;
	if (ratios->find("timeUnits")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->tu) * ratios->at("timeUnits"); }
	if (ratios->find("stamina")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->stamina) * ratios->at("stamina"); }
	if (ratios->find("health")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->health) * ratios->at("health"); }
	if (ratios->find("bravery")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->bravery) * ratios->at("bravery"); }
	if (ratios->find("reactions")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->reactions) * ratios->at("reactions"); }
	if (ratios->find("firing")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->firing) * ratios->at("firing"); }
	if (ratios->find("throwing")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->throwing) * ratios->at("throwing"); }
	if (ratios->find("melee")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->melee) * ratios->at("melee"); }
	if (ratios->find("strength")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->strength) * ratios->at("strength"); }
	if (ratios->find("psiStrength")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->psiStrength) * ratios->at("psiStrength"); }
	if (ratios->find("psiSkill")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->psiSkill) * ratios->at("psiSkill"); }
	if (ratios->find("mana")!=ratios->end()) { contribution+= static_cast<float>(soldier->getCurrentStats()->mana) * ratios->at("mana"); }
	return floor(contribution);
}

void SoldierAssignmentState::fillContributionTable()
{
	for (Soldier * sol : _filteredListOfSoldiers)
	{
		std::vector soldierContribution(_outcomeCount, 0);
		for (int i=0; i<_outcomeCount; i++)
			soldierContribution[i] = countContribution(sol, i);
		_contributionMap.emplace(sol,soldierContribution);
	}
}

void SoldierAssignmentState::recalculateChances()
{
	int totalWeight=0;
	std::vector weights(_outcomeCount,0);
	for (int i=0; i<_outcomeCount; i++)
	{
		std::string calcTypes = _ruleAssignment.getOutcomeCalculationTypes()[i];
		int currentWeight=0;
		if (!_selectedSoldiers.empty())
		{
			currentWeight=_contributionMap[_selectedSoldiers[0]][i];
			for (int j=1; j<_selectedSoldiers.size(); j++)
			{
				if (calcTypes == "STR_ASSIGNMENT_OUTCOME_COUNT_TYPE_ADDITIVE_UC"|| calcTypes == "STR_ASSIGNMENT_OUTCOME_COUNT_TYPE_AVERAGE_UC")
				{
					currentWeight+=_contributionMap[_selectedSoldiers[j]][i];
				} else if (calcTypes == "STR_ASSIGNMENT_OUTCOME_COUNT_TYPE_LOWEST_UC")
				{
					if (abs(_contributionMap[_selectedSoldiers[j]][i])<abs(currentWeight))
						currentWeight=_contributionMap[_selectedSoldiers[j]][i];
				}
			}
			if (calcTypes == "STR_ASSIGNMENT_OUTCOME_COUNT_TYPE_AVERAGE_UC")
				currentWeight/=int(_selectedSoldiers.size());
		}

		currentWeight += _ruleAssignment.getOutcomeBaseWeights()[i];
		currentWeight = std::max(0, currentWeight);
		_resolutionsList.set(std::to_string(i),currentWeight);
		totalWeight+=currentWeight;
		weights[i]=currentWeight;
	}

	std::vector<size_t> widths(_outcomeCount,0);;
	// Log(LOG_DEBUG) << "totalWeight: " << totalWeight;
	for (int i=0; i<_outcomeCount; i++)
	{
		float ratio = (float)weights[i]/(float)totalWeight;
		widths[i]=floor(304.*ratio);
	}
	resetOutcomeDistributionsList(widths);
}

/**
 * Helper for coloring the relevant stats table according to outcomes. If a stat affects a single outcome, it will get colored it in its color, if none - _inactiveStatColor, if multiple - _activeStatColor.
 * @param outcomeIndex the index of the outcome that should color its relevants stats.
 */

void SoldierAssignmentState::colorRelevantStatsTable(int outcomeIndex)
{
	std::vector<Uint8> outcomeIndices;
	std::vector statColoring(11+_game->getMod()->isManaFeatureEnabled(), std::vector<Uint8>(0));

	if (outcomeIndex==-1)
	{
		outcomeIndices.resize(_outcomeCount);
		std::iota(outcomeIndices.begin(), outcomeIndices.end(),0);
	}
	else
		outcomeIndices.resize(1, outcomeIndex);

	for (const Uint8 i:outcomeIndices)
	{
		const std::map<std::string, float>* stats = &_ruleAssignment.getOutcomeStatsAndRatios()[i];
		const Uint8 color=_ruleAssignment.getOutcomeColors()[i];
		if (stats->find("timeUnits")!=stats->end())
			statColoring[0].push_back(color);
		if (stats->find("stamina")!=stats->end())
			statColoring[1].push_back(color);
		if (stats->find("health")!=stats->end())
			statColoring[2].push_back(color);
		if (stats->find("bravery")!=stats->end())
			statColoring[3].push_back(color);
		if (stats->find("reactions")!=stats->end())
			statColoring[4].push_back(color);
		if (stats->find("firing")!=stats->end())
			statColoring[5].push_back(color);
		if (stats->find("throwing")!=stats->end())
			statColoring[6].push_back(color);
		if (stats->find("melee")!=stats->end())
			statColoring[7].push_back(color);
		if (stats->find("strength")!=stats->end())
			statColoring[8].push_back(color);
		if (stats->find("psiStrength")!=stats->end())
			statColoring[9].push_back(color);
		if (stats->find("psiSkill")!=stats->end())
			statColoring[10].push_back(color);
		if (stats->find("mana")!=stats->end())
			statColoring[11].push_back(color);
	}
	_lstRelevantStatsTable->setColor(_inactiveStatColor);
	for (int i=0; i<statColoring.size(); i++)
	{
		if (!statColoring[i].empty())
		{
			if (statColoring[i].size()==1)
				_lstRelevantStatsTable->setCellColor(0,i,statColoring[i][0]);
			else
				_lstRelevantStatsTable->setCellColor(0,i,_activeStatColor);
		}
		else
		{
			_lstRelevantStatsTable->setCellColor(0,i,_inactiveStatColor);
		}
	}
}

void SoldierAssignmentState::resetOutcomeDistributionsList(const std::vector<size_t> &widths)
{
	_lstOutcomeDistribution->clearList();
	_lstOutcomeDistribution->setColumnsVector(widths);
	_lstOutcomeDistribution->addRowVectorString(_ruleAssignment.getOutcomeCharacters());
	for (int i=0; i<_outcomeCount; i++)
	{
		_lstOutcomeDistribution->setCellColor(0, i, _ruleAssignment.getOutcomeColors()[i]);
	}
}

void SoldierAssignmentState::cbxSortByChange()
{
	sortAndReinitSoldierList(_cbxSortBy->getSelected());
}

void SoldierAssignmentState::lstSortByClick(Action * action)
{
	if (_game->isLeftClick(action, true))
	{
		sortAndReinitSoldierList(_lstOutcomeDistribution->getSelectedColumn()+1);
	}
	else if (_game->isRightClick(action, true))
	{
		sortAndReinitSoldierList(0);
	}
}

void SoldierAssignmentState::sortAndReinitSoldierList(int sortIndex)
{
	std::vector<Soldier *> checker = _filteredListOfSoldiers;
	if (sortIndex==0)
	{
		_cbxSortBy->setSelected(0);
		_filteredListOfSoldiers=_origSoldierOrder;
		_txtCalculationType->setText(tr("STR_ASSIGNMENT_OUTCOME_COUNT_TYPE_UC").arg(tr(_calculationType)));
		colorRelevantStatsTable(-1);
	}
	else
	{
		_cbxSortBy->setSelected(sortIndex);
		colorRelevantStatsTable(sortIndex-1);
		_txtCalculationType->setText(tr("STR_ASSIGNMENT_OUTCOME_COUNT_TYPE_UC").arg(tr(_ruleAssignment.getOutcomeCalculationTypes()[sortIndex-1])));
		if (_game->isShiftPressed())
		{
			std::stable_sort(_filteredListOfSoldiers.begin(), _filteredListOfSoldiers.end(),
			[&](Soldier* a, Soldier* b) {
				return abs(_contributionMap[a][sortIndex-1]) < abs(_contributionMap[b][sortIndex-1]);
			});
		}
		else
		{
			std::stable_sort(_filteredListOfSoldiers.begin(), _filteredListOfSoldiers.end(),
			[&](Soldier* a, Soldier* b) {
				return abs(_contributionMap[a][sortIndex-1]) > abs(_contributionMap[b][sortIndex-1]);
			});
		}
		if (checker == _filteredListOfSoldiers)
		{
			sortAndReinitSoldierList(0);
			return;
		}
	}
	initSoldierList(_lstSoldiers->getScroll(), &_filteredListOfSoldiers);
}

void SoldierAssignmentState::resetSoldierSelection()
{
	_selectedSoldiers.clear();
	_txtMaximumSoldiers->setText(tr("STR_ASSIGNMENT_MAX_SOLDIERS_UC").arg(_selectedSoldiers.size()).arg(_maxSoldiersStr));
	recalculateChances();
}

unsigned int SoldierAssignmentState::origSoldierId(unsigned int lstSelectedRow)
{
	if (lstSelectedRow!=4294967295)
	{
		Soldier * soldier = _filteredListOfSoldiers[lstSelectedRow];
		auto soldier_iter = std::find(_origSoldierOrder.begin(), _origSoldierOrder.end(), soldier);
		return soldier_iter-_origSoldierOrder.begin();
	}
	return -1;
}

void SoldierAssignmentState::filterWounded(std::vector<Soldier *> * toFilter)
{
	toFilter->erase(
	std::remove_if(toFilter->begin(), toFilter->end(),
				   [](Soldier* s) { return s->isWounded(); }),
	toFilter->end());
}
}
