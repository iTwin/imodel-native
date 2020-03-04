/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ptcloud2/node.h>
#include <ptengine/VoxelQueryStateSet.h>
#include <ptcloud2/scene.h>
#include <ptengine/pointsvisitor.h>

namespace pointsengine
{

    class VoxelQueryStateRemoveVisitor : public PointsVisitor
    {
    public:
        typedef         VoxelQueryState::ClientID	    ClientID;

    protected:

        ClientID        clientID;

    public:
                        VoxelQueryStateRemoveVisitor    (ClientID id);

        void			setClientID                     (ClientID id)               { clientID = id; }
        ClientID		getClientID                     (void)                      { return clientID; }

        bool			visitNode                       (const pcloud::Node *n)     { return true; }
        bool			voxel                           (pcloud::Voxel *voxel);
    };

}
