//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
inline const Utf8String& HFCURL::GetSchemeType() const
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
inline const Utf8String& HFCURL::GetSchemeSpecificPart() const
    {
    return m_SchemeSpecificPart;
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