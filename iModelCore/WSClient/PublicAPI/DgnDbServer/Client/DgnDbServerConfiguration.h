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
    //__PUBLISH_SECTION_END__
    private:
        static bool s_preDownloadRevisionsEnabled;
        static int  s_preDownloadCacheSize; //10 MB

        //__PUBLISH_SECTION_START__
    public:
        //! Gets if pre-download feature is enabled for briefcases.
        DGNDBSERVERCLIENT_EXPORT static bool GetPreDownloadRevisionsEnabled();

        //! Sets if pre-download feature is enabled for briefcases.
        //! @param[in] enabled
        DGNDBSERVERCLIENT_EXPORT static void SetPreDownloadRevisionsEnabled(bool enabled);

        //! Gets maximum allowed pre-download cache size.
        DGNDBSERVERCLIENT_EXPORT static int GetPreDownloadRevisionsCacheSize();

        //! Sets maximum allowed pre-download cache size in bytes.
        //! @param[in] cacheSize
        DGNDBSERVERCLIENT_EXPORT static void SetPreDownloadRevisionsCacheSize(int cacheSize);
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE
