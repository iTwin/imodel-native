//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLMemFile.hpp $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCURLMemFile
//-----------------------------------------------------------------------------

#include "HFCBuffer.h"

/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the host
 specification included in this URL, A URL contains specification
 for an host.

 @return A constant reference to the string that contains the host
         specification..

 @inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline const WString& HFCURLMemFile::GetHost() const
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
inline const WString& HFCURLMemFile::GetPath() const
    {
    return m_Path;
    }


/**----------------------------------------------------------------------------
Returns a smart pointer to a HFCBuffer.

@return A smart pointer to a HFCBuffer

@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline HFCPtr<HFCBuffer>& HFCURLMemFile::GetBuffer()
    {
    return m_pBuffer;
    }

/**----------------------------------------------------------------------------
Set a internal smart pointer to an HFCBuffer.

@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline void HFCURLMemFile::SetBuffer(HFCPtr<HFCBuffer>& pi_rpBuffer)
    {
    m_pBuffer = pi_rpBuffer;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline void HFCURLMemFile::SetCreationTime(time_t   pi_NewTime)
    {
    m_creationTime=pi_NewTime;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline time_t HFCURLMemFile::GetCreationTime() const
    {
    return m_creationTime;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline void HFCURLMemFile::SetModificationTime(time_t   pi_NewTime)
    {
    m_modificationTime=pi_NewTime;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
inline time_t HFCURLMemFile::GetModificationTime() const
    {
    return m_modificationTime;
    }

