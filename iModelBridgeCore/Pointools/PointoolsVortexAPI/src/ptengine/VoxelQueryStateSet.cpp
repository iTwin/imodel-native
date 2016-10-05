#include "PointoolsVortexAPIInternal.h"

#include <ptengine/VoxelQueryStateSet.h>

namespace pointsengine
{
    typedef VoxelQueryState SS;

    VoxelQueryState::VoxelQueryState(void)
    {
        clear();
    }

    void VoxelQueryState::clear(void)
    {
        setClientID(VOXEL_QUERY_STATE_VOXEL_CLIENT_ID_NULL);

        setLastPoint(VOXEL_QUERY_STATE_POINT_INDEX_NULL);
    }

    bool VoxelQueryStateSet::setQueryState(VoxelQueryState & state)
    {
        try
        {
            voxelQueryStates.push_back(state);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    const VoxelQueryState &VoxelQueryState::operator=(const VoxelQueryState &other)
    {
        setClientID(other.getClientID());
        setLastPoint(other.getLastPoint());

        return *this;
    }

    unsigned unsigned int VoxelQueryStateSet::getNumItems(void) const
    {
        return static_cast<int>(voxelQueryStates.size());
    }

    VoxelQueryState *VoxelQueryStateSet::getQueryState(VoxelQueryState::ClientID id, Iterator *item)
    {
        Iterator    i;
                                                            // Iterate over stored query states for this voxel
        for (i = voxelQueryStates.begin(); i != voxelQueryStates.end(); i++)
        {
                                                            // If state matches ID, state is found
            if (i->getClientID() == id)
            {
                                                            // If clientIndex is requested, return it
                if (item)
                {
                    *item = i;
                }

                (*i).getClientID();
                                                            // Return the query state
                return &(*i);
            }
        }
                                                            // Not found, so return NULL
        if (item)
        {
            *item = voxelQueryStates.end();
        }

        return nullptr;
    }

    bool VoxelQueryStateSet::removeQueryState(VoxelQueryState::ClientID id)
    {
        Iterator i;
                                                            // Get the query state entry
        if (getQueryState(id, &i))
        {
                                                            // If found, erase it
            voxelQueryStates.erase(i);
                                                            // Return removed
            return true;
        }
                                                            // Not found, so return false
        return false;
    }

    void VoxelQueryStateSet::clear(void)
    {
        voxelQueryStates.clear();
    }

}

