/*--------------------------------------------------------------------------------------+                                                                                                                             '
|
|     $Source: PublicAPI/DgnPlatform/TileIO.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/RenderPrimitives.h>
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

    enum class WriteStatus
        {
        Success = 0,
        NoGeometry,
        UnableToLoadTile,
        UnableToOpenFile,
        Aborted,
        };

    enum Flags
        {
        None            = 0,
        ContainsCurves  = 0x0001,
        Incomplete      = 0x0001 << 1,
        IsLeaf          = 0x0001 << 2,
        };

    // DgnTiles are tiles with GLTF like structure (JSon with binary data) - but optimized for caching tiles (storing feature table directly etc.)
    static BentleyStatus WriteDgnTile(StreamBufferR streamBuffer, ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, GeometricModelR model, DPoint3dCR centroid, bool isLeaf);
    static ReadStatus ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem, bool& isLeaf);

};  



END_TILETREE_NAMESPACE
