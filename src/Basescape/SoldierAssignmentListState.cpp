/*
 * Copyright 2010-2019 OpenXcom Developers.
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
#include "SoldierAssignmentListState.h"
#include "SoldierAssignmentState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleAssignment.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Geoscape Event window.
 * @param geoEvent Pointer to the event.
 */
SoldierAssignmentListState::SoldierAssignmentListState(Base *base) : _base(base)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_txtTitle = new Text(310, 17, 5, 8);
	_btnOk = new TextButton(148, 16, 164, 176);
	_lstAssignments = new TextList(288, 120, 8, 44);

	// Set palette
	setInterface("geoscapeEvent");

	add(_window, "window", "geoscapeEvent");
	add(_txtTitle, "text1", "geoscapeEvent");
	add(_btnOk, "button", "geoscapeEvent");
	add(_lstAssignments, "list", "geoscapeEvent");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "soldierAssignments");

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setWordWrap(true);
	_txtTitle->setText(tr("STR_ASSIGNMENT_BOARD").arg(_base->getName()));

	_lstAssignments->setColumns(1, 288);
	_lstAssignments->setAlign(ALIGN_RIGHT, 1);
	_lstAssignments->setSelectable(true);
	_lstAssignments->setBackground(_window);
	_lstAssignments->setMargin(8);
	_lstAssignments->onMouseClick((ActionHandler)&SoldierAssignmentListState::btnAssignmentClick);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)& SoldierAssignmentListState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&SoldierAssignmentListState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&SoldierAssignmentListState::btnOkClick, Options::keyCancel);
}

/**
 *
 */
SoldierAssignmentListState::~SoldierAssignmentListState()
{
	// Empty by design
}

/**
 * Initializes the state.
 */
void SoldierAssignmentListState::init()
{
	State::init();

	_lstAssignments->clearList();

	for (const auto assignmentIndex : *_game->getMod()->getAssignmentList())
	{
		RuleAssignment& assignment = *_game->getMod()->getAssignment(assignmentIndex);
		_assignmentList.push_back(&assignment);
		_lstAssignments->addRow(1, tr(assignment.getName()).c_str());
	}

	_lstAssignments->draw();
}

/**
 * Closes the window and shows a pedia article if needed.
 * @param action Pointer to an action.
 */
void SoldierAssignmentListState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Toggles the view between the description and the ItemsArriving list.
 * @param action Pointer to an action.
 */
void SoldierAssignmentListState::btnAssignmentClick(Action *)
{
	_game->pushState(new SoldierAssignmentState(_base,_assignmentList[_lstAssignments->getSelectedRow()]));
}

}
