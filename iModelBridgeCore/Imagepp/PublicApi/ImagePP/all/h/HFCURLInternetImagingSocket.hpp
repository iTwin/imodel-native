//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLInternetImagingSocket.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCURLInternetImagingSocket
//-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the path to the
 resource included in this URL, which is the path to the image.  This
 method returns the same value than GetURLPath.

 @return A constant reference to the string that contains the path to the
         image, starting from the host specified in this URL.  It does not
         begin by a slash or a backslash.

 @inheritance This method cannot be overriden.

 @see GetURLPath
-----------------------------------------------------------------------------*/
inline const WString& HFCURLInternetImagingSocket::GetImage() const
    {
    return GetURLPath();
    }

