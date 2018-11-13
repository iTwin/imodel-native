/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelJsInternal.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/BeAssert.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeFileName.h>
#include <iModelJs/iModelJs.h>
#include <iModelJs/iModelJsServicesTier.h>

// *** TBD: Logging
#define LOG(s) fputs(s, stderr)
