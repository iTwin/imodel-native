/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "NavigationQueryResultsReader.h"
#include "NavNodesHelper.h"
#include "../Shared/ValueHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static GroupedInstanceKeysList ParseInstanceKeys(ECSqlStatementCR stmt, NavigationQueryContract const& contract, Utf8CP fieldName)
    {
    Utf8CP str = stmt.GetValueText(contract.GetIndex(fieldName));
    return ValueHelpers::GetECInstanceKeysFromJsonString(str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static GenericQueryPtr CreateInstanceKeysQuery(bvector<ECInstanceKey> const& keys)
    {
    Utf8String query;
    BoundQueryValuesList bindings;
    for (size_t i = 0; i < keys.size(); ++i)
        {
        if (i > 0)
            query.append(" UNION ALL ");
        query.append("SELECT ? AS ECClassId, ? AS ECInstanceId");
        bindings.push_back(std::make_unique<BoundQueryId>(keys[i].GetClassId()));
        bindings.push_back(std::make_unique<BoundQueryId>(keys[i].GetInstanceId()));
        }
    return StringGenericQuery::Create(query, bindings);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECInstanceNodeReaderBase : NavNodesReader
{
private:
    NavNodePtr m_previousNode;
protected:
    ECInstanceNodeReaderBase(NavNodesFactory const& factory, NavigationQueryContract const& contract, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contract, connection, extendedData, parentKey)
        {}

    QueryResultReaderStatus _ReadRecord(NavNodePtr& node, ECSqlStatementCR statement) override
        {
        QueryResultReaderStatus baseReaderStatus = NavNodesReader::_ReadRecord(node, statement);
        if (QueryResultReaderStatus::Row != baseReaderStatus)
            return baseReaderStatus;

        if (m_previousNode.IsValid() && m_previousNode->GetKey()->IsSimilar(*node->GetKey()))
            {
            NavNodeExtendedData(*m_previousNode).AddRelatedInstanceKeys(NavNodeExtendedData(*node).GetRelatedInstanceKeys());
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodeExtendedData(*m_previousNode).AddSkippedInstanceKeys(NavNodeExtendedData(*node).GetSkippedInstanceKeys());
#endif
            return QueryResultReaderStatus::Skip;
            }

        m_previousNode = node;
        return QueryResultReaderStatus::Row;
        }
};

/*=================================================================================**//**
* A reader used to read nodes when they represent a single ECInstance. That's generally
* used when filtering by ECInstance ID is required.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECInstanceNodeReader : ECInstanceNodeReaderBase
{
typedef ECInstanceNodesQueryContract Contract;
protected:
    NavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        ECClassId ecClassId = statement.GetValueId<ECClassId>(GetContract().GetIndex(Contract::ECClassIdFieldName));
        ECInstanceId ecInstanceId = statement.GetValueId<ECInstanceId>(GetContract().GetIndex(Contract::ECInstanceIdFieldName));
        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        Utf8CP specificationIdentifier = statement.GetValueText(GetContract().GetIndex(Contract::SpecificationIdentifierFieldName));
        NavNodePtr node = GetFactory().CreateECInstanceNode(GetConnection(), specificationIdentifier, GetParentKey(), ecClassId, ecInstanceId, *LabelDefinition::FromString(displayLabel));
        if (node.IsValid())
            {
            node->SetInstanceKeysSelectQuery(CreateInstanceKeysQuery({ ECInstanceKey(ecClassId, ecInstanceId) }));
            NavNodesHelper::AddRelatedInstanceInfo(*node, statement.GetValueText(GetContract().GetIndex(Contract::RelatedInstanceInfoFieldName)));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }
public:
    ECInstanceNodeReader(NavNodesFactory const& factory, NavigationQueryContract const& contract, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : ECInstanceNodeReaderBase(factory, contract, connection, extendedData, parentKey)
        {}
};

/*=================================================================================**//**
* A reader used to read nodes that may represent multiple ECInstances.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiECInstanceNodeReader : ECInstanceNodeReaderBase
{
typedef MultiECInstanceNodesQueryContract Contract;
protected:
    NavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        GroupedInstanceKeysList keys = ParseInstanceKeys(statement, GetContract(), Contract::InstanceKeysFieldName);
        Utf8CP specificationIdentifier = statement.GetValueText(GetContract().GetIndex(Contract::SpecificationIdentifierFieldName));
        NavNodePtr node = GetFactory().CreateECInstanceNode(GetConnection(), specificationIdentifier, GetParentKey(), keys, *LabelDefinition::FromString(displayLabel));
        if (node.IsValid())
            {
            node->SetInstanceKeysSelectQuery(CreateInstanceKeysQuery(keys));
            NavNodesHelper::AddRelatedInstanceInfo(*node, statement.GetValueText(GetContract().GetIndex(Contract::RelatedInstanceInfoFieldName)));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }
public:
    MultiECInstanceNodeReader(NavNodesFactory const& factory, NavigationQueryContract const& contract, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : ECInstanceNodeReaderBase(factory, contract, connection, extendedData, parentKey)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MergingMultiECInstanceNodeReader : NavNodesReader
{
typedef MultiECInstanceNodesQueryContract Contract;

private:
    NavNodePtr m_inProgressNode;
    GroupedInstanceKeysList m_inProgressInstanceKeys;
    GroupedInstanceKeysList m_inProgressSkippedInstanceKeys;
    bvector<NavNodeExtendedData::RelatedInstanceKey> m_inProgressRelatedInstanceKeys;

protected:
    NavNodePtr _ReadNode(ECSqlStatementCR statement) const override {return nullptr;}
    QueryResultReaderStatus _ReadRecord(NavNodePtr& node, ECSqlStatementCR statement) override
        {
        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        if (m_inProgressNode.IsNull())
            {
            Utf8CP specificationIdentifier = statement.GetValueText(GetContract().GetIndex(Contract::SpecificationIdentifierFieldName));
            m_inProgressNode = GetFactory().CreateECInstanceNode(GetConnection(), specificationIdentifier, GetParentKey(), GroupedInstanceKeysList(), *LabelDefinition::FromString(displayLabel));
            }
        else if (m_inProgressNode->GetLabelDefinition().IsDefinitionValid())
            {
            auto newLabel = LabelDefinition::FromString(displayLabel);
            if (m_inProgressNode->GetLabelDefinition() != *newLabel)
                m_inProgressNode->SetLabelDefinition(*LabelDefinition::Create());
            }
        ContainerHelpers::Push(m_inProgressInstanceKeys, ParseInstanceKeys(statement, GetContract(), Contract::InstanceKeysFieldName));
#ifdef wip_skipped_instance_keys_performance_issue
        ContainerHelpers::Push(m_inProgressSkippedInstanceKeys, ParseInstanceKeys(statement, GetContract(), Contract::SkippedInstanceKeysFieldName));
#endif
        ContainerHelpers::Push(m_inProgressRelatedInstanceKeys, ItemExtendedData::ParseRelatedInstanceKeys(statement.GetValueText(GetContract().GetIndex(Contract::RelatedInstanceInfoFieldName))));
        return QueryResultReaderStatus::Skip;
        }
    bool _Complete(NavNodePtr& node, ECSqlStatementCR) override
        {
        if (m_inProgressNode.IsValid())
            {
            m_inProgressNode->SetInstanceKeysSelectQuery(CreateInstanceKeysQuery(m_inProgressInstanceKeys));
            NavNodeExtendedData extendedData(*m_inProgressNode);
#ifdef wip_skipped_instance_keys_performance_issue
            extendedData.SetSkippedInstanceKeys(m_inProgressSkippedInstanceKeys);
#endif
            extendedData.SetRelatedInstanceKeys(m_inProgressRelatedInstanceKeys);
            InitNode(*m_inProgressNode);
            node = m_inProgressNode;
            node->SetNodeKey(*ECInstancesNodeKey::Create(GetConnection(), m_inProgressNode->GetKey()->GetSpecificationIdentifier(), GetParentKey(), m_inProgressInstanceKeys));
            return true;
            }
        return false;
        }
public:
    MergingMultiECInstanceNodeReader(NavNodesFactory const& factory, NavigationQueryContract const& contract, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contract, connection, extendedData, parentKey)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelGroupingNodeReader : NavNodesReader
{
typedef DisplayLabelGroupingNodesQueryContract Contract;
private:
    Contract const& GetContract() const {return static_cast<Contract const&>(NavNodesReader::GetContract());}
protected:
    NavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        Utf8CP specificationIdentifier = statement.GetValueText(GetContract().GetIndex(Contract::SpecificationIdentifierFieldName));
        auto labelDefinition = LabelDefinition::FromString(statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName)));
        uint64_t groupedInstancesCount = statement.GetValueUInt64(GetContract().GetIndex(Contract::GroupedInstancesCountFieldName));
        std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys;
        if (groupedInstancesCount <= MAX_LABEL_GROUPED_INSTANCE_KEYS)
            groupedInstanceKeys = std::make_unique<bvector<ECInstanceKey>>(ParseInstanceKeys(statement, GetContract(), Contract::GroupedInstanceKeysFieldName));
        NavNodePtr node = GetFactory().CreateDisplayLabelGroupingNode(GetConnection(), specificationIdentifier, GetParentKey(), *labelDefinition, groupedInstancesCount, std::move(groupedInstanceKeys));
        if (node.IsValid())
            {
            node->SetInstanceKeysSelectQuery(GetContract().CreateInstanceKeysSelectQuery(*labelDefinition));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }
public:
    DisplayLabelGroupingNodeReader(NavNodesFactory const& factory, NavigationQueryContract const& contract, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contract, connection, extendedData, parentKey)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECClassGroupingNodeReader : NavNodesReader
{
typedef ECClassGroupingNodesQueryContract Contract;
private:
    Contract const& GetContract() const {return static_cast<Contract const&>(NavNodesReader::GetContract());}
protected:
    NavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        ECClassId classId = statement.GetValueId<ECClassId>(GetContract().GetIndex(Contract::ECClassIdFieldName));
        if (!classId.IsValid())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Invalid ECClassId");

        ECClassCP ecClass = nullptr;
        if (nullptr == (ecClass = statement.GetECDb()->Schemas().GetClass(classId)))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("ECClassId points to invalid ECClass: %" PRIu64, classId.GetValue()));

        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        bool isPolymorphic = statement.GetValueBoolean(GetContract().GetIndex(Contract::IsClassPolymorphicFieldName));
        Utf8CP specificationIdentifier = statement.GetValueText(GetContract().GetIndex(Contract::SpecificationIdentifierFieldName));
        uint64_t groupedInstancesCount = statement.GetValueUInt64(GetContract().GetIndex(Contract::GroupedInstancesCountFieldName));
        NavNodePtr node = GetFactory().CreateECClassGroupingNode(GetConnection(), specificationIdentifier, GetParentKey(), *ecClass, isPolymorphic, *LabelDefinition::FromString(displayLabel), groupedInstancesCount);
        if (node.IsValid())
            {
            node->SetInstanceKeysSelectQuery(GetContract().CreateInstanceKeysSelectQuery(*ecClass, isPolymorphic));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }
public:
    ECClassGroupingNodeReader(NavNodesFactory const& factory, NavigationQueryContract const& contract, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contract, connection, extendedData, parentKey)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPropertyGroupingNodeReader : NavNodesReader
{
typedef ECPropertyGroupingNodesQueryContract Contract;

private:
    Contract const& GetContract() const {return static_cast<Contract const&>(NavNodesReader::GetContract());}
    static rapidjson::Document GetGroupingValuesAsJson(ECPropertyCR prop, Utf8CP valueStr, bool isRangeIndex)
        {
        rapidjson::Document doc;
        doc.Parse(valueStr);
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, !doc.IsNull(), Utf8PrintfString("Failed to parse JSON from string: '%s'", valueStr));
        return doc;
        }

protected:
    NavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        ECClassId classId = statement.GetValueId<ECClassId>(GetContract().GetIndex(Contract::ECPropertyClassIdFieldName));
        if (!classId.IsValid())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Invalid ECClassId");

        ECClassCP ecClass = nullptr;
        if (nullptr == (ecClass = statement.GetECDb()->Schemas().GetClass(classId)))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("ECClassId points to invalid ECClass: %" PRIu64, classId.GetValue()));

        Utf8CP propertyName = statement.GetValueText(GetContract().GetIndex(Contract::ECPropertyNameFieldName));
        if (nullptr == propertyName)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Invalid property name");

        ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
        if (nullptr == ecProperty)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Property not found: '%s.%s'", ecClass->GetFullName(), propertyName));

        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        bool isRangeGroupingNode = statement.GetValueBoolean(GetContract().GetIndex(Contract::IsRangeFieldName));
        rapidjson::Document groupingValues = GetGroupingValuesAsJson(*ecProperty, statement.GetValueText(GetContract().GetIndex(Contract::GroupingValuesFieldName)), isRangeGroupingNode);
        Utf8CP imageId = statement.GetValueText(GetContract().GetIndex(Contract::ImageIdFieldName));
        Utf8CP specificationIdentifier = statement.GetValueText(GetContract().GetIndex(Contract::SpecificationIdentifierFieldName));
        uint64_t groupedInstancesCount = statement.GetValueUInt64(GetContract().GetIndex(Contract::GroupedInstancesCountFieldName));
        NavNodePtr node = GetFactory().CreateECPropertyGroupingNode(GetConnection(), specificationIdentifier, GetParentKey(), *ecClass, *ecProperty, *LabelDefinition::FromString(displayLabel), imageId, groupingValues, isRangeGroupingNode, groupedInstancesCount);
        if (node.IsValid())
            {
            node->SetInstanceKeysSelectQuery(GetContract().CreateInstanceKeysSelectQuery(*node));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }

public:
    ECPropertyGroupingNodeReader(NavNodesFactory const& factory, NavigationQueryContract const& contract, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contract, connection, extendedData, parentKey)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<IQueryResultReader<NavNodePtr>> NavNodesReader::Create(NavNodesFactory const& factory, IConnectionCR connection,
    NavigationQueryContract const& contract, NavigationQueryResultType resultType, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
    {
    switch (resultType)
        {
        case NavigationQueryResultType::ClassGroupingNodes:
            return std::make_unique<ECClassGroupingNodeReader>(factory, contract, connection, extendedData, parentKey);
        case NavigationQueryResultType::PropertyGroupingNodes:
            return std::make_unique<ECPropertyGroupingNodeReader>(factory, contract, connection, extendedData, parentKey);
        case NavigationQueryResultType::DisplayLabelGroupingNodes:
            return std::make_unique<DisplayLabelGroupingNodeReader>(factory, contract, connection, extendedData, parentKey);
        case NavigationQueryResultType::ECInstanceNodes:
            return std::make_unique<ECInstanceNodeReader>(factory, contract, connection, extendedData, parentKey);
        case NavigationQueryResultType::MultiECInstanceNodes:
            if (extendedData.HideNodesInHierarchy())
                return std::make_unique<MergingMultiECInstanceNodeReader>(factory, contract, connection, extendedData, parentKey);
            return std::make_unique<MultiECInstanceNodeReader>(factory, contract, connection, extendedData, parentKey);
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_ERROR, Utf8PrintfString("Unhandled navigation query result type: %d", (int)resultType));
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesReader::InitNode(NavNodeR node) const
    {
    NavNodeExtendedData(node).MergeWith(m_navNodeExtendedData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryResultReaderStatus NavNodesReader::_ReadRecord(NavNodePtr& node, ECSqlStatementCR stmt)
    {
    node = _ReadNode(stmt);
    if (node.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to read node");
    return QueryResultReaderStatus::Row;
    }
