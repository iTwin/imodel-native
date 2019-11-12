/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <folly/BeFolly.h>
#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "../Policy.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct IPolicyProvider> IPolicyProviderPtr;

struct IPolicyProvider
{
public:
    virtual folly::Future<std::shared_ptr<Policy>> GetPolicy() = 0;
    virtual folly::Future<std::shared_ptr<Policy>> GetPolicyWithKey(Utf8StringCR accessKey, Utf8StringCR ultimateId) = 0;
    virtual ~IPolicyProvider() {};
};

END_BENTLEY_LICENSING_NAMESPACE
