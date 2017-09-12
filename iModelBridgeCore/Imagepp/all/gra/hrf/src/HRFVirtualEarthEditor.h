//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFVirtualEarthEditor.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HGFTileIDDescriptor.h>
#include <Imagepp/all/h/HRFResolutionEditor.h>

#include <ImagePPInternal/gra/Task.h>

BEGIN_IMAGEPP_NAMESPACE
class HRFVirtualEarthConnection;
class HRFVirtualEarthFile;
class HRFVirtualEarthEditor;

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
struct VirtualEarthTileQuery : public WorkerPool::Task
{
    VirtualEarthTileQuery(uint64_t tileId, Utf8StringCR tileUri, HRFVirtualEarthFile& rasterFile) 
    :m_tileId(tileId), m_tileUri(tileUri), m_rasterFile(rasterFile) 
        {}

    virtual ~VirtualEarthTileQuery(){};
    
    virtual void _Run() override;

    uint64_t                    m_tileId;
    Utf8String                  m_tileUri;
    bvector<Byte>               m_tileData;
    HRFVirtualEarthFile&        m_rasterFile;  // Do not hold a HRFVirtualEarthEditor since it might be destroyed while query are still running. 
};

//-----------------------------------------------------------------------------
// HRFVirtualEarthEditor class
//-----------------------------------------------------------------------------
class HRFVirtualEarthEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFVirtualEarthFile;

    virtual                         ~HRFVirtualEarthEditor  ();

    //Edition by Block
    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              Byte*   po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const Byte* pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override;

    int GetLevelOfDetail() const;

    Utf8String BuildTileUri(uint64_t tileId);

protected:

    //Constructor
    HRFVirtualEarthEditor    (HFCPtr<HRFRasterFile> pi_rpRasterFile, uint32_t pi_Page, unsigned short pi_Resolution, HFCAccessMode pi_AccessMode);

    //Request look ahead
    virtual void              RequestLookAhead(const HGFTileIDList& pi_rTileIDList, uint32_t pi_ConsumerID);

private:
    HGFTileIDDescriptor m_TileIDDescriptor;

//-----------------------------------------------------------------------------//
//                         Extern - VirtualEarthTileSystem API                 //
// This is an extern API that has been put here instead of in the extern       //
// library because it was more practical.                                      //
//-----------------------------------------------------------------------------//
    class VirtualEarthTileSystem
        {
    private :
        static const double EarthRadius;
        static const double MinLatitude;
        static const double MaxLatitude;
        static const double MinLongitude;
        static const double MaxLongitude;

    public :
        static double  Clip(double n, double minValue, double maxValue);
        static unsigned int MapSize(int levelOfDetail);
        static double  GroundResolution(double latitude, int levelOfDetail);
        static double  MapScale(double latitude, int levelOfDetail, int screenDpi);
        static void    LatLongToPixelXY(double latitude, double longitude, int levelOfDetail, int* pixelX, int* pixelY);
        static void    PixelXYToLatLong(int pixelX, int pixelY, int levelOfDetail, double* latitude, double* longitude);
        static void    PixelXYToTileXY(int pixelX, int pixelY, int* tileX, int* tileY);
        static void    TileXYToPixelXY(int tileX, int tileY, int* pixelX, int* pixelY);
        static string  TileXYToQuadKey(int tileX, int tileY, int levelOfDetail);
        static string  PixelXYToQuadKey(int pi_PixelX, int pi_PixelY, int pi_LevelOfDetail);
        static void    QuadKeyToTileXY(string quadKey, int* tileX, int* tileY, int* levelOfDetail);
        };
//-----------------------------------------------------------------------------//
//                         End of VirtualEarthTileSystem API                   //
//-----------------------------------------------------------------------------//

    // Disabled methods
    HRFVirtualEarthEditor(const HRFVirtualEarthEditor& pi_rObj);
    HRFVirtualEarthEditor& operator=(const HRFVirtualEarthEditor& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
