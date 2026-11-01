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
#include "RuleAssignment.h"
#include "Mod.h"

namespace OpenXcom
{

RuleAssignment::RuleAssignment(const std::string &name) :
	_name(name)
{
}

/**
 * Loads the event definition from YAML.
 * @param node YAML node.
 */
void RuleAssignment::load(const YAML::YamlNodeReader& node)
{
	const auto& reader = node.useIndex();
	if (const auto& parent = reader["refNode"])
	{
		load(parent);
	}
	reader.tryRead("successName", _successName);
	reader.tryRead("description", _description);
	if (reader["weightedEventList"])
	{
		_weightedEventList.load(reader["weightedEventList"]);
	}
}
}
