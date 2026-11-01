#pragma once
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
#include "../Engine/State.h"

namespace OpenXcom
{

class TextButton;
class TextList;
class Window;
class Text;
class Base;
class RuleAssignment;

/**
 * Displays info about a custom Geoscape event.
 */
class SoldierAssignmentListState : public State
{
private:
	Window *_window;
	Text *_txtTitle;
	TextButton *_btnOk;
	TextList *_lstAssignments;
	std::vector<RuleAssignment*> _assignmentList;
	Base *_base;

public:
	/// Creates the GeoscapeAssignmentState.
	SoldierAssignmentListState(Base *base);
	/// Cleans up the GeoscapeAssignmentState.
	~SoldierAssignmentListState();
	/// Initializes the state.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the ItemsArriving button.
	void btnAssignmentClick(Action *action);
};

}
