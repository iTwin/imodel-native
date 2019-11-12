//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HGFTileIDDescriptor.h>
#include <ImagePP/all/h/HRFResolutionEditor.h>

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
    :m_tileId(tileId), m_tileUri(tileUri), m_rasterFile(rasterFile), m_isNoTile(false)
        {}

    virtual ~VirtualEarthTileQuery(){};
    
    virtual void _Run() override;

    uint64_t                    m_tileId;
    Utf8String                  m_tileUri;
    bvector<Byte>               m_tileData;
    bool                        m_isNoTile;    // Bing returns a kodak/camera when tile data is not available at the requested resolution.
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

    HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const Byte* pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override;

    int32_t GetLevelOfDetail() const;

    Utf8String BuildTileUri(uint64_t tileId);

protected:

    //Constructor
    HRFVirtualEarthEditor    (HFCPtr<HRFRasterFile> pi_rpRasterFile, uint32_t pi_Page, uint16_t pi_Resolution, HFCAccessMode pi_AccessMode);

    //Request look ahead
    virtual void              RequestLookAhead(const HGFTileIDList& pi_rTileIDList, uint32_t pi_ConsumerID);

private:
    enum class ReadTileStatus
        {
        Success,
        Error,
        NoTile,
        };

    ReadTileStatus QueryTile(Byte* po_pData, uint64_t pi_PosBlockX, uint64_t pi_PosBlockY);

    bool QueryTileSubstitute(Byte* pOutput, uint64_t blockPosX, uint64_t blockPosY);
    void MagnifyTile(Byte* pOutput, Byte const* pSource, size_t srcOffsetX, size_t srcOffsetY, uint32_t srcResolution);


    HGFTileIDDescriptor     m_TileIDDescriptor;
    std::unique_ptr<Byte[]> m_pSubTileData;

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
        static uint32_t MapSize(int32_t levelOfDetail);
        static double  GroundResolution(double latitude, int32_t levelOfDetail);
        static double  MapScale(double latitude, int32_t levelOfDetail, int32_t screenDpi);
        static void    LatLongToPixelXY(double latitude, double longitude, int32_t levelOfDetail, int32_t* pixelX, int32_t* pixelY);
        static void    PixelXYToLatLong(int32_t pixelX, int32_t pixelY, int32_t levelOfDetail, double* latitude, double* longitude);
        static void    PixelXYToTileXY(int32_t pixelX, int32_t pixelY, int32_t* tileX, int32_t* tileY);
        static void    TileXYToPixelXY(int32_t tileX, int32_t tileY, int32_t* pixelX, int32_t* pixelY);
        static string  TileXYToQuadKey(int32_t tileX, int32_t tileY, int32_t levelOfDetail);
        static string  PixelXYToQuadKey(int32_t pi_PixelX, int32_t pi_PixelY, int32_t pi_LevelOfDetail);
        static void    QuadKeyToTileXY(string quadKey, int32_t* tileX, int32_t* tileY, int32_t* levelOfDetail);
        };
//-----------------------------------------------------------------------------//
//                         End of VirtualEarthTileSystem API                   //
//-----------------------------------------------------------------------------//

    // Disabled methods
    HRFVirtualEarthEditor(const HRFVirtualEarthEditor& pi_rObj);
    HRFVirtualEarthEditor& operator=(const HRFVirtualEarthEditor& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
