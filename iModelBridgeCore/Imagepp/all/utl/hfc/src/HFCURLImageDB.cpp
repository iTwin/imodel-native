//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURLImageDB.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLImageDB
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLImageDB.h>
#include <Imagepp/all/h/HFCException.h>

// imagedb:{//internalname/drivename[/dir.../dir/imagename]}
// imagedb:{//dbtype:ConnectString[?prompt string]/drivename[/dir.../dir/imagename]}


// This is the creator that registers itself in the scheme list.
struct URLImageDBCreator : public HFCURL::Creator
    {
    URLImageDBCreator()
        {
        HFCURLImageDB::GetSchemeList().insert(HFCURLImageDB::SchemeList::value_type(HFCURLImageDB::s_SchemeName(), this));
        }
    virtual HFCURL* Create(const WString& pi_URL) const
        {
        return new HFCURLImageDB(pi_URL);
        }
    } g_URLImageDBCreator;


//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the
// scheme-specific part of the URL string.
//-----------------------------------------------------------------------------
HFCURLImageDB::HFCURLImageDB(const WString& pi_rURL)
    : HFCURL(pi_rURL)
    {
    HPRECONDITION(!pi_rURL.empty());

    if (BeStringUtilities::Wcsicmp(GetSchemeType().c_str(), s_SchemeName().c_str()) != 0)
        throw(HFCUnknownException());

    WString SchemeSpecificPart = GetSchemeSpecificPart();

    // must be start with "//"
    if (SchemeSpecificPart.length() < 2)
        throw(HFCUnknownException());

    if (wcscmp(SchemeSpecificPart.substr(0, 2).c_str(), L"//") != 0)
        throw(HFCUnknownException());

    WString::size_type FirstPos;
    WString::size_type NextPos;

    if ((FirstPos = SchemeSpecificPart.find_first_of(L"/", 2)) == WString::npos)
        throw(HFCUnknownException()); // invalid URL

    WString Database = SchemeSpecificPart.substr(2, FirstPos - 2);

    // The fist value is the table name
    if ((NextPos = SchemeSpecificPart.find_first_of(L"/", ++FirstPos)) == WString::npos)
        m_DriveName = SchemeSpecificPart.substr(FirstPos);
    else
        {
        m_DriveName = SchemeSpecificPart.substr(FirstPos, NextPos - FirstPos);
        m_Path = SchemeSpecificPart.substr(NextPos);
        }

    // Database
    if (Database.length() == 0)
        throw(HFCUnknownException()); // we need a database

    // Check if we have an InternalName
    if ((FirstPos = Database.find_first_of(L":")) == WString::npos)
        m_InternalName = Database;
    else
        {
        m_DatabaseType = Database.substr(0, FirstPos);

        // Extract the Usr
        if ((NextPos = Database.find_first_of(L":", ++FirstPos)) == WString::npos)
            throw(HFCUnknownException()); // we need 2 ":" for Usr and Pwd

        m_Usr = Database.substr(FirstPos, NextPos - FirstPos);

        // Extract the Pwd
        FirstPos = NextPos + 1;

        if ((NextPos = Database.find_first_of(L"@", FirstPos)) != WString::npos)
            NextPos = Database.find_first_of(L"?", FirstPos);

        m_Pwd = Database.substr(FirstPos, NextPos - FirstPos);

        // Check if we have a server name
        FirstPos = NextPos + 1;
        if (Database[NextPos] == L'@')
            {
            if ((NextPos = Database.find_first_of(L"?", FirstPos)) != WString::npos)
                m_Server = Database.substr(FirstPos, FirstPos - NextPos);
            else
                m_Server = Database.substr(FirstPos);
            }

        // Extract the prompt string
        if (NextPos != WString::npos)
            m_PromptString = Database.substr(NextPos + 1);
        }

    if (m_Path.empty())
        m_Path = L"/";

    FREEZE_STL_STRING(m_InternalName);
    FREEZE_STL_STRING(m_DatabaseType);
    FREEZE_STL_STRING(m_Usr);
    FREEZE_STL_STRING(m_Pwd);
    FREEZE_STL_STRING(m_Server);
    FREEZE_STL_STRING(m_DriveName);
    FREEZE_STL_STRING(m_Path);
    FREEZE_STL_STRING(m_PromptString);
    }

//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the
// scheme-specific part of the URL string.
//-----------------------------------------------------------------------------
HFCURLImageDB::HFCURLImageDB(const WString& pi_rInternalName,
                             const WString& pi_rDriveName,
                             const WString& pi_rPath)
    : HFCURL(s_SchemeName(), L"//" + pi_rInternalName + L"/" + pi_rDriveName + pi_rPath),
      m_DriveName(pi_rDriveName),
      m_InternalName(pi_rInternalName),
      m_Path(pi_rPath)
    {
    HPRECONDITION(!pi_rInternalName.empty());
    HPRECONDITION(!pi_rDriveName.empty());

    if (!m_Path.empty() && m_Path[0] != L'/')
        throw(HFCUnknownException());

    if (m_Path.empty())
        m_Path = L"/";

    FREEZE_STL_STRING(m_InternalName);
    FREEZE_STL_STRING(m_DatabaseType);
    FREEZE_STL_STRING(m_Usr);
    FREEZE_STL_STRING(m_Pwd);
    FREEZE_STL_STRING(m_Server);
    FREEZE_STL_STRING(m_DriveName);
    FREEZE_STL_STRING(m_Path);
    FREEZE_STL_STRING(m_PromptString);
    }


