//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCMemoryLineStream.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCMemoryLineStream
//---------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCMemoryLineStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HFCLocalBinStream.h>

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
HFCMemoryLineStream::HFCMemoryLineStream(const WString&     pi_Filename,
                                         char              pi_LineDelimiter,
                                         HFCPtr<HFCBuffer>& pi_rpBuffer)
    : HFCMemoryBinStream(pi_Filename,
                         HFC_READ_ONLY,
                         false,
                         0,
                         0,
                         pi_rpBuffer)
    {
    m_pLineStartingAddresses = new MemAddresses;

    size_t LineAddress = 0;

    m_pLineStartingAddresses->push_back(LineAddress);

    while (LineAddress < pi_rpBuffer->GetDataSize())
        {
        if (strncmp((const char*)&pi_rpBuffer->GetData()[LineAddress], &pi_LineDelimiter, 1) == 0)
            {   //Start the line after the line delimiter
            if (LineAddress + 1 < pi_rpBuffer->GetDataSize())
                {
                m_pLineStartingAddresses->push_back(LineAddress + 1);
                }
            }

        LineAddress++;
        }
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
HFCMemoryLineStream::~HFCMemoryLineStream()
    {
    // There is nothing to do here at this time.
    }

//---------------------------------------------------------------------------
// Read the specified line. The line numbering begins at 0.
//---------------------------------------------------------------------------
size_t HFCMemoryLineStream::ReadLine(uint32_t pi_LineNb,
                                     WString& po_rLine)
    {
    HPRECONDITION(m_pLineStartingAddresses != 0);

    HArrayAutoPtr<Byte> pTempBuffer;
    uint64_t            BytesRead             = 0;
    uint64_t            CurrentTempBufferSize = 0;

    //Valid line number
    if (pi_LineNb < m_pLineStartingAddresses->size())
        {
        uint64_t NbBytesToRead;

        if (pi_LineNb == (m_pLineStartingAddresses->size() - 1))
            {
            NbBytesToRead = GetSize() -
                            (*m_pLineStartingAddresses)[pi_LineNb] - 1;
            }
        else
            {
            NbBytesToRead = (*m_pLineStartingAddresses)[pi_LineNb + 1] -
                            (*m_pLineStartingAddresses)[pi_LineNb] - 1;
            }

        //Allocate a greater buffer if the current one cannot hold the data.
        if (CurrentTempBufferSize < NbBytesToRead + 1)
            {
            pTempBuffer = new Byte[(size_t)NbBytesToRead + 1];
            }

        SeekToPos((*m_pLineStartingAddresses)[pi_LineNb]);

        BytesRead = Read(pTempBuffer.get(), (size_t)NbBytesToRead);

        if (BytesRead != NbBytesToRead)
            {
            m_LastException = HFC_FILE_IO_ERROR_EXCEPTION;
            NbBytesToRead = 0;
            }
        else
            {
            pTempBuffer.get()[NbBytesToRead] = '\0';

            string TempString((char*)pTempBuffer.get());

            po_rLine = WString(TempString.c_str(),false);
            }
        }
    else
        {
        m_LastException = HFC_FILE_OUT_OF_RANGE_EXCEPTION;
        }

    return (size_t)BytesRead;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
HFCPtr<HFCURL> HFCMemoryLineStream::GetURL() const
    {
    return HFCMemoryBinStream::GetURL();
    }