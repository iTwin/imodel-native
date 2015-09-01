//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFResolutionEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFResolutionEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCAccessMode.h"

#include "HRFRasterFile.h"
#include "HRFResolutionDescriptor.h"
#include "HRPPixelType.h"       // Used in .hpp

BEGIN_IMAGEPP_NAMESPACE
class  HCDPacket;
class  HCDPacketRLE;
class  HRPPixelPalette;
class  HRFRasterFileCapabilities;
class  HCDCodecHMRRLE1;

#define RESOLUTION_EDITOR_NOT_64_BITS_READY \
    HPRECONDITION(pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);  \
    HPRECONDITION(GetResolutionDescriptor()->GetWidth() <= ULONG_MAX && GetResolutionDescriptor()->GetHeight() <= ULONG_MAX);


class HRFResolutionEditor
    {

public:
    // friend class HRFRasterFile;
    friend class HRFRasterFile;

    // Destruction
    IMAGEPP_EXPORT virtual                         ~HRFResolutionEditor  ();


    // Get the raster file
    HFCPtr<HRFRasterFile>           GetRasterFile ();

    // Palette information
    IMAGEPP_EXPORT virtual void      SetPalette    (const HRPPixelPalette& pi_rPalette);
    const HRPPixelPalette&          GetPalette    () const;

    const HFCPtr<HRFResolutionDescriptor>&
    GetResolutionDescriptor
    () const;
    const HFCPtr<HRFRasterFileCapabilities>&
    GetResolutionCapabilities
    () const;

    // Access mode
    IMAGEPP_EXPORT virtual HFCAccessMode    GetAccessMode () const;

    // Edition by Block
    IMAGEPP_EXPORT virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                                            uint64_t                pi_PosBlockY,
                                            Byte*                   po_pData,
                                            HFCLockMonitor const*   pi_pSisterFileLock = 0);

    IMAGEPP_EXPORT virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                                            uint64_t                pi_PosBlockY,
                                            HFCPtr<HCDPacket>&      po_rpPacket,
                                            HFCLockMonitor const*   pi_pSisterFileLock = 0);

    // Binary raster optimization.
    // Use for 1 bit processing. Your implementation can avoid the decompression/compression(RLE) overhead
    // by decompressing directly to RLE. The Packet width and height should be at least as big as a block size.
    // Default implementation will do a standard read block and then compress in RLE.
    IMAGEPP_EXPORT virtual HSTATUS ReadBlockRLE(uint64_t                pi_PosBlockX,
                                               uint64_t                pi_PosBlockY,
                                               HFCPtr<HCDPacketRLE>&   po_rpPacketRLE,
                                               HFCLockMonitor const*   pi_pSisterFileLock = 0);

    IMAGEPP_EXPORT virtual HSTATUS WriteBlockRLE(uint64_t                pi_PosBlockX,
                                                uint64_t                pi_PosBlockY,
                                                HFCPtr<HCDPacketRLE>&   pi_rpPacketRLE,
                                                HFCLockMonitor const*   pi_pSisterFileLock = 0);

    IMAGEPP_EXPORT virtual HSTATUS WriteBlock(uint64_t                pi_PosBlockX,
                                             uint64_t                pi_PosBlockY,
                                             const Byte*             pi_pData,
                                             HFCLockMonitor const*   pi_pSisterFileLock = 0);

    IMAGEPP_EXPORT virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                                             uint64_t                 pi_PosBlockY,
                                             const HFCPtr<HCDPacket>& pi_rpPacket,
                                             HFCLockMonitor const*    pi_pSisterFileLock = 0);

    // Your implementation can override OnSynchronizedSharingControl() to execute its specific
    // lock and file synchronization operations.
    // Only file editor and HRFBlockAdapters(via m_pTheTrueOriginalFile) should instantiate lock and
    // pass it as a parameter to the Read/Write operation.
    void                            AssignRasterFileLock(const HFCPtr<HRFRasterFile>& pi_rpRasterFile, HFCLockMonitor& pio_rSisterFileLock, bool pi_SynchronizedSharingControl);

    virtual void                    NoMoreRead();

    unsigned short                 GetResolutionIndex() const;
    uint32_t                        GetPage() const;

    // Used by HRSObjectStore to synchronize TileDataFlag before the save.
    virtual void                    SaveDataFlag() { };

    // notify method
    IMAGEPP_EXPORT virtual void             ResolutionSizeHasChanged() const;

protected:

    // Default implementation does nothing.
    virtual void OnSynchronizedSharingControl() {};

    // Pointer to the raster file to edit, to the resolution descriptor
    // and to the capabilities
    HFCPtr<HRFRasterFile>               m_pRasterFile;
    HFCPtr<HRFResolutionDescriptor>     m_pResolutionDescriptor;
    HFCPtr<HRFRasterFileCapabilities>   m_pResolutionCapabilities; // Use the capabilities for test
    HFCAccessMode                       m_AccessMode;
    uint32_t                           m_Page;
    unsigned short                      m_Resolution;
    double                              m_ResolutionFactor;

    // Constructor
    IMAGEPP_EXPORT                          HRFResolutionEditor(HFCPtr<HRFRasterFile>     pi_rpRasterFile,
                                                        uint32_t                  pi_Page,
                                                        unsigned short           pi_Resolution,
                                                        HFCAccessMode             pi_AccessMode);

    IMAGEPP_EXPORT                          HRFResolutionEditor(HFCPtr<HRFRasterFile>     pi_rpRasterFile,
                                                        uint32_t                  pi_Page,
                                                        double                   pi_ResolutionFactor,
                                                        HFCAccessMode             pi_AccessMode);
private:
    // Methods Disabled
    HRFResolutionEditor(const HRFResolutionEditor& pi_rObj);
    HRFResolutionEditor& operator=(const HRFResolutionEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

#include "HRFResolutionEditor.hpp"




