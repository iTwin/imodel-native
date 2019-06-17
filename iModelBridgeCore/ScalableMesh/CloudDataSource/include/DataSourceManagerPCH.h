/*--------------------------------------------------------------------------------------+
|    $RCSfile: stdafx.h,v $
|   $Revision: 1.11 $
|       $Date: 2012/07/25 14:13:37 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <assert.h>
#include "DataSourceAccount.h"
#include "DataSourceAccountAzure.h"
#include "DataSourceAccountCached.h"
#include "DataSourceAccountFile.h"
#include "DataSourceAzure.h"
#include "DataSourceBuffer.h"
#include "DataSourceBuffered.h"
#include "DataSourceCached.h"
#include "DataSourceCloud.h"
#include "DataSourceFile.h"
#include "DataSourceLocator.h"
#include "DataSourceManager.h"
#include "DataSourceManagerTest.h"
#include "DataSourceMode.h"
#include "DataSourceService.h"
#ifdef USE_WASTORAGE
#include "DataSourceServiceAzure.h"
#endif
#include "DataSourceServiceFile.h"
#include "DataSourceServiceManager.h"
#include "DataSourceStatus.h"
#include "DataSourceStoreConfig.h"
#include "DataSourceTransferScheduler.h"
#include "DataSourceURL.h"
#include "Manager.h"

#ifdef _WIN32
    #include "PerformanceTimer.h"
#endif

