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

static const char s_dgnTileMagic[]              = "dgnT";
static const uint32_t s_dgnTileVersion         = 0;


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct TileIO
{
    enum class ReadStatus
        {
        Success = 0,
        InvalidHeader,    
        ReadError,
        BatchTableParseError,
        SceneParseError,
        SceneDataError,
        FeatureTableError,
        };

    enum Flags
        {
        None            = 0,
        ContainsCurves  = 0x0001,
        Incomplete      = 0x0001 << 1,
        };

    // 3D tiles per the published specification...
    static BentleyStatus Write3dTile(StreamBufferR streamBuffer, Render::Primitives::GeometryCollectionCR geometry, DgnModelR model, DPoint3dCR centroid);
    static ReadStatus Read3dTile(Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, DgnModelR model, Render::System& renderSystem);


    // DgnTiles are tiles with GLTF meshes - but optimized for caching RenderPrimitives (storing feature table directly etc.)
    static BentleyStatus WriteDgnTile(StreamBufferR streamBuffer, ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, DgnModelR model, DPoint3dCR centroid);
    static ReadStatus ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, DgnModelR model, Render::System& renderSystem);
};  


END_TILETREE_NAMESPACE