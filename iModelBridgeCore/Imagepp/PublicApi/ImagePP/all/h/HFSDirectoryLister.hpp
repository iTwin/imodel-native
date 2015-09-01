//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSDirectoryLister.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFSDirectoryLister
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------
// HFSDirectoryLister
//
// Main constructor.
//
// HFCPtr<HFCConnection>& pi_pConnection:
//
//-----------------------------------------------------------------
inline HFSDirectoryLister::HFSDirectoryLister(HFCPtr<HFCConnection>& pi_pConnection)

    {
    m_pConnection = pi_pConnection;
    m_CurrentPath = L"\\";
    m_IsError     = false;
    }

//-----------------------------------------------------------------
// ~HFSDirectoryLister
//
// Destructor.
//
//-----------------------------------------------------------------
inline HFSDirectoryLister::~HFSDirectoryLister()
    {
    }

//-----------------------------------------------------------------
// const HFSDirectoryListItem& GetEntry
//
// Return the current entry.
//
//-----------------------------------------------------------------
inline const HFSDirectoryListItem& HFSDirectoryLister::GetEntry() const
    {
    return (*m_EntryListItr);
    }

//-----------------------------------------------------------------
// UInt32 GetCount
//
// Return the number of entry for the current directory.
//
//-----------------------------------------------------------------
inline uint32_t HFSDirectoryLister::GetCount() const
    {
    return (uint32_t)m_EntryList.size();
    }

//-----------------------------------------------------------------
// bool GotoFirst
//
// Set the lister to the first entry.
//
//-----------------------------------------------------------------
inline bool HFSDirectoryLister::GotoFirst()
    {
    m_EntryListItr = m_EntryList.begin();

    return (m_EntryListItr != m_EntryList.end());
    }

//-----------------------------------------------------------------
// bool GotoNext
//
// Set the lister to the next entry.
//
//-----------------------------------------------------------------
inline bool HFSDirectoryLister::GotoNext()
    {
    m_EntryListItr++;

    return (m_EntryListItr != m_EntryList.end());
    }

//-----------------------------------------------------------------
// void UnifyPath
//
// Change all '/' to '\\'
//
// string& pi_rString:
//
//-----------------------------------------------------------------
inline void HFSDirectoryLister::UnifyPath(WString& pi_rString)
    {
    WString::size_type Pos;

    // Convert all the '/' into '\'
    // Append a trailing '\' at the end
    Pos = pi_rString.find(L'/');
    while( Pos != string::npos )
        {
        pi_rString[Pos] = L'\\';
        Pos = pi_rString.find(L'/', Pos);
        }

    if( pi_rString.find_last_of(L'\\') != (pi_rString.size() - 1) )
        pi_rString += L'\\';
    }

//-----------------------------------------------------------------
// void PrepareForList
//
// Prepare the internal list for a new list command.
//
//-----------------------------------------------------------------
inline void HFSDirectoryLister::PrepareForList()
    {
    m_EntryList.erase(m_EntryList.begin(), m_EntryList.end());
    }

//-----------------------------------------------------------------
// void SetPattern
//
// Set the entry pattern. Pattern are separated by a semicolon ";"
//
// const string& pi_rPattern:
//
//-----------------------------------------------------------------
inline void HFSDirectoryLister::SetPattern(const WString& pi_rPattern)
    {
    WString::size_type Pos = 0;
    WString::size_type StartPos = Pos;

    // Erase the pattern list
    m_PatternList.erase(m_PatternList.begin(), m_PatternList.end());

    Pos = pi_rPattern.find(L";", StartPos);

    // Check the case there is only one pattern
    if( Pos == WString::npos && pi_rPattern.size() > 0 )
        {
        // If *.* is specified, the pattern list is empty
        if( pi_rPattern == WString(L"*.*") )
            {
            m_PatternList.erase(m_PatternList.begin(), m_PatternList.end());
            }
        else
            {
            // Keep only the pattern, eleminate
            // the * and . if found
            WString Pattern;
            WString::size_type DotPos = pi_rPattern.find(L'.');

            if( DotPos != WString::npos )
                Pattern = pi_rPattern.substr(DotPos + 1, pi_rPattern.size() - (DotPos + 1));
            else
                Pattern = pi_rPattern;

            m_PatternList.push_back(Pattern);
            }
        }

    while( Pos != WString::npos )
        {
        WString CurrentPattern = pi_rPattern.substr(StartPos, (Pos - StartPos));

        // If *.* is specified, the pattern list is empty
        if( CurrentPattern == WString(L"*.*") )
            {
            m_PatternList.erase(m_PatternList.begin(), m_PatternList.end());
            Pos = string::npos;
            }
        else
            {
            // Keep only the pattern, eleminate
            // the * and . if found
            WString::size_type DotPos = CurrentPattern.find(L'.');
            if( DotPos != WString::npos )
                CurrentPattern = CurrentPattern.substr(DotPos + 1, CurrentPattern.size() - (DotPos + 1));

            m_PatternList.push_back(CurrentPattern);

            StartPos = Pos + 1;
            Pos = pi_rPattern.find(L";", StartPos);
            }
        }
    }

