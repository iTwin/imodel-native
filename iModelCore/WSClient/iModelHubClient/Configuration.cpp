/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Configuration.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnDbServerUtils.h"
#include "DgnDbServer/Client/DgnDbServerConfiguration.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

bool DgnDbServerConfiguration::s_preDownloadRevisionsEnabled = true;
int DgnDbServerConfiguration::s_preDownloadCacheSize         = 10 * 1024 * 1024; // 10 MB

