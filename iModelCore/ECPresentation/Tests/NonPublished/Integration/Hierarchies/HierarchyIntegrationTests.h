/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../PresentationManagerIntegrationTests.h"
#include "../../Helpers/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerNavigationTests : PresentationManagerIntegrationTests
{
    void _ConfigureManagerParams(ECPresentationManager::Params& params) override
        {
        PresentationManagerIntegrationTests::_ConfigureManagerParams(params);
        params.SetMode(ECPresentationManager::Mode::ReadOnly);
        }

    static CustomNodeSpecificationP CreateCustomNodeSpecification(Utf8String typeAndLabel, std::function<void(CustomNodeSpecificationR)> configure = nullptr);
};
