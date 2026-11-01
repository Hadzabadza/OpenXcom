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
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Savegame/Base.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDeath.h"
#include "SoldierAssignmentState.h"
#include "SoldierInfoState.h"

#include <iomanip>
#include <sstream>

namespace OpenXcom
{
/**
 * Initializes all the elements in the Soldier Assignment screen.
 * @param game Pointer to the core game.
 */
SoldierAssignmentState::SoldierAssignmentState(Base *base, RuleAssignment *assignment) : _base(base), _origSoldierOrder(*_base->getSoldiers()), _dynGetter(NULL), _mainOffset(0), _totalContribution(0), _lmb(false),
																						 _ruleAssignment(*assignment)
{
	_resolutionsList = _ruleAssignment.getWeightedEventList();
	_difficultyWeight = _resolutionsList.getTotalWeight();

	// Create objects
	_window = new Window(this, 320, 200, 0, 0, POPUP_BOTH);
	_btnQuickSearch = new TextEdit(this, 48, 9, 16, 10);
	_btnOk = new TextButton(148, 16, 164, 176);
	_btnSendOff = new TextButton(148, 16, 8, 176);
	_txtSuccessChance = new Text(312, 16, 8, 166);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtName = new Text(114, 9, 148, 26);
	_txtContribution = new Text(160, 9, 148, 26);
	_txtDescriptionTitle = new Text(124, 9, 8, 26);
	_txtDescription = new Text(124, 120, 8, 36);
	_txtRelevantStats = new Text(150, 9, 8, 156);
	_lstRelevantStatsTable = new TextList(288, 9, 8, 156);
	_lstSoldiers = new TextList(160, 104, 140, 34);
	_interactiveWindow = new InteractiveSurface(320, 200, 0, 0);

	// Set palette
	setInterface("soldierAssignments");

	add(_window, "window", "soldierAssignments");
	add(_btnQuickSearch, "button", "soldierAssignments");
	add(_btnOk, "button", "soldierAssignments");
	add(_btnSendOff, "button", "soldierAssignments");
	add(_txtSuccessChance, "text", "soldierAssignments");
	add(_txtTitle, "text", "soldierAssignments");
	add(_txtName, "text", "soldierAssignments");
	add(_txtContribution, "text", "soldierAssignments");
	add(_txtDescriptionTitle, "text", "soldierAssignments");
	add(_txtDescription, "text", "soldierAssignments");
	add(_txtRelevantStats, "text", "soldierAssignments");
	add(_lstRelevantStatsTable, "list2", "soldierAssignments");
	add(_lstSoldiers, "list", "soldierAssignments");
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
	_txtSuccessChance->setText(tr("STR_ASSIGNMENT_SUCCESS_CHANCE").arg(" Select soldiers to calculate"));
	_txtContribution->setText(tr("STR_ASSIGNMENT_CONTRIBUTION_UC"));
	_txtContribution->setAlign(ALIGN_RIGHT);
	_txtDescriptionTitle->setAlign(ALIGN_CENTER);
	_txtDescriptionTitle->setText(tr("STR_ASSIGNMENT_SUMMARY_UC"));
	_txtDescription->setText(tr(_ruleAssignment.getDescription()).arg(tr("STR_BRAVERY_ABBREVIATION")).arg(_difficultyWeight));
	_txtDescription->setVerticalAlign(ALIGN_TOP);
	_txtDescription->setAlign(ALIGN_CENTER);
	_txtDescription->setWordWrap(true);
	_txtDescription->setColor(_txtDescription->getSecondaryColor());
	_txtDescription->setSecondaryColor(127);
	_txtRelevantStats->setText(tr("STR_ASSIGNMENT_RELEVANT_STATS_UC"));

	// _lstSoldiers->setColumns(3, 16, 16, 40);
	_lstSoldiers->setAlign(ALIGN_RIGHT, 1);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	_lstSoldiers->onMousePress((ActionHandler)&SoldierAssignmentState::lstSoldiersMousePress);
	_lstSoldiers->onMouseOver((ActionHandler)&SoldierAssignmentState::lstSoldiersMultiselect);
	// Hack to make sure that mouse releases outside the list get registered.
	_interactiveWindow->onMouseRelease((ActionHandler)&SoldierAssignmentState::lstSoldiersMouseRelease);

	if (_game->getMod()->isManaFeatureEnabled())
	{
		_lstRelevantStatsTable->setColumns(14, 62, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 0);
	}
	else
	{
		_lstRelevantStatsTable->setColumns(13, 80, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 0);
	}
	// _lstRelevantStatsTable->setAlign(ALIGN_RIGHT);
	// _lstRelevantStatsTable->setAlign(ALIGN_LEFT, 0);
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
}

void SoldierAssignmentState::initSoldierList(size_t scrl)
{
	_lstSoldiers->clearList();

	_filteredListOfSoldiers.clear();
	_filteredIndicesOfSoldiers.clear();

	std::string selAction = "STR_SOLDIER_INFO";
	// if (!_availableOptions.empty())
	// {
	// 	selAction = _availableOptions.at(_cbxScreenActions->getSelected());
	// }

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

	if (_dynGetter != NULL)
	{
		_lstSoldiers->setColumns(3, 128, 16, 16);
	}
	else
	{
		_lstSoldiers->setColumns(2, 128, 16);
	}
	// _txtCraft->setX(_txtRank->getX() + 98 - offset);

	BaseSumDailyRecovery recovery = _base->getSumRecoveryPerDay();
	unsigned int row = 0;
	for (const auto* soldier : _filteredListOfSoldiers)
	{
		std::string craftString = soldier->getCraftString(_game->getLanguage(), recovery);
		int soldierContribution = calculateContribution(soldier);
		if (_dynGetter != NULL)
		{
			// call corresponding getter
			int dynStat = (*_dynGetter)(_game, soldier);
			std::ostringstream ss;
			ss << dynStat;
			_lstSoldiers->addRow(3, soldier->getName(true).c_str(), std::to_string(soldierContribution).c_str(), ss.str().c_str());
		}
		else
		{
			_lstSoldiers->addRow(2, soldier->getName(true).c_str(), std::to_string(soldierContribution).c_str());
		}
		_lstSoldiers->setCellColor(row, 1, contributionColor(soldierContribution));
		_selectedSoldiersMap.insert(std::pair(soldier,false));
		_toggledRows.insert(std::pair(row,false));
		row++;
	}
	if (scrl)
		_lstSoldiers->scrollTo(scrl);
	_lstSoldiers->draw();
}

void SoldierAssignmentState::initStatList()
{
	_lstRelevantStatsTable->clearList();

	if (_game->getMod()->isManaFeatureEnabled())
	{
		bool showMana = _game->getSavedGame()->isManaUnlocked(_game->getMod());

		_lstRelevantStatsTable->addRow(14, "",
			tr("STR_TIME_UNITS_ABBREVIATION").c_str(),
			tr("STR_STAMINA_ABBREVIATION").c_str(),
			tr("STR_HEALTH_ABBREVIATION").c_str(),
			tr("STR_BRAVERY_ABBREVIATION").c_str(),
			tr("STR_REACTIONS_ABBREVIATION").c_str(),
			tr("STR_FIRING_ACCURACY_ABBREVIATION").c_str(),
			tr("STR_THROWING_ACCURACY_ABBREVIATION").c_str(),
			tr("STR_MELEE_ACCURACY_ABBREVIATION").c_str(),
			tr("STR_STRENGTH_ABBREVIATION").c_str(),
			tr("STR_MANA_ABBREVIATION").c_str(),
			tr("STR_PSIONIC_STRENGTH_ABBREVIATION").c_str(),
			tr("STR_PSIONIC_SKILL_ABBREVIATION").c_str(),
			"");
		// _lstStatChanges->addRow(14, tr("STR_CURRENT_STATS").c_str(),
		// 	formatStat(currentStats.tu, false, false).c_str(),
		// 	formatStat(currentStats.stamina, false, false).c_str(),
		// 	formatStat(currentStats.health, false, false).c_str(),
		// 	formatStat(currentStats.bravery, false, false).c_str(),
		// 	formatStat(currentStats.reactions, false, false).c_str(),
		// 	formatStat(currentStats.firing, false, false).c_str(),
		// 	formatStat(currentStats.throwing, false, false).c_str(),
		// 	formatStat(currentStats.melee, false, false).c_str(),
		// 	formatStat(currentStats.strength, false, false).c_str(),
		// 	formatStat(currentStats.mana, false, !showMana).c_str(),
		// 	formatStat(currentStats.psiStrength, false, !showPsiStrength).c_str(),
		// 	formatStat(currentStats.psiSkill, false, !showPsiSkill).c_str(),
		// 	"");
		// _lstStatChanges->addRow(14, tr(twoRows ? "STR_CHANGES_MIN" : "STR_CHANGES").c_str(),
		// 	formatStat(changedStatsMin.tu, true, rerollFlags.tu || randomFlags.tu).c_str(),
		// 	formatStat(changedStatsMin.stamina, true, rerollFlags.stamina || randomFlags.stamina).c_str(),
		// 	formatStat(changedStatsMin.health, true, rerollFlags.health || randomFlags.health).c_str(),
		// 	formatStat(changedStatsMin.bravery, true, rerollFlags.bravery || randomFlags.bravery).c_str(),
		// 	formatStat(changedStatsMin.reactions, true, rerollFlags.reactions || randomFlags.reactions).c_str(),
		// 	formatStat(changedStatsMin.firing, true, rerollFlags.firing || randomFlags.firing).c_str(),
		// 	formatStat(changedStatsMin.throwing, true, rerollFlags.throwing || randomFlags.throwing).c_str(),
		// 	formatStat(changedStatsMin.melee, true, rerollFlags.melee || randomFlags.melee).c_str(),
		// 	formatStat(changedStatsMin.strength, true, rerollFlags.strength || randomFlags.strength).c_str(),
		// 	formatStat(changedStatsMin.mana, true, !showMana || rerollFlags.mana || randomFlags.mana).c_str(),
		// 	formatStat(changedStatsMin.psiStrength, true, !showPsiStrength || rerollFlags.psiStrength || randomFlags.psiStrength).c_str(),
		// 	formatStat(changedStatsMin.psiSkill, true, !showPsiSkill || rerollFlags.psiSkill || randomFlags.psiSkill).c_str(),
		// 	"");
		// if (twoRows)
		// {
		// 	_lstStatChanges->addRow(14, tr("STR_CHANGES_MAX").c_str(),
		// 		formatStat(changedStatsMax.tu, true, rerollFlags.tu).c_str(),
		// 		formatStat(changedStatsMax.stamina, true, rerollFlags.stamina).c_str(),
		// 		formatStat(changedStatsMax.health, true, rerollFlags.health).c_str(),
		// 		formatStat(changedStatsMax.bravery, true, rerollFlags.bravery).c_str(),
		// 		formatStat(changedStatsMax.reactions, true, rerollFlags.reactions).c_str(),
		// 		formatStat(changedStatsMax.firing, true, rerollFlags.firing).c_str(),
		// 		formatStat(changedStatsMax.throwing, true, rerollFlags.throwing).c_str(),
		// 		formatStat(changedStatsMax.melee, true, rerollFlags.melee).c_str(),
		// 		formatStat(changedStatsMax.strength, true, rerollFlags.strength).c_str(),
		// 		formatStat(changedStatsMax.mana, true, !showMana || rerollFlags.mana).c_str(),
		// 		formatStat(changedStatsMax.psiStrength, true, !showPsiStrength || rerollFlags.psiStrength).c_str(),
		// 		formatStat(changedStatsMax.psiSkill, true, !showPsiSkill || rerollFlags.psiSkill).c_str(),
		// 		"");
		// }
		// if (bonusRule)
		// {
		// 	_lstStatChanges->addRow(14, tr("STR_BONUS_STATS").c_str(),
		// 		formatStat(bonusStats.tu, true, false).c_str(),
		// 		formatStat(bonusStats.stamina, true, false).c_str(),
		// 		formatStat(bonusStats.health, true, false).c_str(),
		// 		formatStat(bonusStats.bravery, true, false).c_str(),
		// 		formatStat(bonusStats.reactions, true, false).c_str(),
		// 		formatStat(bonusStats.firing, true, false).c_str(),
		// 		formatStat(bonusStats.throwing, true, false).c_str(),
		// 		formatStat(bonusStats.melee, true, false).c_str(),
		// 		formatStat(bonusStats.strength, true, false).c_str(),
		// 		formatStat(bonusStats.mana, true, false).c_str(),
		// 		formatStat(bonusStats.psiStrength, true, false).c_str(),
		// 		formatStat(bonusStats.psiSkill, true, false).c_str(),
		// 		"");
		// }
	}
	else
	{
		_lstRelevantStatsTable->addRow(13, "",
			tr("STR_TIME_UNITS_ABBREVIATION").c_str(),
			tr("STR_STAMINA_ABBREVIATION").c_str(),
			tr("STR_HEALTH_ABBREVIATION").c_str(),
			tr("STR_BRAVERY_ABBREVIATION").c_str(),
			tr("STR_REACTIONS_ABBREVIATION").c_str(),
			tr("STR_FIRING_ACCURACY_ABBREVIATION").c_str(),
			tr("STR_THROWING_ACCURACY_ABBREVIATION").c_str(),
			tr("STR_MELEE_ACCURACY_ABBREVIATION").c_str(),
			tr("STR_STRENGTH_ABBREVIATION").c_str(),
			tr("STR_PSIONIC_STRENGTH_ABBREVIATION").c_str(),
			tr("STR_PSIONIC_SKILL_ABBREVIATION").c_str(),
			"");
	// 	_lstStatChanges->addRow(13, tr("STR_CURRENT_STATS").c_str(),
	// 		formatStat(currentStats.tu, false, false).c_str(),
	// 		formatStat(currentStats.stamina, false, false).c_str(),
	// 		formatStat(currentStats.health, false, false).c_str(),
	// 		formatStat(currentStats.bravery, false, false).c_str(),
	// 		formatStat(currentStats.reactions, false, false).c_str(),
	// 		formatStat(currentStats.firing, false, false).c_str(),
	// 		formatStat(currentStats.throwing, false, false).c_str(),
	// 		formatStat(currentStats.melee, false, false).c_str(),
	// 		formatStat(currentStats.strength, false, false).c_str(),
	// 		formatStat(currentStats.psiStrength, false, !showPsiStrength).c_str(),
	// 		formatStat(currentStats.psiSkill, false, !showPsiSkill).c_str(),
	// 		"");
	// 	_lstStatChanges->addRow(13, tr(twoRows ? "STR_CHANGES_MIN" : "STR_CHANGES").c_str(),
	// 		formatStat(changedStatsMin.tu, true, rerollFlags.tu || randomFlags.tu).c_str(),
	// 		formatStat(changedStatsMin.stamina, true, rerollFlags.stamina || randomFlags.stamina).c_str(),
	// 		formatStat(changedStatsMin.health, true, rerollFlags.health || randomFlags.health).c_str(),
	// 		formatStat(changedStatsMin.bravery, true, rerollFlags.bravery || randomFlags.bravery).c_str(),
	// 		formatStat(changedStatsMin.reactions, true, rerollFlags.reactions || randomFlags.reactions).c_str(),
	// 		formatStat(changedStatsMin.firing, true, rerollFlags.firing || randomFlags.firing).c_str(),
	// 		formatStat(changedStatsMin.throwing, true, rerollFlags.throwing || randomFlags.throwing).c_str(),
	// 		formatStat(changedStatsMin.melee, true, rerollFlags.melee || randomFlags.melee).c_str(),
	// 		formatStat(changedStatsMin.strength, true, rerollFlags.strength || randomFlags.strength).c_str(),
	// 		formatStat(changedStatsMin.psiStrength, true, !showPsiStrength || rerollFlags.psiStrength || randomFlags.psiStrength).c_str(),
	// 		formatStat(changedStatsMin.psiSkill, true, !showPsiSkill || rerollFlags.psiSkill || randomFlags.psiSkill).c_str(),
	// 		"");
	// 	if (twoRows)
	// 	{
	// 		_lstStatChanges->addRow(13, tr("STR_CHANGES").c_str(),
	// 			formatStat(changedStatsMax.tu, true, rerollFlags.tu).c_str(),
	// 			formatStat(changedStatsMax.stamina, true, rerollFlags.stamina).c_str(),
	// 			formatStat(changedStatsMax.health, true, rerollFlags.health).c_str(),
	// 			formatStat(changedStatsMax.bravery, true, rerollFlags.bravery).c_str(),
	// 			formatStat(changedStatsMax.reactions, true, rerollFlags.reactions).c_str(),
	// 			formatStat(changedStatsMax.firing, true, rerollFlags.firing).c_str(),
	// 			formatStat(changedStatsMax.throwing, true, rerollFlags.throwing).c_str(),
	// 			formatStat(changedStatsMax.melee, true, rerollFlags.melee).c_str(),
	// 			formatStat(changedStatsMax.strength, true, rerollFlags.strength).c_str(),
	// 			formatStat(changedStatsMax.psiStrength, true, !showPsiStrength || rerollFlags.psiStrength).c_str(),
	// 			formatStat(changedStatsMax.psiSkill, true, !showPsiSkill || rerollFlags.psiSkill).c_str(),
	// 			"");
	// 	}
	// 	if (bonusRule)
	// 	{
	// 		_lstStatChanges->addRow(13, tr("STR_BONUS_STATS").c_str(),
	// 			formatStat(bonusStats.tu, true, false).c_str(),
	// 			formatStat(bonusStats.stamina, true, false).c_str(),
	// 			formatStat(bonusStats.health, true, false).c_str(),
	// 			formatStat(bonusStats.bravery, true, false).c_str(),
	// 			formatStat(bonusStats.reactions, true, false).c_str(),
	// 			formatStat(bonusStats.firing, true, false).c_str(),
	// 			formatStat(bonusStats.throwing, true, false).c_str(),
	// 			formatStat(bonusStats.melee, true, false).c_str(),
	// 			formatStat(bonusStats.strength, true, false).c_str(),
	// 			formatStat(bonusStats.psiStrength, true, false).c_str(),
	// 			formatStat(bonusStats.psiSkill, true, false).c_str(),
	// 			"");
	// 	}
	}
	_lstRelevantStatsTable->setColor(7);
	_lstRelevantStatsTable->setCellColor(0,4,127);
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
* @param action Pointer to an action.
*/
void SoldierAssignmentState::btnSendOffClick(Action *)
{
	if (!_resolutionsList.empty())
	{
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
		Soldier *_soldier = _base->getSoldiers()->at(row);
		if (_selectedSoldiersMap.at(_soldier) == false)
		{
			_totalContribution+=calculateContribution(_soldier);
			_selectedSoldiersMap.at(_soldier) = true;
			_lstSoldiers->setCellColor(row,0, _lstSoldiers->getSecondaryColor());
		}
		else
		{
			_totalContribution-=calculateContribution(_soldier);
			_selectedSoldiersMap.at(_soldier) = false;
			_lstSoldiers->setCellColor(row,0, _lstSoldiers->getColor());
		}
		_resolutionsList.set(_ruleAssignment.getSuccessName(), _totalContribution);
		float success_chance = float(_totalContribution)/float(_resolutionsList.getTotalWeight());
		std::stringstream fomrattedChance;
		fomrattedChance << std::fixed << std::setprecision(0) << success_chance*100;
		_txtSuccessChance->setText(tr("STR_ASSIGNMENT_SUCCESS_CHANCE").arg(fomrattedChance.str()+"%"));
		_txtSuccessChance->setSecondaryColor(probabilityColor(success_chance));
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

/**
 * Helper for giving out relevant coloring for a single soldier's total contribution stat.
 * @param contribution The total sum of relevant stats.
 */

Uint8 SoldierAssignmentState::contributionColor(int contribution)
{
	float contributionStrength = float(contribution)/float(_difficultyWeight+contribution);
	return probabilityColor(contributionStrength);
}

Uint8 SoldierAssignmentState::probabilityColor(float probability)
{
	static Uint8 color;
	if(probability > 0.33)
	{
		if (probability > 0.66)
		{
			color = 47;
		}
		else
		{
			color = 95;
		}
	}
	else
	{
		color = 35;
	}
	return color;
}

/**
 * Helper for giving out relevant coloring for a single soldier's total contribution stat.
 * @param contribution The total sum of relevant stats.
 */

int SoldierAssignmentState::calculateContribution(const Soldier *soldier)
{
	return soldier->getCurrentStats()->bravery;
}
}