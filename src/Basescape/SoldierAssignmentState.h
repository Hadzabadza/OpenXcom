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
class SoldierAssignmentState : public State
{
private:
	TextButton *_btnOk, *_btnSendOff;
	TextEdit *_btnQuickSearch;
	Window *_window;
	InteractiveSurface *_interactiveWindow;
	Text *_txtTitle, *_txtName, *_txtContribution, *_txtRelevantStats, *_txtSuccessChance, *_txtDescriptionTitle, *_txtDescription;
	// ComboBox *_cbxSortBy, *_cbxScreenActions;
	TextList *_lstSoldiers;
	TextList *_lstRelevantStatsTable;
	Base *_base;
	std::vector<int> _indices, _filteredIndicesOfSoldiers;
	std::vector<Soldier *> _origSoldierOrder, _filteredListOfSoldiers;
	std::map<const Soldier*, bool> _selectedSoldiersMap;
	std::map<int, bool> _toggledRows;
	std::vector<SortFunctor *> _sortFunctors;
	getStatFn_t _dynGetter;
	std::vector<std::string> _availableOptions;
	size_t _mainOffset;
	RuleAssignment& _ruleAssignment;
	WeightedOptions _resolutionsList;
	int _totalContribution, _difficultyWeight;
	bool _lmb;

public:
	/// Creates the Soldiers state.
	SoldierAssignmentState(Base *base, RuleAssignment *assignment);
	/// Cleans up the Soldiers state.
	~SoldierAssignmentState();
	/// Initializes the state.
	void init() override;
	/// Separate helper for initializing the list of soldiers.
	void initSoldierList(size_t scrl);
	/// Another helper for initializing the list of relevant stats for the check.
	void initStatList();
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Send Off button.
	void btnSendOffClick(Action *action);
	/// Helper for including/excluding a soldier from the assignment, used in multiple mouse events.
	void toggleSoldier(int row);
	/// Handler for pressing a mouse button over the Soldiers list.
	void lstSoldiersMousePress(Action *action);
	/// Handler for releasing a mouse button over the Soldiers list.
	void lstSoldiersMouseRelease(Action *action);
	/// Handler for moving a clicked mouse button over soldier entries.
	void lstSoldiersMultiselect();
	/// Helper for enering the inventory screen. Copypasted from SoldierState.
	void enterInventory();
	/// Helper for coloring contributions by weight.
	Uint8 contributionColor(int contribution);
	/// General helper for colors.
	Uint8 probabilityColor(float probabilityColor);
	/// Helper for calculating a soldier's contribution to assignment success.
	int calculateContribution(const Soldier* soldier);
};

}
