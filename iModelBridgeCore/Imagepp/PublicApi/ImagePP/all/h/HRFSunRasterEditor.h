//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSunRasterEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFSunRasterEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFSunRasterFile;

class HRFSunRasterLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFSunRasterFile;

    virtual ~HRFSunRasterLineEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t    pi_PosBlockX,
                              uint64_t    pi_PosBlockY,
                              Byte*       po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


    virtual HSTATUS WriteBlock(uint64_t       pi_PosBlockX,
                               uint64_t       pi_PosBlockY,
                               const Byte*    pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t      pi_PosBlockX,
                               uint64_t      pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFSunRasterLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                           uint32_t              pi_Page,
                           unsigned short       pi_Resolution,
                           HFCAccessMode         pi_AccessMode);
private:

    HFCPtr<HRFSunRasterFile>    m_pRasterFile;

    HArrayAutoPtr<Byte>       m_pLineBuffer;
    uint32_t                    m_DataOffset;

    // 0 if bits per row not multiple of 8
    uint32_t                    m_ExactBytesPerRow;

    // Methods Disabled
    HRFSunRasterLineEditor(const HRFSunRasterLineEditor& pi_rObj);
    HRFSunRasterLineEditor& operator=(const HRFSunRasterLineEditor& pi_rObj);
    };

class HRFSunRasterImageEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFSunRasterFile;

    virtual ~HRFSunRasterImageEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t    pi_PosBlockX,
                                uint64_t    pi_PosBlockY,
                                Byte*       po_pData,
                                HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                                uint64_t                pi_PosBlockY,
                                HFCPtr<HCDPacket>&      po_rpPacket,
                                HFCLockMonitor const*   pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlock(uint64_t       pi_PosBlockX,
                                 uint64_t       pi_PosBlockY,
                                 const Byte*    pi_pData,
                                 HFCLockMonitor const* pi_pSisterFileLock = 0);

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
    HRFSunRasterImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                            uint32_t              pi_Page,
                            unsigned short       pi_Resolution,
                            HFCAccessMode         pi_AccessMode);
private:

    HFCPtr<HRFSunRasterFile>    m_pRasterFile;


    // Methods Disabled
    HRFSunRasterImageEditor(const HRFSunRasterImageEditor& pi_rObj);
    HRFSunRasterImageEditor& operator=(const HRFSunRasterImageEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


