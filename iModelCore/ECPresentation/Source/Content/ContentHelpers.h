/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/Content.h>
#include "../RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=============================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+==*/
enum class RulesetCompareOption
    {
    None,
    ByPointer,
    ById,
    ByHash,
    };

/*=============================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+==*/
struct ContentHelpers
    {
    ContentHelpers() = delete;
    ECPRESENTATION_EXPORT static bool AreDescriptorsEqual(ContentDescriptor const& lhs, ContentDescriptor const& rhs, RulesetCompareOption = RulesetCompareOption::ByPointer);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
