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
 * Loads the assignment definition from YAML.
 * @param node YAML node.
 */
void RuleAssignment::load(const YAML::YamlNodeReader& node)
{
	const auto& reader = node.useIndex();
	if (const auto& parent = reader["refNode"])
	{
		load(parent);
	}
	reader.tryRead("description", _description);
	reader.tryRead("minimumSoldiers", _minimumSoldiers);
	reader.tryRead("maximumSoldiers", _maximumSoldiers);
	if (const auto& outcomesNode = reader["outcomes"])
	{
		if (outcomesNode.isSeq())
		{
			const size_t outcomes = outcomesNode.childrenCount();

			_outcomeNames.clear();
			_outcomeBaseWeights.clear();
			_outcomeCalculationTypes.clear();
			_outcomeColors.clear();
			_outcomeResolutionTypes.clear();
			_outcomeStatsAndRatios.clear();

			_outcomeNames.resize(outcomes);
			_outcomeBaseWeights.resize(outcomes);
			_outcomeCalculationTypes.resize(outcomes);
			_outcomeColors.resize(outcomes);
			_outcomeResolutionTypes.resize(outcomes);
			_outcomeStatsAndRatios.resize(outcomes);

			for (size_t i = 0; i < outcomes; ++i)
			{
				outcomesNode[i].readNode("name", _outcomeNames[i]);
				outcomesNode[i].readNode("baseWeight", _outcomeBaseWeights[i]);
				outcomesNode[i].readNode("calculationType", _outcomeCalculationTypes[i]);
				outcomesNode[i].readNode("color", _outcomeColors[i]);
				outcomesNode[i].readNode("resolutionType", _outcomeResolutionTypes[i]);
				outcomesNode[i].readNode("statsAndRatios", _outcomeStatsAndRatios[i]);
			}
		}
	}
}
}

