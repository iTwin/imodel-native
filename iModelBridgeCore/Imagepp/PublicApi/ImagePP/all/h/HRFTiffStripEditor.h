//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTiffStripEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFTiffFile;
class HRFiTiffFile;
class HRFGeoTiffFile;

class HRFTiffStripEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFTiffFile;
    friend class HRFiTiffFile;
    friend class HRFGeoTiffFile;

    virtual                         ~HRFTiffStripEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              Byte*   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlockRLE(uint64_t pi_PosBlockX,
                                 uint64_t pi_PosBlockY,
                                 HFCPtr<HCDPacketRLE>&    po_rpPacketRLE,
                                 HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const Byte*            pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlockRLE(uint64_t pi_PosBlockX,
                                  uint64_t pi_PosBlockY,
                                  HFCPtr<HCDPacketRLE>& pi_rpPacketRLE,
                                  HFCLockMonitor const* pi_pSisterFileLock = 0) override;

protected:

    virtual void OnSynchronizedSharingControl();

    uint32_t        m_IndexOfPage;
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFTiffStripEditor
    (HFCPtr<HRFRasterFile> pi_rpRasterFile,
     uint32_t              pi_Page,
     unsigned short       pi_Resolution,
     HFCAccessMode         pi_AccessMode);

private:

    // Pointer to runs Compression/Decompression RLE1
    unsigned short*        m_pRuns;
    size_t          m_NumberOfRuns;
    bool           m_HasDecompressToRLE;

    // Methods Disabled
    HRFTiffStripEditor(const HRFTiffStripEditor& pi_rObj);
    HRFTiffStripEditor& operator=(const HRFTiffStripEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
