/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/IFreeClient.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Bentley/BeVersion.h>
#include <folly/BeFolly.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct IFreeClient
{
public:
    virtual folly::Future<BentleyStatus> TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId) { return BentleyStatus::SUCCESS; };
    virtual ~IFreeClient() {};
};


END_BENTLEY_LICENSING_NAMESPACE
