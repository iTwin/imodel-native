/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "NodeLabelCalculator.h"
#include "Queries/QueryBuilderHelpers.h"
#include "Queries/QueryExecutor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PresentationQuery> CreateInstanceLabelQuery(ECSchemaHelper const& schemaHelper, ECClassInstanceKeyCR key,
    bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs, bvector<ECInstanceKey> const& prevLabelRequestsStack)
    {
    bvector<ECInstanceKey> labelRequestsStack(prevLabelRequestsStack);
    labelRequestsStack.push_back(ECInstanceKey(key.GetClass()->GetId(), key.GetId()));

    SelectClass<ECClass> selectClass(*key.GetClass(), "this", false);
    auto labelField = QueryBuilderHelpers::CreateDisplayLabelField("/DisplayLabel/", schemaHelper, selectClass,
        nullptr, nullptr, bvector<RelatedClassPath>(), labelOverrideValueSpecs, labelRequestsStack);
    RefCountedPtr<SimpleQueryContract> contract = SimpleQueryContract::Create({ labelField });
    auto query = ComplexQueryBuilder::Create();
    query->SelectContract(*contract, selectClass.GetAlias().c_str());
    query->From(selectClass);
    query->Where("[this].[ECInstanceId] = ?", { std::make_shared<BoundQueryId>(key.GetId()) });
    return query->CreateQuery();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinitionPtr NodeLabelCalculator::_GetNodeLabel(ECClassInstanceKeyCR key, bvector<ECInstanceKey> const& prevLabelRequestsStack) const
    {
    auto instanceLabelOverridesValuesMap = QueryBuilderHelpers::GetLabelOverrideValuesMap(m_schemaHelper, m_rulesPreprocessor.GetInstanceLabelOverrides());
    auto labelOverrideSpecs = QueryBuilderHelpers::GetInstanceLabelOverrideSpecsForClass(instanceLabelOverridesValuesMap, *key.GetClass());
    auto labelQuery = CreateInstanceLabelQuery(m_schemaHelper, key, labelOverrideSpecs, prevLabelRequestsStack);
    Utf8String label = QueryExecutorHelper::ReadText(m_schemaHelper.GetConnection(), *labelQuery);
    return LabelDefinition::FromString(label.c_str());
    }
