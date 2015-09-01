//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSLocalLister.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFSLocalLister
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------
// HFSLocalLister
//
// Main constructor.
//
// HFCPtr<HFCConnection>& pi_pConnection:
// string pi_DefaultPattern:
//
//-----------------------------------------------------------------
inline HFSLocalLister::HFSLocalLister(HFCPtr<HFCConnection>& pi_pConnection,
                                      WString                pi_DefaultPattern)
    :HFSDirectoryLister(pi_pConnection)
    {
    SetPattern(pi_DefaultPattern);
    }

//-----------------------------------------------------------------
// ~HFSLocalLister
//
// Destructor.
//
//-----------------------------------------------------------------
inline HFSLocalLister::~HFSLocalLister()
    {
    // Do nothing
    }

//-----------------------------------------------------------------
// bool List
//
// List a specified directory.
//
// const string& pi_rPath:
//
//-----------------------------------------------------------------
inline bool HFSLocalLister::List(const WString& pi_rPath)
    {
    bool Status = false;
    WString FileSpec;
    WString TmpPath(pi_rPath);
    int32_t hSearch;
    struct _wfinddata_t FileInfo;

    HFCPtr<HFCLocalConnection> pConnection = (HFCPtr<HFCLocalConnection>&)(m_pConnection);

    // Be sure the connection is still alive
    if( pConnection->ValidateConnect(5000) )
        {

        PrepareForList();

        UnifyPath(TmpPath);

        HFCPtr<HFCURLFile>  pURL((HFCURLFile*)HFCURL::Instanciate(pConnection->GetServer()));

        FileSpec  = pURL->GetHost();
        if( TmpPath[0] != L'\\' )
            FileSpec += L"\\";
        FileSpec += TmpPath;
        FileSpec += L"*.*";

        hSearch = _wfindfirst(const_cast<WChar*>(FileSpec.c_str()), &FileInfo);

        if( hSearch != -1 )
            {
            Status = true;
            m_CurrentPath = pi_rPath;

            // Insert all the file into the list
            do
                {
                // skip "." and ".."
                if( (wcscmp(FileInfo.name, L".") != 0) &&
                    (wcscmp(FileInfo.name, L"..") != 0) )
                    {
                    if( IsEntryCompliant(WString(FileInfo.name)) ||
                        (FileInfo.attrib & HFC_FILE_ATTRIBUTES_SUBDIR) )
                        {
                        WChar Buffer[255];
                        if (FileInfo.time_write != -1)
                            _tcsftime(Buffer, 255, L"%H:%M:%S %m/%d/%Y", gmtime(&FileInfo.time_write));
                        else
                            Buffer[0] = L'\0';
                        HFSDirectoryListItem Entry(FileInfo.name,
                                                   FileInfo.attrib,
                                                   FileInfo.size,
                                                   WString(Buffer));

                        m_EntryList.push_back(Entry);
                        }
                    }
                }
            while( _wfindnext(hSearch, &FileInfo) == 0 );

            _findclose(hSearch);
            }
        }

    m_IsError = !Status;

    return Status;
    }


//-----------------------------------------------------------------
// bool SupportFileDate
//
// Return true if the date field is valid, false if not.
//
//-----------------------------------------------------------------
inline bool HFSLocalLister::SupportFileDate() const
    {
    return true;
    }

//-----------------------------------------------------------------
// bool SupportFileSize
//
// Return true if the file size field is valid, false if not.
//
//-----------------------------------------------------------------
inline bool HFSLocalLister::SupportFileSize() const
    {
    return true;
    }
END_IMAGEPP_NAMESPACE