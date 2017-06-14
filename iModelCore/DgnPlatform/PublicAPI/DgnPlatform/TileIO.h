/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/TileIO.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/DgnMaterial.h>

BEGIN_TILETREE_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct TileIO
{

    static BentleyStatus WriteTile(StreamBufferR streamBuffer, Render::Primitives::GeometryCollectionCR geometry, DgnModelR model, DPoint3dCR centroid);
    static BentleyStatus ReadTile(Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, DgnModelR model);
};  


END_TILETREE_NAMESPACE