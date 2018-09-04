/*-------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/IModelTileWriter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>
#include <DgnPlatform/TileWriter.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Tile::IO::WriteIModelTile(StreamBufferR streamBuffer, Tile::Content::MetadataCR metadata, Render::Primitives::GeometryCollectionCR geometry, Tile::LoaderCR loader)
    {
    return ERROR; // ###TODO...
    }

