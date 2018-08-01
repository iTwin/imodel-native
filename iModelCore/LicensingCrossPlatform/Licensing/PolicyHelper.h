/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/PolicyHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Policy.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct PolicyHelper
{
private:
public:
	static bool IsPolicyExpired(std::shared_ptr<Policy> policy)
		{
		time_t ct = time(0);
		time_t currentTime = (time_t)gmtime(&ct);
		auto expirationTime = policy->GetPolicyExpiresOn();
		auto timeLeft = difftime(expirationTime, currentTime);
		return timeLeft > 0;
		};
};

END_BENTLEY_LICENSING_NAMESPACE
