//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTgaCompressLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Class HRFTgaCompressLineEditor
//---------------------------------------------------------------------------------------


#pragma once

#include "HFCPtr.h"
#include "HRFResolutionEditor.h"
#include "HCDCodecTgaRLE.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
class HRFTgaFile;
//:End Ignore

/**----------------------------------------------------------------------------
 This class provides the interface to read and write raster data in a
 compressed tga file. It uses the HCDCodecTGARLE codec to decompress the raster
 data from RLE to raw.

  @see HCDCodecTGARLE
-----------------------------------------------------------------------------*/

class HRFTgaCompressLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFTgaFile;

    virtual ~HRFTgaCompressLineEditor  ();

    //:> Edition by block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override;
protected:

    virtual void OnSynchronizedSharingControl();

    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    //:> Constructor
    HRFTgaCompressLineEditor(HFCPtr<HRFTgaFile> pi_rpRasterFile,
                             uint32_t              pi_Page,
                             unsigned short       pi_Resolution,
                             HFCAccessMode         pi_AccessMode);

private:

    //:> Attributes
    HFCPtr<HCDCodecTGARLE>  m_pCodec;

    HArrayAutoPtr<uint32_t>   m_pLineOffsetTbl;

    //:> Methods
    bool                   IsLineOffsetTableFull();
    bool                   GetLineOffsetTableFromFile();


    //:> Methods Disabled
    HRFTgaCompressLineEditor(const HRFTgaCompressLineEditor& pi_rObj);
    HRFTgaCompressLineEditor& operator=(const HRFTgaCompressLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

