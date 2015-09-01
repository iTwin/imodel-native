//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCLocalBinStream.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCLocalBinStream
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCLocalBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>

#define SLEEP_TIME 50

// This is the creator that registers itself in the stream-type list.
static struct LocalBinStreamCreator : public HFCBinStream::Creator
    {
    LocalBinStreamCreator()
        {
        HFCLocalBinStream::GetStreamTypeList().insert(HFCLocalBinStream::StreamTypeList::value_type(HFCURLFile::s_SchemeName(), this));
        }
    virtual HFCBinStream* Create(HFCPtr<HFCURL> pi_pURL, uint64_t pi_offSet, HFCAccessMode pi_AccessMode, short pi_NbRetry=0) const override
        {
        HPRECONDITION(pi_pURL != 0);
        HPRECONDITION(pi_pURL->GetSchemeType() == HFCURLFile::s_SchemeName());
        HPRECONDITION(pi_pURL->IsCompatibleWith(HFCURLFile::CLASS_ID));

        HFCURLFile* pFileURL = (HFCURLFile*)pi_pURL.GetPtr();
            
        // No read no write and no create access means that access mode is automatically choosen

        HFCBinStream* pLocalBinStream = 0;
        if ((!pi_AccessMode.m_HasWriteAccess) && (!pi_AccessMode.m_HasReadAccess) && (!pi_AccessMode.m_HasReadShare) && (!pi_AccessMode.m_HasCreateAccess) )
            pLocalBinStream =  new HFCLocalBinStream(pFileURL->GetAbsoluteFileName(), pi_AccessMode.m_HasWriteShare,
                                                     pi_AccessMode.m_HasReadShare,
                                                     false, false, pi_offSet, pi_NbRetry);
        else
            pLocalBinStream = new HFCLocalBinStream(pFileURL->GetAbsoluteFileName(), pi_AccessMode, false, pi_offSet);

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
    m_pLastException.reset(NULL);
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
        dwShareMode = BeFileSharing::ReadWrite;
    else if (pi_ShareRead)
        dwShareMode = BeFileSharing::Read;
    else if (pi_ShareWrite)
        dwShareMode = BeFileSharing::Write;
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

                // Try to Create the file if the open failed, try to open it because BeFile set no-sharing on create 
                if (pi_CreateFile && OpenStatus != BeFileStatus::Success)
                    OpenStatus = m_BeFile.Open (FilenameWithLongNameTagW.c_str(), access/*, WIP_BEFILE_SHARING dwShareMode*/);
                else
                    {   // Reopen the file after creation to set the required access mode.
                    if (OpenStatus == BeFileStatus::Success && (pi_Mode.m_HasReadAccess || m_AccessMode.m_HasWriteShare || m_AccessMode.m_HasWriteShare))
                        {
                        m_BeFile.Close();
                        OpenStatus = m_BeFile.Open(FilenameWithLongNameTagW.c_str(), access/*, WIP_BEFILE_SHARING dwShareMode*/);
                        }
                    }
                }
            else
                {
                OpenStatus = m_BeFile.Open (FilenameWithLongNameTagW.c_str(), access /*WIP_BEFILE_SHARING , dwShareMode*/);

                // Try to Create the file if the open failed. (simulate Open_ALWAYS of Windows CreateFile) BeFile doesn't support it
                if (pi_CreateFile && OpenStatus != BeFileStatus::Success)
                    OpenStatus = m_BeFile.Create (FilenameWithLongNameTagW.c_str(), true);
                }
            }
        --NbTry;
        if ((NbTry > 0) && !m_BeFile.IsOpen())
            {
            BeThreadUtilities::BeSleep(SLEEP_TIME * 20);

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
          m_pLastException.reset(NULL);

        // Validate if we exceed the maximum file size.
        if (m_WeAreWritingAtTheEnd)
            {
            m_CurrentFileSize += pi_DataSize;
            if (m_CurrentFileSize > m_MaxOffsetAcceptable)
                {
                // Indicate an error
                BytesWritten = 0;
                m_pLastException.reset(new HFCFileOutOfRangeException(m_Filename));
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
        FileExceptionFromBeFileStatus(LastError);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HFCLocalBinStream::FileExceptionFromBeFileStatus(BeFileStatus pi_Status)
{
    switch(pi_Status)
        {
        case BeFileStatus::SharingViolationError:
            m_pLastException.reset(new HFCSharingViolationException(m_Filename));
            m_pLastException->ThrowMyself();
            break;
        case BeFileStatus::ReadError:
            m_pLastException.reset(new HFCReadFaultException(m_Filename));
            m_pLastException->ThrowMyself();
            break;
            case BeFileStatus::NotLockedError:
            m_pLastException.reset(new HFCFileLockViolationException(m_Filename));
            m_pLastException->ThrowMyself();
            break;
        case BeFileStatus::AccessViolationError:
            m_pLastException.reset(new HFCFilePermissionDeniedException(m_Filename));
            m_pLastException->ThrowMyself();
            break;

        case BeFileStatus::DiskFull:
            m_pLastException.reset(new HFCNoDiskSpaceLeftException(m_Filename));
            m_pLastException->ThrowMyself();
            break;

        case BeFileStatus::TooManyOpenFilesError:
        case BeFileStatus::FileNotOpenError:
        default:
            m_pLastException.reset(new HFCFileNotCreatedException(m_Filename));
            m_pLastException->ThrowMyself();
            break;
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
    	m_pLastException.reset(NULL);

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