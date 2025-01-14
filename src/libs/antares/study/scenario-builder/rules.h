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
#ifndef __LIBS_STUDY_SCENARIO_BUILDER_RULES_H__
#define __LIBS_STUDY_SCENARIO_BUILDER_RULES_H__

#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "../fwd.h"
#include "TSnumberData.h"
#include "hydroLevelsData.h"
#include <map>

namespace Antares
{
namespace Data
{
namespace ScenarioBuilder
{
/*!
** \brief Rules for TS numbers, for all years and a single timeseries
*/
class Rules final : private Yuni::NonCopyable<Rules>
{
public:
    //! Smart pointer
    typedef Yuni::SmartPtr<Rules> Ptr;
    //! Map
    typedef std::map<RulesScenarioName, Ptr> Map;
    //! Map ID
    typedef std::map<int, Ptr> MappingID;

public:
    //! \name Constructor & Destructor
    //@{
    /*!
    ** \brief Default constructor
    **
    ** \param tstype Type of the timeseries
    */
    Rules();
    //! Destructor
    ~Rules();
    //@}

    //! \name Data manupulation
    //@{
    /*!
    ** \brief Initialize data from the study
    */
    bool reset(const Study& study);

    /*!
    ** \brief Load information from a single line (extracted from an INI file)
    */
    void loadFromInstrs(Study& study,
                        const AreaName::Vector& instrs,
                        String value,
                        bool updaterMode);

    /*!
    ** \brief Export the data into a mere INI file
    */
    void saveToINIFile(const Study& study, Yuni::IO::File::Stream& file) const;
    //@}

    //! Get the number of areas
    uint areaCount() const;

    //! Name of the rules set
    const RulesScenarioName& name() const;

    /*!
    ** \brief Apply the changes to the timeseries number matrices
    **
    ** This method is only useful when launched from the solver.
    */
    void apply(Study& study);

    // When current rule is the active one, sends warnings for disabled clusters.
    void sendWarningsForDisabledClusters();

public:
    //! Load
    loadTSNumberData load;
    //! Solar
    solarTSNumberData solar;
    //! Hydro
    hydroTSNumberData hydro;
    //! Wind
    windTSNumberData wind;
    //! Thermal (array [0..pAreaCount - 1])
    thermalTSNumberData* thermal;
    //! hydro levels
    hydroLevelsData hydroLevels;

private:
    //! Total number of areas
    uint pAreaCount;
    //! Name of the rules
    RulesScenarioName pName;
    // Disabled clusters when current rule is active (useful for sending warnings)
    map<string, vector<uint>> disabledClustersOnRuleActive;
    // Friend !
    friend class Sets;

}; // class Rules

} // namespace ScenarioBuilder
} // namespace Data
} // namespace Antares

#include "rules.hxx"

#endif // __LIBS_STUDY_SCENARIO_BUILDER_RULES_H__