//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the
// scheme-specific part of the URL string.
//-----------------------------------------------------------------------------
HFCURLImageDB::HFCURLImageDB(const WString& pi_rDatabaseType,
                             const WString& pi_rUsrName,
                             const WString& pi_rPwd,
                             const WString& pi_rServerName,
                             const WString& pi_rPromptString,
                             const WString& pi_rDriveName,
                             const WString& pi_rPath)
    : m_DatabaseType(pi_rDatabaseType),
      m_Usr(pi_rUsrName),
      m_Pwd(pi_rPwd),
      m_Server(pi_rServerName),
      m_PromptString(pi_rPromptString),
      m_DriveName(pi_rDriveName),
      m_Path(pi_rPath)
    {
    HPRECONDITION(!pi_rDatabaseType.empty());
    HPRECONDITION(!pi_rDriveName.empty());

    if (m_Path.empty())
        m_Path = L"/";

    FREEZE_STL_STRING(m_InternalName);
    FREEZE_STL_STRING(m_DatabaseType);
    FREEZE_STL_STRING(m_Usr);
    FREEZE_STL_STRING(m_Pwd);
    FREEZE_STL_STRING(m_Server);
    FREEZE_STL_STRING(m_DriveName);
    FREEZE_STL_STRING(m_Path);
    FREEZE_STL_STRING(m_PromptString);
    }

//-----------------------------------------------------------------------------
// The destructor for this class.
//-----------------------------------------------------------------------------
HFCURLImageDB::~HFCURLImageDB()
    {
    // Nothing to do here.
    }

//-----------------------------------------------------------------------------
// public
// GetURL
// Returns the standardized and complete URL string.
//-----------------------------------------------------------------------------
WString HFCURLImageDB::GetURL() const
    {
    WString URL = (s_SchemeName() + L"://" + m_InternalName + L"/" + m_DriveName + m_Path);

    return URL;
    }


//-----------------------------------------------------------------------------
// HasPathTo
// Returns true only if a relative path can be calculated from this URL and the
// specified one.
//-----------------------------------------------------------------------------
bool HFCURLImageDB::HasPathTo(HFCURL* pi_pURL)
    {
    return false;
    }

//-----------------------------------------------------------------------------
// public
// FindPathTo
//-----------------------------------------------------------------------------
WString HFCURLImageDB::FindPathTo(HFCURL* pi_pDest)
    {
    HPRECONDITION(0);
    return WString();
    }


//-----------------------------------------------------------------------------
// public
// MakeURLTo
//-----------------------------------------------------------------------------
HFCURL* HFCURLImageDB::MakeURLTo(const WString& pi_Path)
    {
    HASSERT(0);
    return 0;
    }




//-----------------------------------------------------------------------------
// public
// HasInternalName
//-----------------------------------------------------------------------------
bool HFCURLImageDB::HasInternalName() const
    {
    return !m_InternalName.empty();
    }

//-----------------------------------------------------------------------------
// public
// HasConnectString
//-----------------------------------------------------------------------------
bool HFCURLImageDB::HasConnectString() const
    {
    return !HasInternalName();
    }


//-----------------------------------------------------------------------------
// public
// GetInternalName
//-----------------------------------------------------------------------------
const WString& HFCURLImageDB::GetInternalName() const
    {
    HPRECONDITION(HasInternalName());

    return m_InternalName;
    }

//-----------------------------------------------------------------------------
// public
// GetUsr
//
// Avalaible only if we have a connect string
//-----------------------------------------------------------------------------
const WString& HFCURLImageDB::GetUsr() const
    {
    HPRECONDITION(HasConnectString());

    return m_Usr;
    }

//-----------------------------------------------------------------------------
// public
// GetPwd
//
// Avalaible only if we have a connect string
//-----------------------------------------------------------------------------
const WString& HFCURLImageDB::GetPwd() const
    {
    HPRECONDITION(HasConnectString());

    return m_Pwd;
    }

//-----------------------------------------------------------------------------
// public
// GetServer
//
// Avalaible only if we have a connect string
//-----------------------------------------------------------------------------
const WString& HFCURLImageDB::GetServer() const
    {
    HPRECONDITION(HasConnectString());

    return m_Server;
    }

//-----------------------------------------------------------------------------
// public
// GetPromptString
//
// Avalaible only if we have a connect string
//-----------------------------------------------------------------------------
const WString& HFCURLImageDB::GetPromptString() const
    {
    HPRECONDITION(HasConnectString());

    return m_PromptString;
    }

//-----------------------------------------------------------------------------
// public
// GetDriveName
//-----------------------------------------------------------------------------
const WString& HFCURLImageDB::GetDriveName() const
    {
    return m_DriveName;
    }

//-----------------------------------------------------------------------------
// public
// GetPath
//-----------------------------------------------------------------------------
const WString& HFCURLImageDB::GetPath() const
    {
    return m_Path;
    }







#ifdef __HMR_DEBUG_MEMBER
//-----------------------------------------------------------------------------
// Test routine
//-----------------------------------------------------------------------------
void HFCURLImageDB::PrintState() const
    {
#   define out wcout

    out << "Object of type HFCURLImageDB" << endl;
    out << "InternalName = " << GetInternalName() << endl;
    out << "Path = " << GetPath() << endl;
    out << "Standardized URL = " << GetURL() << endl;
    }
#endif
