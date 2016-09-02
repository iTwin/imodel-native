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

#define MB_MAP_RESOLUTION       19

BEGIN_IMAGEPP_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  1/2016
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
        
    char tempBuffer[300];
    int zoomFactor = MB_MAP_RESOLUTION - m_Resolution;
    
	sprintf(tempBuffer, "http://api.mapbox.com/v4/mapbox.satellite/%i/%llu/%llu.png32?%s", zoomFactor, pi_PosBlockX / 256, pi_PosBlockY / 256, "access_token=pk%2EeyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0%2Ef7c9GAxz6j10kZvL%5F2DBHg");
	

    Utf8String tileUri(tempBuffer);

        
    // Tile was not in lookAHead create a request.    
    MapBoxTileQuery tileQuery(TileID, tileUri, rasterFile);    

    tileQuery._Run();
    
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