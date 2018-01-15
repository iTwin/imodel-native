/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentQueryBuilder.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "QueryBuilder.h"
#include "ECExpressionContextsProvider.h"
#include "LoggingHelper.h"
#include "NavNodeProviders.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ParsedInput::ParsedInput(NavNodeKeyListCR nodeKeys, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    Parse(nodeKeys, nodesLocater, connection, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedInput::GetNodeClasses(NavNodeKeyCR nodeKey, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    if (nullptr != nodeKey.AsDisplayLabelGroupingNodeKey() && !nodeKey.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))
        {
        // Custom nodes
        // This type of nodes don't supply any content for class-based content specifications
        return;
        }

    if (nullptr != nodeKey.AsECInstanceNodeKey())
        {
        // ECInstance node
        ECInstanceNodeKey const& instanceNodeKey = *nodeKey.AsECInstanceNodeKey();
        ECClassCP ecClass = helper.GetECClass(instanceNodeKey.GetECClassId());
        if (nullptr == ecClass || !ecClass->IsEntityClass())
            {
            BeAssert(false);
            return;
            }
        if (m_classInput.end() == m_classInput.find(ecClass))
            m_orderedClasses.push_back(ecClass);
        m_classInput[ecClass].push_back(instanceNodeKey.GetInstanceId());
        return;
        }
        
    // Some grouping node
    NavNodeCPtr node = nodesLocater.LocateNode(connection, nodeKey);
    if (node.IsNull())
        {
        BeAssert(false);
        return;
        }

    NavNodeExtendedData extendedData(*node);
    bvector<ECInstanceKey> groupedInstanceKeys = extendedData.GetGroupedInstanceKeys();
    for (ECInstanceKeyCR key : groupedInstanceKeys)
        GetNodeClasses(*ECInstanceNodeKey::Create(key), nodesLocater, connection, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedInput::Parse(NavNodeKeyListCR keys, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    for (NavNodeKeyCPtr const& key : keys)
        GetNodeClasses(*key, nodesLocater, connection, helper);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceId> const& ParsedInput::_GetInstanceIds(ECClassCR selectClass) const
    {
    auto iter = m_classInput.find(&selectClass);
    if (m_classInput.end() != iter)
        return iter->second;
        
    static bvector<ECInstanceId> s_empty;
    return s_empty;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct RecursiveQueriesHelper
{
private:
    ContentDescriptorCR m_descriptor;
    BoundQueryRecursiveChildrenIdSet* m_fwdRecursiveChildrenIds;
    BoundQueryRecursiveChildrenIdSet* m_bwdRecursiveChildrenIds;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool IsRecursiveJoinForward(SelectClassInfo const& selectInfo)
        {
        if (selectInfo.GetPathToPrimaryClass().empty())
            {
            BeAssert(false);
            return true;
            }
    
        return !selectInfo.GetPathToPrimaryClass().back().IsForwardRelationship(); // invert direction
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool IsPathValidForRecursiveSelect(Utf8StringR errorMessage, RelatedClassPath const& path, ECClassCR selectClass)
        {
        RelatedClass const& relatedClassDef = path.front();
        if (!selectClass.Is(relatedClassDef.GetSourceClass()))
            {
            errorMessage = "Using IsRecursive requires recursive relationship";
            return false;
            }
        return true;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    RecursiveQueriesHelper(ContentDescriptorCR descriptor) 
        : m_descriptor(descriptor), m_fwdRecursiveChildrenIds(nullptr), m_bwdRecursiveChildrenIds(nullptr)
        {}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ~RecursiveQueriesHelper() 
        {
        DELETE_AND_CLEAR(m_bwdRecursiveChildrenIds);
        DELETE_AND_CLEAR(m_fwdRecursiveChildrenIds);
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BoundQueryRecursiveChildrenIdSet const& GetRecursiveChildrenIds(IParsedInput const& input, SelectClassInfo const& thisInfo)
        {
        bool forward = IsRecursiveJoinForward(thisInfo);
        BoundQueryRecursiveChildrenIdSet*& set = forward ? m_fwdRecursiveChildrenIds : m_bwdRecursiveChildrenIds;
        if (nullptr == set)
            {
            bset<ECRelationshipClassCP> relationships;
            for (SelectClassInfo const& selectClassInfo : m_descriptor.GetSelectClasses())
                {
                Utf8String validationErrorMessage;
                if (!IsPathValidForRecursiveSelect(validationErrorMessage, selectClassInfo.GetPathToPrimaryClass(), selectClassInfo.GetSelectClass()))
                    {
                    BeAssert(false);
                    LoggingHelper::LogMessage(Log::Content, validationErrorMessage.c_str(), NativeLogging::LOG_ERROR);
                    }
                else
                    {
                    for (RelatedClassCR rel : selectClassInfo.GetPathToPrimaryClass())
                        relationships.insert(rel.GetRelationship());
                    }
                }
            set = new BoundQueryRecursiveChildrenIdSet(relationships, forward, input.GetInstanceIds(*thisInfo.GetPrimaryClass()));
            }
        return *set;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(SelectedNodeInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedInput const& input)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, input);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery);
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);

        // handle filtering by input
        bvector<ECInstanceId> const& selectedInstanceIds = input.GetInstanceIds(selectClassInfo.GetSelectClass());
        if (!selectedInstanceIds.empty())
            classQuery->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet(std::move(selectedInstanceIds))});

        // handle selecting property for distinct values 
        if (descriptor.OnlyDistinctValues() && descriptor.GetVisibleFields().size() == 1 && nullptr != descriptor.GetVisibleFields()[0]->AsPropertiesField())
            classQuery->GroupByContract(*contract);

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }

    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentRelatedInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedInput const& input)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, input);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        RecursiveQueriesHelper recursiveQueries(*specificationDescriptor);

        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery);
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        if (specification.IsRecursive())
            {
            // in case of recursive query just bind the children ids
            classQuery->Where("InVirtualSet(?, [this].[ECInstanceId])", 
                {new BoundQueryRecursiveChildrenIdSet(recursiveQueries.GetRecursiveChildrenIds(input, selectClassInfo))});
            }
        else
            {
            BeAssert(!selectClassInfo.GetPathToPrimaryClass().empty());
            classQuery->Join(selectClassInfo.GetPathToPrimaryClass(), false);
            classQuery->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet(input.GetInstanceIds(*selectClassInfo.GetPrimaryClass()))});
            }

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);
            
        // handle instance filtering
        if (!specification.GetInstanceFilter().empty())
            classQuery->Where(ECExpressionsHelper(m_params.GetECExpressionsCache()).ConvertToECSql(specification.GetInstanceFilter()).c_str(), BoundQueryValuesList());

        // handle selecting property for distinct values 
        if (descriptor.OnlyDistinctValues() && descriptor.GetVisibleFields().size() == 1 && nullptr != descriptor.GetVisibleFields()[0]->AsPropertiesField())
            classQuery->GroupByContract(*contract);

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }

    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());

    return query;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentInstancesOfSpecificClassesSpecificationCR specification, ContentDescriptorCR descriptor)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification);
    if (specificationDescriptor.IsNull())
        return nullptr;
        
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery);
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);

        // handle instance filtering
        if (!specification.GetInstanceFilter().empty())
            classQuery->Where(ECExpressionsHelper(m_params.GetECExpressionsCache()).ConvertToECSql(specification.GetInstanceFilter()).c_str(), BoundQueryValuesList());

        // handle selecting property for distinct values 
        if (descriptor.OnlyDistinctValues() && descriptor.GetVisibleFields().size() == 1 && nullptr != descriptor.GetVisibleFields()[0]->AsPropertiesField())
            classQuery->GroupByContract(*contract);

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }
    
    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentDescriptor::NestedContentField const& contentField)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(), 
        ContentDisplayType::Undefined, m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider(), *NavNodeKeyListContainer::Create(), nullptr);
    ContentDescriptorPtr descriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(contentField);
    if (!descriptor.IsValid())
        return nullptr;
        
    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, *descriptor, &contentField.GetContentClass(), *query, false);
    query->SelectContract(*contract, contentField.GetContentClassAlias().c_str());
    query->From(contentField.GetContentClass(), true, contentField.GetContentClassAlias().c_str());

    RelatedClassPath relationshipPath;
    for (auto iter = contentField.GetRelationshipPath().rbegin(); iter != contentField.GetRelationshipPath().rend(); ++iter)
        relationshipPath.push_back(*iter);
    query->Join(relationshipPath, false);
    
    return query;
    }
