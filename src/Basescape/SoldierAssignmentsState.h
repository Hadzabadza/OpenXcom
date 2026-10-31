#pragma once
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
#include "../Engine/State.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleAssignment.h"
#include "SoldierSortUtil.h"

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextEdit;
class TextList;
class ComboBox;

/**
 * Screen that shows all the soldiers
 * that have died throughout the game.
 */
class SoldierAssignmentsState : public State
{
private:
	TextButton *_btnOk, *_btnSendOff;
	TextEdit *_btnQuickSearch;
	Window *_window;
	Text *_txtTitle, *_txtName, *_txtRank, *_txtDate, *_txtStat, *_txtStatCount;
	// ComboBox *_cbxSortBy, *_cbxScreenActions;
	TextList *_lstSoldiers;
	Base *_base;
	std::vector<int> _indices;
	std::vector<Soldier *> _origSoldierOrder, _filteredListOfSoldiers;
	std::vector<int> _filteredIndicesOfSoldiers;
	std::vector<SortFunctor *> _sortFunctors;
	getStatFn_t _dynGetter;
	std::vector<std::string> _availableOptions;
	size_t _mainOffset;
	RuleAssignment& _ruleAssignment;
	WeightedOptions _resolutionsList;
	int _checkStat;

	void initList(size_t scrl);
public:
	/// Creates the Soldiers state.
	SoldierAssignmentsState(Base *base);
	/// Cleans up the Soldiers state.
	~SoldierAssignmentsState();
	/// Initializes the state.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Send Off button.
	void btnSendOffClick(Action *action);
	/// Handler for clicking the Soldiers list.
	void lstSoldiersClick(Action *action);
};

}
