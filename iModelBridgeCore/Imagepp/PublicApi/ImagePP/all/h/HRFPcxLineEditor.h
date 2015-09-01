//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPcxLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
class HRFPcxFile;
//:End Ignore

/**
    This class provides the interface to read and write raster data in
    the correct format for PCX file. For the reading operation, it reads
    the raster data in the file, decompresses it from RLE and parses it
    into a recognized format for the I++ interface. For the writing
    operation, it does the exact inverse. It parses the raster data to
    convert it into the format of PCX and compressed it to RLE. The data
    are written into lines.
*/

class HRFPcxLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFPcxFile;

    virtual ~HRFPcxLineEditor  ();

    //:> Edition by Block
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



    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                   pi_PosBlockX,
                               uint64_t                   pi_PosBlockY,
                               const HFCPtr<HCDPacket>&   pi_rpPacket,
                               HFCLockMonitor const*      pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }

protected:
    virtual void OnSynchronizedSharingControl();

    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    //:> and to the capabilities

    //:> Constructor
    HRFPcxLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     unsigned short       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);
private:
    //:> Methods Disabled
    HRFPcxLineEditor(const HRFPcxLineEditor& pi_rObj);
    HRFPcxLineEditor& operator=(const HRFPcxLineEditor& pi_rObj);

    //:> Other method
    HSTATUS  ReadAndDecompressPcxLine  (Byte* pi_pConvert, HFCLockMonitor const* pi_pSisterFileLock);
    bool ConvertDataToPcxFormat    (Byte* pi_pConvert, Byte* po_pData);


    //:> Attributes
    uint32_t         m_ConvertSize;
    uint32_t        m_FileBufferSize;
    uint32_t        m_FileBufferIndex;
    uint32_t        m_FilePos;
    uint32_t        m_CurrentLineNumber;
    HArrayAutoPtr<Byte> m_pFileBuffer;

    HArrayAutoPtr<Byte> m_pConvert;
    HArrayAutoPtr<Byte> m_pEncoded;
    };
END_IMAGEPP_NAMESPACE

