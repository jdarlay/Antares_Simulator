/*
** Copyright 2007-2018 RTE
** Authors: Antares_Simulator Team
**
** This file is part of Antares_Simulator.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** There are special exceptions to the terms and conditions of the
** license as they are applied to this software. View the full text of
** the exceptions in file COPYING.txt in the directory of this software
** distribution
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Antares_Simulator. If not, see <http://www.gnu.org/licenses/>.
**
** SPDX-License-Identifier: licenceRef-GPL3_WITH_RTE-Exceptions
*/
#ifndef __SOLVER_VARIABLE_ECONOMY_ReducedCost_H__
# define __SOLVER_VARIABLE_ECONOMY_ReducedCost_H__

# include "../../variable.h"
#include "linkBaseCost.h"


namespace Antares
{
namespace Solver
{
namespace Variable
{
namespace Economy
{


	struct VCardReducedCost : public VCardLinkBaseCost
	{
		//! Caption
		static const char* Caption() { return "REDU. COST"; }
		//! Unit
		static const char* Unit() { return "Euro/MW"; }

		//! The short description of the variable
		static const char* Description() { return "To be completed..."; }

	}; // class VCard


	/*!
	** \brief ReducedCost
	*/
	template<class NextT = Container::EndOfList>
	class ReducedCost : public LinkBaseCost<NextT, VCardReducedCost>
	{

	public:

		void hourForEachLink(State& state, unsigned int numSpace)
		{
			pValuesForTheCurrentYear[numSpace][state.hourInTheYear] =
				Yuni::Math::Abs(state.problemeHebdo->VariablesDualesDesContraintesDeNTC[state.hourInTheWeek]->VariableDualeParInterconnexion[state.link->index]);

			// Next item in the list
			NextType::hourForEachLink(state, numSpace);
		}

	}; // class ReducedCost


} // namespace Economy
} // namespace Variable
} // namespace Solver
} // namespace Antares

#endif // __SOLVER_VARIABLE_ECONOMY_ReducedCost_H__
