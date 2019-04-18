//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLCommonInternet
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HFCURLCommonInternet.h>
#include <ImagePP/all/h/HFCEncodeDecodeASCII.h>

//:Ignore
//-----------------------------------------------------------------------------
// Little static function that is used by constructor

static Utf8String BuildHostString(const Utf8String& pi_User, const Utf8String& pi_Password,
                               const Utf8String& pi_Host, const Utf8String& pi_Port)
    {
    Utf8String Result;
    if (!pi_User.empty())
        {
        Result += pi_User;
        if (!pi_Password.empty())
            {
            Result += ":";
            Result += pi_Password;
            }
        Result += "@";
        }
    Result += pi_Host;
    if (!pi_Port.empty())
        {
        Result += ":";
        Result += pi_Port;
        }
    return Result;
    }

//:End Ignore

/**----------------------------------------------------------------------------
 This constructor configures the object from the detached parts of the
 scheme-specific part of the URL string.

 This class is an abstract one and is not instanciable alone.

 Copy constructor and assignment operators are disabled for this class.

 @param pi_SchemeType  Constant reference to a string that contains the name
                       of a scheme type, without the colon ":".

 @param pi_User        Constant reference to a string that contains the name
                       of the user, if a username has to be specified in this
                       URL, or an empty string.

 @param pi_Password    Constant reference to a string that contains the
                       password needed for the user specified in this URL,
                       if any, or an empty string.

 @param pi_Host        Constant reference to a string that contains the
                       specification of the host (IP address or domain name).

 @param pi_Port        Constant reference to a string that contains the port
                       number to use onto the host, if required for this URL,
                       or an empty string.

 @param pi_URLPath     Constant reference to a string that contains the path
                       to the resource on the host, if the host is not the
                       resource itself.  Must not begin by a slash or blackslash.
                       If no path, the string must be empty.
-----------------------------------------------------------------------------*/
HFCURLCommonInternet::HFCURLCommonInternet(const Utf8String& pi_SchemeType,
                                           const Utf8String& pi_User,
                                           const Utf8String& pi_Password,
                                           const Utf8String& pi_Host,
                                           const Utf8String& pi_Port,
                                           const Utf8String& pi_URLPath)
    : HFCURL(pi_SchemeType,
             Utf8String("//")
             + BuildHostString(pi_User, pi_Password, pi_Host, pi_Port)
             + Utf8String("/") + pi_URLPath),
    m_User(pi_User), m_Password(pi_Password), m_Host(pi_Host),
    m_Port(pi_Port), m_URLPath(pi_URLPath)
    {
    }

