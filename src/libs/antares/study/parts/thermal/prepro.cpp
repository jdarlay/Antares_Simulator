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

#include <yuni/yuni.h>
#include <yuni/io/file.h>
#include <yuni/io/directory.h>
#include <yuni/core/math.h>
#include "../../study.h"
#include "../../memory-usage.h"
#include "prepro.h"
#include "../../../logs.h"
#include "../../../array/array1d.h"

using namespace Yuni;

#define SEP IO::Separator

namespace Antares
{
namespace Data
{
PreproThermal::PreproThermal()
{
}

void PreproThermal::copyFrom(const PreproThermal& rhs)
{
    data = rhs.data;
    rhs.data.unloadFromMemory();
}

bool PreproThermal::saveToFolder(const AnyString& folder)
{
    if (IO::Directory::Create(folder))
    {
        String buffer;
        buffer.clear() << folder << SEP << "data.txt";
        return data.saveToCSVFile(buffer, /*decimal*/ 6);
    }
    return false;
}

static bool LoadPreproThermal350(Study& study, Matrix<>& data, const AnyString& folder)
{
    // very old code for loading thermal ts-generator data for Antares <3.5
    // resize the matrix
    data.resize(PreproThermal::thermalPreproMax, DAYS_PER_YEAR, true);

    String buffer;
    double* tmp = new double[DAYS_PER_YEAR];
    bool ret = true;

    buffer.clear() << folder << SEP << "fo-duration." << study.inputExtension;
    ret = Array1DLoadFromFile(buffer.c_str(), tmp, DAYS_PER_YEAR) and ret;
    data.pasteToColumn(PreproThermal::foDuration, tmp);

    buffer.clear() << folder << SEP << "po-duration." << study.inputExtension;
    ret = Array1DLoadFromFile(buffer.c_str(), tmp, DAYS_PER_YEAR) and ret;
    data.pasteToColumn(PreproThermal::poDuration, tmp);

    buffer.clear() << folder << SEP << "fo-rate." << study.inputExtension;
    ret = Array1DLoadFromFile(buffer.c_str(), tmp, DAYS_PER_YEAR) and ret;
    data.pasteToColumn(PreproThermal::foRate, tmp);

    buffer.clear() << folder << SEP << "po-rate." << study.inputExtension;
    ret = Array1DLoadFromFile(buffer.c_str(), tmp, DAYS_PER_YEAR) and ret;
    data.pasteToColumn(PreproThermal::poRate, tmp);

    // release
    delete[] tmp;
    return ret;
}

bool PreproThermal::loadFromFolder(Study& study,
                                   const AnyString& folder,
                                   const AreaName& areaID,
                                   const AnyString& clustername)
{
    bool ret = true;
    auto& buffer = study.bufferLoadingTS;

    if (study.header.version < 350)
    {
        ret = LoadPreproThermal350(study, data, folder) and ret;
        data.flush();
    }
    else
    {
        buffer.clear() << folder << SEP << "data.txt";

        if (study.header.version < 440)
        {
            // temporary matrix
            Matrix<> tmp;
            // reset
            data.reset(thermalPreproMax, DAYS_PER_YEAR, true);

            enum
            {
                flags = Matrix<>::optFixedSize | Matrix<>::optImmediate,
            };
            if (tmp.loadFromCSVFile(buffer, 4, DAYS_PER_YEAR, flags, &study.dataBuffer))
            {
                for (uint x = 0; x != 4; ++x)
                    data.pasteToColumn(x, tmp.column(x));

                // reset NPO max to cluster size
                // retrieving the appropriate area
                auto* area = study.areas.findFromName(areaID);
                if (area)
                {
                    // retrieving the appropriate thermal cluster
                    auto* ourselves = area->thermal.list.find(clustername);
                    if (ourselves)
                    {
                        uint clusterSize = ourselves->unitCount;
                        auto& npomax = data[npoMax];
                        for (uint y = 0; y != data.height; ++y)
                            npomax[y] = clusterSize;
                    }
                    else
                        logs.error() << "NPO max reset: impossible to find the thermal cluster '"
                                     << areaID << '.' << clustername << "'";
                }
                else
                    logs.error() << "NPO max reset: impossible to find the area '" << areaID << "'";
            }
            else
                ret = false;
            // the structure must be marked as modified
            data.markAsModified();
            data.flush();
        }
        else
        {
            // standard loading
            ret = data.loadFromCSVFile(buffer,
                                       thermalPreproMax,
                                       DAYS_PER_YEAR,
                                       Matrix<>::optFixedSize,
                                       &study.dataBuffer)
                  and ret;
        }
    }

    if (study.header.version < 390)
    {
        data.invalidate(true);
        auto& colFoDuration = data[foDuration];
        auto& colPoDuration = data[poDuration];
        for (uint i = 0; i != DAYS_PER_YEAR; ++i)
        {
            colFoDuration[i] = Math::Round(colFoDuration[i]);
            colPoDuration[i] = Math::Round(colPoDuration[i]);
        }
        data.markAsModified();
    }

    if (study.usedByTheSolver)
    {
        auto& colFoRate = data[foRate];
        auto& colPoRate = data[poRate];
        auto& colFoDuration = data[foDuration];
        auto& colPoDuration = data[poDuration];
        auto& colNPOMin = data[npoMin];
        auto& colNPOMax = data[npoMax];
        uint errors = 0;

        for (uint i = 0; i != DAYS_PER_YEAR; ++i)
        {
            double foRate = colFoRate[i];
            double poRate = colPoRate[i];
            double foDuration = colFoDuration[i];
            double poDuration = colPoDuration[i];
            double cNPOMin = colNPOMin[i];
            double cNPOMax = colNPOMax[i];

            if (cNPOMin < 0)
            {
                logs.error() << "Thermal: Prepro: " << areaID << '/' << clustername
                             << ": NPO min can not be negative (line:" << (i + 1) << ")";
                ++errors;
            }
            if (cNPOMax < 0)
            {
                logs.error() << "Thermal: Prepro: " << areaID << '/' << clustername
                             << ": NPO max can not be negative (line:" << (i + 1) << ")";
                ++errors;
            }
            if (cNPOMin > cNPOMax)
            {
                logs.error() << "Thermal: Prepro: " << areaID << '/' << clustername
                             << ": NPO max must be greater or equal to NPO min (line:" << (i + 1)
                             << ")";
                ++errors;
            }

            if (foRate < 0. or foRate > 1.)
            {
                logs.error() << "Thermal: Prepro: " << areaID << '/' << clustername
                             << ": invalid value for FO rate (line:" << (i + 1) << ")";
                ++errors;
            }

            if (poRate < 0. or poRate > 1.)
            {
                logs.error() << "Thermal: Prepro: " << areaID << '/' << clustername
                             << ": invalid value for PO rate (line:" << (i + 1) << ")";
                ++errors;
            }

            if (foDuration < 1. or foDuration > 365.)
            {
                logs.error() << "Thermal: Prepro: " << areaID << '/' << clustername
                             << ": invalid value for FO Duration (line:" << (i + 1) << ")";
                ++errors;
            }
            if (foDuration < 1. or poDuration > 365.)
            {
                logs.error() << "Thermal: Prepro: " << areaID << '/' << clustername
                             << ": invalid value for PO Duration (line:" << (i + 1) << ")";
                ++errors;
            }

            if (errors > 30)
            {
                logs.error() << "Thermal: Prepro: " << areaID << '/' << clustername
                             << ": too many errors. skipping";
                break;
            }
        }

        // memory swapping
        data.flush();
    }

    return ret;
}

bool PreproThermal::invalidate(bool reload) const
{
    return data.invalidate(reload);
}

void PreproThermal::markAsModified() const
{
    data.markAsModified();
}

void PreproThermal::estimateMemoryUsage(StudyMemoryUsage& u) const
{
    if (timeSeriesThermal & u.study.parameters.timeSeriesToGenerate)
    {
        data.estimateMemoryUsage(u, true, thermalPreproMax, DAYS_PER_YEAR);
        u.requiredMemoryForInput += sizeof(PreproThermal);
    }
}

void PreproThermal::reset()
{
    data.reset(thermalPreproMax, DAYS_PER_YEAR, true);

    auto& colFoDuration = data[foDuration];
    auto& colPoDuration = data[poDuration];

    for (uint i = 0; i != DAYS_PER_YEAR; ++i)
    {
        colFoDuration[i] = 1.;
        colPoDuration[i] = 1.;
    }
}

bool PreproThermal::normalizeAndCheckNPO(const AnyString& clustername, uint clusterSize)
{
    // alias to our data columns
    auto& columnNPOMax = data[npoMax];
    auto& columnNPOMin = data[npoMin];
    // errors management
    uint errors = 0;
    enum
    {
        maxErrors = 10
    };
    // Flag to determine whether the column NPO max has been normalized or not
    bool normalized = false;

    for (uint y = 0; y != data.height; ++y)
    {
        if (columnNPOMax[y] > clusterSize)
        {
            columnNPOMax[y] = clusterSize;
            normalized = true;
        }

        if (columnNPOMin[y] > columnNPOMax[y])
        {
            if (++errors < maxErrors)
            {
                logs.error() << clustername
                             << ": NPO min can not be greater than NPO max (hour: " << (y + 1)
                             << ", npo-min: " << columnNPOMin[y] << ", npo-max: " << columnNPOMax[y]
                             << ')';
            }
        }
    }

    if (errors >= maxErrors)
        logs.error() << clustername << ": too many errors. skipping (total: " << errors << ')';

    if (normalized)
        logs.info() << "  NPO max for the thermal cluster '" << clustername
                    << "' has been normalized";

    // modified and flushed
    data.markAsModified();
    data.flush();
    return (0 == errors);
}

} // namespace Data
} // namespace Antares
