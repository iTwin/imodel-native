#include "ScalableMeshPCH.h"
#include <ScalableMesh/IScalableMesh.h>
#include "MapBoxTextureProvider.h"
#include "ImagePPHeaders.h"
#include <ImagePP\all\h\HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HRARaster.h>
#include <ImagePP/all/h/HIMMosaic.h>
#include <ImagePP/all/h/HPMPooledVector.h>
#include <ImagePP/all/h/HRAClearOptions.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP\all\h\HCDPacket.h>
#include "RasterUtilities.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

DPoint2d MapBoxTextureProvider::_GetMinPixelSize()
    {
    DPoint2d minSize;

    minSize.x = m_minExt.GetWidth();
    minSize.y = m_minExt.GetHeight();
    return minSize;
    }

DRange2d MapBoxTextureProvider::_GetTextureExtent()
    {
    DRange2d rasterBox = DRange2d::From(m_totalExt);
    return rasterBox;
    }

StatusInt MapBoxTextureProvider::_GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area)
    {
    double unitsPerPixelX = (area.high.x - area.low.x) / width;
    double unitsPerPixelY = (area.high.y - area.low.y) / height;
    area.low.x -= 5 * unitsPerPixelX;
    area.low.y -= 5 * unitsPerPixelY;
    area.high.x += 5 * unitsPerPixelX;
    area.high.y += 5 * unitsPerPixelY;
    return SUCCESS;
    }

MapBoxTextureProvider::MapBoxTextureProvider(WString url, DRange3d totalExtent, GeoCoordinates::BaseGCSCPtr targetCS)
    {
    m_totalExt = totalExtent;

    DRange2d extent2d = DRange2d::From(m_totalExt);
    auto mapBoxRaster = RasterUtilities::LoadRaster(url, targetCS, extent2d);
    HGF2DExtent maxExt;
    mapBoxRaster->GetPixelSizeRange(m_minExt, maxExt);
    m_minExt.ChangeCoordSys(mapBoxRaster->GetCoordSys());
    }
