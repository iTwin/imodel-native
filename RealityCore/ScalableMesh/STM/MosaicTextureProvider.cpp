/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include <ScalableMesh/IScalableMesh.h>
#include "MosaicTextureProvider.h"
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

DPoint2d MosaicTextureProvider::_GetMinPixelSize()
    {
    DPoint2d minSize;

    minSize.x = m_minExt.GetWidth();
    minSize.y = m_minExt.GetHeight();
    return minSize;
    }

DRange2d MosaicTextureProvider::_GetTextureExtent()
    {
    DRange2d rasterBox = DRange2d::From(m_targetMosaic->GetEffectiveShape()->GetExtent().GetXMin(), m_targetMosaic->GetEffectiveShape()->GetExtent().GetYMin(),
                                        m_targetMosaic->GetEffectiveShape()->GetExtent().GetXMax(), m_targetMosaic->GetEffectiveShape()->GetExtent().GetYMax());
    return rasterBox;
    }

StatusInt MosaicTextureProvider::_GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area)
    {
    double unitsPerPixelX = (area.high.x - area.low.x) / width;
    double unitsPerPixelY = (area.high.y - area.low.y) / height;
    area.low.x -= 5 * unitsPerPixelX;
    area.low.y -= 5 * unitsPerPixelY;
    area.high.x += 5 * unitsPerPixelX;
    area.high.y += 5 * unitsPerPixelY;
    
    HFCMatrix<3, 3> transfoMatrix;
    transfoMatrix[0][0] = (area.high.x - area.low.x) / width;
    transfoMatrix[0][1] = 0;
    transfoMatrix[0][2] = area.low.x;
    transfoMatrix[1][0] = 0;
    transfoMatrix[1][1] = -(area.high.y - area.low.y) / height;
    transfoMatrix[1][2] = area.high.y;
    transfoMatrix[2][0] = 0;
    transfoMatrix[2][1] = 0;
    transfoMatrix[2][2] = 1;

    HFCPtr<HGF2DTransfoModel> pTransfoModel((HGF2DTransfoModel*)new HGF2DProjective(transfoMatrix));

    HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pTransfoModel->CreateSimplifiedModel();

    if (pSimplifiedModel != 0)
        {
        pTransfoModel = pSimplifiedModel;
        }

    HFCPtr<HRABitmap> pTextureBitmap;

    HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV32R8G8B8A8());
#ifdef VANCOUVER_API
    HFCPtr<HCDCodec>     pCodec(new HCDCodecIdentity());
#endif

#ifdef VANCOUVER_API
    pTextureBitmap = new HRABitmap(width,
                                   height,
                                   pTransfoModel.GetPtr(),
                                   m_targetMosaic->GetCoordSys(),
                                   pPixelType,
                                   8,
                                   HRABitmap::UPPER_LEFT_HORIZONTAL,
                                   pCodec);
#else
    pTextureBitmap = HRABitmap::Create(width,
                                       height,
                                       pTransfoModel.GetPtr(),
                                       m_targetMosaic->GetCoordSys(),
                                       pPixelType,
                                       8);
#endif
    m_minExt.ChangeCoordSys(pTextureBitmap->GetCoordSys());

    pTextureBitmap = 0;

    return RasterUtilities::CopyFromArea(texData, width, height, area, nullptr, *m_targetMosaic);
    }

MosaicTextureProvider::MosaicTextureProvider(HFCPtr<HIMMosaic>& mosaic)
    : m_targetMosaic(mosaic)
    {
    HGF2DExtent maxExt;
    m_targetMosaic->GetPixelSizeRange(m_minExt, maxExt);
    m_minExt.ChangeCoordSys(mosaic->GetCoordSys());
    }
