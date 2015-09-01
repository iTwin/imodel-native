//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCEmbedBinStream.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCEmbedBinStream
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCEmbedBinStream.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLEmbedFile.h>


//---------------------------------------------------------------------------
// This is the creator that registers itself in the stream-type list.
//---------------------------------------------------------------------------

static struct EmbedBinStreamCreator : public HFCBinStream::Creator
    {
    EmbedBinStreamCreator()
        {
        HFCEmbedBinStream::GetStreamTypeList().insert(HFCEmbedBinStream::StreamTypeList::value_type(HFCURLEmbedFile::s_SchemeName(), this));
        }

    // The parameter pi_NbRetryBeforeThrow is not used presently
    virtual HFCBinStream* Create(HFCPtr<HFCURL> pi_pURL, uint64_t pi_offSet, HFCAccessMode pi_AccessMode, short pi_NbRetryBeforeThrow=0) const override
        {
        HPRECONDITION(pi_pURL != 0);
        HPRECONDITION(pi_offSet == 0); // not supported.
        HPRECONDITION(pi_pURL->GetSchemeType() == HFCURLEmbedFile::s_SchemeName());

        HFCURLEmbedFile* pURL = (HFCURLEmbedFile*)pi_pURL.GetPtr();

        return new HFCEmbedBinStream(pURL->GetPath(), pURL->GetFileReaderHandler());
        }
    } s_EmbedBinStreamCreator;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCEmbedBinStream::HFCEmbedBinStream(WString const& pi_Filename, IFileReaderHandler* pi_pHandler)
    :m_pHandler(pi_pHandler),
     m_Filename(pi_Filename)
    {
    m_CurrentPos = 0;
    m_AccessMode = HFC_READ_ONLY;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCEmbedBinStream::~HFCEmbedBinStream()
    {
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCPtr<HFCURL>  HFCEmbedBinStream::GetURL() const
    {
    return new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + m_Filename);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
size_t HFCEmbedBinStream::Read(void* po_pData, size_t pi_DataSize)
    {
    HASSERT_X64(pi_DataSize < UINT32_MAX);

    m_pHandler->Seek(GetCurrentPos());

    size_t bytesRead = 0;
    m_pHandler->Read(po_pData, pi_DataSize, bytesRead);

    m_CurrentPos += pi_DataSize;

    return bytesRead;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
size_t HFCEmbedBinStream::Write(const void* pi_pData, size_t pi_DataSize)
    {
    // Do nothing at this time.

    return 0;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
uint64_t HFCEmbedBinStream::GetSize() const
    {
    uint64_t size;
    m_pHandler->GetSize(size);

    return size;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
HFCAccessMode HFCEmbedBinStream::GetAccessMode() const
    {
    return m_AccessMode;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void HFCEmbedBinStream::Seek(int64_t pi_Delta)
    {
    m_CurrentPos += (size_t)pi_Delta;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void HFCEmbedBinStream::SeekToPos(uint64_t pi_NewPos)
    {
    m_CurrentPos = (size_t)pi_NewPos;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void HFCEmbedBinStream::SeekToBegin()
    {
    SeekToPos(0);
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void HFCEmbedBinStream::SeekToEnd()
    {
    SeekToPos(GetSize());
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
uint64_t HFCEmbedBinStream::GetCurrentPos()
    {
    return m_CurrentPos;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
bool HFCEmbedBinStream::EndOfFile()
    {
    return (GetCurrentPos() >= GetSize());
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void HFCEmbedBinStream::Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share)
    {
    // Do nothing at this time.
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void HFCEmbedBinStream::Unlock(uint64_t pi_Pos, uint64_t pi_Size)
    {
    // Do nothing at this time.
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
bool HFCEmbedBinStream::Flush()
    {
    // There is nothing to do here, a embed buffer is always flush...
    // The purpose of this method is only the preservation of
    // keeping HFCLocalBinStream interface and behavior.

    return true;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
bool HFCEmbedBinStream::SetEOF()
    {
    // There is nothing to do here, a embed buffer is always flush...
    // The purpose of this method is only the preservation of
    // keeping HFCLocalBinStream interface and behavior.

    return false;
    }