//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFDoqEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFDoqFile;

class HRFDoqEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFDoqFile;

    virtual ~HRFDoqEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              Byte*                    po_pData,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
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
    HRFDoqEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                 uint32_t              pi_Page,
                 unsigned short       pi_Resolution,
                 HFCAccessMode         pi_AccessMode);

    HSTATUS                         Read24BitRgbBlock ( uint64_t                 pi_PosBlockX,
                                                        uint64_t                 pi_PosBlockY,
                                                        Byte*                    po_pData,
                                                        HFCLockMonitor const*    pi_pSisterFileLock);




private:
    // Methods Disabled
    HRFDoqEditor(const HRFDoqEditor& pi_rObj);
    HRFDoqEditor& operator=(const HRFDoqEditor& pi_rObj);

    HFCPtr<HRFDoqFile>      m_pRasterFile;

    uint64_t m_Offset;
    uint32_t m_BandNumber;
    HAutoPtr<Byte>  m_pRedLineBuffer;
    HAutoPtr<Byte>  m_pGreenLineBuffer;
    HAutoPtr<Byte>  m_pBlueLineBuffer;
    uint32_t m_LineWidth;
    bool   m_isMsByteFirst;
    uint32_t m_nbBitsPerBandPerPixel;
    uint32_t m_nNbChannel;
    double m_RedBandScaling;
    double m_GreenBandScaling;
    double m_BlueBandScaling;

    };

END_IMAGEPP_NAMESPACE
