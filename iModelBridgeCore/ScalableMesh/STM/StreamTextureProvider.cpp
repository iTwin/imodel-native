/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include <ScalableMesh/IScalableMesh.h>
#include "StreamTextureProvider.h"
#include "ImagePPHeaders.h"
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HRARaster.h>
#include <ImagePP/all/h/HIMMosaic.h>
#include <ImagePP/all/h/HRAClearOptions.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDPacket.h>
#include "RasterUtilities.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

static double s_minPixelSizeLimit = 0;

DPoint2d StreamTextureProvider::_GetMinPixelSize()
    {
    DPoint2d minSize;

    minSize.x = m_minExt.GetWidth();
    minSize.y = m_minExt.GetHeight();  

#ifndef NDEBUG
    if (s_minPixelSizeLimit != 0)
        {
        minSize.x = s_minPixelSizeLimit;
        minSize.y = s_minPixelSizeLimit;
        }
#endif

    return minSize;
    }

DRange2d StreamTextureProvider::_GetTextureExtent()
    {
    DRange2d rasterBox = DRange2d::From(m_totalExt);
    return rasterBox;
    }

StatusInt StreamTextureProvider::_GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area)
    {
    double unitsPerPixelX = (area.high.x - area.low.x) / width;
    double unitsPerPixelY = (area.high.y - area.low.y) / height;
    area.low.x -= 5 * unitsPerPixelX;
    area.low.y -= 5 * unitsPerPixelY;
    area.high.x += 5 * unitsPerPixelX;
    area.high.y += 5 * unitsPerPixelY;
    return SUCCESS;
    }

StreamTextureProvider::StreamTextureProvider(HFCPtr<HRARASTER>& textureSource, DRange3d totalExtent)
    {
    m_totalExt = totalExtent;    
    
    HGF2DExtent maxExt;
    textureSource->GetPixelSizeRange(m_minExt, maxExt);
    m_minExt.ChangeCoordSys(textureSource->GetCoordSys());
    }
