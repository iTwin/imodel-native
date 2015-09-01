//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFOGCServiceEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HRFWMSFile.h"
#include "HRFOGCService.h"
#include "HFCThread.h"
#include "HRFTilePool.h"

BEGIN_IMAGEPP_NAMESPACE
class HMDVolatileLayers;
class HFCInternetConnection;
class BlockReaderThread;

class HRFOGCServiceEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFWMSFile;
    friend class HRFOGCService;

    virtual         ~HRFOGCServiceEditor  ();


    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }



protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFOGCServiceEditor    (HFCPtr<HRFRasterFile>      pi_rpRasterFile,
                            uint32_t                   pi_Page,
                            double                    pi_Resolution,
                            HFCAccessMode              pi_AccessMode);

    virtual void    RequestLookAhead(       const HGFTileIDList&       pi_rTileIDList);
    virtual void    CancelLookAhead ();

    virtual void    ContextChanged  ();


private:

    friend BlockReaderThread;


    enum ImageType
        {
        JPEG,
        PNG,
        BMP,
        GIF,
        GEOTIFF
        };

    HRFTilePool         m_TilePool;

    typedef struct
        {
        uint64_t m_MinX;
        uint64_t m_MinY;
        uint64_t m_MaxX;
        uint64_t m_MaxY;
        } BlocksExtent;

    HFCExclusiveKey                 m_RequestKey;
    list<BlocksExtent>              m_RequestList;
    HFCSemaphore                    m_RequestEvent;

    // optimization

    HArrayAutoPtr<HAutoPtr<BlockReaderThread> > m_ppBlocksReadersThread;

    HFCPtr<HGF2DTransfoModel>              m_pTransfoModel;
    HFCPtr<HRFOGCServiceConnection>        m_pConnection;
    HGFTileIDDescriptor                    m_TileIDDescriptor;
    ImageType                              m_ImageType;

    // optimization for LookAhead
    uint64_t                       m_MaxTilesPerBlockWidth;
    uint64_t                       m_MaxTilesPerBlockHeight;

    HFCExclusiveKey                 m_ExceptionKey;
    HAutoPtr<HFCException>          m_pException;

    WString                         m_grayPixelType;

    static size_t                   s_UncompressedInvalidTileBitmapSize;
    static size_t                   s_CompressedInvalidTileBitmapSize;
    //if the picture is in gray scale.
    static size_t                   s_GrayUncompressedInvalidTileBitmapSize;
    static size_t                   s_GrayCompressedInvalidTileBitmapSize;
    static size_t                   s_Gray16UncompressedInvalidTileBitmapSize;
    static Byte                    s_CompressedInvalidTileBitmap[];
    static Byte                    s_GrayCompressedInvalidTileBitmap[];
    HFCPtr<HCDPacket>               m_pInvalidTileBitmap;



    // Methods Disabled
    HRFOGCServiceEditor(const HRFOGCServiceEditor& pi_rObj);
    HRFOGCServiceEditor& operator=(const HRFOGCServiceEditor& pi_rObj);
    };




class BlockReaderThread : public HFCThread
    {
public:
    BlockReaderThread(const string&  pi_rThreadName,
                      HRFOGCServiceEditor*  pi_pEditor);
    virtual ~BlockReaderThread();

    virtual void Go();

    void        ReadBlocksFromServer(uint64_t pi_MinX,
                                     uint64_t pi_YMin,
                                     uint64_t pi_XMax,
                                     uint64_t pi_YMax);

private:

    HRFOGCServiceEditor*               m_pEditor;

    HFCPtr<HRFOGCServiceConnection>    m_pConnection;
    HFCPtr<HGF2DTransfoModel>          m_pTransfoModel;

    // for optimization
    uint32_t                    m_BlockWidth;
    uint32_t                    m_BlockHeight;
    uint32_t                    m_BytesPerBlockWidth;
    uint32_t                    m_BlockSizeInBytes;
    HGFTileIDDescriptor         m_TileIDDescriptor;
    unsigned short             m_BytesPerPixel;

    HFCEvent                    m_ThreadStarted;


    bool       ProcessRequest  (HFCPtr<HRFOGCServiceConnection>&  pi_rpConnection,
                                 const string&                     pi_rRequest,
                                 HFCPtr<HFCBuffer>&                po_rpBuffer) const;

    void        InvalidateTiles (uint64_t                  pi_MinX,
                                 uint64_t                  pi_MinY,
                                 uint64_t                  pi_MaxX,
                                 uint64_t                  pi_MaxY);


    bool       UncompressBuffer(HFCPtr<HFCBuffer>&         pi_rpBuffer,
                                 uint32_t                   pi_Width,
                                 uint32_t                   pi_Height,
                                 Byte*                     po_pUncompressedData,
                                 uint32_t                   pi_UncompressedDataSize) const;


    };
END_IMAGEPP_NAMESPACE
