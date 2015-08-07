//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCLocalBinStream.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCLocalBinStream
//---------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCLocalBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCUtility.h>

#define SLEEP_TIME 50

// This is the creator that registers itself in the stream-type list.

static struct LocalBinStreamCreator : public HFCBinStream::Creator
    {
    LocalBinStreamCreator()
        {
        HFCLocalBinStream::GetStreamTypeList().insert(HFCLocalBinStream::StreamTypeList::value_type(HFCURLFile::s_SchemeName(), this));
        }
    virtual HFCBinStream* Create(HFCPtr<HFCURL> pi_pURL, HFCAccessMode pi_AccessMode, short pi_NbRetry=0) const
        {
        HPRECONDITION(pi_pURL != 0);
        HPRECONDITION(pi_pURL->GetSchemeType() == HFCURLFile::s_SchemeName());
        uint64_t OriginOffset = 0;
        HFCURLFile* pURL = (HFCURLFile*)pi_pURL.GetPtr();
        WString Filename = pURL->GetPath();

        if (!pURL->GetHost().empty())
            {
            WString::size_type ColonPos = Filename.find(L':');
            if (ColonPos != WString::npos)
                {
                WString OffsetString = Filename.substr(ColonPos+1, Filename.length() - ColonPos - 1);
                Filename.erase(ColonPos, Filename.length()-ColonPos);
                swscanf(OffsetString.c_str(), L"%lld", &OriginOffset);
                }
            Filename = pURL->GetHost() + L"\\" + Filename;   
            }

        // No read no write and no create access means that access mode is automatically choosen

        HFCBinStream* pLocalBinStream = 0;
        if ((!pi_AccessMode.m_HasWriteAccess) && (!pi_AccessMode.m_HasReadAccess) && (!pi_AccessMode.m_HasCreateAccess) )
            pLocalBinStream =  new HFCLocalBinStream(Filename, pi_AccessMode.m_HasWriteShare,
                                                     pi_AccessMode.m_HasReadShare,
                                                     false, false, OriginOffset, pi_NbRetry);
        else
            pLocalBinStream = new HFCLocalBinStream(Filename, pi_AccessMode, false, OriginOffset);

        return pLocalBinStream;
        }
    } s_LocalBinStreamCreator;


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCLocalBinStream::HFCLocalBinStream(const WString&  pi_Filename,
    HFCAccessMode   pi_AccessMode,
    bool            pi_AutoRemove,
    uint64_t        pi_OriginOffset,
    short pi_NbRetry)
{
    SetMaxFileSizeSupported(OffsetIs32Bits);

    Open(pi_Filename,
        pi_AccessMode,
        pi_AccessMode.m_HasWriteShare,
        pi_AccessMode.m_HasReadShare,
        pi_AccessMode.m_OpenAlways,
        pi_OriginOffset,
        false,
        pi_AutoRemove,
        pi_NbRetry);

    m_LastSeekStatus = true;
    m_HasToBeFlushed = false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCLocalBinStream::HFCLocalBinStream(const WString&  pi_Filename,
    HFCAccessMode   pi_AccessMode,
    bool           pi_CreateFile,
    bool           pi_AutoRemove,
    uint64_t       pi_OriginOffset,
    short pi_NbRetry)
{
    SetMaxFileSizeSupported(OffsetIs32Bits);

    Open(pi_Filename,
        pi_AccessMode,
        pi_AccessMode.m_HasWriteShare,
        pi_AccessMode.m_HasReadShare,
        pi_CreateFile,
        pi_OriginOffset,
        false,
        pi_AutoRemove,
        pi_NbRetry);

    m_LastSeekStatus = true;
    m_HasToBeFlushed = false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCLocalBinStream::HFCLocalBinStream(const WString&  pi_Filename,
    bool           pi_ShareWrite,
    bool           pi_ShareRead,
    bool           pi_CreateFile,
    bool           pi_AutoRemove,
    uint64_t       pi_OriginOffset,
    short pi_NbRetry)
{
    SetMaxFileSizeSupported(OffsetIs32Bits);

    Open(pi_Filename,
        HFC_READ_WRITE_OPEN,
        pi_ShareWrite,
        pi_ShareRead,
        pi_CreateFile,
        pi_OriginOffset,
        true,
        pi_AutoRemove,
        pi_NbRetry);

    m_LastSeekStatus = true;
    m_HasToBeFlushed = false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCLocalBinStream::~HFCLocalBinStream()
    {
    m_BeFile.Close();

    if (m_AutoRemove)
        {
        BeFileName::BeDeleteFile(CookFilenameWithLongNameTagW().c_str());
        }
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCLocalBinStream::Open(const WString& pi_Filename,
                             HFCAccessMode pi_Mode,
                             bool    pi_ShareWrite,
                             bool    pi_ShareRead,
                             bool    pi_CreateFile,         // OPEN_ALWAYS
                             uint64_t pi_OriginOffset,
                             bool    pi_IgnoreMode,
                             bool    pi_AutoRemove,
                             short pi_NbRetry)
    {
    m_AutoRemove        = pi_AutoRemove;
    m_AccessMode        = HFC_NO_ACCESS;
    m_OriginOffset      = pi_OriginOffset;
    m_LastException     = NO_EXCEPTION;
    m_LastSeekStatus    = true;
    m_HasToBeFlushed    = false;
    m_CurrentFileSize       = 0;
    m_WeAreWritingAtTheEnd  = false;

    BeFileName::BeGetFullPathName(m_Filename, pi_Filename.c_str());
        
    m_AccessMode.m_HasWriteShare = pi_ShareWrite;
    m_AccessMode.m_HasReadShare = pi_ShareRead;
#if defined (WIP_BEFILE_SHARING)
    DWORD dwShareMode = 0;
    if (pi_ShareRead && pi_ShareWrite)
        dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE;
    else if (pi_ShareRead)
        dwShareMode = FILE_SHARE_READ;
    else if (pi_ShareWrite)
        dwShareMode = FILE_SHARE_WRITE;
#endif
    WString FilenameWithLongNameTagW = CookFilenameWithLongNameTagW();

    uint32_t NbTry = (pi_NbRetry >= 0) ? (pi_NbRetry+1) : abs(pi_NbRetry);

    BeFileStatus OpenStatus = BeFileStatus::UnknownError;
    while ((OpenStatus != BeFileStatus::Success) && (NbTry > 0))
        {
        if (pi_IgnoreMode)
            {
            OpenStatus = m_BeFile.Open (FilenameWithLongNameTagW.c_str(),BeFileAccess::ReadWrite /*WIP_BEFILE_SHARING ,dwShareMode*/);

            if (OpenStatus != BeFileStatus::Success)
                {
                OpenStatus = m_BeFile.Open (FilenameWithLongNameTagW.c_str(), BeFileAccess::Read /*WIP_BEFILE_SHARING ,dwShareMode*/);
                m_AccessMode |= HFC_READ_ONLY;
                }
            else
                m_AccessMode |= HFC_READ_WRITE_OPEN;
            }
        else
            {
            BeFileAccess access = BeFileAccess::Read;
            if (pi_Mode.m_HasReadAccess && pi_Mode.m_HasWriteAccess)
                {
                access = BeFileAccess::ReadWrite;
                m_AccessMode |= pi_Mode;
                }
            else if (pi_Mode.m_HasReadAccess)
                {
                access = BeFileAccess::Read;
                m_AccessMode |= HFC_READ_ONLY;
                }
            else if (pi_Mode.m_HasWriteAccess)
                {
                access = BeFileAccess::Write;
                m_AccessMode |= HFC_WRITE_ONLY;
                }
            else if (pi_Mode.m_HasCreateAccess)
                {
                access = BeFileAccess::Write;
                m_AccessMode |= HFC_CREATE_ONLY;
                }

            if (pi_Mode.m_HasCreateAccess)
                {
                OpenStatus = m_BeFile.Create (FilenameWithLongNameTagW.c_str(), true);

                //DM-Android
                // Try to Create the file if the open failed, try to open it because BeFile set no-sharing on create 
                if (pi_CreateFile && OpenStatus != BeFileStatus::Success)
                    OpenStatus = m_BeFile.Open (FilenameWithLongNameTagW.c_str(), access /*WIP_BEFILE_SHARING ,dwShareMode*/);
                }
            else
                {
                OpenStatus = m_BeFile.Open (FilenameWithLongNameTagW.c_str(), access /*WIP_BEFILE_SHARING , dwShareMode*/);

                //DM-Android
                // Try to Create the file if the open failed. (simulate Open_ALWAYS of Windows CreateFile) BeFile doesn't support it //DM-Android
                if (pi_CreateFile && OpenStatus != BeFileStatus::Success)
                    OpenStatus = m_BeFile.Create (FilenameWithLongNameTagW.c_str(), true);
                }
            }
        --NbTry;
        if ((NbTry > 0) && !m_BeFile.IsOpen())
            {
            BeThreadUtilities::BeSleep(SLEEP_TIME*20);

            // Trace for debugging...
            HDEBUGCODE(WChar pTxt[512]);
            HDEBUGCODE(BeStringUtilities::Snwprintf(pTxt, L"NbTry:%d (%ls)\n", NbTry, m_Filename.c_str()););
            HDEBUGTEXT(pTxt);
            }
        }

    if (OpenStatus != BeFileStatus::Success)
        {
        // Exit from the method because throw...
        SetLastExceptionClassID();
        }
    else
        {
        m_BinStreamOpen = true;

        if (m_OriginOffset != 0)
            SeekToBegin();

        // Initialize stuff to verify if we are creating file over the limit
        m_CurrentFileSize = GetSize();
        if (m_AccessMode.m_HasCreateAccess)
            m_WeAreWritingAtTheEnd  = true;


#define BUFFERSIZE  256
        m_StackChar = new WChar[BUFFERSIZE+1];    // +1 for a temporary '\0'
        m_StackSize=0;
        m_StackCharIndex=0;
        m_BufferChar = new char[BUFFERSIZE+1];     // +1 for a temporary '\0'
        m_BufferCharIndex=0;

        // Check if the file has the UTF-8 header bytes, we verify it only if the file is open
        // in read mode at least.
        m_FileIsUTF8Encoded = false;
        if (pi_IgnoreMode || m_AccessMode.m_HasReadAccess)
            {
            unsigned char UTF8Heabder[3];
            UTF8Heabder[0] = 0;
            UTF8Heabder[1] = 0;
            UTF8Heabder[2] = 0;

            // If we found UTF-8 heabder skip it, else reset to the start position.
            if (Read(UTF8Heabder, 3) == 3)
                {
                if (UTF8Heabder[0] == 0xef &&
                    UTF8Heabder[1] == 0xbb &&
                    UTF8Heabder[2] == 0xbf)
                    m_FileIsUTF8Encoded = true;
                else
                    SeekToBegin();
                }
            else
                SeekToBegin();
            }
        }
    }



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCPtr<HFCURL> HFCLocalBinStream::GetURL() const
    {
    return new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + m_Filename);
    }


//-----------------------------------------------------------------------------
// This method is used to set a maximum file size based on the number of bit
// supported.
// An error code will be returned while the method "write" is called
// if the maximum is reached.
//
// By default the MaxFileSize supported is based on a 32bits.
// To support file greater than 4Giga(32bit), the parameter must be set to
// 64Bits.
//
//-----------------------------------------------------------------------------
void HFCLocalBinStream::SetMaxFileSizeSupported(MaxOffsetBitsSupported pi_OffsetBits)
    {
    switch(pi_OffsetBits)
        {
        case OffsetIs64Bits:            // SetFilePointer is 64bits signed --> / 2
            m_MaxOffsetAcceptable = (UINT64_MAX / 2) - ULONG_MAX;
            break;

        case OffsetIs32Bits:
        default:
            m_MaxOffsetAcceptable = ULONG_MAX - (ULONG_MAX / 20);
            break;
        }
    }

//---------------------------------------------------------------------------
// We are considered that we read all the file in one shot...
// Return the number of wideChar read.
//---------------------------------------------------------------------------
size_t HFCLocalBinStream::Read(WChar* po_pData, size_t pi_DataSize)
    {
    HPRECONDITION(pi_DataSize <= UINT32_MAX);

    size_t BytesRead = 0;
    bool Success = false;

    if (m_LastSeekStatus)
        {
        Success = true;
        while (pi_DataSize > 0)
            {
            size_t NbChar = min (pi_DataSize, (m_StackSize-m_StackCharIndex));
            if (NbChar > 0)
                {
                wmemcpy(po_pData, &(m_StackChar[m_StackCharIndex]), NbChar);
                m_StackCharIndex += NbChar;
                pi_DataSize -= NbChar;
                BytesRead += NbChar;
                }

            // Need more char
            if (pi_DataSize > 0)
                {
                size_t NbByteInBuffer;

                m_StackCharIndex = 0;
                m_StackSize = 0;

                // We read at the minimun one line of text.
                uint32_t ReadSize;
                Success = m_BeFile.Read(&(m_BufferChar[m_BufferCharIndex]),
                                        &ReadSize,
                                        (uint32_t)(BUFFERSIZE-m_BufferCharIndex)) == BeFileStatus::Success;

                NbByteInBuffer = ReadSize + m_BufferCharIndex;

                if (NbByteInBuffer > 0)
                    {
                    // Search the first Ansi char (<128, that inclue '\n')
                    // to be able to convert all previous char
                    size_t AnsiCharPos;
                    for(AnsiCharPos=NbByteInBuffer-1;
                        (m_BufferChar[AnsiCharPos] == 0x00 || m_BufferChar[AnsiCharPos] >= 0x80) && AnsiCharPos>=0;
                        --AnsiCharPos);

                    // Convert it
                    if (AnsiCharPos >= 0)
                        {
                        WString tempoStr;
                        if (m_FileIsUTF8Encoded)
                            {
                            BeStringUtilities::Utf8ToWChar(tempoStr, m_BufferChar, AnsiCharPos+1);
                            }
                        else
                            {
                            uint32_t codePage;
                            BeStringUtilities::GetCurrentCodePage(codePage);
                            BeStringUtilities::LocaleCharToWChar(tempoStr, m_BufferChar, codePage, AnsiCharPos+1);
                            }

                        // ***** Do not use the "Be" version since it happens a NULL and thus override the last entry.
                        // BeStringUtilities::Wcsncpy(m_StackChar.get(), BUFFERSIZE, tempoStr.c_str());
                        wcsncpy(m_StackChar.get(), tempoStr.c_str(), BUFFERSIZE);
                        m_StackSize =  tempoStr.length();
                        }
                    else
                        HASSERT(false);     // problem no char(<128) found ???

                    // move the chars not used, at the beginning of the buffer.
                    if (AnsiCharPos < NbByteInBuffer-1)
                        {
                        m_BufferCharIndex = NbByteInBuffer-(AnsiCharPos+1);
                        memcpy(m_BufferChar, &(m_BufferChar[AnsiCharPos+1]), m_BufferCharIndex);
                        }
                    else
                        m_BufferCharIndex = 0;          // The buffer was converted completely
                    }

                // No enough char...
                if (m_StackSize == 0)
                    pi_DataSize = 0;
                }
            }
        }
    if (!Success)
        {
        //Set last error
        SetLastExceptionClassID();
        }
    else
        m_LastException = NO_EXCEPTION;
    return BytesRead;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
size_t HFCLocalBinStream::Write(const void* pi_pData, size_t pi_DataSize)
    {
    HASSERT_X64(pi_DataSize <= UINT32_MAX);

    uint32_t BytesWritten = 0;
    bool Success = false;
    if (m_LastSeekStatus)
        Success = m_BeFile.Write(&BytesWritten, pi_pData, (uint32_t)pi_DataSize) == BeFileStatus::Success;

    if (!Success)
        {
        BytesWritten = 0;
        }
    else
        {
        m_LastException = NO_EXCEPTION;

        // Validate if we exceed the maximum file size.
        if (m_WeAreWritingAtTheEnd)
            {
            m_CurrentFileSize += pi_DataSize;
            if (m_CurrentFileSize > m_MaxOffsetAcceptable)
                {
                // Indicate an error
                BytesWritten = 0;
                m_LastException = HFC_FILE_OUT_OF_RANGE_EXCEPTION;
                }
            }
        }

    m_HasToBeFlushed = (BytesWritten != 0);

    return BytesWritten;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
size_t HFCLocalBinStream::Write(const WChar* pi_pData, size_t pi_DataSize)
    {
    HPRECONDITION(pi_DataSize <= UINT32_MAX);

    uint32_t BytesWritten = 0;
    bool Success = false;

    if (m_LastSeekStatus)
        {
        Utf8String utf8Str;
        BeStringUtilities::WCharToUtf8(utf8Str,pi_pData,pi_DataSize);

        Success = m_BeFile.Write(&BytesWritten,utf8Str.c_str(), (uint32_t)utf8Str.length()) == BeFileStatus::Success;
        }
    if (!Success)
        {
        BytesWritten = 0;
        }
    else
        {
        m_LastException = NO_EXCEPTION;

        // Validate if we exceed the maximum file size.
        if (m_WeAreWritingAtTheEnd)
            {
            m_CurrentFileSize += pi_DataSize;
            if (m_CurrentFileSize > m_MaxOffsetAcceptable)
                {
                // Indicate an error
                BytesWritten = 0;
                m_LastException = HFC_FILE_OUT_OF_RANGE_EXCEPTION;
                }
            }
        }

    m_HasToBeFlushed = (BytesWritten != 0);

    return BytesWritten;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCLocalBinStream::Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share)
    {
#if defined (ANDROID) || defined (__APPLE__)      
    //DM-Android

#elif defined (_WIN32)
    OVERLAPPED info;
    info.Offset = (DWORD)pi_Pos;
    info.OffsetHigh = (DWORD)(pi_Pos >> 32);
    info.hEvent = 0;
    DWORD flags = pi_Share ? (LOCKFILE_FAIL_IMMEDIATELY) : (LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY);
    while (!LockFileEx(m_BeFile.GetHandle(), flags, 0, (DWORD)pi_Size, (DWORD)(pi_Size >> 32), &info))
        {
        DWORD LastError=GetLastError();
        if (LastError == 6 /* ERROR_INVALID_HANDLE */)
            break;

        // We are not sure the lock have been complete,
        // but be sure to unlock the file....
        Unlock(pi_Pos, pi_Size);
        Sleep(SLEEP_TIME);
        }
#endif
    }


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCLocalBinStream::Unlock(uint64_t pi_Pos, uint64_t pi_Size)
    {
#if defined (ANDROID) || defined (__APPLE__)      
        //DM-Android

#elif defined (_WIN32)
    OVERLAPPED info;
    info.Offset = (DWORD)pi_Pos;
    info.OffsetHigh = (DWORD)(pi_Pos >> 32);
    info.hEvent = 0;
    if (!UnlockFileEx(m_BeFile.GetHandle(), 0, (DWORD)pi_Size, (DWORD)(pi_Size >> 32), &info))
        {
        SetLastExceptionClassID();
        }
#endif
    }



void HFCLocalBinStream::SetLastExceptionClassID()
    {
    BeFileStatus LastError = m_BeFile.GetLastError();

    HDEBUGCODE(if ((LastError != BeFileStatus::NotLockedError)) {
              );
    HDEBUGCODE(WChar Msg[512];
                  );
        HDEBUGCODE(BeStringUtilities::Snwprintf (Msg, L"GetLastError: %lu (%ls)\n", LastError, m_Filename.c_str());
                  );
        HDEBUGTEXT(Msg);
        HDEBUGCODE(
        });

    if (LastError != BeFileStatus::NotLockedError) // Don't throw.. error is normal
        {
        MapHFCFileExceptionFromBeFileStatus(LastError,
                                            L"",
                                            HFC_FILE_NOT_CREATED_EXCEPTION,
                                            &m_LastException);
        }
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
uint64_t HFCLocalBinStream::GetSize() const
{
    uint64_t FileSize;
    if (m_BeFile.GetSize(FileSize) != BeFileStatus::Success)
        FileSize = 0;

    return FileSize - m_OriginOffset;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
uint64_t HFCLocalBinStream::GetCurrentFileSize() const
    {
    return m_CurrentFileSize;
    }


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
size_t HFCLocalBinStream::Read(void* po_pData, size_t pi_DataSize)
{
    HASSERT_X64(pi_DataSize < UINT32_MAX);

    uint32_t BytesRead = 0;
    bool Success = false;

    if (m_LastSeekStatus)
        Success = m_BeFile.Read(po_pData, &BytesRead, (uint32_t)pi_DataSize) == BeFileStatus::Success;

    if (!Success)
    {
        BytesRead = 0;
    }
    else
        m_LastException = NO_EXCEPTION;

    return BytesRead;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCLocalBinStream::Seek(int64_t pi_Delta)
{
    if (m_BeFile.SetPointer (pi_Delta, BeFileSeekOrigin::Current) != BeFileStatus::Success)
        m_LastSeekStatus = false;
    else
    {
        m_WeAreWritingAtTheEnd  = false;
        m_LastSeekStatus = true;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCLocalBinStream::SeekToPos(uint64_t pi_NewPos)
{
    pi_NewPos += m_OriginOffset;

    if (m_BeFile.SetPointer (pi_NewPos, BeFileSeekOrigin::Begin) != BeFileStatus::Success)
        m_LastSeekStatus = false;
    else
    {
        if (pi_NewPos < m_CurrentFileSize)
            m_WeAreWritingAtTheEnd  = false;
        else
            m_WeAreWritingAtTheEnd  = true;

        m_LastSeekStatus = true;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCLocalBinStream::SeekToBegin()
{
    if (m_BeFile.SetPointer (m_OriginOffset, BeFileSeekOrigin::Begin) != BeFileStatus::Success)
        m_LastSeekStatus = false;
    else
    {
        m_WeAreWritingAtTheEnd  = false;
        m_LastSeekStatus = true;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCLocalBinStream::SeekToEnd()
{
    if (m_BeFile.SetPointer (0, BeFileSeekOrigin::End) != BeFileStatus::Success)
        m_LastSeekStatus = false;
    else
    {
        m_WeAreWritingAtTheEnd  = true;
        m_LastSeekStatus = true;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HFCLocalBinStream::Flush()
{
    bool Success = false;

    if (m_HasToBeFlushed)
    {
        HASSERT(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);

        Success = (m_BeFile.Flush() == BeFileStatus::Success);

        m_HasToBeFlushed = (Success != true);

        if (!Success)
            SetLastExceptionClassID();
    }

    return Success;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HFCLocalBinStream::SetEOF()
{
    bool Success = false;
    if (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess)
        {
        uint64_t CurrentPos;
        if (m_BeFile.GetPointer(CurrentPos) == BeFileStatus::Success)
            {
            Success = m_BeFile.SetSize(CurrentPos) == BeFileStatus::Success;
            }

        if (!Success)
            SetLastExceptionClassID();
        }
    return Success;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
WString HFCLocalBinStream::CookFilenameWithLongNameTagW() const
    {
    // TR 276264: Add support to filename longer than MAX_PATH
    //            Need to use unicode version and prepend "\\?\"
    if(m_Filename.size() >= MAX_PATH-2)
        return WString(L"\\\\\?\\") + m_Filename;

    return m_Filename;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
uint64_t HFCLocalBinStream::GetCurrentPos()
    {
    uint64_t Pos;
    m_BeFile.GetPointer(Pos);
    return Pos - m_OriginOffset;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCAccessMode HFCLocalBinStream::GetAccessMode() const
    {
    return m_AccessMode;
    }

//-----------------------------------------------------------------------------
// Returns the offset between the beginning of the file managed by this handle
// and the physical beginning of the file on disk.
//-----------------------------------------------------------------------------
uint64_t HFCLocalBinStream::GetOriginOffset() const
    {
    return m_OriginOffset;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HFCLocalBinStream::EndOfFile()
    {
    return GetCurrentPos() >= GetSize();
    }