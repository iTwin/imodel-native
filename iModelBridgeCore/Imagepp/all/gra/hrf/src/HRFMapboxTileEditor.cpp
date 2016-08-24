//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFMapboxTileEditor.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFMapBoxTileEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HRFMapBoxFile.h>
#include <Imagepp/all/h/HRFMapBoxTileEditor.h>
#include <ImagePP\all\h\HRFPngFile.h>
#include <ImagePPInternal\HttpConnection.h>
#include <BeJpeg\BeJpeg.h>

#include <ImagePP\all\h\HGF2DStretch.h>

#define VE_MAP_RESOLUTION       18

BEGIN_IMAGEPP_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
void MapBoxTileQuery::_Run()
    {
    HttpSession session;
    HttpRequest request(m_tileUri.c_str());
    HttpResponsePtr response;
    if(HttpRequestStatus::Success != session.Request(response, request) || response.IsNull() || response->GetBody().empty())
        return;

    auto contentTypeItr = response->GetHeader().find("Content-Type"); // case insensitive find.
    if(contentTypeItr == response->GetHeader().end())
        return;

    // We can tell from the header response if the tile is a missing tile(Kodak) or the real thing.
    // ("X-VE-Tile-Info", "no-tile")     
       
    if(contentTypeItr->second.EqualsI("image/png"))
        {
        bool isRGBA = false;
        uint32_t width, height;       
        if(SUCCESS == HRFPngFile::ReadToBuffer(m_tileData, width, height, isRGBA, response->GetBody().data(), response->GetBody().size()))
            {
            BeAssert(256 == width && 256 == height);

            if(isRGBA)  // we always output rgb, convert now if not.
                {
                bvector<Byte> rgbData(256*256*3);

                HRPPixelTypeV24R8G8B8 v24;
                HFCPtr<HRPPixelConverter> pRgbaToRgb = HRPPixelTypeV32R8G8B8A8().GetConverterTo(&v24);

                for(uint32_t line=0; line < height; ++line)
                    pRgbaToRgb->Convert(m_tileData.data() + line*width*4, rgbData.data() + line*256*3, width);

                m_tileData = std::move(rgbData);
                }
            }
        else
            {
            m_tileData.clear(); // ERROR
            }
        }
    else if(contentTypeItr->second.EqualsI("image/jpeg"))
        {
        m_tileData.resize(256*256*3); // assumed we have rgb data.
        
        BeJpegDecompressor decomp;                             
        if(SUCCESS != decomp.Decompress(m_tileData.data(), m_tileData.size(), response->GetBody().data(), response->GetBody().size(), BeJpegPixelType::BE_JPEG_PIXELTYPE_Rgb))
            m_tileData.clear(); // ERROR
        }
    else
        {
        BeAssertOnce(!"unsupported Content-Type");
        }
    
    }


//-----------------------------------------------------------------------------//
//                         Extern - MapBoxTileSystem API                 //
// This is an extern API that has been put here instead of in the extern       //
// library because it was more practical.                                      //
//-----------------------------------------------------------------------------//
const double MapBoxTileSystem::EarthRadius = 6378137;
const double MapBoxTileSystem::MinLatitude = -85.05112878;
const double MapBoxTileSystem::MaxLatitude = 85.05112878;
const double MapBoxTileSystem::MinLongitude = -180;
const double MapBoxTileSystem::MaxLongitude = 180;

double MapBoxTileSystem::Clip(double n, double minValue, double maxValue)
    {
    return MIN(MAX(n, minValue), maxValue);
    }

unsigned int MapBoxTileSystem::MapSize(int levelOfDetail)
    {
    return (unsigned int) 256 << levelOfDetail;
    }

double MapBoxTileSystem::GroundResolution(double latitude, int levelOfDetail)
    {
    latitude = Clip(latitude, MinLatitude, MaxLatitude);
    return cos(latitude * PI / 180) * 2 * PI * EarthRadius /
           MapSize(levelOfDetail);
    }

double MapBoxTileSystem::MapScale(double latitude, int levelOfDetail, int screenDpi)
    {
    return GroundResolution(latitude, levelOfDetail) * screenDpi / 0.0254;
    }

