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
#define __SOLVER_VARIABLE_ECONOMY_MarginalCost_H__

#include "../../variable.h"

namespace Antares
{
namespace Solver
{
namespace Variable
{
namespace Economy
{
struct VCardMarginalCost
{
    //! Caption
    static const char* Caption()
    {
        return "MARG. COST";
    }
    //! Unit
    static const char* Unit()
    {
        return "Euro/MW";
    }

    //! The short description of the variable
    static const char* Description()
    {
        return "Decrease of the overall operating cost expected by a 1MW capacity reinforcement";
    }

    //! The expecte results
    typedef Results<R::AllYears::Average< // The average values throughout all years
      R::AllYears::StdDeviation<          // The standard deviation values throughout all years
        R::AllYears::Min<                 // The minimum values throughout all years
          R::AllYears::Max<               // The maximum values throughout all years
            >>>>>
      ResultsType;

    enum
    {
        //! Data Level
        categoryDataLevel = Category::link,
        //! File level (provided by the type of the results)
        categoryFileLevel = ResultsType::categoryFile & (Category::id | Category::va),
        //! Precision (views)
        precision = Category::all,
        //! Indentation (GUI)
        nodeDepthForGUI = +0,
        //! Decimal precision
        decimal = 2,
        //! Number of columns used by the variable (One ResultsType per column)
        columnCount = 1,
        //! The Spatial aggregation
        spatialAggregate = Category::spatialAggregateSum,
        spatialAggregateMode = Category::spatialAggregateEachYear,
        spatialAggregatePostProcessing = 0,
        //! Intermediate values
        hasIntermediateValues = 1,
        //! Can this variable be non applicable (0 : no, 1 : yes)
        isPossiblyNonApplicable = 0,
    };

    typedef IntermediateValues IntermediateValuesBaseType;
    typedef IntermediateValues* IntermediateValuesType;

}; // class VCard

/*!
** \brief Marginal MarginalCost
*/
template<class NextT = Container::EndOfList>
class MarginalCost : public Variable::IVariable<MarginalCost<NextT>, NextT, VCardMarginalCost>
{
public:
    //! Type of the next static variable
    typedef NextT NextType;
    //! VCard
    typedef VCardMarginalCost VCardType;
    //! Ancestor
    typedef Variable::IVariable<MarginalCost<NextT>, NextT, VCardType> AncestorType;

    //! List of expected results
    typedef typename VCardType::ResultsType ResultsType;

    typedef VariableAccessor<ResultsType, VCardType::columnCount> VariableAccessorType;

    enum
    {
        //! How many items have we got
        count = 1 + NextT::count,
    };

    template<int CDataLevel, int CFile>
    struct Statistics
    {
        enum
        {
            count
            = ((VCardType::categoryDataLevel & CDataLevel && VCardType::categoryFileLevel & CFile)
                 ? (NextType::template Statistics<CDataLevel, CFile>::count
                    + VCardType::columnCount * ResultsType::count)
                 : NextType::template Statistics<CDataLevel, CFile>::count),
        };
    };

public:
    ~MarginalCost()
    {
        delete[] pValuesForTheCurrentYear;
    }

    void initializeFromStudy(Data::Study& study)
    {
        pNbYearsParallel = study.maxNbYearsInParallel;

        // Average on all years
        AncestorType::pResults.initializeFromStudy(study);
        AncestorType::pResults.reset();

        // Intermediate values
        pValuesForTheCurrentYear = new VCardType::IntermediateValuesBaseType[pNbYearsParallel];
        for (unsigned int numSpace = 0; numSpace < pNbYearsParallel; numSpace++)
            pValuesForTheCurrentYear[numSpace].initializeFromStudy(study);

        // Next
        NextType::initializeFromStudy(study);
    }

    void initializeFromArea(Data::Study* study, Data::Area* area)
    {
        // Next
        NextType::initializeFromArea(study, area);
    }

    void initializeFromAreaLink(Data::Study* study, Data::AreaLink* link)
    {
        // Next
        NextType::initializeFromAreaLink(study, link);
    }

    void initializeFromThermalCluster(Data::Study* study,
                                      Data::Area* area,
                                      Data::ThermalCluster* cluster)
    {
        // Next
        NextType::initializeFromThermalCluster(study, area, cluster);
    }

    void simulationBegin()
    {
        // Next
        NextType::simulationBegin();
    }

    void simulationEnd()
    {
        NextType::simulationEnd();
    }

    void yearBegin(uint year, unsigned int numSpace)
    {
        // Reset
        pValuesForTheCurrentYear[numSpace].reset();
        // Next variable
        NextType::yearBegin(year, numSpace);
    }

