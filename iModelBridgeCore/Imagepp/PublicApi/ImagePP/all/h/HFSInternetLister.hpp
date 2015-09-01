//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSInternetLister.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFSInternetLister
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
// Max timeout in seconde
static const uint32_t s_MAX_WAITTIME_LIST = 120;

//-----------------------------------------------------------------
// HFSInternetLister
//
// Main constructor.
//
// HFCPtr<HFCConnection>& pi_pConnection:
// bool pi_UseExtendedProtocol:
//
//-----------------------------------------------------------------
inline HFSInternetLister::HFSInternetLister(HFCPtr<HFCConnection>& pi_pConnection,
                                            bool pi_UseExtendedProtocol)
    :HFSDirectoryLister(pi_pConnection)
    {
    m_ProtocolChecked     = false;

#ifdef _USE_EXTENDED_PROTOCOL
    m_UseExtendedProtocol = pi_UseExtendedProtocol;
#else
    m_UseExtendedProtocol = false;
#endif
    }

//-----------------------------------------------------------------
// ~HFSInternetLister
//
// Destructor.
//
//-----------------------------------------------------------------
inline HFSInternetLister::~HFSInternetLister()
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
inline bool HFSInternetLister::List(const WString& pi_rPath)
    {
    bool  Done    = false;
    bool  Status  = false;
    bool  Timeout = false;
    time_t Start;
    time_t Current;
    string Responce;
    WString Request;
    WString TmpPath(pi_rPath);


    // Here we must find if the
    // server support the extended
    // protocol. So try a query and
    // check the result. Do only once.
#ifdef _USE_EXTENDED_PROTOCOL
    if( !m_ProtocolChecked )
        {
        if( m_UseExtendedProtocol )
            {
            m_Command = L"HIP-List";
            m_UseExtendedProtocol = VerifyExtendedProtocol();
            }

        if( !m_UseExtendedProtocol )
            m_Command = L"list";

        m_ProtocolChecked = true;
        }
#else
    m_Command = L"list";
#endif

    HFCPtr<HFCInternetConnection> pConnection = (HFCPtr<HFCInternetConnection>&)(m_pConnection);

    try
        {
        // Be sure that the connection is alive
        if( pConnection->ValidateConnect(5000) )
            {
            PrepareForList();

            UnifyPath(TmpPath);

            if( !m_UseExtendedProtocol )
                {
                Request  = L"obj=";
                Request += m_Command;
                Request += L",";
                }
            else
                {
                Request  = m_Command;
                Request += L"=";
                }

            Request += TmpPath;
            Request += L"\r\n";

            // convert the command from WideChar to UTF8
            Utf8String utf8Str;
            BeStringUtilities::WCharToUtf8(utf8Str,Request);

            // Send the list request
            pConnection->Send((const Byte*)utf8Str.c_str(), utf8Str.size());

            // Wait the server response
            time(&Start);
            while( pConnection->IsConnected() && !Done && !Timeout )
                {
                size_t InByte = pConnection->WaitDataAvailable(100);
                if( InByte > 0 )
                    {
                    // Each time we receive some
                    // bytes, restart the timer
                    time(&Start);

                    HArrayAutoPtr<Byte> pBuffer(new Byte[InByte]);

                    pConnection->Receive(pBuffer, InByte);

                    Responce.append((char*)pBuffer.get(), InByte);

                    // UNICODE note : we can search directly \r\n in a UTF8 string because
                    // these character have ASCII code under 127
                    if( (Responce.size() == 2 && Responce.find("\r\n") != string::npos) ||
                        Responce.find("\r\n\r\n") != string::npos )
                        Done = true;
                    }
                else
                    {
                    ::Sleep(100);
                    // Check for a timeout
                    time(&Current);
                    if( difftime(Current, Start) > s_MAX_WAITTIME_LIST )
                        Timeout = true;
                    }
                }

            // Must have received
            // a complete response
            if( Responce.size() > 0 && Done )
                {
                // UNICODE note : we can compare the string directly in UTF8
                string CheckError = Responce.substr(0, 6);
                if( BeStringUtilities::Stricmp(CheckError.c_str(), "error/") == 0 )
                    {
                    Status = false;
                    }
                else
                    {
                    Status = BuildFileList(Responce); // Responce in UTF8
                    if( Status )
                        m_CurrentPath = TmpPath;
                    }
                }
            else
                Status = false;

            pConnection->Disconnect();
            }
        }
    catch(HFCInternetConnectionException&)
        {
        Status = false;
        }

    m_IsError = !Status;

    return Status;
    }