void MapBoxTileSystem::LatLongToPixelXY(double latitude, double longitude, int levelOfDetail,
                                                                     int* pixelX, int* pixelY)
    {
    latitude = Clip(latitude, MinLatitude, MaxLatitude);
    longitude = Clip(longitude, MinLongitude, MaxLongitude);

    double x = (longitude + 180) / 360;
    double sinLatitude = sin(latitude * PI / 180);
    double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * PI);

    unsigned int mapSize = MapSize(levelOfDetail);
    *pixelX = (int) Clip(x * mapSize + 0.5, 0, mapSize - 1);
    *pixelY = (int) Clip(y * mapSize + 0.5, 0, mapSize - 1);
    }

void MapBoxTileSystem::PixelXYToLatLong(int pixelX, int pixelY, int levelOfDetail,
                                                                     double* latitude, double* longitude)
    {
    double mapSize = MapSize(levelOfDetail);
    double x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;
    double y = 0.5 - (Clip(pixelY, 0, mapSize - 1) / mapSize);

    *latitude = 90 - 360 * atan(exp(-y * 2 * PI)) / PI;
    *longitude = 360 * x;
    }

void MapBoxTileSystem::PixelXYToTileXY(int pixelX, int pixelY, int* tileX, int* tileY)
    {
    *tileX = pixelX / 256;
    *tileY = pixelY / 256;
    }

void MapBoxTileSystem::TileXYToPixelXY(int tileX, int tileY,
                                                                    int* pixelX, int* pixelY)
    {
    *pixelX = tileX * 256;
    *pixelY = tileY * 256;
    }


string MapBoxTileSystem::TileXYToQuadKey(int tileX, int tileY, int levelOfDetail)
    {
    string quadKey;
    for (int i = levelOfDetail; i > 0; i--)
        {
        char digit = '0';
        int mask = 1 << (i - 1);
        if ((tileX & mask) != 0)
            {
            digit++;
            }
        if ((tileY & mask) != 0)
            {
            digit++;
            digit++;
            }
        quadKey += digit;
        }
    return quadKey;
    }

string MapBoxTileSystem::PixelXYToQuadKey(int pi_PixelX, int pi_PixelY, int pi_LevelOfDetail)
    {
    string quadKey;

    pi_PixelX /= 256;
    pi_PixelY /= 256;

    for (int i = pi_LevelOfDetail; i > 0; i--)
        {
        char digit = '0';
        int mask = 1 << (i - 1);
        if ((pi_PixelX & mask) != 0)
            {
            digit++;
            }
        if ((pi_PixelY & mask) != 0)
            {
            digit++;
            digit++;
            }
        quadKey += digit;
        }
    return quadKey;
    }

void MapBoxTileSystem::QuadKeyToTileXY(string quadKey,
                                                                    int* tileX, int* tileY, int* levelOfDetail)
    {
    *tileX = *tileY = 0;
    *levelOfDetail = (int)quadKey.size();
    for (int i = *levelOfDetail; i > 0; i--)
        {
        int mask = 1 << (i - 1);
        switch (quadKey[*levelOfDetail - i])
            {
            case '0':
                break;

            case '1':
                *tileX |= mask;
                break;

            case '2':
                *tileY |= mask;
                break;

            case '3':
                *tileX |= mask;
                *tileY |= mask;
                break;

            default:
                break;
            }
        }
    }



