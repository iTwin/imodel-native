//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFVirtualEarthEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------
#pragma once

#include "HFCSemaphore.h"
#include "HFCThread.h"
#include "HGFTileIDDescriptor.h"
#include "HRFResolutionEditor.h"
#include "HRFTilePool.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFVirtualEarthConnection;
class HRFVirtualEarthFile;
class HRFVirtualEarthTileReaderThread;
struct HRFVirtualEarthTileRequest;

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
                              Byte*   po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const Byte* pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    int GetLevelOfDetail() const;

protected:

    //Constructor
    HRFVirtualEarthEditor
    (HFCPtr<HRFRasterFile> pi_rpRasterFile,
     uint32_t              pi_Page,
     unsigned short       pi_Resolution,
     HFCAccessMode         pi_AccessMode);

    //Request look ahead
        virtual void                    RequestLookAhead(const HGFTileIDList& pi_rTileIDList);       
        virtual void                    InitTileRequest(uint64_t TileId, HRFVirtualEarthTileRequest& Request);

private:

    friend class HRFVirtualEarthTileReaderThread;

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

//-----------------------------------------------------------------------------
// HRFVirtualEarthTileReaderThread class
//-----------------------------------------------------------------------------
class HRFVirtualEarthTileReaderThread : public HFCThread
    {
public:
    HRFVirtualEarthTileReaderThread(const string&        pi_rThreadName,
                                    HRFVirtualEarthFile* pi_pVirtualEarthFile);
    virtual ~HRFVirtualEarthTileReaderThread();

    virtual void Go();

    void         ReadBlockFromServer(HRFVirtualEarthTileRequest const& Request);

private:

    //Thread control
    HFCEvent             m_ThreadStarted;
    HRFVirtualEarthFile* m_pVirtualEarthFile;
    };
END_IMAGEPP_NAMESPACE