//-----------------------------------------------------------------
// bool BuildFileList
//
// Build the internal list from a formatted string.
//
// const string& pi_rData : The data is in UTF8
//
//-----------------------------------------------------------------
inline bool HFSInternetLister::BuildFileList(const string& pi_rData)
    {
    bool  Status = true;
    string::size_type EndPos = pi_rData.find("\r\n"); // can find direclty into the UTF8 buffer
    string NextString = pi_rData;

    // In the first version, the format was:
    // list,attribute,filename\r\n
    // The new format is: list,attribute,filename,size,date\r\n
    // Search while an empty line is not found
    while( Status && EndPos != string::npos && EndPos != 0 )
        {
        // The first version of the protocol
        // did not include size and date,
        // so we need to count the number of
        // separator to find if the size and date
        // are specified

        HArrayAutoPtr<WChar> pCmd;
        HArrayAutoPtr<WChar> pAttrib;
        HArrayAutoPtr<WChar> pName;
        HArrayAutoPtr<WChar> pSize;
        HArrayAutoPtr<WChar> pDate;
        uint32_t Attrib;
        uint32_t Size = 0;
        WString DateTime;
        string  Token = NextString.substr(0, EndPos);

        string CheckError   = Token.substr(0, 6);

        WString CheckCommand;
        BeStringUtilities::Utf8ToWChar(CheckCommand,Token.substr(0, 4).c_str());

        if( BeStringUtilities::Stricmp(CheckError.c_str(), "error/") != 0 &&
            m_Command == CheckCommand )
            {
            WString Converted;
            BeStringUtilities::Utf8ToWChar(Converted,Token.c_str());

#ifdef _USE_EXTENDED_PROTOCOL
            int CtrSepartor = 0;


            for(int Index = 0; Index < Converted.size(); Index++)
                {
                if( Converted[Index] == L',' )
                    CtrSepartor++;
                }

            // Check protocol
            if( CtrSepartor == 4 )
                {
                uint32_t Pos = 0;

                // Read command
                pCmd = GetNextLabel(Converted.c_str(), &Pos);
                HASSERT(pCmd != 0);

                // Read attribute
                pAttrib = GetNextLabel(Converted.c_str(), &Pos);
                HASSERT(pAttrib != 0);
                Attrib = BeStringUtilities::Wtoi(pAttrib);

                // Read name
                pName = GetNextLabel(Converted.c_str(), &Pos);
                HASSERT(pName != 0);

                // Read size
                pSize = GetNextLabel(Converted.c_str(), &Pos);
                HASSERT(pSize != 0);
                Size = BeStringUtilities::Wtoi(pSize);

                // Read date
                pDate = GetNextLabel(Converted.c_str(), &Pos);
                HASSERT(pDate != 0);
                DateTime = pDate;

                if( m_Command != pCmd )
                    Status = false;
                }
            else if( CtrSepartor == 2 )
                {
#endif
                size_t Pos = 0;

                // Read command
                pCmd = GetNextLabel(Converted.c_str(), &Pos);
                HASSERT(pCmd != 0);

                if( m_Command == WString(pCmd) )
                    {
                    // Read attribute
                    pAttrib= GetNextLabel(Converted.c_str(), &Pos);
                    HASSERT(pAttrib != 0);
                    Attrib = BeStringUtilities::Wtoi(pAttrib);

                    // Read name
                    // Don't use the GetNextLabel beceause
                    // file name can contains comma. The file name
                    // is from the Pos to the end of the string
                    WString NameTemp = Converted.substr(Pos);

                    pName = new WChar[NameTemp.size() + 1];
                    HASSERT(pName != 0);

                    wcscpy(pName, NameTemp.c_str());
                    }
                else
                    Status = false;
#ifdef _USE_EXTENDED_PROTOCOL
                }
            else
                Status = false;
#endif

            if( Status )
                {
                bool  IsCompliant = false;

                // If entry is a directory
                // it is compliant, otherwise
                // check file extension
                if( Attrib & HFC_FILE_ATTRIBUTES_LOGICAL_PATH ||
                    Attrib & HFC_FILE_ATTRIBUTES_SUBDIR )
                    IsCompliant = true;
                else
                    IsCompliant = IsEntryCompliant(WString(pName));

                if( IsCompliant )
                    {
                    HFSDirectoryListItem Entry(pName.get(),
                                               Attrib,
                                               Size,
                                               DateTime);

                    m_EntryList.push_back(Entry);
                    }
                }
            }
        NextString = NextString.substr(EndPos + 2, string::npos);
        EndPos     = NextString.find("\r\n");
        }

    return Status;
    }

