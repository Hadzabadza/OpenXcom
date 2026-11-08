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
#include <limits>

namespace OpenXcom
{

RuleAssignment::RuleAssignment(const std::string &name) :
_name(name), _minimumSoldiers(0), _maximumSoldiers(std::numeric_limits<int>::max())
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

			_outcomeNames.assign(outcomes, "STR_ASSIGNMENT_NO_NAME_UC");
			_outcomeBaseWeights.assign(outcomes, 0);
			_outcomeCalculationTypes.assign(outcomes, "STR_ASSIGNMENT_OUTCOME_COUNT_TYPE_ADDITIVE_UC");
			_outcomeColors.assign(outcomes, 0);
			_outcomeResolutionTypes.assign(outcomes, "");
			_outcomeStatsAndRatios.assign(outcomes, std::map<std::string, float>());
			_outcomeCharacters.assign(outcomes, "");
			_outcomeTransferTimes.assign(outcomes, 0);
			_outcomeRecoveryTimes.assign(outcomes, 0);


			for (size_t i = 0; i < outcomes; ++i)
			{
				outcomesNode[i].tryRead("name", _outcomeNames[i]);
				outcomesNode[i].tryRead("baseWeight", _outcomeBaseWeights[i]);
				outcomesNode[i].tryRead("calculationType", _outcomeCalculationTypes[i]);
				outcomesNode[i].tryRead("color", _outcomeColors[i]);
				outcomesNode[i].readNode("resolutionType", _outcomeResolutionTypes[i]);
				std::string symbols;
				outcomesNode[i].tryRead("symbols", symbols);
				// 160 symbols for 320 px, assuming symbols aren't smaller than 2 px.
				if (!symbols.empty())
				{
					_outcomeCharacters[i].reserve(160);
					for (std::size_t j = 0; j < 160; j+=symbols.size())
						_outcomeCharacters[i] += symbols;
				}
				else
				{
					_outcomeCharacters[i].assign(160, '|');
				}
				outcomesNode[i].tryRead("transferTime", _outcomeTransferTimes[i]);
				outcomesNode[i].tryRead("recoveryTime", _outcomeRecoveryTimes[i]);
				outcomesNode[i].readNode("statsAndRatios", _outcomeStatsAndRatios[i]);
			}
		}
	}
}
}

