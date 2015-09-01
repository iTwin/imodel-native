//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFFliCompressLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFFliFile;

class HRFFliCompressLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFFliFile;

    virtual ~HRFFliCompressLineEditor  ();

    // Edition by block

    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t               pi_PosBlockX,
                              uint64_t               pi_PosBlockY,
                              HFCPtr<HCDPacket>&     po_rpPacket,
                              HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t               pi_PosBlockX,
                              uint64_t               pi_PosBlockY,
                              Byte*                  po_pData,
                              HFCLockMonitor const*  pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_pData,pi_pSisterFileLock);
        }


    virtual HSTATUS WriteBlock(uint64_t                  pi_PosBlockX,
                               uint64_t                  pi_PosBlockY,
                               const HFCPtr<HCDPacket>&  pi_rpPacket,
                               HFCLockMonitor const*     pi_pSisterFileLock = 0) override;

protected:

    virtual void OnSynchronizedSharingControl();

    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFFliCompressLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                             uint32_t              pi_Page,
                             unsigned short       pi_Resolution,
                             HFCAccessMode         pi_AccessMode);
private:
    // Members
    HFCPtr<HRFFliFile>      m_pRasterFile;

    // Codec
    HFCPtr<HCDCodecImage>    m_pCodec;

    // Position of the file ptr for that editor
    uint32_t m_PosInFile;


    // Methods Disabled
    HRFFliCompressLineEditor(const HRFFliCompressLineEditor& pi_rObj);
    HRFFliCompressLineEditor& operator=(const HRFFliCompressLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