void HFCURLCommonInternet::SplitPath(const Utf8String& pi_rURL,
                                     Utf8String*       po_pScheme,
                                     Utf8String*       po_pHost,
                                     Utf8String*       po_pPort,
                                     Utf8String*       po_pUser,
                                     Utf8String*       po_pPassword,
                                     Utf8String*       po_pPath)
    {
    // initialize all string
    *po_pScheme = "";
    *po_pHost = "";
    *po_pPort = "";
    *po_pUser = "";
    *po_pPassword = "";
    *po_pPath = "";

    Utf8String SchemeSpecificPart;
    Utf8String::size_type ColonPos = pi_rURL.find(':');
    if (ColonPos != Utf8String::npos)
        {
        *po_pScheme = pi_rURL.substr(0, ColonPos);
        if (HFCURL::GetSchemeList().find(*po_pScheme) != HFCURL::GetSchemeList().end())
            {
            // the scheme type must be
            SchemeSpecificPart = pi_rURL.substr(ColonPos+1,
                                                pi_rURL.length() - ColonPos - 1);
            }
        else
            {
            SchemeSpecificPart = pi_rURL;
            *po_pScheme = "";
            }
        }
    else
        {
        SchemeSpecificPart = pi_rURL;
        }

    bool HasDoubleSlash = false;
    if (SchemeSpecificPart.length() > 2)
        {
        HasDoubleSlash = ((SchemeSpecificPart.substr(0,2) == "//") ||
                          (SchemeSpecificPart.substr(0,2) == "\\\\"));
        }

    // First : find the "user:password" part by locating the "@"
    Utf8String::size_type ArobasPos = Utf8String::npos;
    if (SchemeSpecificPart.size() > 0)
        ArobasPos = SchemeSpecificPart.find('@');

    // If there is a user specification, check for ":" before "@" to know
    // if there is a password.
    Utf8String::size_type PasswordColonPos = Utf8String::npos;
    if (ArobasPos != Utf8String::npos)
        {
        PasswordColonPos = SchemeSpecificPart.find(':');
        if (PasswordColonPos != Utf8String::npos)
            {
            if (PasswordColonPos > ArobasPos)
                {
                PasswordColonPos = Utf8String::npos;
                ArobasPos = Utf8String::npos;
                }
            }
        else
            {
            // Since there are no password, the arobas must not be the authentication part but something more specific part.
            ArobasPos = Utf8String::npos;
            }

        }

    // Find the first single slash to isolate the URL-path.  If none, there is
    // no url-path.
    Utf8String::size_type SingleSlashPos =
        SchemeSpecificPart.find_first_of("\\/", HasDoubleSlash ? 2 : 0);

    if (ArobasPos > SingleSlashPos)
        {
        // If this occurs then the arobas is not the password arobas
        ArobasPos = Utf8String::npos;
        PasswordColonPos = Utf8String::npos;
        }

    // Then look at the possible port number by looking for ":" after the "@"
    // if a user was specified, of just find the ":" if no user, before the
    // url-path specification.
    Utf8String::size_type PortColonPos;
    if (ArobasPos != Utf8String::npos)
        PortColonPos = SchemeSpecificPart.find(':', ArobasPos+1);
    else
        PortColonPos = SchemeSpecificPart.find(':');

    if ((PortColonPos != Utf8String::npos) && (SingleSlashPos != Utf8String::npos))
        if (PortColonPos > SingleSlashPos)
            PortColonPos = Utf8String::npos;

    // Now we have found all the components, so we extract them!
    Utf8String TempString(SchemeSpecificPart);
    if (HasDoubleSlash)  // Removing the double slash, if any.
        {
        TempString.erase(0, 2);
        if (SingleSlashPos != Utf8String::npos)
            SingleSlashPos -= 2;
        if (PortColonPos != Utf8String::npos)
            PortColonPos -= 2;
        if (ArobasPos != Utf8String::npos)
            ArobasPos -= 2;
        if (PasswordColonPos != Utf8String::npos)
            PasswordColonPos -= 2;
        }

    if (SingleSlashPos != Utf8String::npos)    // Splitting the URL-path from the rest
        {
        *po_pPath = TempString.substr(SingleSlashPos+1, TempString.length() - 1 - SingleSlashPos);
        TempString.erase(SingleSlashPos, TempString.length() - SingleSlashPos);
        }
    if (PortColonPos != Utf8String::npos)   // Splitting the port number from the rest
        {
        *po_pPort = TempString.substr(PortColonPos + 1, TempString.length() - 1 - PortColonPos);
        TempString.erase(PortColonPos, TempString.length() - PortColonPos);
        }
    if (ArobasPos != Utf8String::npos)  // Splitting the host from the user spec.
        {
        *po_pHost = TempString.substr(ArobasPos + 1, TempString.length() - 1 - ArobasPos);
        if (PasswordColonPos != Utf8String::npos) // Getting the password if any
            {
            WString passwordW(TempString.substr(PasswordColonPos + 1, ArobasPos - PasswordColonPos - 1).c_str(), BentleyCharEncoding::Utf8);
            HFCEncodeDecodeASCII::EscapeToASCII (passwordW);
            po_pPassword->Assign(passwordW.c_str());

            WString userW(TempString.substr(0, PasswordColonPos).c_str(), BentleyCharEncoding::Utf8);
            HFCEncodeDecodeASCII::EscapeToASCII (userW);
            po_pUser->Assign(userW.c_str());
            }
        else
            {
            WString userW(TempString.substr(0, ArobasPos).c_str(), BentleyCharEncoding::Utf8);
            HFCEncodeDecodeASCII::EscapeToASCII(userW);
            po_pUser->Assign(userW.c_str());
            }
        }
    else
        *po_pHost = TempString;
    }

