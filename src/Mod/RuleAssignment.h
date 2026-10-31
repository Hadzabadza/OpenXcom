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
#include "../Savegame/WeightedOptions.h"

namespace OpenXcom
{

/**
 * Represents a custom soldier assignment.
 */
class RuleAssignment
{
private:
	std::string _name;
	std::string _successName, _description;
	WeightedOptions _weightedEventList;
public:
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

	/// Gets the name of the successful resolution.
	const std::string &getSuccessName() const { return _successName; }

	/// Gets a list of events; one of them is randomly selected (considering weights) and triggered instantly.
	const WeightedOptions &getWeightedEventList() const { return _weightedEventList; }
};

}
