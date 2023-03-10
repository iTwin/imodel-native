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

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECInstanceNodeReaderBase : NavNodesReader
{
private:
    NavNodePtr m_previousNode;
protected:
    ECInstanceNodeReaderBase(NavNodesFactory const& factory, IContractProvider<NavigationQueryContract> const& contractProvider, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contractProvider, connection, extendedData, parentKey)
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
        uint64_t contractId = statement.GetValueUInt64(0);
        auto contract = static_cast<Contract const*>(GetContract(contractId));
        if (!contract)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to find query contract when reading query results");

        ECClassId ecClassId = statement.GetValueId<ECClassId>(contract->GetIndex(Contract::ECClassIdFieldName));
        ECInstanceId ecInstanceId = statement.GetValueId<ECInstanceId>(contract->GetIndex(Contract::ECInstanceIdFieldName));
        Utf8CP displayLabel = statement.GetValueText(contract->GetIndex(Contract::DisplayLabelFieldName));
        Utf8CP specificationIdentifier = statement.GetValueText(contract->GetIndex(Contract::SpecificationIdentifierFieldName));
        NavNodePtr node = GetFactory().CreateECInstanceNode(GetConnection(), specificationIdentifier, GetParentKey(), ecClassId, ecInstanceId, *LabelDefinition::FromString(displayLabel));
        if (node.IsValid())
            {
            node->GetKey()->SetInstanceKeysSelectQuery(contract->CreateInstanceKeysSelectQuery(ECInstanceKey(ecClassId, ecInstanceId))->CreateQuery());
            NavNodesHelper::AddRelatedInstanceInfo(*node, statement.GetValueText(contract->GetIndex(Contract::RelatedInstanceInfoFieldName)));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(contract->GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }
public:
    ECInstanceNodeReader(NavNodesFactory const& factory, IContractProvider<NavigationQueryContract> const& contractProvider, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : ECInstanceNodeReaderBase(factory, contractProvider, connection, extendedData, parentKey)
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
        uint64_t contractId = statement.GetValueUInt64(0);
        auto contract = static_cast<Contract const*>(GetContract(contractId));
        if (!contract)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to find query contract when reading query results");

        Utf8CP displayLabel = statement.GetValueText(contract->GetIndex(Contract::DisplayLabelFieldName));
        GroupedInstanceKeysList keys = ParseInstanceKeys(statement, *contract, Contract::InstanceKeysFieldName);
        Utf8CP specificationIdentifier = statement.GetValueText(contract->GetIndex(Contract::SpecificationIdentifierFieldName));
        NavNodePtr node = GetFactory().CreateECInstanceNode(GetConnection(), specificationIdentifier, GetParentKey(), keys, *LabelDefinition::FromString(displayLabel));
        if (node.IsValid())
            {
            node->GetKey()->SetInstanceKeysSelectQuery(contract->CreateInstanceKeysSelectQuery(keys)->CreateQuery());
            NavNodesHelper::AddRelatedInstanceInfo(*node, statement.GetValueText(contract->GetIndex(Contract::RelatedInstanceInfoFieldName)));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(contract->GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }
public:
    MultiECInstanceNodeReader(NavNodesFactory const& factory, IContractProvider<NavigationQueryContract> const& contractProvider, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : ECInstanceNodeReaderBase(factory, contractProvider, connection, extendedData, parentKey)
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

private:
    Contract const& GetContract(uint64_t id) const
        {
        auto contract = static_cast<Contract const*>(NavNodesReader::GetContract(id));
        if (!contract)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to find query contract when reading query results");
        return *contract;
        }

protected:
    NavNodePtr _ReadNode(ECSqlStatementCR statement) const override {return nullptr;}
    QueryResultReaderStatus _ReadRecord(NavNodePtr& node, ECSqlStatementCR statement) override
        {
        uint64_t contractId = statement.GetValueUInt64(0);
        auto const& contract = GetContract(contractId);

        Utf8CP displayLabel = statement.GetValueText(contract.GetIndex(Contract::DisplayLabelFieldName));
        if (m_inProgressNode.IsNull())
            {
            Utf8CP specificationIdentifier = statement.GetValueText(contract.GetIndex(Contract::SpecificationIdentifierFieldName));
            m_inProgressNode = GetFactory().CreateECInstanceNode(GetConnection(), specificationIdentifier, GetParentKey(), GroupedInstanceKeysList(), *LabelDefinition::FromString(displayLabel));
            }
        else if (m_inProgressNode->GetLabelDefinition().IsDefinitionValid())
            {
            auto newLabel = LabelDefinition::FromString(displayLabel);
            if (m_inProgressNode->GetLabelDefinition() != *newLabel)
                m_inProgressNode->SetLabelDefinition(*LabelDefinition::Create());
            }
        ContainerHelpers::Push(m_inProgressInstanceKeys, ParseInstanceKeys(statement, contract, Contract::InstanceKeysFieldName));
#ifdef wip_skipped_instance_keys_performance_issue
        ContainerHelpers::Push(m_inProgressSkippedInstanceKeys, ParseInstanceKeys(statement, GetContract(), Contract::SkippedInstanceKeysFieldName));
#endif
        ContainerHelpers::Push(m_inProgressRelatedInstanceKeys, ItemExtendedData::ParseRelatedInstanceKeys(statement.GetValueText(contract.GetIndex(Contract::RelatedInstanceInfoFieldName))));
        return QueryResultReaderStatus::Skip;
        }
    bool _Complete(NavNodePtr& node, ECSqlStatementCR) override
        {
        if (m_inProgressNode.IsValid())
            {
            NavNodeExtendedData extendedData(*m_inProgressNode);
#ifdef wip_skipped_instance_keys_performance_issue
            extendedData.SetSkippedInstanceKeys(m_inProgressSkippedInstanceKeys);
#endif
            extendedData.SetRelatedInstanceKeys(m_inProgressRelatedInstanceKeys);
            InitNode(*m_inProgressNode);
            node = m_inProgressNode;
            node->SetNodeKey(*ECInstancesNodeKey::Create(GetConnection(), m_inProgressNode->GetKey()->GetSpecificationIdentifier(), GetParentKey(), m_inProgressInstanceKeys));
            node->GetKey()->SetInstanceKeysSelectQuery(GetContract(0).CreateInstanceKeysSelectQuery(m_inProgressInstanceKeys)->CreateQuery());
            return true;
            }
        return false;
        }
public:
    MergingMultiECInstanceNodeReader(NavNodesFactory const& factory, IContractProvider<NavigationQueryContract> const& contractProvider, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contractProvider, connection, extendedData, parentKey)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelGroupingNodeReader : NavNodesReader
{
typedef DisplayLabelGroupingNodesQueryContract Contract;
protected:
    NavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        uint64_t contractId = statement.GetValueUInt64(0);
        auto contract = static_cast<Contract const*>(GetContract(contractId));
        if (!contract)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to find query contract when reading query results");

        Utf8CP specificationIdentifier = statement.GetValueText(contract->GetIndex(Contract::SpecificationIdentifierFieldName));
        auto labelDefinition = LabelDefinition::FromString(statement.GetValueText(contract->GetIndex(Contract::DisplayLabelFieldName)));
        uint64_t groupedInstancesCount = statement.GetValueUInt64(contract->GetIndex(Contract::GroupedInstancesCountFieldName));
        std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys;
        if (groupedInstancesCount <= MAX_LABEL_GROUPED_INSTANCE_KEYS)
            groupedInstanceKeys = std::make_unique<bvector<ECInstanceKey>>(ParseInstanceKeys(statement, *contract, Contract::GroupedInstanceKeysFieldName));
        auto instanceKeysSelectQuery = contract->CreateInstanceKeysSelectQuery(*labelDefinition)->CreateQuery();
        NavNodePtr node = GetFactory().CreateDisplayLabelGroupingNode(GetConnection(), specificationIdentifier, GetParentKey(), *labelDefinition,
            groupedInstancesCount, instanceKeysSelectQuery.get(), std::move(groupedInstanceKeys));
        if (node.IsValid())
            {
            node->GetKey()->SetInstanceKeysSelectQuery(std::move(instanceKeysSelectQuery));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(contract->GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }
public:
    DisplayLabelGroupingNodeReader(NavNodesFactory const& factory, IContractProvider<NavigationQueryContract> const& contractProvider, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contractProvider, connection, extendedData, parentKey)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECClassGroupingNodeReader : NavNodesReader
{
typedef ECClassGroupingNodesQueryContract Contract;
protected:
    NavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        uint64_t contractId = statement.GetValueUInt64(0);
        auto contract = static_cast<Contract const*>(GetContract(contractId));
        if (!contract)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to find query contract when reading query results");

        ECClassId classId = statement.GetValueId<ECClassId>(contract->GetIndex(Contract::ECClassIdFieldName));
        if (!classId.IsValid())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Invalid ECClassId");

        ECClassCP ecClass = nullptr;
        if (nullptr == (ecClass = statement.GetECDb()->Schemas().GetClass(classId)))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("ECClassId points to invalid ECClass: %" PRIu64, classId.GetValue()));

        Utf8CP displayLabel = statement.GetValueText(contract->GetIndex(Contract::DisplayLabelFieldName));
        bool isPolymorphic = statement.GetValueBoolean(contract->GetIndex(Contract::IsClassPolymorphicFieldName));
        Utf8CP specificationIdentifier = statement.GetValueText(contract->GetIndex(Contract::SpecificationIdentifierFieldName));
        uint64_t groupedInstancesCount = statement.GetValueUInt64(contract->GetIndex(Contract::GroupedInstancesCountFieldName));
        auto instanceKeysSelectQuery = contract->CreateInstanceKeysSelectQuery(*ecClass, isPolymorphic)->CreateQuery();
        NavNodePtr node = GetFactory().CreateECClassGroupingNode(GetConnection(), specificationIdentifier, GetParentKey(), *ecClass, isPolymorphic, *LabelDefinition::FromString(displayLabel),
            groupedInstancesCount, instanceKeysSelectQuery.get());
        if (node.IsValid())
            {
            node->GetKey()->SetInstanceKeysSelectQuery(std::move(instanceKeysSelectQuery));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(contract->GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }
public:
    ECClassGroupingNodeReader(NavNodesFactory const& factory, IContractProvider<NavigationQueryContract> const& contractProvider, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contractProvider, connection, extendedData, parentKey)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPropertyGroupingNodeReader : NavNodesReader
{
typedef ECPropertyGroupingNodesQueryContract Contract;

private:
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
        uint64_t contractId = statement.GetValueUInt64(0);
        auto contract = static_cast<Contract const*>(GetContract(contractId));
        if (!contract)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to find query contract when reading query results");

        ECClassId classId = statement.GetValueId<ECClassId>(contract->GetIndex(Contract::ECPropertyClassIdFieldName));
        if (!classId.IsValid())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Invalid ECClassId");

        ECClassCP ecClass = nullptr;
        if (nullptr == (ecClass = statement.GetECDb()->Schemas().GetClass(classId)))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("ECClassId points to invalid ECClass: %" PRIu64, classId.GetValue()));

        Utf8CP propertyName = statement.GetValueText(contract->GetIndex(Contract::ECPropertyNameFieldName));
        if (nullptr == propertyName)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Invalid property name");

        ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
        if (nullptr == ecProperty)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Property not found: '%s.%s'", ecClass->GetFullName(), propertyName));

        Utf8CP displayLabel = statement.GetValueText(contract->GetIndex(Contract::DisplayLabelFieldName));
        bool isRangeGroupingNode = statement.GetValueBoolean(contract->GetIndex(Contract::IsRangeFieldName));
        rapidjson::Document groupingValues = GetGroupingValuesAsJson(*ecProperty, statement.GetValueText(contract->GetIndex(Contract::GroupingValuesFieldName)), isRangeGroupingNode);
        Utf8CP imageId = statement.GetValueText(contract->GetIndex(Contract::ImageIdFieldName));
        Utf8CP specificationIdentifier = statement.GetValueText(contract->GetIndex(Contract::SpecificationIdentifierFieldName));
        uint64_t groupedInstancesCount = statement.GetValueUInt64(contract->GetIndex(Contract::GroupedInstancesCountFieldName));
        auto instanceKeysSelectQuery = contract->CreateInstanceKeysSelectQuery(groupingValues)->CreateQuery();
        NavNodePtr node = GetFactory().CreateECPropertyGroupingNode(GetConnection(), specificationIdentifier, GetParentKey(), *ecClass, *ecProperty, *LabelDefinition::FromString(displayLabel), imageId, groupingValues, isRangeGroupingNode,
            groupedInstancesCount, instanceKeysSelectQuery.get());
        if (node.IsValid())
            {
            node->GetKey()->SetInstanceKeysSelectQuery(std::move(instanceKeysSelectQuery));
#ifdef wip_skipped_instance_keys_performance_issue
            NavNodesHelper::SetSkippedInstanceKeys(*node, statement.GetValueText(contract->GetIndex(Contract::SkippedInstanceKeysFieldName)));
#endif
            InitNode(*node);
            }
        return node;
        }

public:
    ECPropertyGroupingNodeReader(NavNodesFactory const& factory, IContractProvider<NavigationQueryContract> const& contractProvider, IConnectionCR connection, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
        : NavNodesReader(factory, contractProvider, connection, extendedData, parentKey)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<IQueryResultReader<NavNodePtr>> NavNodesReader::Create(NavNodesFactory const& factory, IConnectionCR connection,
    IContractProvider<NavigationQueryContract> const& contractProvider, NavigationQueryResultType resultType, NavNodeExtendedData const& extendedData, NavNodeKeyCP parentKey)
    {
    switch (resultType)
        {
        case NavigationQueryResultType::ClassGroupingNodes:
            return std::make_unique<ECClassGroupingNodeReader>(factory, contractProvider, connection, extendedData, parentKey);
        case NavigationQueryResultType::PropertyGroupingNodes:
            return std::make_unique<ECPropertyGroupingNodeReader>(factory, contractProvider, connection, extendedData, parentKey);
        case NavigationQueryResultType::DisplayLabelGroupingNodes:
            return std::make_unique<DisplayLabelGroupingNodeReader>(factory, contractProvider, connection, extendedData, parentKey);
        case NavigationQueryResultType::ECInstanceNodes:
            return std::make_unique<ECInstanceNodeReader>(factory, contractProvider, connection, extendedData, parentKey);
        case NavigationQueryResultType::MultiECInstanceNodes:
            if (extendedData.HideNodesInHierarchy())
                return std::make_unique<MergingMultiECInstanceNodeReader>(factory, contractProvider, connection, extendedData, parentKey);
            return std::make_unique<MultiECInstanceNodeReader>(factory, contractProvider, connection, extendedData, parentKey);
        }
    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Unhandled navigation query result type: %d", (int)resultType));
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