//-----------------------------------------------------------------
// bool VerifyExtendedProtocol
//
// Check the server compliance for the extended protocol.
//
//-----------------------------------------------------------------
inline bool HFSInternetLister::VerifyExtendedProtocol()
    {
    bool   Done    = false;
    bool   Status  = false;
    bool   Timeout = false;
    time_t  Start;
    time_t  Current;
    string  Responce;
    WString Request;

    HFCPtr<HFCInternetConnection> pConnection = (HFCPtr<HFCInternetConnection>&)(m_pConnection);

    try
        {
        // Be sure that the connection is alive
        if( pConnection->ValidateConnect(5000) )
            {
            Request  = m_Command;
            Request += L"=";
            Request += L"\\";
            Request += L"\r\n";
            Utf8String utf8Str;
            BeStringUtilities::WCharToUtf8(utf8Str,Request);

            // Send the list request
            pConnection->Send((const Byte*)utf8Str.c_str(), utf8Str.size());

            // Wait the server response
            time(&Start);
            while( pConnection->IsConnected() && !Done && !Timeout )
                {
                size_t InByte = pConnection->WaitDataAvailable(100);
                if( InByte > 0 )
                    {
                    // Each time we receive some
                    // bytes, restart the timer
                    time(&Start);

                    HArrayAutoPtr<Byte> pBuffer(new Byte[InByte]);

                    pConnection->Receive(pBuffer, InByte);

                    Responce.append((char*)pBuffer.get(), InByte);

                    // UNICODE note : can find \r\n directly in UTF8
                    if( (Responce.size() == 2 && Responce.find("\r\n") != string::npos) ||
                        Responce.find("\r\n\r\n") != string::npos )
                        Done = true;
                    }
                else
                    {
                    // Check for a timeout
                    time(&Current);
                    if( difftime(Current, Start) > s_MAX_WAITTIME_LIST )
                        Timeout = true;
                    }
                }

            // Must have received
            // a complete response
            if( Responce.size() > 0 && Done )
                {
                // UNICODE note : can find "error/" directly in UTF8
                string CheckError = Responce.substr(0, 6);
                if( BeStringUtilities::Stricmp(CheckError.c_str(), "error/") == 0 )
                    Status = false;
                else
                    Status = true;
                }
            else
                Status = false;

            pConnection->Disconnect();
            }
        }
    catch(HFCInternetConnectionException&)
        {
        Status = false;
        }

    return Status;
    }

//-----------------------------------------------------------------
// bool SupportFileDate
//
// Return true if the date field is valid, false if not.
//
//-----------------------------------------------------------------
inline bool HFSInternetLister::SupportFileDate() const
    {
    return m_UseExtendedProtocol;
    }

//-----------------------------------------------------------------
// bool SupportFileSize
//
// Return true if the file size field is valid, false if not.
//
//-----------------------------------------------------------------
inline bool HFSInternetLister::SupportFileSize() const
    {
    return m_UseExtendedProtocol;
    }

END_IMAGEPP_NAMESPACE