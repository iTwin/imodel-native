//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRLCLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFRLCLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFRLCLineEditor.h>
#include <Imagepp/all/h/HRFRLCFile.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFRLCLineEditor::HRFRLCLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {

    HPRECONDITION(!pi_AccessMode.m_HasCreateAccess && !pi_AccessMode.m_HasWriteAccess);

    m_pRLCFile = const_cast<HFCBinStream*>(static_cast<HRFRLCFile*>(GetRasterFile().GetPtr())->GetRLCFilePtr());

    m_Width = (uint32_t)m_pResolutionDescriptor->GetWidth();

    m_pBuffer = new unsigned short[m_Width * 2];
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
                                    Byte*     po_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION(pi_PosBlockX == 0);
    HPRECONDITION(pi_PosBlockY >= 0);
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_CurrentLine <= pi_PosBlockY);


    HSTATUS Status = H_SUCCESS;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    unsigned short NbRuns;
    if (pi_PosBlockY < m_CurrentLine || m_CurrentLine == 0)
        {
        m_pRLCFile->SeekToPos(2 * sizeof(unsigned short)); // skip the image size
        m_CurrentLine = 0;
        }

    // skip the pi_PosBlockY - 1 lines
    while (Status == H_SUCCESS && m_CurrentLine < pi_PosBlockY)
        {
        if (m_pRLCFile->Read(&NbRuns, sizeof(unsigned short)) != sizeof(unsigned short)) // read the nb run for this line
            Status = H_ERROR;
        else
            {
            SwabArrayOfShort(&NbRuns, 1);

            m_pRLCFile->Seek(NbRuns * 2 * sizeof(unsigned short)); // skip runs

            m_CurrentLine++;
            }
        }

    if (Status == H_SUCCESS)
        {
        if (m_pRLCFile->Read(&NbRuns, sizeof(unsigned short)) != sizeof(unsigned short)) // the the nb runs into this line
            Status = H_ERROR;
        else
            {
            SwabArrayOfShort(&NbRuns, 1);

            uint32_t ReadSize = NbRuns * 2 * sizeof(unsigned short);
            if (m_pRLCFile->Read(m_pBuffer, ReadSize) != ReadSize)
                Status = H_ERROR;
            }

        if (Status == H_SUCCESS)
            {
            m_CurrentLine++;

            SwabArrayOfShort(m_pBuffer, NbRuns * 2);

            // clear the buffer in black
            memset(po_pData, 0, m_BytesPerRow);
            unsigned short* pRun = m_pBuffer;
            Byte Mask;
            Byte* pBytePtr;
            unsigned short j;

            for (unsigned short i = 0; i < NbRuns; i++)
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
                        unsigned short Count;
                        // Check if we can fill a least one byte completly
                        if ((Count = pRun[1] - j + 1) >= 8)
                            {
                            // Can fill a least one byte
                            unsigned short NbByte = Count / 8;
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
                                     const Byte*  pi_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    // not support write access
    HASSERT(0);
    return H_ERROR;
    }
