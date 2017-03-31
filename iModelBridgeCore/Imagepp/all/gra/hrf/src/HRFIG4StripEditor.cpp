//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIG4StripEditor.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIG4StripEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFIG4StripEditor.h>
#include <ImagePP/all/h/HRFIG4File.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDPacketRLE.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------

HRFIG4StripEditor::HRFIG4StripEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     uint16_t       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode),
      m_CompressPacket()
    {
    HPRECONDITION(m_pResolutionDescriptor != 0);
    HPRECONDITION(m_pResolutionDescriptor->GetWidth() <= UINT32_MAX);
    HPRECONDITION(m_pResolutionDescriptor->GetHeight() <= UINT32_MAX);

    m_pIG4File = const_cast<HFCBinStream*>(((HFCPtr<HRFIG4File >&)GetRasterFile())->GetIG4FilePtr());
    m_pCodec    = ((HFCPtr<HRFIG4File >&)GetRasterFile())->GetIG4CodecPtr();

    // Reset the Codec because the user may not read the raster completely and
    // ask again the line zero.
    m_pCodec->Reset();

    m_BitPerPixel = 1;
    m_StripHeight = ((HFCPtr<HRFIG4File >&)GetRasterFile())->GetStripHeight();

    m_UncompressedBufferSize = (uint32_t)ceil((float)(m_pResolutionDescriptor->GetWidth()) *
                                            ((float)m_BitPerPixel / 8.0)) *
                               m_StripHeight;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------

HRFIG4StripEditor::~HRFIG4StripEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by block
//-----------------------------------------------------------------------------

HSTATUS HRFIG4StripEditor::ReadBlock(uint64_t pi_PosBlockX,
                                     uint64_t pi_PosBlockY,
                                     Byte*    po_pData)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_pCodec != 0);


    HSTATUS Status = H_ERROR;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // Ensure that the Codec didn't keep any previous state.
    m_pCodec->Reset();

    // After a Reset the Codec subset need to be set again because by default it use
    // the whole raster area.
    if (pi_PosBlockY + m_StripHeight <= m_pResolutionDescriptor->GetHeight())
        {
        m_pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(), m_StripHeight);
        }
    else
        {
        m_pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(),
                            (uint32_t)m_pResolutionDescriptor->GetHeight() - pi_PosBlockY);
        }

        {
        uint32_t StripOffset;
        uint32_t StripSizeInBytes;

        ((HFCPtr<HRFIG4File >&)GetRasterFile())->GetStripInfo(pi_PosBlockY,
                                                              StripOffset,
                                                              StripSizeInBytes);

        // this is the first line we move to the begin of image data
        m_pIG4File->SeekToPos(StripOffset);

        // Read the entire compressed pixels in memory
        HAutoPtr<Byte> pCompressedData(new Byte[StripSizeInBytes]);

        if(m_pIG4File->Read(pCompressedData, StripSizeInBytes) != StripSizeInBytes)
            goto WRAPUP;

        m_CompressPacket.SetBuffer(pCompressedData, StripSizeInBytes);
        m_CompressPacket.SetBufferOwnership(false);
        m_CompressPacket.SetDataSize(StripSizeInBytes);
        m_CompressPacket.SetCodec((HFCPtr<class HCDCodec>)(m_pCodec));

        // We decompress the specified line from the image buffer
        HCDPacket uncompress(po_pData, m_UncompressedBufferSize, ((HFCPtr<HRFIG4File >&)GetRasterFile())->GetStripHeight());

        HCDPacket compressSubset((HFCPtr<class HCDCodec>)(m_pCodec),
                                 m_CompressPacket.GetBufferAddress() + m_pCodec->GetCompressedImageIndex(),
                                 StripSizeInBytes - m_pCodec->GetCompressedImageIndex(),
                                 StripSizeInBytes - m_pCodec->GetCompressedImageIndex());

        compressSubset.Decompress(&uncompress);
        }

    Status = H_SUCCESS;

WRAPUP:

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFIG4StripEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                      uint64_t     pi_PosBlockY,
                                      const Byte*  pi_pData)
    {
    HASSERT(0); // not supported

    return H_ERROR;
    }
