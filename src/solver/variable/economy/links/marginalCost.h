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
#ifndef __SOLVER_VARIABLE_ECONOMY_MarginalCost_H__
# define __SOLVER_VARIABLE_ECONOMY_MarginalCost_H__

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


	struct VCardMarginalCost : public VCardLinkBaseCost
	{
		//! Caption
		static const char* Caption() {return "MARG. COST";}
		//! Unit
		static const char* Unit() {return "Euro/MW";}

		//! The short description of the variable
		static const char* Description() {return "Decrease of the overall operating cost expected by a 1MW capacity reinforcement";}

	}; // class VCard


	/*!
	** \brief MarginalCost
	*/
	template<class NextT = Container::EndOfList>
	class MarginalCost : public LinkBaseCost<NextT, VCardMarginalCost>
	{

	public:
		
		void hourForEachLink(State& state, unsigned int numSpace)
		{
			pValuesForTheCurrentYear[numSpace][state.hourInTheYear] =
				Yuni::Math::Abs(state.problemeHebdo->VariablesDualesDesContraintesDeNTC[state.hourInTheWeek]->VariableDualeParInterconnexion[state.link->index]);

			// This value should be reset to zero if  (flow_lowerbound) < flow < (flow_upperbound)  (with signed values)
			double flow = state.problemeHebdo->ValeursDeNTC[state.hourInTheWeek]->ValeurDuFlux[state.link->index];
			double flow_lowerbound = -state.problemeHebdo->ValeursDeNTC[state.hourInTheWeek]->ValeurDeNTCExtremiteVersOrigine[state.link->index];
			double flow_upperbound = state.problemeHebdo->ValeursDeNTC[state.hourInTheWeek]->ValeurDeNTCOrigineVersExtremite[state.link->index];

			if (flow - 0.001 > flow_lowerbound && flow + 0.001 < flow_upperbound)
				pValuesForTheCurrentYear[numSpace][state.hourInTheYear] = 0.;
		
			// Next item in the list
			NextType::hourForEachLink(state, numSpace);
		}

	}; // class MarginalCost





} // namespace Economy
} // namespace Variable
} // namespace Solver
} // namespace Antares

#endif // __SOLVER_VARIABLE_ECONOMY_MarginalCost_H__
