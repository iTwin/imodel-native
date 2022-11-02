/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "../../RulesEngineTypes.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DiagnosticsHelpers::CreateRuleIdentifier(PresentationKey const& rule)
    {
    // TODO: might want to combine type with the hash - that should allow identifying specific spec
    return rule.GetJsonElementType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DiagnosticsHelpers::CreateNodeKeyIdentifier(NavNodeKeyCR key)
    {
    return key.GetHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DiagnosticsHelpers::CreateNodeIdentifier(NavNodeCR node)
    {
    return Utf8String("`").append(node.GetLabelDefinition().GetDisplayValue()).append("` [").append(CreateNodeKeyIdentifier(*node.GetKey())).append("]");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DiagnosticsHelpers::CreateECInstanceKeyStr(ECClassInstanceKeyCR key)
    {
    if (key.GetClass())
        return Utf8String(key.GetClass()->GetName()).append(":").append(key.GetId().ToString());
    return key.GetId().ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DiagnosticsHelpers::CreateContentSetItemStr(ContentSetItemCR item)
    {
    return Utf8String("[")
        .append(BeStringUtilities::Join(ContainerHelpers::TransformContainer<bvector<Utf8String>>(item.GetKeys(), &DiagnosticsHelpers::CreateECInstanceKeyStr), ","))
        .append("]");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DiagnosticsHelpers::CreateRelatedClassPathStr(RelatedClassPathCR path)
    {
    Utf8String str;
    bool first = true;
    for (RelatedClassCR relatedClass : path)
        {
        if (first && relatedClass.GetSourceClass())
            str.append(relatedClass.GetSourceClass()->GetFullName());
        if (!str.empty())
            str.append(relatedClass.IsForwardRelationship() ? "->" : "<-");
        str.append(relatedClass.GetRelationship().GetClass().GetFullName());
        str.append(relatedClass.IsForwardRelationship() ? "->" : "<-");
        str.append(relatedClass.GetTargetClass().GetClass().GetFullName());
        first = false;
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DiagnosticsHelpers::ReportRule(PresentationKey const& rule)
    {
    Diagnostics::AddValueToArrayAttribute(DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules, rule.GetJsonElementType(), true);
    }
