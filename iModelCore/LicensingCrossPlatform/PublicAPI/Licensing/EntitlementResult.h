/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

 //__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>
#include <Licensing/UsageType.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

class EntitlementResult
{
public:
	bool allowed;
	Utf8String principalId;
	UsageType usageType;
};

END_BENTLEY_LICENSING_NAMESPACE