/**----------------------------------------------------------------------------
 This constructor configures the object from the complete URL specification.
 The URL syntax is:

 @c{<scheme-type>[//][user[:password]@]host[:port][/[URLPath]]}

 Slashes are replaceable by backslaches; the slash just before the path is
 @u{NOT} kept as being in the path.

 @param pi_URL Constant reference to a string that contains a complete URL
               specification.
-----------------------------------------------------------------------------*/
HFCURLCommonInternet::HFCURLCommonInternet(const Utf8String& pi_URL)
    : HFCURL(pi_URL)
    {
    // Is there a double-slash?

    bool HasDoubleSlash = false;
    if (GetSchemeSpecificPart().length() > 2)
        {
        HasDoubleSlash = ((GetSchemeSpecificPart().substr(0,2) == "//") ||
                          (GetSchemeSpecificPart().substr(0,2) == "\\\\"));
        }

    // First : find the "user:password" part by locating the "@"

    Utf8String::size_type ArobasPos = Utf8String::npos;
    if (GetSchemeSpecificPart().size() > 0)
        ArobasPos = GetSchemeSpecificPart().find('@');

    // If there is a user specification, check for ":" before "@" to know
    // if there is a password.

    Utf8String::size_type PasswordColonPos = Utf8String::npos;
    if (ArobasPos != Utf8String::npos)
        {
        PasswordColonPos = GetSchemeSpecificPart().find(':');
        if (PasswordColonPos != Utf8String::npos)
            {
            if (PasswordColonPos > ArobasPos)
                {
                PasswordColonPos = Utf8String::npos;
                ArobasPos = Utf8String::npos;
                }
            }
        else
            {
            // There is no password clause ... the Arobas portion must be part of the remainder.
            ArobasPos = Utf8String::npos;
            }
        }

    // Find the first single slash to isolate the URL-path.  If none, there is
    // no url-path.

    Utf8String::size_type SingleSlashPos =
        GetSchemeSpecificPart().find_first_of("\\/", HasDoubleSlash ? 2 : 0);

    if (ArobasPos > SingleSlashPos)
        {
        // If this occurs then the arobas is not the password arobas
        ArobasPos = Utf8String::npos;
        PasswordColonPos = Utf8String::npos;
        }

    // Then look at the possible port number by looking for ":" after the "@"
    // if a user was specified, of just find the ":" if no user, before the
    // url-path specification.

    Utf8String::size_type PortColonPos;
    if (ArobasPos != Utf8String::npos)
        PortColonPos = GetSchemeSpecificPart().find(':', ArobasPos+1);
    else
        PortColonPos = GetSchemeSpecificPart().find(':');
    if ((PortColonPos != Utf8String::npos) && (SingleSlashPos != Utf8String::npos))
        if (PortColonPos > SingleSlashPos)
            PortColonPos = Utf8String::npos;

    // Now we have found all the components, so we extract them!

    Utf8String TempString(GetSchemeSpecificPart());
    if (HasDoubleSlash)  // Removing the double slash, if any.
        {
        TempString.erase(0, 2);
        if (SingleSlashPos != Utf8String::npos)
            SingleSlashPos -= 2;
        if (PortColonPos != Utf8String::npos)
            PortColonPos -= 2;
        if (ArobasPos != Utf8String::npos)
            ArobasPos -= 2;
        if (PasswordColonPos != Utf8String::npos)
            PasswordColonPos -= 2;
        }
    if (SingleSlashPos != Utf8String::npos)    // Splitting the URL-path from the rest
        {
        m_URLPath = TempString.substr(SingleSlashPos+1, TempString.length() - 1 - SingleSlashPos);
        TempString.erase(SingleSlashPos, TempString.length() - SingleSlashPos);
        }
    if (PortColonPos != Utf8String::npos)   // Splitting the port number from the rest
        {
        m_Port = TempString.substr(PortColonPos + 1, TempString.length() - 1 - PortColonPos);
        TempString.erase(PortColonPos, TempString.length() - PortColonPos);
        }
    if (ArobasPos != Utf8String::npos)  // Splitting the host from the user spec.
        {
        m_Host = TempString.substr(ArobasPos + 1, TempString.length() - 1 - ArobasPos);
        if (PasswordColonPos != Utf8String::npos) // Getting the password if any
            {
            m_Password = TempString.substr(PasswordColonPos + 1, ArobasPos - PasswordColonPos - 1);
            m_User = TempString.substr(0, PasswordColonPos);
            }
        else
            m_User = TempString.substr(0, ArobasPos);
        }
    else
        m_Host = TempString;
    }

