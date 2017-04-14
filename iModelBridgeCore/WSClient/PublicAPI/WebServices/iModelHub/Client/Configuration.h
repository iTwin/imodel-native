/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/Configuration.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//=======================================================================================
//! Client API configuration.
//@bsiclass                                      Algirdas.Mikoliunas            01/2017
//=======================================================================================
struct Configuration
{
private:
    IMODELHUBCLIENT_EXPORT static bool s_preDownloadChangeSetsEnabled;
    IMODELHUBCLIENT_EXPORT static int  s_preDownloadCacheSize; //10 MB

public:
    //! Gets if pre-download feature is enabled for briefcases.
    static bool GetPredownloadChangeSetsEnabled() {return s_preDownloadChangeSetsEnabled;}

    //! Sets if pre-download feature is enabled for briefcases.
    //! @param[in] enabled
    static void SetPredownloadChangeSetsEnabled(bool enabled) {s_preDownloadChangeSetsEnabled = enabled;}

    //! Gets maximum allowed pre-download cache size.
    static int GetPredownloadChangeSetsCacheSize() {return s_preDownloadCacheSize;}

    //! Sets maximum allowed pre-download cache size in bytes.
    //! @param[in] cacheSize
    static void SetPredownloadChangeSetsCacheSize(int cacheSize) {if (cacheSize > 0) s_preDownloadCacheSize = cacheSize;}
};
END_BENTLEY_IMODELHUB_NAMESPACE
