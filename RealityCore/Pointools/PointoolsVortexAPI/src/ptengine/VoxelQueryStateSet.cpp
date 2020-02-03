/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"

#include <ptengine/VoxelQueryStateSet.h>

namespace pointsengine
{
    typedef VoxelQueryState SS;


    VoxelQueryState::VoxelQueryState(void)
    {
        clear();
    }

    VoxelQueryState::VoxelQueryState(ClientID id, PointIndex lastPointIndex) : VoxelQueryState()
    {
        setClientID(id);

        setLastPoint(lastPointIndex);
    }

    void VoxelQueryState::clear(void)
    {
        setClientID(VOXEL_QUERY_STATE_VOXEL_CLIENT_ID_NULL);

        setLastPoint(VOXEL_QUERY_STATE_POINT_INDEX_NULL);
    }

    bool VoxelQueryStateSet::setQueryState(VoxelQueryState & state)
    {
        VoxelQueryState *existingState;
                                                            // If state already exists for state's ClientID
        if (existingState = getQueryState(state.getClientID()))
        {
                                                            // Update the existing state
            *existingState = state;
        }
        else
        {
                                                            // It doesn't exist, so add a new query state
            try
            {
                voxelQueryStates.push_back(state);
            }
            catch (...)
            {
                                                            // An error occcured, so return false
                return false;
            }
        }
                                                            // Return OK
        return true;
    }

    const VoxelQueryState &VoxelQueryState::operator=(const VoxelQueryState &other)
    {
        setClientID(other.getClientID());
        setLastPoint(other.getLastPoint());

        return *this;
    }

    unsigned int VoxelQueryStateSet::getNumItems(void) const
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

