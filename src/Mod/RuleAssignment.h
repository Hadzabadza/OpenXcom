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
#include <string>
#include "../Engine/Yaml.h"

namespace OpenXcom
{

/**
 * Represents a custom soldier assignment.
 */
class RuleAssignment
{
private:
	std::string _name, _description;
	int _minimumSoldiers, _maximumSoldiers;
	std::vector<std::string> _outcomeCharacters;
	std::vector<std::string> _outcomeNames;
	std::vector<std::string> _outcomeResolutionTypes;
	std::vector<int> _outcomeBaseWeights, _outcomeTransferTimes, _outcomeRecoveryTimes;
	std::vector<std::string> _outcomeCalculationTypes, _outcomeStatBonuses;
	std::vector<Uint8> _outcomeColors;
	std::vector<std::map<std::string, float>> _outcomeStatsAndRatios;

public:
	std::string debug;
	/// Creates a blank RuleAssignment.
	RuleAssignment(const std::string &name);
	/// Cleans up the assignment ruleset.
	~RuleAssignment() = default;
	/// Loads the assignment definition from YAML.
	void load(const YAML::YamlNodeReader& reader);
	/// Gets the assignment's name.
	const std::string &getName() const { return _name; }
	/// Gets the assignment's description.
	const std::string &getDescription() const { return _description; }
	/// Gets the assignment's maximum allowed soldiers.
	const int &getMaxSoldiers() const { return _maximumSoldiers; }
	/// Gets the assignment's minimum necessary soldiers.
	const int &getMinSoldiers() const { return _minimumSoldiers; }
	/// Gets the assignment's outcome names.
	const std::vector<std::string> &getOutcomeNames() const { return _outcomeNames; }
	/// Gets the assignment's outcome resolution types.
	const std::vector<std::string> &getResolutionTypes() const { return _outcomeResolutionTypes; }
	/// Gets the assignment's outcome base weights.
	const std::vector<int> &getOutcomeBaseWeights() const { return _outcomeBaseWeights; }
	/// Gets the assignment's outcome transfer times.
	const std::vector<int> &getOutcomeTransferTimes() const { return _outcomeTransferTimes; }
	/// Gets the assignment's outcome recovery times.
	const std::vector<int> &getOutcomeRecoveryTimes() const { return _outcomeRecoveryTimes; }
	/// Gets the assignment's outcome calculation types (additive, average, lowest).
	const std::vector<std::string> &getOutcomeCalculationTypes() const { return _outcomeCalculationTypes; }
	/// Gets the assignment's outcome stat bonuses type (all, no armor, base).
	const std::vector<std::string> &getOutcomeStatBonuses() const { return _outcomeStatBonuses; }
	/// Gets the assignment's outcome colors.
	const std::vector<Uint8> &getOutcomeColors() const { return _outcomeColors; }
	/// Gets the outcome's success bar filler character.
	const std::vector<std::string> & getOutcomeCharacters() const { return _outcomeCharacters; }
	/// Gets the assignment's outcome used stats and their ratios.
	const std::vector<std::map<std::string, float>> &getOutcomeStatsAndRatios() const { return _outcomeStatsAndRatios; }

};

}
