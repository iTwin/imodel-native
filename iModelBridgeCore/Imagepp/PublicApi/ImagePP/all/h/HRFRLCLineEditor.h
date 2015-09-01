//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRLCLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFRLCLineEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

#include "HFCBinStream.h"
#include "HTiffUtils.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFRLCFile;

class HRFRLCLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFRLCFile;

    virtual        ~HRFRLCLineEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t     pi_PosBlockX,
                              uint64_t     pi_PosBlockY,
                              Byte*        po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock) override;


    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


    virtual HSTATUS WriteBlock(uint64_t     pi_PosBlockX,
                               uint64_t     pi_PosBlocY,
                               const Byte*  pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock) override;

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
    HRFRLCLineEditor(HFCPtr<HRFRasterFile>  pi_rpRasterFile,
                     uint32_t        pi_Page,
                     unsigned short pi_Resolution,
                     HFCAccessMode   pi_AccessMode);
private:
    HFCBinStream*       m_pRLCFile;

    uint32_t            m_Width;
    uint32_t            m_BytesPerRow;
    uint32_t            m_CurrentLine;
    HTIFFByteOrdering   m_ByteOrdering;
    HArrayAutoPtr<unsigned short>
    m_pBuffer;

    // Methods Disabled
    HRFRLCLineEditor(const HRFRLCLineEditor& pi_rObj);
    HRFRLCLineEditor& operator=(const HRFRLCLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE




