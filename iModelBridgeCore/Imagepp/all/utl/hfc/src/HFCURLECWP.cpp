//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURLECWP.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLECWP
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLECWP.h>

// Constants

static const WString s_DefaultPort(L"80");

//URLECWPCreator g_URLECWPCreator;
//:Ignore

// This is the creator that registers itself in the scheme list.
struct URLECWPCreator : public HFCURL::Creator
    {
    URLECWPCreator()
        {
        HFCURLECWP::GetSchemeList().insert(HFCURLECWP::SchemeList::value_type(HFCURLECWP::s_SchemeName(), this));
        }
    virtual HFCURL* Create(const WString& pi_URL) const
        {
        return new HFCURLECWP(pi_URL);
        }
    } g_URLECWPCreator;


//-----------------------------------------------------------------------------
// This little static function builds the URLPath portion of URL string from
// a path and a search expression.  Used by constructor.  Needed because
// compiler have difficulties with "?:" expressions and strings.

static WString BuildURLPathString(const WString& pi_Path, const WString& pi_SearchPart)
    {
    WString Result(pi_Path);
    if (!pi_SearchPart.empty())
        Result += L"?" + pi_SearchPart;
    return Result;
    }

//:End Ignore


/**----------------------------------------------------------------------------
 This constructor configures the object from the detached parts of the URL
 specification.

 Copy constructor and assignment operators are disabled for this class.

 @param pi_User       Constant reference to a string that contains the name
                      of the user, if a username has to be specified in this
                      URL, or an empty string.

 @param pi_Password   Constant reference to a string that contains the
                      password needed for the user specified in this URL,
                      if any, or an empty string.

 @param pi_Host       Constant reference to a string that contains the
                      specification of the host (IP address or domain name).

 @param pi_Port       Constant reference to a string that contains the port
                      number to use onto the host, if required for this URL,
                      or an empty string.

 @param pi_Path       Constant reference to a string that contains the path
                      to the resource on the host, if the host is not the
                      resource itself.  Must not begin by a slash or
                      blackslash.  If no path, the string must be empty.

 @param pi_SearchPart Constant reference to a string that contains the search
                      expression to append to the path.  Must not begin by a
                      question mark.

 @inheritance This class is an instanciable one that correspond to one
              precise kind of URL and no child of it are expected to be
              defined.
-----------------------------------------------------------------------------*/
HFCURLECWP::HFCURLECWP(const WString& pi_User,
                       const WString& pi_Password,
                       const WString& pi_Host,
                       const WString& pi_Port,
                       const WString& pi_Path,
                       const WString& pi_SearchPart)
    : HFCURLCommonInternet(s_SchemeName(), pi_User, pi_Password, pi_Host, (pi_Port.empty() ? s_DefaultPort : pi_Port),
                           BuildURLPathString(pi_Path, pi_SearchPart)),
    m_Path(pi_Path), m_SearchPart(pi_SearchPart)
    {
    FREEZE_STL_STRING(m_Path);
    FREEZE_STL_STRING(m_SearchPart);
    }

/**----------------------------------------------------------------------------
 This constructor configure the object from the complete URL specification.

 Syntax: @c{ ecwp:[//][user[:password]@]host[:port][/[Path[?SearchPart]]]}

 @param pi_rURL Constant reference to a string that contains a complete URL
                specification.

 @inheritance This class is an instanciable one that correspond to one
              precise kind of URL and no child of it are expected to be
              defined.
-----------------------------------------------------------------------------*/
HFCURLECWP::HFCURLECWP(const WString& pi_pURL)
    : HFCURLCommonInternet(pi_pURL)
    {
    // if no port was detect in the URL, set it to the default
    if (GetPort().empty())
        m_Port = s_DefaultPort;

    // Get the URL path and search part
    WString::size_type QuestionMarkPos = GetURLPath().find(L'?');
    if (QuestionMarkPos != WString::npos)
        {
        m_SearchPart = GetURLPath().substr(QuestionMarkPos+1,
                                           GetURLPath().length() - QuestionMarkPos - 1);
        m_Path = GetURLPath().substr(0, QuestionMarkPos);
        }
    else
        m_Path = GetURLPath();

    FREEZE_STL_STRING(m_Path);
    FREEZE_STL_STRING(m_SearchPart);
    }

/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HFCURLECWP::~HFCURLECWP()
    {
    // Nothing to do here.
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
WString HFCURLECWP::GetURL() const
    {
    WString Result(s_SchemeName() + L"://");
    if (!GetUser().empty())
        {
        Result += GetUser();
        if (!GetPassword().empty())
            {
            Result += L":";
            Result += GetPassword();
            }
        Result += L"@";
        }
    Result += GetHost();
    if (!GetPort().empty())
        {
        Result += L":";
        Result += GetPort();
        }
    Result += L"/";
    Result += GetPath();
    if (!GetSearchPart().empty())
        {
        Result += L"?";
        Result += GetSearchPart();
        }
    return Result;
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
bool HFCURLECWP::HasPathTo(HFCURL* pi_pURL)
    {
    if (HFCURLCommonInternet::HasPathTo(pi_pURL))
        {
        if (!(((HFCURLECWP*)pi_pURL)->GetSearchPart().empty())
            && !(GetSearchPart().empty()))
            {
            return true;
            }
        }
    return false;
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
WString HFCURLECWP::FindPathTo(HFCURL* pi_pDest)
    {
    HPRECONDITION(HasPathTo(pi_pDest));
    return FindPath(GetPath(), ((HFCURLECWP*)pi_pDest)->GetPath());
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
HFCURL* HFCURLECWP::MakeURLTo(const WString& pi_Path)
    {
    return new HFCURLECWP(GetUser(), GetPassword(), GetHost(), GetPort(),
                          AddPath(GetPath(), pi_Path), WString());
    }


#ifdef __HMR_DEBUG_MEMBER
//-----------------------------------------------------------------------------
// Test routine
//-----------------------------------------------------------------------------
void HFCURLECWP::PrintState() const
    {
#   define out wcout

    out << "Object of type HFCURLECWP" << endl;
    out << "User     = " << GetUser() << endl;
    out << "Password = " << GetPassword() << endl;
    out << "Host     = " << GetHost() << endl;
    out << "Port     = " << GetPort() << endl;
    out << "Path     = " << GetPath() << endl;
    out << "Search   = " << GetSearchPart() << endl;
    out << "Standardized URL = " << GetURL() << endl;
    }
#endif
