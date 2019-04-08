//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRLCLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFRLCLineEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFRLCLineEditor.h>
#include <ImagePP/all/h/HRFRLCFile.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFRLCLineEditor::HRFRLCLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   uint16_t       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {

    HPRECONDITION(!pi_AccessMode.m_HasCreateAccess && !pi_AccessMode.m_HasWriteAccess);

    m_pRLCFile = const_cast<HFCBinStream*>(static_cast<HRFRLCFile*>(GetRasterFile().GetPtr())->GetRLCFilePtr());

    m_Width = (uint32_t)m_pResolutionDescriptor->GetWidth();

    m_pBuffer = new uint16_t[m_Width * 2];
    m_BytesPerRow = (m_Width + 7) / 8;
    m_CurrentLine = 0;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFRLCLineEditor::~HRFRLCLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by block
//
// The file was compressed, but we uncompressed directly the data.
// Each line was compressed in this format
// NbRun (FirstRunBegin,FirstRunEnd), (SecondRunBegin,SecondRunEnd)...
// each run give the with color
//-----------------------------------------------------------------------------
HSTATUS HRFRLCLineEditor::ReadBlock(uint64_t  pi_PosBlockX,
                                    uint64_t  pi_PosBlockY,
                                    Byte*     po_pData)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION(pi_PosBlockX == 0);
    HPRECONDITION(pi_PosBlockY >= 0);
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_CurrentLine <= pi_PosBlockY);


    HSTATUS Status = H_SUCCESS;

    uint16_t NbRuns;
    if (pi_PosBlockY < m_CurrentLine || m_CurrentLine == 0)
        {
        m_pRLCFile->SeekToPos(2 * sizeof(uint16_t)); // skip the image size
        m_CurrentLine = 0;
        }

    // skip the pi_PosBlockY - 1 lines
    while (Status == H_SUCCESS && m_CurrentLine < pi_PosBlockY)
        {
        if (m_pRLCFile->Read(&NbRuns, sizeof(uint16_t)) != sizeof(uint16_t)) // read the nb run for this line
            Status = H_ERROR;
        else
            {
            SwabArrayOfShort(&NbRuns, 1);

            m_pRLCFile->Seek(NbRuns * 2 * sizeof(uint16_t)); // skip runs

            m_CurrentLine++;
            }
        }

    if (Status == H_SUCCESS)
        {
        if (m_pRLCFile->Read(&NbRuns, sizeof(uint16_t)) != sizeof(uint16_t)) // the the nb runs into this line
            Status = H_ERROR;
        else
            {
            SwabArrayOfShort(&NbRuns, 1);

            uint32_t ReadSize = NbRuns * 2 * sizeof(uint16_t);
            if (m_pRLCFile->Read(m_pBuffer, ReadSize) != ReadSize)
                Status = H_ERROR;
            }

        if (Status == H_SUCCESS)
            {
            m_CurrentLine++;

            SwabArrayOfShort(m_pBuffer, NbRuns * 2);

            // clear the buffer in black
            memset(po_pData, 0, m_BytesPerRow);
            uint16_t* pRun = m_pBuffer;
            Byte Mask;
            Byte* pBytePtr;
            uint16_t j;

            for (uint16_t i = 0; i < NbRuns; i++)
                {
                HPRECONDITION(pRun[1] <= m_Width);

                for (j = pRun[0], pBytePtr = po_pData + m_BytesPerRow - 1 - j/8, Mask = 0x01 << (j % 8);
                     j <= pRun[1];
                     j++, Mask <<=1)
                    {
                    if (!Mask)
                        {
                        Mask = 0x01;
                        pBytePtr--;
                        }

                    // At the beginning of a byte
                    if (Mask == 0x01)
                        {
                        uint16_t Count;
                        // Check if we can fill a least one byte completly
                        if ((Count = pRun[1] - j + 1) >= 8)
                            {
                            // Can fill a least one byte
                            uint16_t NbByte = Count / 8;
                            pBytePtr -= NbByte;

                            memset((pBytePtr+1), 0xff, NbByte);
                            j += (NbByte * 8);
                            }
                        // Set next pixel, if not complete.
                        if (j < pRun[1])
                            *pBytePtr |= Mask;
                        }
                    else
                        *pBytePtr |= Mask;
                    }
                pRun += 2;
                }
            }
        }
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by block
//-----------------------------------------------------------------------------
HSTATUS HRFRLCLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlocY,
                                     const Byte*  pi_pData)
    {
    // not support write access
    HASSERT(0);
    return H_ERROR;
    }
