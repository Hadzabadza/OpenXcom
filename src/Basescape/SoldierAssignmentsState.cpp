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
#include "SoldierAssignmentsState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextEdit.h"
#include "../Interface/TextList.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDeath.h"
#include "../Savegame/GameTime.h"
#include "SoldierInfoState.h"
#include "../Menu/StatisticsState.h"
#include "../Geoscape/GeoscapeEventState.h"


namespace OpenXcom
{
/**
 * Initializes all the elements in the Soldier Assignment screen.
 * @param game Pointer to the core game.
 */
SoldierAssignmentsState::SoldierAssignmentsState(Base *base) : _base(base), _origSoldierOrder(*_base->getSoldiers()), _dynGetter(NULL), _mainOffset(0), _ruleAssignment(*_game->getMod()->getAssignment("STR_DEBUG_ASSIGNMENT"))
{
	_resolutionsList = _ruleAssignment.getWeightedEventList();

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnQuickSearch = new TextEdit(this, 48, 9, 16, 10);
	_btnOk = new TextButton(148, 16, 164, 176);
	_btnSendOff = new TextButton(148, 16, 8, 176);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtName = new Text(114, 9, 16, 36);
	_txtRank = new Text(102, 9, 130, 36);
	_txtDate = new Text(90, 9, 218, 36);
	_txtStat = new Text(150, 9, 16, 24);
	_txtStatCount = new Text(150, 9, 160, 24);
	_lstSoldiers = new TextList(288, 120, 8, 44);

	// Set palette
	setInterface("soldierAssignments");

	add(_window, "window", "soldierAssignments");
	add(_btnQuickSearch, "button", "soldierAssignments");
	add(_btnOk, "button", "soldierAssignments");
	add(_btnSendOff, "button", "soldierAssignments");
	add(_txtTitle, "text", "soldierAssignments");
	add(_txtName, "text", "soldierAssignments");
	add(_txtRank, "text", "soldierAssignments");
	add(_txtDate, "text", "soldierAssignments");
	add(_txtStat, "text", "soldierAssignments");
	add(_txtStatCount, "text", "soldierAssignments");
	add(_lstSoldiers, "list", "soldierAssignments");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "soldierAssignments");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&SoldierAssignmentsState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&SoldierAssignmentsState::btnOkClick, Options::keyCancel);

	_btnSendOff->setText(tr("STR_SEND_OFF").arg(""));
	_btnSendOff->onMouseClick((ActionHandler)&SoldierAssignmentsState::btnSendOffClick);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_ASSIGNMENTS"));

	_txtName->setText(tr("STR_NAME_UC"));

	_txtRank->setText(tr("STR_RANK"));

	_txtDate->setText(tr("STR_DATE_UC"));

	size_t lost = _game->getSavedGame()->getDeadSoldiers()->size();
	size_t recruited = lost;
	for (const auto* xbase : *_game->getSavedGame()->getBases())
	{
		recruited += xbase->getTotalSoldiers();
	}

	// _txtStat->setText(tr("STR_ASSIGNMENT_STAT").arg(tr("STR_HEALTH")));
	// _txtStatCount->setText(tr("STR_ASSIGNMENT_STAT_COUNT").arg("Pick a soldier"));

	_txtStat->setText(tr("STR_ASSIGNMENT_STAT_UC").arg(tr("STR_BRAVERY")));
	_txtStatCount->setText(tr("STR_ASSIGNMENT_STAT_COUNT_UC").arg("..."));

	// _lstSoldiers->setColumns(5, 114, 88, 30, 25, 35);
	// _lstSoldiers->setSelectable(true);
	// _lstSoldiers->setBackground(_window);
	// _lstSoldiers->setMargin(8);
	// _lstSoldiers->onMouseClick((ActionHandler)&SoldierAssignmentsState::lstSoldiersClick);

	//_lstSoldiers->setArrowColumn(188, ARROW_VERTICAL);
	_lstSoldiers->setColumns(3, 106, 98, 76);
	_lstSoldiers->setAlign(ALIGN_RIGHT, 3);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	_lstSoldiers->onMouseClick((ActionHandler)&SoldierAssignmentsState::lstSoldiersClick);

	// _btnQuickSearch->setText(""); // redraw
	// _btnQuickSearch->onEnter((ActionHandler)&SoldierAssignmentsState::btnQuickSearchApply);
	// _btnQuickSearch->setVisible(Options::oxceQuickSearchButton);

	// _btnOk->onKeyboardRelease((ActionHandler)&SoldierAssignmentsState::btnQuickSearchToggle, Options::keyToggleQuickSearch);
}

