//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLHTTP
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCURLHTTP.h>

//:Ignore

// This is the creator that registers itself in the scheme list.
struct URLHTTPCreator : public HFCURL::Creator
    {
    URLHTTPCreator()
        {
        HFCURL::RegisterCreator(HFCURLHTTP::s_SchemeName(), this);
        }
    virtual HFCURL* Create(const Utf8String& pi_URL) const
        {
        return new HFCURLHTTP(pi_URL);
        }
    } g_URLHTTPCreator;

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
HFCURLHTTP::HFCURLHTTP(const Utf8String& pi_User,
                       const Utf8String& pi_Password,
                       const Utf8String& pi_Host,
                       const Utf8String& pi_Port,
                       const Utf8String& pi_Path,
                       const Utf8String& pi_SearchPart)
    : HFCURLHTTPBase(pi_User,
                     pi_Password,
                     pi_Host,
                     pi_Port,
                     pi_Path,
                     pi_SearchPart,
                     true)
    {
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
HFCURLHTTP::HFCURLHTTP(const Utf8String& pi_pURL)
    : HFCURLHTTPBase(pi_pURL, true)
    {
    }

/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HFCURLHTTP::~HFCURLHTTP()
    {
    // Nothing to do here.
    }
