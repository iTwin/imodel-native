//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
                              Byte*          po_pData) override;

    HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData) override;

    HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }


protected:

    // Constructor
    HRFEpsLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     uint16_t       pi_Resolution,
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
    uint16_t m_CurrentOutputPosition;

    HFCPtr<HRFEpsFile>
    m_pRasterFile;
    };
END_IMAGEPP_NAMESPACE

