/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterSource.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterSchemaInternal.h"
#include "RasterSource.h"

#define SQUARE(a) (a*a)
#define EPSILON 0.0000001
#define EQUAL_EPSILON(v1,v2)  ((v1 <= (v2+EPSILON)) && (v1 >= (v2-EPSILON)))

/*---------------------------------------------------------------------------------**//**
* @Description Compute a model that transform a unit cube to the provided corners.
*              4 corners mapping model (perspective).
*              Ref: Wolberg, George. 1990. Digital Image Warping, pp. 53-54
*
* @bsimethod                                                    StephanePoulin  8/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_ComputeUnitSquareToQuadrilateralTransfoModel(DMatrix4dR outMat, DPoint2dCR ul, DPoint2dCR ur, DPoint2dCR lf, DPoint2dCR lr)
    {
    double x0, x1, x2, x3;
    double y0, y1, y2, y3;

    x0 = ul.x;
    x1 = ur.x;
    x2 = lr.x;
    x3 = lf.x;
    y0 = ul.y;
    y1 = ur.y;
    y2 = lr.y;
    y3 = lf.y;

    double dx3 = x0 - x1 + x2 - x3;
    double dy3 = y0 - y1 + y2 - y3;
    
    DMatrix4d resultMat;
    resultMat.InitIdentity();
    resultMat.coff[2][2] = 1.0;

    if (!EQUAL_EPSILON (dx3, 0.0) || !EQUAL_EPSILON (dy3, 0.0))
        {
        double dx1 = x1 - x2;
        double dx2 = x3 - x2;
        double dy1 = y1 - y2;
        double dy2 = y3 - y2;

        // The transformation is projective
        resultMat.coff[0][2] = (dx3 * dy2 - dy3 * dx2) / (dx1 * dy2 - dy1 * dx2);
        resultMat.coff[1][2] = (dx1 * dy3 - dy1 * dx3) / (dx1 * dy2 - dy1 * dx2);
        resultMat.coff[0][0] = x1 - x0 + resultMat.coff[0][2] * x1;
        resultMat.coff[1][0] = x3 - x0 + resultMat.coff[1][2] * x3;
        resultMat.coff[3][0] = x0;
        resultMat.coff[0][1] = y1 - y0 + resultMat.coff[0][2] * y1;
        resultMat.coff[1][1] = y3 - y0 + resultMat.coff[1][2] * y3;
        resultMat.coff[3][1] = y0;
        }
    else
        {
        // The transformation is affine
        resultMat.coff[0][0] = x1 - x0;
        resultMat.coff[1][0] = x2 - x1;
        resultMat.coff[3][0] = x0;
        resultMat.coff[0][1] = y1 - y0;
        resultMat.coff[1][1] = y2 - y1;
        resultMat.coff[3][1] = y0;
        resultMat.coff[0][2] = 0.0;
        resultMat.coff[1][2] = 0.0;
        }

    outMat.TransposeOf(resultMat);

    if (sqrt(SQUARE(outMat.coff[0][0]) + SQUARE(outMat.coff[1][0])) > EPSILON &&
        sqrt(SQUARE(outMat.coff[0][1]) + SQUARE(outMat.coff[1][1])) > EPSILON)
        {
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @Description Compute a model that transform source points to destination points.
*             4 corners mapping model (perspective).
* @param pSrcPoint IN      array of 4 DPoint2d. organized: [UpperLeft, UpperRight, LowerLeft, LowerRight]
* @param pDstPoints IN     array of 4 DPoint2d. organized: [UpperLeft, UpperRight, LowerLeft, LowerRight]
* @bsimethod                                                    StephanePoulin  9/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_ComputeQuadrilateralToQuadrilateralTransfoModel(DMatrix4dR outMat, DPoint2dCP pSrcPoint, DPoint2dCP pDstPoints)
    {
    // create a model that transform a unit cube to the raster corners
    DMatrix4d unitToRasterModel;
    if(!s_ComputeUnitSquareToQuadrilateralTransfoModel(unitToRasterModel, pSrcPoint[0], pSrcPoint[1], pSrcPoint[2], pSrcPoint[3]))
        return false;

    // create a model that transform a unit cube to the projected raster corners
    DMatrix4d unitToProjectedRasterModel;
    if(!s_ComputeUnitSquareToQuadrilateralTransfoModel(unitToProjectedRasterModel, pDstPoints[0], pDstPoints[1], pDstPoints[2], pDstPoints[3]))
        return false;

    // Compute the projecton model
    // perspectiveModel = pUnitToProjectedRasterModel * INV(pUnitToRasterModel)
    DMatrix4d invUnitToRasterModel;
    if(!invUnitToRasterModel.QrInverseOf(unitToRasterModel))
        return false;
    
    outMat.InitProduct(unitToProjectedRasterModel, invUnitToRasterModel);   // A * B

    return true;
    }

//----------------------------------------------------------------------------------------
//-------------------------------  Bitmap ------------------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
size_t Bitmap::GetBytePerPixel(Bitmap::PixelType pixelType)
    {
    switch(pixelType)
        {
        case PixelType::Rgba:
        case PixelType::Bgra:
            return 4;
        case PixelType::Rgb:
        case PixelType::Bgr:
            return 3;
        case PixelType::Gray:
            return 1;
        default: 
            break;
        };

    BeAssert(!"Invalid pixeltype");
    return 0;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
uint32_t Bitmap::GetWidth() const {return m_width;}
uint32_t Bitmap::GetHeight() const {return m_height;}
Bitmap::PixelType Bitmap::GetPixelType() const {return m_pixelType;}
bool Bitmap::IsTopDown() const {return m_isTopDown;} 
Byte const* Bitmap::GetBufferCP(size_t& pitch) const {pitch=m_pitch; return m_pBuffer.get();}
Byte* Bitmap::GetBufferP(size_t& pitch) {pitch=m_pitch; return m_pBuffer.get();}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
BitmapPtr Bitmap::Create(uint32_t width, uint32_t height, PixelType pixelType, bool isTopDown/*=true*/)
    {
    size_t pitch = width*GetBytePerPixel(pixelType);
    Byte* pBuf = new Byte[pitch*height];
    if(nullptr == pBuf)
        return NULL;   

    return new Bitmap(width, height, pixelType, isTopDown, pBuf, pitch);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
Bitmap::Bitmap(uint32_t width, uint32_t height, PixelType pixelType, bool isTopDown, Byte* pBuf, size_t pitch)
:m_width(width),
 m_height(height),
 m_pixelType(pixelType),
 m_isTopDown(isTopDown),
 m_pBuffer(pBuf),   // take ownership of memory.
 m_pitch(pitch)
    {
    BeAssert(m_width*GetBytePerPixel(m_pixelType) >= pitch);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
DisplayTilePtr DisplayTile::Create(uint32_t width, uint32_t height, DisplayTile::PixelType pixelType, Byte const* pData, uint32_t pitch)
    {
    DisplayTilePtr pTile = new DisplayTile();

    Point2d size = {width, height};

    pTile->m_haveTexture = true;
    T_HOST.GetGraphicsAdmin()._DefineTile(pTile->GetTextureId(), NULL, size, true/*enableAlpha &&MM - add as parameter */, static_cast<uint32_t>(pixelType), pitch, pData);
    return pTile;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
DisplayTile::~DisplayTile()
    {
    if(m_haveTexture)
        T_HOST.GetGraphicsAdmin()._DeleteTexture (GetTextureId());

    m_haveTexture = false;
    }

//----------------------------------------------------------------------------------------
//-------------------------------  RasterSource   ----------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
DisplayTilePtr RasterSource::QueryTile(TileId const& id, bool request) {return _QueryTile(id, request);}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
RasterSource::RasterSource()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
RasterSource::~RasterSource(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
BentleyStatus RasterSource::Initialize(DPoint3dCP corners, bvector<Resolution>const& resolution, GeoCoordinates::BaseGCSP pGcs)
    {
    BentleyStatus status = SUCCESS;
    m_resolution = resolution;
    
    memcpy(m_corners, corners, sizeof(m_corners));

    // Build the physical to cartesian matrix.
    DPoint2d srcCorners[4];
    srcCorners[0].x = srcCorners[2].x = 0; 
    srcCorners[1].x = srcCorners[3].x = m_resolution[0].GetWidth();  
    srcCorners[0].y = srcCorners[1].y = 0; 
    srcCorners[2].y = srcCorners[3].y = m_resolution[0].GetHeight(); 

    DPoint2d destCorners[4];
    for (size_t i=0; i < 4; ++i)
        destCorners[i].Init(m_corners[i].x, m_corners[i].y);
        
    if(!s_ComputeQuadrilateralToQuadrilateralTransfoModel(m_physicalToCartesian, srcCorners, destCorners))
        {
        BeAssert(!"Cannot compute physicalToCartesian matrix"); // How that happen?
        m_physicalToCartesian.InitIdentity();
        status = ERROR;
        }    

    m_pGcs = pGcs;  // add a ref

    return status;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterSource::GenerateResolution(bvector<Resolution>& resolution, uint32_t width, uint32_t height, uint32_t tileSizeX, uint32_t tileSizeY)
    {
    resolution.push_back(Resolution(width, height, tileSizeX, tileSizeY));

    // Add the resolution until we have reach the lower limits.
    while (height > tileSizeY || width > tileSizeX)
        {
        // Compute the lower resolution.
        width = (uint32_t)ceil(width / 2.0);
        height = (uint32_t)ceil(height / 2.0);

        resolution.push_back(Resolution(width, height, tileSizeX, tileSizeY));
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
bool RasterSource::IsValidTileId(TileId const& id) const
    {
    if(id.resolution >= GetResolutionCount() || 
       id.tileX >= GetResolution(id.resolution).GetTileCountX() ||
       id.tileY >= GetResolution(id.resolution).GetTileCountY())
        return false;

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterSource::ComputeTileCorners(DPoint3dP pCorners, TileId const& id) const
    {
    BeAssert(IsValidTileId(id));

    uint32_t xMinInRes = id.tileX * GetResolution(id.resolution).GetTileSizeX();
    uint32_t yMinInRes = id.tileY * GetResolution(id.resolution).GetTileSizeY();
    uint32_t xMaxInRes = xMinInRes + GetResolution(id.resolution).GetTileSizeX();
    uint32_t yMaxInRes = yMinInRes + GetResolution(id.resolution).GetTileSizeY();

    uint32_t xMin = xMinInRes << id.resolution;
    uint32_t yMin = yMinInRes << id.resolution;
    uint32_t xMax = xMaxInRes << id.resolution;
    uint32_t yMax = yMaxInRes << id.resolution;

    // Limit the tile extent to the raster physical size 
    if (xMax > GetWidth())
        xMax = GetWidth();
    if (yMax > GetHeight())
        yMax = GetHeight();

    BeAssert(xMax >= xMin);  // For a tile of one pixel, xMin == xMax
    BeAssert(yMax >= yMin);

    // Convert pixel to coordinates.
    DPoint3d physicalCorners[4];
    physicalCorners[0].x = physicalCorners[2].x = xMin; 
    physicalCorners[1].x = physicalCorners[3].x = xMax;  
    physicalCorners[0].y = physicalCorners[1].y = yMin; 
    physicalCorners[2].y = physicalCorners[3].y = yMax; 
    physicalCorners[0].z = physicalCorners[1].z = physicalCorners[2].z = physicalCorners[3].z = 0;

    m_physicalToCartesian.MultiplyAndRenormalize(pCorners, physicalCorners, 4);

    for(uint32_t i=0; i < 4; ++i)
        {
        BeAssert(IN_RANGE(pCorners[i].x, m_corners[0].x-EPSILON, m_corners[3].x+EPSILON));
        BeAssert(IN_RANGE(pCorners[i].y, m_corners[0].y-EPSILON, m_corners[3].y+EPSILON));
        }
    }

    