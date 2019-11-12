/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

enum class AuthType
    {
    None = 0,
    SAML = 1,
    OIDC = 2
    };

END_BENTLEY_LICENSING_NAMESPACE