//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFMapBoxTileEditor::HRFMapBoxTileEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_TileIDDescriptor = HGFTileIDDescriptor(m_pResolutionDescriptor->GetWidth(),
                                             m_pResolutionDescriptor->GetHeight(),
                                             m_pResolutionDescriptor->GetBlockWidth(),
                                             m_pResolutionDescriptor->GetBlockHeight());

    // Our tile query assumed that we received 256x256 tiles and that we want RGB output.
    BeAssertOnce(m_pResolutionDescriptor->GetBlockWidth() == 256 && m_pResolutionDescriptor->GetBlockHeight() == 256 && 
                 m_pResolutionDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeId_V24R8G8B8));    
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFMapBoxTileEditor::~HRFMapBoxTileEditor()
    {
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
/*
Utf8String BuildTileUri(uint64_t tileId)
    {    
    uint64_t tilePosX, tilePosY;
    uint32_t level; 
    m_TileIDDescriptor.GetPositionFromID(tileId, &tilePosX, &tilePosY);
    level = m_TileIDDescriptor.GetPositionFromID(tileId);
    
    char tempBuffer[300];

    sprintf(tempBuffer, "http://api.mapbox.com/v4/mapbox.satellite/%ui/%uli/%uli.jpg80?access_token=pk%2EeyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0%2Ef7c9GAxz6j10kZvL%5F2DBHg", level, tilePosX, tilePosY);
    Utf8String tileUri(tempBuffer);
    return tileUri;
    }
    */
//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFMapBoxTileEditor::ReadBlock(uint64_t pi_PosBlockX,
                                    uint64_t pi_PosBlockY,
                                    Byte* po_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);
    
    // check if the tile is already in the pool
    uint64_t TileID = m_TileIDDescriptor.ComputeID(pi_PosBlockX, pi_PosBlockY, m_Resolution);                

    // Compute URI
    uint64_t tilePosX, tilePosY;    
    m_TileIDDescriptor.GetPositionFromID(TileID, &tilePosX, &tilePosY);
    
    HRFMapBoxFile& rasterFile = static_cast<HRFMapBoxFile&>(*GetRasterFile());
    HFCPtr<HRFPageDescriptor> pageDescriptor(rasterFile.GetPageDescriptor(0));

    HFCPtr<HGF2DTransfoModel> transfoModel(pageDescriptor->GetTransfoModel());

    /*
    GeoPoint2d outLatLong;
    DPoint2d   inCartesian; 
    */
    //HGF2DDisplacement translation(0, 0);    
    //double factor = pow(2, m_Resolution);
    //HGF2DStretch resolutionStretch(translation, factor, factor);
            
    //HFCPtr<HGF2DTransfoModel> pComposedModel1 = transfoModel->ComposeInverseWithDirectOf(resolutionStretch);
    //HFCPtr<HGF2DTransfoModel> pComposedModel1 = resolutionStretch.ComposeInverseWithDirectOf(*transfoModel);
        
    //pComposedModel1->ConvertDirect((double)pi_PosBlockX, (double)pi_PosBlockY, &inCartesian.x, &inCartesian.y);
    //GeoCoordinates::BaseGCSCP baseGcsP(pageDescriptor->GetGeocodingCP());                                                                                            
    //baseGcsP->LatLongFromCartesian2D(outLatLong, inCartesian);    

        
    char tempBuffer[300];
    int zoomFactor = VE_MAP_RESOLUTION - m_Resolution;

    //sprintf(tempBuffer, "http://api.mapbox.com/v4/mapbox.satellite/%i/%llu/%llu.jpg80?%s", zoomFactor, pi_PosBlockX / 256, pi_PosBlockY / 256, "access_token=pk%2EeyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0%2Ef7c9GAxz6j10kZvL%5F2DBHg");    
	sprintf(tempBuffer, "http://api.mapbox.com/v4/mapbox.satellite/%i/%llu/%llu.png32?%s", zoomFactor, pi_PosBlockX / 256, pi_PosBlockY / 256, "access_token=pk%2EeyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0%2Ef7c9GAxz6j10kZvL%5F2DBHg");
	

    Utf8String tileUri(tempBuffer);

        
    // Tile was not in lookAHead create a request.    

    MapBoxTileQuery tileQuery(TileID, tileUri, rasterFile);    

    tileQuery._Run();

    //assert(!tileQuery.m_tileData.empty());           

    if (!tileQuery.m_tileData.empty())
        {        
        BeAssert(tileQuery.m_tileData.size() == GetResolutionDescriptor()->GetBlockSizeInBytes());
        memcpy(po_pData, tileQuery.m_tileData.data(), GetResolutionDescriptor()->GetBlockSizeInBytes());
        }

    return H_SUCCESS;    
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFMapBoxTileEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlockY,
                                     const Byte*  pi_pData)
    {
    HASSERT(false);
    return H_ERROR;    
    }

END_IMAGEPP_NAMESPACE