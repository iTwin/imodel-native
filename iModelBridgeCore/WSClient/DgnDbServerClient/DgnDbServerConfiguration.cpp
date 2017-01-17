/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerConfiguration.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnDbServerUtils.h"
#include "DgnDbServer/Client/DgnDbServerConfiguration.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

bool DgnDbServerConfiguration::s_preDownloadRevisionsEnabled = true;
int DgnDbServerConfiguration::s_preDownloadCacheSize         = 10 * 1024 * 1024; // 10 MB

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
bool DgnDbServerConfiguration::GetPreDownloadRevisionsEnabled()
    {
    return s_preDownloadRevisionsEnabled;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
void DgnDbServerConfiguration::SetPreDownloadRevisionsEnabled(bool enabled)
    {
    s_preDownloadRevisionsEnabled = enabled;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
int DgnDbServerConfiguration::GetPreDownloadRevisionsCacheSize()
    {
    return s_preDownloadCacheSize;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
void DgnDbServerConfiguration::SetPreDownloadRevisionsCacheSize(int cacheSize)
    {
    if (cacheSize > 0)
        s_preDownloadCacheSize = cacheSize;
    }
