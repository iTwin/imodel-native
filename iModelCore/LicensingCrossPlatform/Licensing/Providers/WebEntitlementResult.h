/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

 //__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

class WebEntitlementResult
{
public:
	int ProductId;
	Licensing::LicenseStatus Status;
	Utf8String PrincipalId;
    Utf8String ExpiresOn; //For Trials / Evaluation
};

END_BENTLEY_LICENSING_NAMESPACE
