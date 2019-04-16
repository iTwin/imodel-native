//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFGifImageEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRFGifImageEditor.h>
#include <ImagePP/all/h/HRFGifLineEditor.h>
#include <ImagePP/all/h/HRFGifFile.h>


//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFGifImageEditor::HRFGifImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     uint16_t       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile = static_cast<HRFGifFile*>(GetRasterFile().GetPtr());

    m_pLineEditor = new HRFGifLineEditor(pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode);

    m_Width  = (uint32_t)m_pResolutionDescriptor->GetWidth();
    m_Height = (uint32_t)m_pResolutionDescriptor->GetHeight();

    // Obtain the number of bytes per line.
    m_ExactBytesPerImageWidth = m_pLineEditor->GetResolutionDescriptor()->GetBlockWidth();
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFGifImageEditor::~HRFGifImageEditor()
    {
    }


//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGifImageEditor::ReadBlock(uint64_t pi_PosBlockX,
                                     uint64_t pi_PosBlockY,
                                     Byte*   po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status       = H_SUCCESS;
    uint32_t ImageHeight  = (uint32_t)m_pResolutionDescriptor->GetHeight();

    // Temporary need for virtual ptr.
    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        if (m_pRasterFile->m_ListGifGraphicBlock[m_Page].ImageDescriptor.PackedField & 0x40)
            {
            uint32_t NoLine = 0;
            // Pass 1 : Every 8th. row, starting with row 0.
            for(uint32_t pass1 = 0; pass1<m_Height && (Status == H_SUCCESS); pass1 += 8)
                {
                Status = m_pLineEditor->ReadBlock(0, NoLine, po_pData+(pass1*m_ExactBytesPerImageWidth));
                NoLine++;
                }

            // Pass 2: Every 8th. row, starting with row 4.
            for(uint32_t pass2 = 4; pass2<m_Height && (Status == H_SUCCESS); pass2 += 8)
                {
                Status = m_pLineEditor->ReadBlock(0, NoLine, po_pData+(pass2*m_ExactBytesPerImageWidth));
                NoLine++;
                }

            // Pass 3: Every 4th. row, starting with row 2.
            for(uint32_t pass3 = 2; pass3<m_Height && (Status == H_SUCCESS); pass3 += 4)
                {
                Status = m_pLineEditor->ReadBlock(0, NoLine, po_pData+(pass3*m_ExactBytesPerImageWidth));
                NoLine++;
                }

            // Pass 4: Every 2nd. row, starting with row 1.
            for(uint32_t pass4 = 1; pass4<m_Height && (Status == H_SUCCESS); pass4 += 2)
                {
                Status = m_pLineEditor->ReadBlock(0, NoLine, po_pData+(pass4*m_ExactBytesPerImageWidth));
                NoLine++;
                }
            }
        else
            {
            // Read these line from file and copy their data to the output buffer
            for (uint32_t NoLine=0; NoLine < ImageHeight && (Status == H_SUCCESS); NoLine++)
                Status = m_pLineEditor->ReadBlock(0, NoLine, po_pData+(NoLine * m_ExactBytesPerImageWidth));
            }
        }
    else
        Status = H_NOT_FOUND;

    return Status;
    }


//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGifImageEditor::WriteBlock(uint64_t        pi_PosBlockX,
                                      uint64_t        pi_PosBlockY,
                                      const Byte*     pi_pData)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);

    HSTATUS Status      = H_SUCCESS;
    uint32_t ImageHeight = (uint32_t)m_pResolutionDescriptor->GetHeight();

    // Image is interlace.
    if (m_pRasterFile->m_ListGifGraphicBlock[m_Page].ImageDescriptor.PackedField & 0x40)
        {
        uint32_t NoLine = 0;
        // Pass 1 : Every 8th. row, starting with row 0.
        for(uint32_t pass1 = 0; pass1<m_Height && (Status == H_SUCCESS); pass1 += 8)
            {
            Status = m_pLineEditor->WriteBlock(0, NoLine, pi_pData+(pass1*m_ExactBytesPerImageWidth));
            NoLine++;
            }

        // Pass 2: Every 8th. row, starting with row 4.
        for(uint32_t pass2 = 4; pass2<m_Height && (Status == H_SUCCESS); pass2 += 8)
            {
            Status = m_pLineEditor->WriteBlock(0, NoLine, pi_pData+(pass2*m_ExactBytesPerImageWidth));
            NoLine++;
            }

        // Pass 3: Every 4th. row, starting with row 2.
        for(uint32_t pass3 = 2; pass3<m_Height && (Status == H_SUCCESS); pass3 += 4)
            {
            Status = m_pLineEditor->WriteBlock(0, NoLine, pi_pData+(pass3*m_ExactBytesPerImageWidth));
            NoLine++;
            }

        // Pass 4: Every 2nd. row, starting with row 1.
        for(uint32_t pass4 = 1; pass4<m_Height && (Status == H_SUCCESS); pass4 += 2)
            {
            Status = m_pLineEditor->WriteBlock(0, NoLine, pi_pData+(pass4*m_ExactBytesPerImageWidth));
            NoLine++;
            }
        }
    else
        {
        // Write these strip from file and copy their data to the output buffer
        for (uint32_t NoLine=0; NoLine < ImageHeight && (Status == H_SUCCESS); NoLine++)
            Status = m_pLineEditor->WriteBlock(0, NoLine, pi_pData+(NoLine * m_ExactBytesPerImageWidth));
        }

    return Status;
    }
