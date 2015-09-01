//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLHTTPBase.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCURLHTTPBaseBase
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the path to the
 resource included in this URL, which is present when the host is not
 itself the resource designated by the URL.  This path does not include
 the search expression, unlike what is returned by @k{GetURLPath}.

 @return A constant reference to the string that contains the path to the
         resource, starting from the host specified in this URL.  It does
         not begin by a slash or a backslash.  May be empty if the host is
         itself the resource to be located.

 @inheritance This method cannot be overriden.

 @see GetURLPath
 @see GetSearchPart
-----------------------------------------------------------------------------*/
inline const WString& HFCURLHTTPBase::GetPath() const
    {
    return m_Path;
    }

/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the search
 expression included in this URL.  The search expression is an expression
 understood by the HTTP server, that is embedded in the "url-path"
 section of the URL; this means that calling @k{GetURLPath} returns the
 combination of the path and the search part, to get only the path the
 method @k{GetPath} should be called.

 @return A constant reference to the string that contains the search
         expression that was added to the path in this URL.  It does not
         begin by the question mark used as separator.  May be empty if
         there is no search expression or no path in this URL.

 @inheritance This method cannot be overriden.

 @see GetURLPath
 @see GetPath
-----------------------------------------------------------------------------*/
inline const WString& HFCURLHTTPBase::GetSearchPart() const
    {
    return m_SearchPart;
    }

END_IMAGEPP_NAMESPACE