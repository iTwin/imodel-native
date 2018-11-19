/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientInterface.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>
#include <Licensing/Utils/FeatureUserDataMap.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct ClientInterface
{
public:
	virtual LicenseStatus StartApplication() = 0;
	virtual BentleyStatus StopApplication() = 0;
	virtual BentleyStatus MarkFeature(Utf8String featureId, FeatureUserDataMap* featureUserData) = 0;
};


END_BENTLEY_LICENSING_NAMESPACE