//-----------------------------------------------------------------
// bool IsEntryCompliant
//
// Check if an entry match one of the pattern.
//
// const string& pi_rFileName:
//
//-----------------------------------------------------------------
inline bool HFSDirectoryLister::IsEntryCompliant(const WString& pi_rFileName)
    {
    bool Status = false;
    HFSPATTERN_LIST_ITR PatternItr;
    WString Extension = GetFileExtension(pi_rFileName);

    // If pattern list is empty,
    // accept all file
    if( m_PatternList.size() > 0 )
        {
        for(PatternItr  = m_PatternList.begin();
            PatternItr != m_PatternList.end();
            PatternItr++)
            {
            if( BeStringUtilities::Wcsicmp(Extension.c_str(), (*PatternItr).c_str()) == 0 )
                Status = true;
            }
        }
    else
        Status = true;

    return Status;
    }

//-----------------------------------------------------------------
// string GetFileExtension
//
// Extract and return the file extension.
//
// const string& pi_rFileName:
//
//-----------------------------------------------------------------
inline WString HFSDirectoryLister::GetFileExtension(const WString& pi_rFileName)
    {
    WString Extension;

    // Retrieve the file extention
    WString::size_type Pos = pi_rFileName.find_last_of(L".");

    // File has an extention?
    if( Pos != WString::npos )
        {
        ++Pos;
        Extension = pi_rFileName.substr(Pos, pi_rFileName.size() - Pos);
        }

    return Extension;
    }

//-----------------------------------------------------------------
// bool IsConnected
//
// Return the connection state.
//
//-----------------------------------------------------------------
inline bool HFSDirectoryLister::IsConnected() const
    {
    return m_pConnection->IsConnected();
    }

//-----------------------------------------------------------------
// bool IsError
//
// Return the error state.
//
//-----------------------------------------------------------------
inline bool HFSDirectoryLister::IsError() const
    {
    return m_IsError;
    }

//-----------------------------------------------------------------
// string GetServerURL
//
// Return the server URL.
//
//-----------------------------------------------------------------
inline WString HFSDirectoryLister::GetServerURL() const
    {
    return m_pConnection->GetServer();
    }

//-----------------------------------------------------------------
// char* GetNextLabel
//
// Break a string separated with comma into parts.
//
//-----------------------------------------------------------------
inline WChar* HFSDirectoryLister::GetNextLabel(const WChar* pi_pString,
                                                size_t*       pi_pPos)
    {
    const WChar* pStart = pi_pString + *pi_pPos;
    const WChar* pResult = wcspbrk(pStart, L",");

    if (!pResult)
        pResult = wcschr(pStart, 0);

    HASSERT(pResult != 0);

    WChar* pLabel = new WChar[pResult - pStart + 1];

    wcsncpy(pLabel, pStart, pResult - pStart);

    pLabel[pResult - pStart] = 0;

    *pi_pPos += (pResult - pStart + 1);

    return pLabel;
    }

//-----------------------------------------------------------------
// bool SupportFileDate
//
// Return true if the date field is valid, false if not.
//
//-----------------------------------------------------------------
inline bool HFSDirectoryLister::SupportFileDate() const
    {
    return false;
    }

//-----------------------------------------------------------------
// bool SupportFileSize
//
// Return true if the file size field is valid, false if not.
//
//-----------------------------------------------------------------
inline bool HFSDirectoryLister::SupportFileSize() const
    {
    return false;
    }

//-----------------------------------------------------------------
// const string& GetCurrentPath()
//
// Return the current path.
//
//-----------------------------------------------------------------
inline const WString& HFSDirectoryLister::GetCurrentPath() const
    {
    return m_CurrentPath;
    }

//-----------------------------------------------------------------
// string GetPatternList
//
// Return a string that contains all the pattern separated by ;
//
//-----------------------------------------------------------------
inline WString HFSDirectoryLister::GetPatternList()
    {
    WString Pattern;
    HFSPATTERN_LIST_ITR Itr;

    for(Itr = m_PatternList.begin();
        Itr != m_PatternList.end();
        Itr++)
        {
        Pattern += *Itr;
        Pattern += L";";
        }

    return Pattern;
    }

END_IMAGEPP_NAMESPACE