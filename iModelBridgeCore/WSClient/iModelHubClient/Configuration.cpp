/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Configuration.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Utils.h"
#include <WebServices/iModelHub/Client/Configuration.h>

USING_NAMESPACE_BENTLEY_IMODELHUB

bool Configuration::s_preDownloadChangeSetsEnabled = true;
int Configuration::s_preDownloadCacheSize         = 10 * 1024 * 1024; // 10 MB

