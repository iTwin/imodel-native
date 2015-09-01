//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFBmpCompressLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFBmpCompressLineEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HCDCodecBMPRLE8.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFBmpFile;

class HRFBmpCompressLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFBmpFile;

    virtual ~HRFBmpCompressLineEditor  ();

    // Edition by block
    virtual HSTATUS        ReadBlock     (uint64_t            pi_PosBlockX,
                                          uint64_t            pi_PosBlockY,
                                          HFCPtr<HCDPacket>&  po_rpPacket) override;

    virtual HSTATUS        WriteBlock    (uint64_t        pi_PosBlockX,
                                          uint64_t        pi_PosBlockY,
                                          const Byte*     pi_pData) override;

    virtual HSTATUS        WriteBlock    (uint64_t                    pi_PosBlockX,
                                          uint64_t                    pi_PosBlockY,
                                          const HFCPtr<HCDPacket>&    pi_rpPacket) override;

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFBmpCompressLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                             uint32_t              pi_Page,
                             unsigned short       pi_Resolution,
                             HFCAccessMode         pi_AccessMode);
private:

    HFCPtr<HRFBmpFile>         m_pRasterFile;

    // Number of line read
    uint32_t m_NumberOfLineRead;

    // Codec
    HFCPtr<HCDCodecBMPRLE8>    m_pCodec;

    // Buffer that will contains compress data
    Byte* m_pCompressBuffer;

    // Maximum valid offset in the buffer
    uint32_t m_MaxOffsetInBuffer;

    // Maximum buffer size
    uint32_t m_BufferSize;

    // Minimum size to add to the buffer
    uint32_t m_MininumSizeToAdd;

    // Position of the file ptr for that editor
    uint32_t m_PosInFile;

    // Array of lines offset.
    uint32_t* m_pLinesOffsetBuffer;

    // Methods Disabled
    HRFBmpCompressLineEditor(const HRFBmpCompressLineEditor& pi_rObj);
    HRFBmpCompressLineEditor& operator=(const HRFBmpCompressLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

