//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLFile.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCURLFile
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the host
 specification included in this URL, which is the network name of the
 target that contains the file to be located. A URL contains
 specification for either a drive or a host, cannot contain both.

 @return A constant reference to the string that contains the host
         specification, or an empty string if this URL uses drive
         specification.

 @inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline const WString& HFCURLFile::GetHost() const
    {
    return m_Host;
    }

/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the path
 description included in this URL, which is the path to the file to be
 located, relative to the root of the drive or of the host.  It may be
 absent if the drive or the host is the resource to be located.

 @return A constant reference to the string that contains the path
         description, or an empty string if this URL does not specify a path.
         It does not begin by a slash or a backslash.

 @inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline const WString& HFCURLFile::GetPath() const
    {
    return m_Path;
    }
END_IMAGEPP_NAMESPACE