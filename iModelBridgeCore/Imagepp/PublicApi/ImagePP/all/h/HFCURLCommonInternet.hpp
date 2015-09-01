//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLCommonInternet.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCURLCommonInternet
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the name of the
 user included in this URL, that is used when the resource restricts its
 access to specific users or group of users.  The user specification is
 often accompanied by the password specification, when a password is
 required to be recognized as the specified user.

 @return A constant reference to the string that contains the username
         specified in this URL.  May be empty if no username is required
         for resource to be located.

 @inheritance This method cannot be overriden.

 @see GetPassword
-----------------------------------------------------------------------------*/
inline const WString& HFCURLCommonInternet::GetUser() const
    {
    return m_User;
    }

/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the password included
 in this URL, that is needed for the user specified in this URL.

 @return A constant reference to the string that contains the password.
         That string may be empty if no password is required or no username
         was specified.

 @inheritance This method cannot be overriden.

 @see GetUser
-----------------------------------------------------------------------------*/
inline const WString& HFCURLCommonInternet::GetPassword() const
    {
    return m_Password;
    }

/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the host
 specification included in this URL, which is its IP address or domain
 name.  This does not include the path to the resource (when the
 resource is not the host itself).

 @return A constant reference to the string that contains the host
         specification.

 @inheritance This method cannot be overriden.

 @see GetURLPath
-----------------------------------------------------------------------------*/
inline const WString& HFCURLCommonInternet::GetHost() const
    {
    return m_Host;
    }

/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the port number
 included in this URL, to be used onto the host specified for this URL.

 @return A constant reference to the string that contains the port number.
         That string may be empty if no port number is required.

 @inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline const WString& HFCURLCommonInternet::GetPort() const
    {
    return m_Port;
    }

/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the path to the
 resource included in this URL, which is present when the host is not
 itself the resource designated by the URL.

 @return A constant reference to the string that contains the path to the
         resource, starting from the host specified in this URL.  It does not
         begin by a slash or a backslash.  May be empty if the host is itself
         the resource to be located.

 @see GetHost
-----------------------------------------------------------------------------*/
inline const WString& HFCURLCommonInternet::GetURLPath() const
    {
    return m_URLPath;
    }

/**----------------------------------------------------------------------------
 Returns an URL path which is encoded in UTF8 and escaped.

 @return An URL path encoded in UTF8 and escaped.

 @see GetURLPath
-----------------------------------------------------------------------------*/
inline const string& HFCURLCommonInternet::GetUTF8EscapedURLPath() const
    {
    return m_UTF8EscapedURLPath;
    }
END_IMAGEPP_NAMESPACE