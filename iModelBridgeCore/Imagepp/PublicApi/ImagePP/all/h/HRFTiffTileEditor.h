//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTiffTileEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

class HRFTiffFile;
class HRFGeoTiffFile;
class HRFiTiffFile;
class HRFHMRFile;

class HRFTiffTileEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFTiffFile;
    friend class HRFGeoTiffFile;
    friend class HRFiTiffFile;
    friend class HRFHMRFile;

    virtual                         ~HRFTiffTileEditor  ();

    // Edition by Block
    virtual HSTATUS                 ReadBlock (uint32_t                 pi_PosBlockX,
                                               uint32_t                 pi_PosBlockY,
                                               Byte*                   po_pData,
                                               HFCLockMonitor const*    pi_pSisterFileLock = 0);

    virtual HSTATUS                 ReadBlock (uint32_t                 pi_PosBlockX,
                                               uint32_t                 pi_PosBlockY,
                                               HFCPtr<HCDPacket>&       po_rpPacket,
                                               HFCLockMonitor const*    pi_pSisterFileLock = 0);

    virtual HSTATUS                 WriteBlock(uint32_t                 pi_PosBlockX,
                                               uint32_t                 pi_PosBlockY,
                                               const Byte*             pi_pData,
                                               HFCLockMonitor const*    pi_pSisterFileLock = 0);

    virtual HSTATUS                 WriteBlock(uint32_t                 pi_PosBlockX,
                                               uint32_t                 pi_PosBlockY,
                                               const HFCPtr<HCDPacket>& pi_rpPacket,
                                               HFCLockMonitor const*    pi_pSisterFileLock = 0);

protected:

    virtual void OnSynchronizedSharingControl();

    uint32_t        m_IndexOfPage;
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFTiffTileEditor
    (HFCPtr<HRFRasterFile> pi_rpRasterFile,
     uint32_t              pi_Page,
     unsigned short       pi_Resolution,
     HFCAccessMode         pi_AccessMode);

private:

    // Pointer to runs Compression/Decompression RLE1
    unsigned short*        m_pRuns;
    size_t          m_NumberOfRuns;

    // Methods Disabled
    HRFTiffTileEditor(const HRFTiffTileEditor& pi_rObj);
    HRFTiffTileEditor& operator=(const HRFTiffTileEditor& pi_rObj);
    };

