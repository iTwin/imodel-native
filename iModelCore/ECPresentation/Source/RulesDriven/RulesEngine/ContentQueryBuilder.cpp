/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentQueryBuilder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
ParsedSelectionInfo::ParsedSelectionInfo(NavNodeKeyListCR nodeKeys, INavNodeLocater const& nodesLocater, ECSchemaHelper const& helper)
    {
    Parse(nodeKeys, nodesLocater, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedSelectionInfo::GetNodeClasses(NavNodeKeyCR nodeKey, INavNodeLocater const& nodesLocater, ECSchemaHelper const& helper)
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
        if (m_classSelection.end() == m_classSelection.find(ecClass))
            m_orderedClasses.push_back(ecClass);
        m_classSelection[ecClass].push_back(instanceNodeKey.GetInstanceId());
        return;
        }
        
    // Some grouping node
    NavNodeCPtr node = nodesLocater.LocateNode(nodeKey);
    if (node.IsNull())
        {
        BeAssert(false);
        return;
        }

    NavNodeExtendedData extendedData(*node);
    bvector<ECInstanceKey> groupedInstanceKeys = extendedData.GetGroupedInstanceKeys();
    for (ECInstanceKeyCR key : groupedInstanceKeys)
        GetNodeClasses(*ECInstanceNodeKey::Create(key), nodesLocater, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedSelectionInfo::Parse(NavNodeKeyListCR keys, INavNodeLocater const& nodesLocater, ECSchemaHelper const& helper)
    {
    for (NavNodeKeyCPtr const& key : keys)
        GetNodeClasses(*key, nodesLocater, helper);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceId> const& ParsedSelectionInfo::_GetInstanceIds(ECClassCR selectClass) const
    {
    auto iter = m_classSelection.find(&selectClass);
    if (m_classSelection.end() != iter)
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
    BoundQueryRecursiveChildrenIdSet const& GetRecursiveChildrenIds(IParsedSelectionInfo const& selection, SelectClassInfo const& thisInfo)
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
            set = new BoundQueryRecursiveChildrenIdSet(relationships, forward, selection.GetInstanceIds(*thisInfo.GetPrimaryClass()));
            }
        return *set;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(SelectedNodeInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedSelectionInfo const& selection)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, selection);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        classQuery->SelectContract(*ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery), "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);

        // handle filtering by selection
        bvector<ECInstanceId> const& selectedInstanceIds = selection.GetInstanceIds(selectClassInfo.GetSelectClass());
        if (!selectedInstanceIds.empty())
            classQuery->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet(std::move(selectedInstanceIds))});

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }

    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentRelatedInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedSelectionInfo const& selection)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, selection);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
    RecursiveQueriesHelper recursiveQueries(*specificationDescriptor);

    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        classQuery->SelectContract(*ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery), "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        if (specification.IsRecursive())
            {
            // in case of recursive query just bind the children ids
            classQuery->Where("InVirtualSet(?, [this].[ECInstanceId])", 
                {new BoundQueryRecursiveChildrenIdSet(recursiveQueries.GetRecursiveChildrenIds(selection, selectClassInfo))});
            }
        else
            {
            BeAssert(!selectClassInfo.GetPathToPrimaryClass().empty());
            classQuery->Join(selectClassInfo.GetPathToPrimaryClass(), false);
            classQuery->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet(selection.GetInstanceIds(*selectClassInfo.GetPrimaryClass()))});
            }

            // handle related properties
            for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
                classQuery->Join(path, true);
            
            // handle instance filtering
            if (!specification.GetInstanceFilter().empty())
                classQuery->Where(ECExpressionsHelper(m_params.GetECExpressionsCache()).ConvertToECSql(specification.GetInstanceFilter()).c_str(), BoundQueryValuesList());

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
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification);
    if (specificationDescriptor.IsNull())
        return nullptr;
        
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        classQuery->SelectContract(*ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery), "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);

        // handle instance filtering
        if (!specification.GetInstanceFilter().empty())
            classQuery->Where(ECExpressionsHelper(m_params.GetECExpressionsCache()).ConvertToECSql(specification.GetInstanceFilter()).c_str(), BoundQueryValuesList());

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
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetRuleset(), 
        ContentDisplayType::Undefined, m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider());
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
