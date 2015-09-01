//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURL.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCURL
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Returns a string that contain the text of the scheme type for this URL.

 At this level, the text of a URL is splitted into two components : the
 scheme type specification, and the rest of the text that is the "scheme
 specific part".  The colon that separates both parts is stripped.

 @return A string containing the text of the scheme type, without the colon
         that follows it in the URL text.

 @inheritance This method cannot be overriden.

 @see GetSchemeSpecificPart
-----------------------------------------------------------------------------*/
inline const WString& HFCURL::GetSchemeType() const
    {
    return m_SchemeType;
    }

/**----------------------------------------------------------------------------
 Returns a string that contain the text after the scheme type for this
 URL, which is the "main part" of the URL.

 At this level, the text of a URL is splitted into two components : the
 scheme type specification, and the rest of the text that is the "scheme
 specific part".  The colon that separates both parts is stripped.

 @return A string containing the text of this URL without the scheme type
         specification.

 @inheritance This method cannot be overriden.

 @see GetSchemeType
 @see GetURL
-----------------------------------------------------------------------------*/
inline const WString& HFCURL::GetSchemeSpecificPart() const
    {
    return m_SchemeSpecificPart;
    }

/**----------------------------------------------------------------------------
 Scheme list access method.  This static method is required to insure proper
 initialization of the list, which is allocated on the heap instead of
 created statically, because order of creation of static objects is unknown.
-----------------------------------------------------------------------------*/
inline HFCURL::SchemeList& HFCURL::GetSchemeList()
    {
    if (!s_pSchemeList)
        s_pSchemeList = new SchemeList;
    return *s_pSchemeList;
    }


/**----------------------------------------------------------------------------
 Return true id the URL is UTF8, false otherwise

 see SetUTF8URL
-----------------------------------------------------------------------------*/
inline bool HFCURL::IsUTF8URL() const
    {
    return m_UTF8URL;
    }

/**----------------------------------------------------------------------------
Return true if the URL is encoded, false otherwise

see SetEncodeURL
-----------------------------------------------------------------------------*/
inline bool HFCURL::IsEncodedURL() const
    {
    return m_EncodedURL;
    }
END_IMAGEPP_NAMESPACE