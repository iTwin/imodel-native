//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFSWinDrive.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HFSException.h>

//:>---------------------------------------------------------------------------
//:> inline methods for class HFSWinDrive
//:>---------------------------------------------------------------------------

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Get the current path.

 @return HFCPtr<HFCURL> The current path.
-----------------------------------------------------------------------------*/
inline HFCPtr<HFCURL> HFSWinDrive::GetCurrentURLPath() const
    {
    // return the path without the drive name
    return new HFCURLFile(HFCURLFile::s_SchemeName() + L"//" + m_pCurrentFolder->GetPath());
    }

/**----------------------------------------------------------------------------
 Get the current path.

 @return WString The current path.
-----------------------------------------------------------------------------*/
inline WString HFSWinDrive::GetCurrentPath() const
    {
    // return the path without the drive name
    return m_pCurrentFolder->GetPath();
    }

/**----------------------------------------------------------------------------
 Get the current folder.

 @return const HFCPtr<HFSItem>& The current folder.
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFSItem>& HFSWinDrive::GetCurrentFolder() const
    {
    return (const HFCPtr<HFSItem>&)m_pCurrentFolder;
    }


//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------


