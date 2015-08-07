//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCMemoryBinStream.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCMemoryBinStream
//---------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCMemoryBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HFCLocalBinStream.h>

#define SLEEP_TIME 50

//---------------------------------------------------------------------------
// This is the creator that registers itself in the stream-type list.
//---------------------------------------------------------------------------

static struct MemoryBinStreamCreator : public HFCBinStream::Creator
    {
    MemoryBinStreamCreator()
        {
        HFCMemoryBinStream::GetStreamTypeList().insert(HFCMemoryBinStream::StreamTypeList::value_type(HFCURLMemFile::s_SchemeName(), this));
        }

    // The parameter pi_NbRetryBeforeThrow is not used presently
    virtual HFCBinStream* Create(HFCPtr<HFCURL> pi_pURL, HFCAccessMode pi_AccessMode, short pi_NbRetryBeforeThrow=0) const
        {
        HPRECONDITION(pi_pURL != 0);
        HPRECONDITION(pi_pURL->GetSchemeType() == HFCURLMemFile::s_SchemeName());

        uint64_t OriginOffset = 0;
        HFCURLMemFile* pURL = (HFCURLMemFile*)pi_pURL.GetPtr();
        WString Filename = pURL->GetPath();
        WString::size_type ColonPos = Filename.find(L':');

        if (ColonPos != WString::npos)
            {
            WString OffsetString = Filename.substr(ColonPos+1, Filename.length() - ColonPos - 1);
            Filename.erase(ColonPos, Filename.length()-ColonPos);
            swscanf(OffsetString.c_str(), L"%lld", &OriginOffset);
            }
        Filename = pURL->GetHost() + L"\\" + Filename;

        // No read no write and no create access means that access mode is automatically choosen
        if ((!pi_AccessMode.m_HasWriteAccess) && (!pi_AccessMode.m_HasReadAccess) && (!pi_AccessMode.m_HasCreateAccess))
            return new HFCMemoryBinStream(Filename, pi_AccessMode.m_HasWriteShare, pi_AccessMode.m_HasReadShare, false, false, OriginOffset, 0, pURL->GetBuffer());
        else
            return new HFCMemoryBinStream(Filename, pi_AccessMode, false, OriginOffset, 0, pURL->GetBuffer());

        }
    } s_MemoryBinStreamCreator;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