    void yearEndBuildPrepareDataForEachThermalCluster(State& state,
                                                      uint year,
                                                      unsigned int numSpace)
    {
        // Next variable
        NextType::yearEndBuildPrepareDataForEachThermalCluster(state, year, numSpace);
    }

    void yearEndBuildForEachThermalCluster(State& state, uint year, unsigned int numSpace)
    {
        // Next variable
        NextType::yearEndBuildForEachThermalCluster(state, year, numSpace);
    }

    void yearEndBuild(State& state, unsigned int year)
    {
        // Next variable
        NextType::yearEndBuild(state, year);
    }

    void yearEnd(unsigned int year, unsigned int numSpace)
    {
        // Compute all statistics for the current year (daily,weekly,monthly)
        pValuesForTheCurrentYear[numSpace].computeStatisticsForTheCurrentYear();

        // Next variable
        NextType::yearEnd(year, numSpace);
    }

    void computeSummary(std::map<unsigned int, unsigned int>& numSpaceToYear,
                        unsigned int nbYearsForCurrentSummary)
    {
        for (unsigned int numSpace = 0; numSpace < nbYearsForCurrentSummary; ++numSpace)
        {
            // Merge all those values with the global results
            AncestorType::pResults.merge(numSpaceToYear[numSpace],
                                         pValuesForTheCurrentYear[numSpace]);
        }

        // Next variable
        NextType::computeSummary(numSpaceToYear, nbYearsForCurrentSummary);
    }

    void hourBegin(uint hourInTheYear)
    {
        // Next variable
        NextType::hourBegin(hourInTheYear);
    }

    void hourForEachArea(State& state, unsigned int numSpace)
    {
        // Next variable
        NextType::hourForEachArea(state, numSpace);
    }

    void hourForEachThermalCluster(State& state, unsigned int numSpace)
    {
        // Next item in the list
        NextType::hourForEachThermalCluster(state, numSpace);
    }

    void hourForEachLink(State& state, unsigned int numSpace)
    {
        pValuesForTheCurrentYear[numSpace][state.hourInTheYear] = Yuni::Math::Abs(
          state.problemeHebdo->VariablesDualesDesContraintesDeNTC[state.hourInTheWeek]
            ->VariableDualeParInterconnexion[state.link->index]);

        // This value should be reset to zero if  (flow_lowerbound) < flow < (flow_upperbound) (with
        // signed values)
        double flow
          = state.problemeHebdo->ValeursDeNTC[state.hourInTheWeek]->ValeurDuFlux[state.link->index];
        double flow_lowerbound = -state.problemeHebdo->ValeursDeNTC[state.hourInTheWeek]
                                    ->ValeurDeNTCExtremiteVersOrigine[state.link->index];
        double flow_upperbound = state.problemeHebdo->ValeursDeNTC[state.hourInTheWeek]
                                   ->ValeurDeNTCOrigineVersExtremite[state.link->index];

        if (flow - 0.001 > flow_lowerbound && flow + 0.001 < flow_upperbound)
            pValuesForTheCurrentYear[numSpace][state.hourInTheYear] = 0.;

        // Next item in the list
        NextType::hourForEachLink(state, numSpace);
    }

    void hourEnd(State& state, uint hourInTheYear)
    {
        // Next
        NextType::hourEnd(state, hourInTheYear);
    }

    void buildDigest(SurveyResults& results, int digestLevel, int dataLevel) const
    {
        // Next
        NextType::buildDigest(results, digestLevel, dataLevel);
    }

    Antares::Memory::Stored<double>::ConstReturnType retrieveRawHourlyValuesForCurrentYear(
      uint,
      uint numSpace) const
    {
        return pValuesForTheCurrentYear[numSpace].hour;
    }

    void localBuildAnnualSurveyReport(SurveyResults& results,
                                      int fileLevel,
                                      int precision,
                                      uint numSpace) const
    {
        // Initializing external pointer on current variable non applicable status
        results.isCurrentVarNA = AncestorType::isNonApplicable;

        if (AncestorType::isPrinted[0])
        {
            // Write the data for the current year
            results.variableCaption = VCardType::Caption();
            pValuesForTheCurrentYear[numSpace].template buildAnnualSurveyReport<VCardType>(
              results, fileLevel, precision);
        }
    }

private:
    //! Intermediate values for each year
    typename VCardType::IntermediateValuesType pValuesForTheCurrentYear;
    unsigned int pNbYearsParallel;

}; // class MarginalCost

} // namespace Economy
} // namespace Variable
} // namespace Solver
} // namespace Antares

#endif // __SOLVER_VARIABLE_ECONOMY_MarginalCost_H__