/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HFCURLCommonInternet::~HFCURLCommonInternet()
    {
    // Nothing to do here.
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Returns true only if a relative path can be calculated from this URL and the
 specified one. At this level some generic checks are done : the host and port
 must be the same.

 @see HFCURL
-----------------------------------------------------------------------------*/
bool HFCURLCommonInternet::HasPathTo(HFCURL* pi_pURL)
    {
    if (HFCURL::HasPathTo(pi_pURL))
        {
        if ((GetHost() == ((HFCURLCommonInternet*)pi_pURL)->GetHost())
            && (GetPort() == ((HFCURLCommonInternet*)pi_pURL)->GetPort()))
            return true;
        }
    return false;
    }

/**----------------------------------------------------------------------------
 Set an URL path which is encoded in UTF8 and escaped.

 @see GetURLPath
-----------------------------------------------------------------------------*/
void HFCURLCommonInternet::SetUTF8EscapedURLPath(const string* pi_pURLPath)
    {
    if (pi_pURLPath == 0)
        {
        m_UTF8EscapedURLPath.clear();
        }
    else
        {
        m_UTF8EscapedURLPath = *pi_pURLPath;
        }
    }

/**----------------------------------------------------------------------------
 Encode the value of a parameter passed to an URL in accordance to the
 rfc2396 specification. The string passed in parameter is assumed to be encoded
 according to a byte encoding algorithm (i.e. : UTF-8).

 @see HFCURL
-----------------------------------------------------------------------------*/
string HFCURLCommonInternet::EscapeURLParamValue(const string& pi_rURLPart)
    {
    static string          UnreservedChars = "-_.!~*'()";

    string                 EncodedURL;
    string::const_iterator CharIter    = pi_rURLPart.begin();
    string::const_iterator CharIterEnd = pi_rURLPart.end();
    char                   EscapedChar[6];

    while (CharIter != CharIterEnd)
        {
        //Escaped every characters which are not alphanumeric characters and
        //are not reserved.
        if ((isalnum((Byte)*CharIter) == 0) &&
            (UnreservedChars.find_first_of(*CharIter) == string::npos))
            {
            int32_t charCount = BeStringUtilities::Snprintf(EscapedChar, sizeof(EscapedChar), "%x", (Byte)*CharIter);
            HASSERT(charCount > 0);

            if ((isascii(EscapedChar[0]) != 0) && (islower(EscapedChar[0]) != 0))
                {
                EscapedChar[0] = (char)toupper(EscapedChar[0]);
                }

            if ((isascii(EscapedChar[1]) != 0) && (islower(EscapedChar[1]) != 0))
                {
                EscapedChar[1] = (char)toupper(EscapedChar[1]);
                }

            EncodedURL += "%" + string(EscapedChar);
            }
        else
            {
            EncodedURL += *CharIter;
            }

        CharIter++;
        }

    return EncodedURL;
    }
