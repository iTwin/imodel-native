/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/TileReader.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>

BEGIN_TILETREE_IO_NAMESPACE

//=======================================================================================
//! A read-only view of a GLTF buffer, pointing directly into the buffer data.
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct BufferView
{
    void const* pData = nullptr;
    size_t      count;
    size_t      byteLength;
    uint32_t    type;
    Json::Value accessor;

    bool IsValid() const { return nullptr != pData; }
};

END_TILETREE_IO_NAMESPACE