/**
 *
 */
SoldierAssignmentsState::~SoldierAssignmentsState()
{

}

/**
 * Initializes the screen (fills the list).
 */
void SoldierAssignmentsState::init()
{
	State::init();

	// fillMemorialList();
	initList(_lstSoldiers->getScroll());
}

void SoldierAssignmentsState::initList(size_t scrl)
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
		_lstSoldiers->setArrowColumn(188, ARROW_VERTICAL);

		// all soldiers in the base
		_filteredListOfSoldiers = *_base->getSoldiers();
	}
	else
	{
		offset = 20;
		_lstSoldiers->setArrowColumn(-1, ARROW_VERTICAL);

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
		_lstSoldiers->setColumns(4, 106, 98 - offset, 60 + offset, 16);
	}
	else
	{
		_lstSoldiers->setColumns(3, 106, 98 - offset, 76 + offset);
	}
	// _txtCraft->setX(_txtRank->getX() + 98 - offset);

	BaseSumDailyRecovery recovery = _base->getSumRecoveryPerDay();
	unsigned int row = 0;
	for (const auto* soldier : _filteredListOfSoldiers)
	{
		std::string craftString = soldier->getCraftString(_game->getLanguage(), recovery);

		if (_dynGetter != NULL)
		{
			// call corresponding getter
			int dynStat = (*_dynGetter)(_game, soldier);
			std::ostringstream ss;
			ss << dynStat;
			_lstSoldiers->addRow(4, soldier->getName(true).c_str(), tr(soldier->getRankString()).c_str(), craftString.c_str(), ss.str().c_str());
		}
		else
		{
			_lstSoldiers->addRow(3, soldier->getName(true).c_str(), tr(soldier->getRankString()).c_str(), craftString.c_str());
		}

		if (soldier->getCraft() == 0)
		{
			_lstSoldiers->setRowColor(row, _lstSoldiers->getSecondaryColor());
		}
		if (soldier->getDeath())
		{
			// _lstSoldiers->setRowColor(row, _txtCraft->getColor());
		}
		row++;
	}
	if (scrl)
		_lstSoldiers->scrollTo(scrl);
	_lstSoldiers->draw();
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void SoldierAssignmentsState::btnOkClick(Action *)
{
	_game->popState();
}

/**
* Shows the Statistics screen.
* @param action Pointer to an action.
*/
void SoldierAssignmentsState::btnSendOffClick(Action *)
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

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void SoldierAssignmentsState::lstSoldiersClick(Action *)
{
	// _game->pushState(new SoldierInfoState(0, _indices[_lstSoldiers->getSelectedRow()]));
	Soldier *_soldier = _base->getSoldiers()->at(_lstSoldiers->getSelectedRow());
	_checkStat = _soldier->getCurrentStats()->bravery;
	_resolutionsList.set(_ruleAssignment.getSuccessName(), _checkStat);
	float success_chance = float(_checkStat)/float(_resolutionsList.getTotalWeight());
	if(success_chance > 0.33)
	{
		if (success_chance < 0.66)
		{
			_txtStatCount->setSecondaryColor(95);
		}
		else
		{
			_txtStatCount->setSecondaryColor(47);
		}
	}
	else
	{
		_txtStatCount->setSecondaryColor(35);
	}
	_txtStatCount->setText(tr("STR_ASSIGNMENT_STAT_COUNT_UC").arg(_checkStat));
	_btnSendOff->setText(tr("STR_SEND_OFF").arg(success_chance));
}
}