HFCMemoryBinStream::HFCMemoryBinStream()
    {
    m_CurrentOffset = 0;
    m_BinStreamBuffer = new HFCBuffer(100 * 1024);

    m_AutoRemove    = false;
    m_AccessMode    = HFC_READ_WRITE_OPEN;
    m_OriginOffset  = 0;
    m_LastException = NO_EXCEPTION;
    m_BinStreamOpen = true;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

HFCMemoryBinStream::HFCMemoryBinStream(const WString&        pi_Filename,
    HFCAccessMode         pi_AccessMode,
    bool                  pi_AutoRemove,
    uint64_t              pi_OriginOffset,
    short pi_NbRetryBeforeThrow,
    const HFCPtr<HFCBuffer>&    pi_rpBuffer)
{
    m_AutoRemove      = pi_AutoRemove;
    m_AccessMode      = pi_AccessMode;
    if (pi_rpBuffer == 0)
        m_BinStreamBuffer = new HFCBuffer(100 * 1024);
    else
        m_BinStreamBuffer = pi_rpBuffer;
    m_CurrentOffset   = 0;
    m_Filename        = pi_Filename;
    m_LastException   = NO_EXCEPTION;
    m_LastSeekStatus  = true;
    m_OriginOffset    = pi_OriginOffset;

    if (m_OriginOffset)
        SeekToBegin();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

HFCMemoryBinStream::HFCMemoryBinStream(const WString&        pi_Filename,
    HFCAccessMode         pi_AccessMode,
    bool                 pi_CreateFile,
    bool                 pi_AutoRemove,
    uint64_t             pi_OriginOffset,
    short pi_NbRetryBeforeThrow,
    const HFCPtr<HFCBuffer>&     pi_rpBuffer)
{
    m_AutoRemove      = pi_AutoRemove;
    m_AccessMode      = pi_AccessMode;
    if (pi_rpBuffer == 0)
        m_BinStreamBuffer = new HFCBuffer(100 * 1024);
    else
        m_BinStreamBuffer = pi_rpBuffer;
    m_CurrentOffset   = 0;
    m_Filename        = pi_Filename;
    m_LastException   = NO_EXCEPTION;
    m_LastSeekStatus  = true;
    m_OriginOffset    = pi_OriginOffset;

    if (m_OriginOffset)
        SeekToBegin();

}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

HFCMemoryBinStream::HFCMemoryBinStream(const WString&        pi_Filename,
    bool                 pi_ShareWrite,
    bool                 pi_ShareRead,
    bool                 pi_CreateFile,
    bool                 pi_AutoRemove,
    uint64_t             pi_OriginOffset,
    short pi_NbRetryBeforeThrow,
    const HFCPtr<HFCBuffer>&     pi_rpBuffer)
{
    m_AccessMode                    = HFC_READ_WRITE_OPEN;
    m_AccessMode.m_HasCreateAccess  = pi_CreateFile;
    m_AccessMode.m_HasReadShare     = pi_ShareRead;
    m_AccessMode.m_HasWriteShare    = pi_ShareWrite;
    m_AutoRemove      = pi_AutoRemove;
    if (pi_rpBuffer == 0)
        m_BinStreamBuffer = new HFCBuffer(100 * 1024);
    else
        m_BinStreamBuffer = pi_rpBuffer;
    m_CurrentOffset   = 0;
    m_Filename        = pi_Filename;
    m_LastException   = NO_EXCEPTION;
    m_LastSeekStatus  = true;
    m_OriginOffset    = pi_OriginOffset;

    if (m_OriginOffset)
        SeekToBegin();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

HFCMemoryBinStream::~HFCMemoryBinStream()
    {
    // There is nothing to do here at this time.
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

HFCPtr<HFCURL> HFCMemoryBinStream::GetURL() const
    {
    return new HFCURLMemFile(WString(HFCURLMemFile::s_SchemeName() + L"://") + m_Filename);
    }


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void HFCMemoryBinStream::Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share)
    {
    // Do nothing at this time.
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void HFCMemoryBinStream::Unlock(uint64_t pi_Pos, uint64_t pi_Size)
    {
    // Do nothing at this time.
    }

//---------------------------------------------------------------------------
// HFCMemoryBinStream::Read
//
// Note that the parameter pi_DataSize represents the number of characters
// to read. The value returned should be the number of characters written.
//---------------------------------------------------------------------------
inline size_t HFCMemoryBinStream::Read(WChar* po_pData, size_t pi_DataSize)
    {
    size_t StreamSize   = m_BinStreamBuffer->GetDataSize();
    size_t DataSizeRead = 0;
    size_t DataSize     = pi_DataSize;

    DataSize = DataSize * sizeof(WChar);

    if (m_CurrentOffset + DataSize <= StreamSize)
        {
        // Be sure to not read more data than wich really are.
        memcpy(po_pData, m_BinStreamBuffer->GetData() + m_CurrentOffset, DataSize);
        m_CurrentOffset += DataSize;
        DataSizeRead = pi_DataSize;
        m_LastException = NO_EXCEPTION;
        }
    else
        {
        m_LastException = HFC_FILE_OUT_OF_RANGE_EXCEPTION;
        }

    return DataSizeRead;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline size_t HFCMemoryBinStream::Write(const WChar* pi_pData, size_t pi_DataSize)
    {
    size_t TSize = pi_DataSize * sizeof(WChar);
    m_BinStreamBuffer->AddData((const Byte*)pi_pData, TSize);
    m_CurrentOffset += TSize;

    return pi_DataSize;
    }

#ifdef __HMR_DEBUG_MEMBER

#include "HFCURLFile.h"

//---------------------------------------------------------------------------
// This method is intend to be used as a debugging tools only.
//---------------------------------------------------------------------------

void HFCMemoryBinStream::DumpToFile() const
    {
    WString TempFileName(L"X:\\raster_file.memory_dump.pgn");

    // Open the physical file.
    HFCLocalBinStream PhysicalFile(TempFileName,  false, true, true, false, 0, 0);

    // Dump the buffer into the file.
    HASSERT_X64(m_BinStreamBuffer->GetDataSize() < ULONG_MAX);
    PhysicalFile.Write(m_BinStreamBuffer->GetData(), (DWORD)m_BinStreamBuffer->GetDataSize());
    PhysicalFile.Flush();
    }

#endif // __HMR_DEBUG