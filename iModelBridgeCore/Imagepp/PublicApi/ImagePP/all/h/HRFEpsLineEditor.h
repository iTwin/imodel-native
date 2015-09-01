//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFEpsLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"


BEGIN_IMAGEPP_NAMESPACE
class HRFEpsFile;


/** -----------------------------------------------------------------------------
    Line editor for the Encapsulated Postscript (EPS) file format. Lines must be
    written sequentially. ReadBlock method implemented only so that read
    requests don't loop indefinitely, reading is not supported.
    -----------------------------------------------------------------------------
*/
class HRFEpsLineEditor : public HRFResolutionEditor
    {
    friend class HRFEpsFile;

public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    virtual         ~HRFEpsLineEditor  ();

    //:> Edition by Block

    virtual HSTATUS ReadBlock(uint64_t       pi_PosBlockX,
                              uint64_t       pi_PosBlockY,
                              Byte*          po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


protected:

    // Constructor
    HRFEpsLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     unsigned short       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);
private:
    //:> Methods Disabled
    HRFEpsLineEditor(const HRFEpsLineEditor& pi_rObj);
    HRFEpsLineEditor& operator=(const HRFEpsLineEditor& pi_rObj);


    // Current output line. Must write sequentially
    uint32_t        m_CurrentLine;

    // Number of bytes for one line of input data
    uint32_t        m_InputBytesPerLine;

    // Number of bytes to write for one input line
    uint32_t        m_OutputBytesPerLine;

    // One output line
    HAutoPtr<char>  m_pLineBuffer;

    // Current position in line of output file
    unsigned short m_CurrentOutputPosition;

    HFCPtr<HRFEpsFile>
    m_pRasterFile;
    };
END_IMAGEPP_NAMESPACE

