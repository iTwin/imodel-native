#pragma once

#include <vector>


namespace pointsengine
{

    class VoxelQueryState
    {
    public:

        typedef int                 ClientID;
        typedef int                 PointIndex;

    public:

        const ClientID              VOXEL_QUERY_STATE_VOXEL_CLIENT_ID_NULL  = -1;
        const PointIndex            VOXEL_QUERY_STATE_POINT_INDEX_NULL      = -1;
        const PointIndex            VOXEL_QUERY_STATE_INDEX_NULL            = -1;

    protected:

        ClientID                    clientID;
        PointIndex                  lastPoint;

    public:
                                    VoxelQueryState     (void);
                                    VoxelQueryState     (ClientID id, PointIndex lastPointIndex);

        void                        clear               (void);

        void                        setClientID         (ClientID id)           {clientID = id;}
        ClientID                    getClientID         (void) const            {return clientID;}

        void                        setLastPoint        (PointIndex index)      {lastPoint = index;}
        PointIndex                  getLastPoint        (void) const            {return lastPoint;}

        const VoxelQueryState    &  operator=           (const VoxelQueryState &other);
    };


    class VoxelQueryStateSet
    {
    protected:

        typedef     std::vector<VoxelQueryState>    VoxelQueryStates;
        typedef     int                             StateIndex;

        typedef     VoxelQueryStates::iterator      Iterator;

    protected:

        VoxelQueryStates                            voxelQueryStates;

    public:

        bool                                        setQueryState               (VoxelQueryState &state);
        VoxelQueryState                         *   getQueryState               (VoxelQueryState::ClientID id, Iterator *item = nullptr);

        bool                                        removeQueryState            (VoxelQueryState::ClientID id);

        void                                        clear                       (void);

        unsigned int                                getNumItems                 (void) const;
    };

}


