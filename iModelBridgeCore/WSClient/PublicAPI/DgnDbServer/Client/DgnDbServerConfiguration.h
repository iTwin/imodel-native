/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerConfiguration.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

//=======================================================================================
//! Client API configuration.
//@bsiclass                                      Algirdas.Mikoliunas            01/2017
//=======================================================================================
struct DgnDbServerConfiguration
{
private:
    DGNDBSERVERCLIENT_EXPORT static bool s_preDownloadRevisionsEnabled;
    DGNDBSERVERCLIENT_EXPORT static int  s_preDownloadCacheSize; //10 MB

public:
    //! Gets if pre-download feature is enabled for briefcases.
    static bool GetPreDownloadRevisionsEnabled() {return s_preDownloadRevisionsEnabled;}

    //! Sets if pre-download feature is enabled for briefcases.
    //! @param[in] enabled
    static void SetPreDownloadRevisionsEnabled(bool enabled) {s_preDownloadRevisionsEnabled = enabled;}

    //! Gets maximum allowed pre-download cache size.
    static int GetPreDownloadRevisionsCacheSize() {return s_preDownloadCacheSize;}

    //! Sets maximum allowed pre-download cache size in bytes.
    //! @param[in] cacheSize
    static void SetPreDownloadRevisionsCacheSize(int cacheSize) {if (cacheSize > 0) s_preDownloadCacheSize = cacheSize;}
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
