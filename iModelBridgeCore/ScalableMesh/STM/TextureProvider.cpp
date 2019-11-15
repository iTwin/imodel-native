/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include <ScalableMesh/ITextureProvider.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH


DPoint2d ITextureProvider::GetMinPixelSize()
    {
    return _GetMinPixelSize();
    }

DRange2d ITextureProvider::GetTextureExtent()
    {
    return _GetTextureExtent();
    }

StatusInt ITextureProvider::GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area)
    {
    return _GetTextureForArea(texData, width, height, area);
    }

