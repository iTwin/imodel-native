//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURLHTTPBase.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLHTTPBase
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLHTTPBase.h>
#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HFCURLHTTPS.h>

// Constants
static const WString s_DefaultPort(L"80");
static const WString s_DefaultSecurePort(L"443");

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
HFCURLHTTPBase::HFCURLHTTPBase(const WString& pi_User,
                               const WString& pi_Password,
                               const WString& pi_Host,
                               const WString& pi_Port,
                               const WString& pi_Path,
                               const WString& pi_SearchPart,
                               bool    pi_IsHTTPURL)
: HFCURLCommonInternet((pi_IsHTTPURL ? HFCURLHTTP::s_SchemeName() : HFCURLHTTPS::s_SchemeName()), 
                           pi_User,
                           pi_Password,
                           pi_Host,
                           (pi_Port.empty() ? ((pi_IsHTTPURL) ? s_DefaultPort : s_DefaultSecurePort) : pi_Port),
                           BuildURLPathString(pi_Path, pi_SearchPart)),
    m_Path(pi_Path),
    m_SearchPart(pi_SearchPart),
    m_IsHTTPURL(pi_IsHTTPURL)
    {
    FREEZE_STL_STRING(m_Path);
    FREEZE_STL_STRING(m_SearchPart);
    }

/**----------------------------------------------------------------------------
 This constructor configure the object from the complete URL specification.

 Syntax: @c{ http:[//][user[:password]@]host[:port][/[Path[?SearchPart]]]}

 @param pi_rURL Constant reference to a string that contains a complete URL
                specification.

 @inheritance This class is an instanciable one that correspond to one
              precise kind of URL and no child of it are expected to be
              defined.
-----------------------------------------------------------------------------*/
HFCURLHTTPBase::HFCURLHTTPBase(const WString& pi_pURL,
                               bool    pi_IsHTTPURL)
    : HFCURLCommonInternet(pi_pURL),
      m_IsHTTPURL(pi_IsHTTPURL)
    {
    // if no port was detect in the URL, set it to the default
    if (GetPort().empty())
        m_Port = pi_IsHTTPURL ? s_DefaultPort : s_DefaultSecurePort;

    // Get the URL path and search part
    WString::size_type QuestionMarkPos = GetURLPath().find(L'?');
    if (QuestionMarkPos != WString::npos)
        {
        m_SearchPart = GetURLPath().substr(QuestionMarkPos+1,
                                           GetURLPath().length() - QuestionMarkPos - 1);

        // For some obscene reason, sometimes the search part contains the ending & which we do not want
        while (m_SearchPart.size() > 0 && m_SearchPart.substr(m_SearchPart.size()-1, 1) == L"&")
            m_SearchPart = m_SearchPart.substr(0, m_SearchPart.size() - 1);

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
HFCURLHTTPBase::~HFCURLHTTPBase()
    {
    // Nothing to do here.
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
WString HFCURLHTTPBase::GetURL() const
    {
    WString Result;

    if (m_IsHTTPURL)
        {
        Result = HFCURLHTTP::s_SchemeName() + L"://";
        }
    else
        {
        Result = HFCURLHTTPS::s_SchemeName() + L"://";
        }

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
bool HFCURLHTTPBase::HasPathTo(HFCURL* pi_pURL)
    {
    if (HFCURLCommonInternet::HasPathTo(pi_pURL))
        {
        if (!(((HFCURLHTTPBase*)pi_pURL)->GetSearchPart().empty())
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
WString HFCURLHTTPBase::FindPathTo(HFCURL* pi_pDest)
    {
    HPRECONDITION(HasPathTo(pi_pDest));
    return FindPath(GetPath(), ((HFCURLHTTPBase*)pi_pDest)->GetPath());
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
HFCURL* HFCURLHTTPBase::MakeURLTo(const WString& pi_Path)
    {
    HFCURL* pURL = 0;

    if (m_IsHTTPURL)
        {
        pURL = new HFCURLHTTP(GetUser(), GetPassword(), GetHost(), GetPort(),
                              AddPath(GetPath(), pi_Path), WString());
        }
    else
        {
        pURL = new HFCURLHTTPS(GetUser(), GetPassword(), GetHost(), GetPort(),
                               AddPath(GetPath(), pi_Path), WString());
        }

    return pURL;
    }

/**----------------------------------------------------------------------------
 Set an URL path which is encoded in UTF8 and escaped.

 @see GetURLPath
-----------------------------------------------------------------------------*/
void HFCURLHTTPBase::SetUTF8EscapedURLPath(const string* pi_pURLPath)
    {
    HFCURLCommonInternet::SetUTF8EscapedURLPath(pi_pURLPath);

    if (pi_pURLPath != 0)
        {
        // Get the URL path and search part
        string::size_type QuestionMarkPos = pi_pURLPath->find('?');
        if (QuestionMarkPos != string::npos)
            {
            m_UTF8EscapedSearchPart = pi_pURLPath->substr(QuestionMarkPos + 1,
                                                          pi_pURLPath->length() -
                                                          QuestionMarkPos - 1);
            }
        }
    else
        {
        m_UTF8EscapedSearchPart.clear();
        }
    }

#ifdef __HMR_DEBUG_MEMBER
//-----------------------------------------------------------------------------
// Test routine
//-----------------------------------------------------------------------------
void HFCURLHTTPBase::PrintState() const
    {
#   define out wcout

    if (m_IsHTTPURL)
        {
        out << "Object of type HFCURLHTTP" << endl;
        }
    else
        {
        out << "Object of type HFCURLHTTPS" << endl;
        }

    out << "User     = " << GetUser() << endl;
    out << "Password = " << GetPassword() << endl;
    out << "Host     = " << GetHost() << endl;
    out << "Port     = " << GetPort() << endl;
    out << "Path     = " << GetPath() << endl;
    out << "Search   = " << GetSearchPart() << endl;
    out << "Standardized URL = " << GetURL() << endl;
    }
#endif
