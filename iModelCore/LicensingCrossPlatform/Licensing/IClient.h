/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/IClient.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>
#include <Licensing/Utils/FeatureUserDataMap.h>
#include <Bentley/BeVersion.h>
#include <folly/BeFolly.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct IClient
    {
public:
        virtual LicenseStatus StartApplication() = 0;
        virtual BentleyStatus StopApplication() = 0;
        virtual BentleyStatus MarkFeature(Utf8StringCR featureId, FeatureUserDataMap* featureUserData) = 0;
        virtual ~IClient() {};
    };


END_BENTLEY_LICENSING_NAMESPACE
