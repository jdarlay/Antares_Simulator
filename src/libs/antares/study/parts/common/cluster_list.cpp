#include "cluster_list.h"
#include "../../memory-usage.h"
#include "../../../logs.h"
#include "../../study.h"

using namespace Yuni;
using namespace Antares;

#define SEP IO::Separator

namespace // anonymous
{
struct TSNumbersPredicate
{
    uint32 operator()(uint32 value) const
    {
        return value + 1;
    }
};
} // namespace

namespace Antares
{
namespace Data
{
Cluster* ClusterList::detach(iterator i)
{
    auto* c = i->second;
    cluster.erase(i);
    return c;
}

void ClusterList::remove(iterator i)
{
    cluster.erase(i);
}

bool ClusterList::exists(const Data::ClusterName& id) const
{
    if (not cluster.empty())
    {
        auto element = cluster.find(id);
        return (element != cluster.end());
    }
    return false;
}
Data::ClusterList::ClusterList(uint sizeGroup) : byIndex(nullptr),
                                                 groupCount(sizeGroup, 0)
{
}

Data::ClusterList::~ClusterList()
{
    // deleting all renewable clusters
    clear();
}

void ClusterList::clear()
{
    if (byIndex)
    {
        delete[] byIndex;
        byIndex = nullptr;
    }

    if (not cluster.empty())
        cluster.clear();
}

const Cluster* ClusterList::find(const Cluster* p) const
{
    auto end = cluster.end();
    for (auto i = cluster.begin(); i != end; ++i)
    {
        if (p == i->second)
            return i->second;
    }
    return nullptr;
}

Data::Cluster* ClusterList::find(const Cluster* p)
{
    auto end = cluster.end();
    for (auto i = cluster.begin(); i != end; ++i)
    {
        if (p == i->second)
            return i->second;
    }
    return nullptr;
}

void ClusterList::resizeAllTimeseriesNumbers(uint n)
{
    assert(n < 200000); // arbitrary number
    if (not cluster.empty())
    {
        if (0 == n)
        {
            each([&](Cluster& cluster) { cluster.series->timeseriesNumbers.clear(); });
        }
        else
        {
            each([&](Cluster& cluster) { cluster.series->timeseriesNumbers.resize(1, n); });
        }
    }
}

void ClusterList::estimateMemoryUsage(StudyMemoryUsage& u) const
{
    u.requiredMemoryForInput += (sizeof(void*) * 4 /*overhead map*/) * cluster.size();

    each([&](const Cluster& cluster) {
        u.requiredMemoryForInput += sizeof(Cluster);
        u.requiredMemoryForInput += sizeof(void*);
        if (cluster.series)
            cluster.series->estimateMemoryUsage(u, timeSeriesRenewable /* FIXME */);

        // From the solver
        u.requiredMemoryForInput += 70 * 1024;
    });
}

void ClusterList::rebuildIndex()
{
    delete[] byIndex;

    if (not empty())
    {
        uint indx = 0;
        typedef Cluster* ClusterWeakPtr;
        byIndex = new ClusterWeakPtr[size()];

        auto end = cluster.end();
        for (auto i = cluster.begin(); i != end; ++i)
        {
            auto* cluster = i->second;
            byIndex[indx] = cluster;
            cluster->index = indx;
            ++indx;
        }
    }
    else
        byIndex = nullptr;
}

bool ClusterList::add(Cluster* newcluster)
{
    if (newcluster)
    {
        if (exists(newcluster->id()))
            return true;

        newcluster->index = (uint)size();
        cluster[newcluster->id()] = newcluster;
        ++(groupCount[newcluster->groupId()]);
        rebuildIndex();
        return true;
    }
    return false;
}

static bool ClusterLoadFromProperty(Cluster& cluster, const IniFile::Property* p)
{
    if (p->key.empty())
        return false;

    if (p->key == "group")
    {
        cluster.setGroup(p->value);
        return true;
    }

    if (p->key == "name")
        return true;

    if (p->key == "enabled")
        return p->value.to<bool>(cluster.enabled);

    // The property is unknown
    return false;
}

static bool ClusterLoadFromSection(const AnyString& filename,
                                   Cluster& cluster,
                                   const IniFile::Section& section)
{
    if (section.name.empty())
        return false;

    cluster.setName(section.name);

    if (section.firstProperty)
    {
        // Browse all properties
        for (auto* property = section.firstProperty; property; property = property->next)
        {
            if (property->key.empty())
            {
                logs.warning() << '`' << filename << "`: `" << section.name
                               << "`: Invalid key/value";
                continue;
            }
            if (not ClusterLoadFromProperty(cluster, property))
            {
                logs.warning() << '`' << filename << "`: `" << section.name << "`/`"
                               << property->value << "`: The property is unknown and ignored";
            }
        }
        // update the minUpDownTime
    }
    return true;
}

bool ClusterList::loadFromFolder(Study& study, const AnyString& folder, Area* area)
{
    assert(area and "A parent area is required");

    // logs
    logs.info() << "Loading renewable configuration for the area " << area->name;

    // Open the ini file
    study.buffer.clear() << folder << SEP << "list.ini";
    IniFile ini;
    if (ini.open(study.buffer))
    {
        bool ret = true;

        if (ini.firstSection)
        {
            for (auto* section = ini.firstSection; section; section = section->next)
            {
                if (section->name.empty())
                    continue;

                Cluster* cluster = clusterFactory(area, study.maxNbYearsInParallel);

                // Load data of a renewable cluster from a ini file section
                if (not ClusterLoadFromSection(study.buffer, *cluster, *section))
                {
                    delete cluster;
                    continue;
                }

                // Check the data integrity of the cluster
                cluster->integrityCheck();

                // adding the renewable cluster
                if (not add(cluster))
                {
                    // This error should never happen
                    logs.error() << "Impossible to add the renewable cluster '" << cluster->name()
                                 << "'";
                    delete cluster;
                    continue;
                }

                cluster->flush();
            }
        }

        return ret;
    }
    return false;
}

Yuni::uint64 ClusterList::memoryUsage() const
{
    uint64 ret = sizeof(ClusterList) + (2 * sizeof(void*)) * this->size();

    each([&](const Data::Cluster& cluster) { ret += cluster.memoryUsage(); });
    return ret;
}

bool ClusterList::rename(Data::ClusterName idToFind, Data::ClusterName newName)
{
    if (not idToFind or newName.empty())
        return false;

    // Internal:
    // It is vital to make copy of these strings. We can not make assumption that these
    // CString are not from the same buffer (name, id) than ours.
    // It may have an undefined behavior.
    // Consequently, the parameters `idToFind` and `newName` shall not be `const &`.

    // Making sure that the id is lowercase
    idToFind.toLower();

    // The new ID
    Data::ClusterName newID;
    TransformNameIntoID(newName, newID);

    // Looking for the renewable cluster in the list
    auto it = cluster.find(idToFind);
    if (it == cluster.end())
        return true;

    Data::Cluster* p = it->second;

    if (idToFind == newID)
    {
        p->setName(newName);
        return true;
    }

    // The name is the same. Aborting nicely.
    if (p->name() == newName)
        return true;

    // Already exist
    if (this->exists(newID))
        return false;

    cluster.erase(it);

    p->setName(newName);
    cluster[newID] = p;

    // Invalidate matrices attached to the area
    // It is a bit excessive (all matrices not only those related to the renewable cluster)
    // will be rewritten but currently it is the less error-prone.
    if (p->parentArea)
        (p->parentArea)->invalidateJIT = true;

    // Rebuilding the index
    rebuildIndex();
    return true;
}

bool Data::ClusterList::invalidate(bool reload) const
{
    bool ret = true;
    auto end = cluster.end();
    for (auto i = cluster.begin(); i != end; ++i)
        ret = (i->second)->invalidate(reload) and ret;
    return ret;
}

void Data::ClusterList::markAsModified() const
{
    auto end = cluster.end();
    for (auto i = cluster.begin(); i != end; ++i)
        (i->second)->markAsModified();
}

bool ClusterList::storeTimeseriesNumbers(Study& study)
{
    if (cluster.empty())
        return true;

    bool ret = true;
    TSNumbersPredicate predicate;

    each([&](const Data::Cluster& cluster) {
        study.buffer = study.folderOutput;
        study.buffer << SEP << "ts-numbers" << SEP << "renewable" << SEP << cluster.parentArea->id
                     << SEP << cluster.id() << ".txt";
        ret = cluster.series->timeseriesNumbers.saveToCSVFile(study.buffer, 0, true, predicate)
              and ret;
    });
    return ret;
}

void ClusterList::retrieveTotalCapacity(double& total) const
{
    total = 0.;

    if (not cluster.empty())
    {
        auto end = cluster.cend();
        for (auto i = cluster.cbegin(); i != end; ++i)
        {
            if (not i->second)
                return;

            // Reference to the renewable cluster
            auto& cluster = *(i->second);
            total += cluster.nominalCapacity;
        }
    }
}

bool ClusterList::remove(const Data::ClusterName& id)
{
    auto i = cluster.find(id);
    if (i == cluster.end())
        return false;

    // Getting the pointer on the cluster
    auto* c = i->second;

    // Removing it from the list
    cluster.erase(i);
    // Invalidating the parent area
    c->parentArea->invalidate();

    // delete the cluster
    delete c;

    // Rebuilding the index
    rebuildIndex();
    return true;
}

bool ClusterList::saveToFolder(const AnyString& folder) const
{
    // Make sure the folder is created
    if (IO::Directory::Create(folder))
    {
        Clob buffer;
        bool ret = true;

        // Allocate the inifile structure
        IniFile ini;

        // Browse all clusters
        each([&](const Data::Cluster& cluster) {
            // Adding a section to the inifile
            IniFile::Section* s = ini.addSection(cluster.name());

            // The section must not be empty
            // This key will be silently ignored the next time
            s->add("name", cluster.name());

            if (not cluster.group().empty())
                s->add("group", cluster.group());
            if (not cluster.enabled)
                s->add("enabled", "false");
        });

        // Write the ini file
        buffer.clear() << folder << SEP << "list.ini";
        ret = ini.save(buffer) and ret;
    }
    else
    {
        logs.error() << "I/O Error: impossible to create '" << folder << "'";
        return false;
    }

    return true;
}

int ClusterList::saveDataSeriesToFolder(const AnyString& folder) const
{
    if (empty())
        return 1;

    int ret = 1;

    auto end = cluster.end();
    for (auto it = cluster.begin(); it != end; ++it)
    {
        auto& cluster = *(it->second);
        if (cluster.series)
            ret = cluster.saveDataSeriesToFolder(folder) and ret;
    }
    return ret;
}

int ClusterList::saveDataSeriesToFolder(const AnyString& folder, const String& msg) const
{
    if (empty())
        return 1;

    int ret = 1;
    uint ticks = 0;

    auto end = cluster.end();
    for (auto it = cluster.begin(); it != end; ++it)
    {
        auto& cluster = *(it->second);
        if (cluster.series)
        {
            logs.info() << msg << "  " << (ticks * 100 / (1 + this->cluster.size()))
                        << "% complete";
            ret = cluster.saveDataSeriesToFolder(folder) and ret;
        }
        ++ticks;
    }
    return ret;
}

int ClusterList::loadDataSeriesFromFolder(Study& s,
                                          const StudyLoadOptions& options,
                                          const AnyString& folder)
{
    if (empty())
        return 1;

    int ret = 1;

    each([&](Data::Cluster& cluster) {
        if (cluster.series)
            ret = cluster.loadDataSeriesFromFolder(s, folder) and ret;

        ++options.progressTicks;
        options.pushProgressLogs();
    });
    return ret;
}

void ClusterList::ensureDataTimeSeries()
{
    auto end = cluster.end();
    for (auto it = cluster.begin(); it != end; ++it)
    {
        auto& cluster = *(it->second);
        if (not cluster.series)
            cluster.series = new DataSeriesCommon();
    }
}

} // namespace Data
} // namespace Antares