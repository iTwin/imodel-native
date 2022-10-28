/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "ContentHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool InstanceFiltersEqual(InstanceFilterDefinition const* lhs, InstanceFilterDefinition const* rhs)
    {
    return (lhs == nullptr && rhs == nullptr) || (lhs != nullptr && rhs != nullptr && *lhs == *rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentHelpers::AreDescriptorsEqual(ContentDescriptor const& lhs, ContentDescriptor const& rhs, RulesetCompareOption rulesetCompareOption)
    {
    if (lhs.GetPreferredDisplayType() != rhs.GetPreferredDisplayType()
        || lhs.GetContentFlags() != rhs.GetContentFlags()
        || lhs.GetUnitSystem() != rhs.GetUnitSystem()
        || lhs.GetSortingFieldIndex() != rhs.GetSortingFieldIndex()
        || lhs.GetSortDirection() != rhs.GetSortDirection()
        || lhs.GetFieldsFilterExpression() != rhs.GetFieldsFilterExpression()
        || lhs.GetSelectClasses().size() != rhs.GetSelectClasses().size()
        || lhs.GetCategories().size() != rhs.GetCategories().size()
        || lhs.GetAllFields().size() != rhs.GetAllFields().size()
        || lhs.GetConnectionId() != rhs.GetConnectionId()
        || lhs.GetInputNodeKeys().GetHash() != rhs.GetInputNodeKeys().GetHash()
        || lhs.GetRulesetVariables() != rhs.GetRulesetVariables()
        || !InstanceFiltersEqual(lhs.GetInstanceFilter().get(), rhs.GetInstanceFilter().get()))
        {
        return false;
        }

    switch (rulesetCompareOption)
        {
        case RulesetCompareOption::ByPointer:
            {
            if (&lhs.GetRuleset() != &rhs.GetRuleset())
                return false;
            }
        case RulesetCompareOption::ByHash:
            {
            if (lhs.GetRuleset().GetHash() != rhs.GetRuleset().GetHash())
                return false;
            }
        case RulesetCompareOption::ById:
            {
            if (lhs.GetRuleset().GetRuleSetId() != rhs.GetRuleset().GetRuleSetId())
                return false;
            }
        }

    for (size_t i = 0; i < lhs.GetSelectClasses().size(); ++i)
        {
        if (lhs.GetSelectClasses()[i] != rhs.GetSelectClasses()[i])
            return false;
        }

    for (size_t i = 0; i < lhs.GetCategories().size(); ++i)
        {
        if (*lhs.GetCategories()[i] != *rhs.GetCategories()[i])
            return false;
        }

    for (size_t i = 0; i < lhs.GetAllFields().size(); ++i)
        {
        if (*lhs.GetAllFields()[i] != *rhs.GetAllFields()[i])
            return false;
        }

    if ((lhs.GetSelectionInfo() && !rhs.GetSelectionInfo()) || (!lhs.GetSelectionInfo() && rhs.GetSelectionInfo()))
        return false;

    if (lhs.GetSelectionInfo() && rhs.GetSelectionInfo() && *lhs.GetSelectionInfo() != *rhs.GetSelectionInfo())
        return false;

    return true;
    }
