/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ContentSpecificationsHandler.h"
#include "ECExpressionContextsProvider.h"
#include "QueryBuilderHelpers.h"
#include "LoggingHelper.h"

#define NO_RELATED_PROPERTIES_KEYWORD "_none_"

static RelatedClassPath s_emptyRelationshipPath;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetRelatedClassAlias(ECClassCR ecClass, ContentSpecificationsHandler::Context& context)
    {
    return Utf8PrintfString("rel_%s_%s_%" PRIu64, ecClass.GetSchema().GetAlias().c_str(), ecClass.GetName().c_str(), (uint64_t)context.GetClassCount(ecClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetNavigationClassAlias(ECClassCR ecClass, ContentSpecificationsHandler::Context& context)
    {
    return Utf8PrintfString("nav_%s_%s_%" PRIu64, ecClass.GetSchema().GetAlias().c_str(), ecClass.GetName().c_str(), (uint64_t)context.GetClassCount(ecClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename SpecificationType>
static int GetRelationshipDirection(SpecificationType const& specification)
    {
    int relationshipDirection = 0;
    switch (specification.GetRequiredRelationDirection())
        {
        case RequiredRelationDirection_Forward:  relationshipDirection = (int)ECRelatedInstanceDirection::Forward; break;
        case RequiredRelationDirection_Backward: relationshipDirection = (int)ECRelatedInstanceDirection::Backward; break;
        case RequiredRelationDirection_Both:     relationshipDirection = (int)ECRelatedInstanceDirection::Forward | (int)ECRelatedInstanceDirection::Backward; break;
        }
    return relationshipDirection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateRelatedClassNamesList(bvector<RelatedClassPath> const& paths)
    {
    Utf8String list;
    ECSchemaCP prevSchema = nullptr;
    for (RelatedClassPath const& path : paths)
        {
        if (path.empty() || path.size() > 1)
            {
            BeAssert(false);
            continue;
            }
        RelatedClassCR relatedClass = path.back();
        if (prevSchema != &relatedClass.GetTargetClass()->GetSchema())
            {
            if (!list.empty())
                list.append(";");
            list.append(relatedClass.GetTargetClass()->GetSchema().GetName()).append(":");
            prevSchema = &relatedClass.GetTargetClass()->GetSchema();
            }
        if (!list.EndsWith(":"))
            list.append(",");
        list.append(relatedClass.GetTargetClass()->GetName());
        }
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecificationsHandler::AppendProperty(PropertyAppender& appender, bvector<RelatedClass>& navigationPropertyPaths, 
    ECPropertyCR prop, Utf8CP alias)
    {
    if (!appender.Supports(prop))
        return false;

    if (prop.GetIsNavigation())
        {
        RelatedClass navigationPropertyClass = GetContext().GetSchemaHelper().GetForeignKeyClass(prop);
        navigationPropertyClass.SetIsForwardRelationship(!navigationPropertyClass.IsForwardRelationship());
        navigationPropertyClass.SetTargetClassAlias(GetNavigationClassAlias(*navigationPropertyClass.GetTargetClass(), GetContext()));
        navigationPropertyClass.SetRelationshipAlias(GetNavigationClassAlias(*navigationPropertyClass.GetRelationship(), GetContext()));
        navigationPropertyPaths.push_back(navigationPropertyClass);
        alias = navigationPropertyPaths.back().GetTargetClassAlias();
        }

    return appender.Append(prop, alias);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsHandler::AppendRelatedPropertyParams
{
private:
    RelatedClassPath const& m_relatedClassPath;
    ECClassCR m_relatedClass;
    Utf8String m_relatedClassAlias;
    RelatedPropertiesSpecificationCR m_relatedPropertySpec;
    InstanceFilteringParams const& m_instanceFilteringParams;
public:
    AppendRelatedPropertyParams(RelatedClassPath const& relatedClassPath, ECClassCR relatedClass, 
        Utf8String relatedClassAlias, RelatedPropertiesSpecificationCR relatedPropertySpec, InstanceFilteringParams const& instanceFilteringParams)
        : m_relatedClassPath(relatedClassPath), m_relatedClass(relatedClass), m_relatedClassAlias(relatedClassAlias), 
        m_relatedPropertySpec(relatedPropertySpec), m_instanceFilteringParams(instanceFilteringParams)
        {}
    RelatedClassPath const& GetRelatedClassPath() const {return m_relatedClassPath;}
    ECClassCR GetRelatedClass() const {return m_relatedClass;}
    Utf8StringCR GetRelatedClassAlias() const {return m_relatedClassAlias;}
    RelatedPropertiesSpecificationCR GetRelatedPropertySpec() const {return m_relatedPropertySpec;}
    InstanceFilteringParams const& GetInstanceFilteringParams() const {return m_instanceFilteringParams;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsHandler::AppendRelatedPropertiesParams
{
private:
    RelatedClassPath const& m_relatedClassPath;
    ECClassCR m_relatedClass;
    Utf8String m_relatedClassAlias;
    RelatedPropertiesSpecificationList const& m_relatedPropertySpecs;
    InstanceFilteringParams const& m_instanceFilteringParams;
public:
    AppendRelatedPropertiesParams(RelatedClassPath const& relatedClassPath, ECClassCR relatedClass, 
        Utf8String relatedClassAlias, RelatedPropertiesSpecificationList const& specs, InstanceFilteringParams const& instanceFilteringParams)
        : m_relatedClassPath(relatedClassPath), m_relatedClass(relatedClass), m_relatedClassAlias(relatedClassAlias), 
        m_relatedPropertySpecs(specs), m_instanceFilteringParams(instanceFilteringParams)
        {}
    RelatedClassPath const& GetRelatedClassPath() const {return m_relatedClassPath;}
    ECClassCR GetRelatedClass() const {return m_relatedClass;}
    Utf8StringCR GetRelatedClassAlias() const {return m_relatedClassAlias;}
    RelatedPropertiesSpecificationList const& GetRelatedPropertySpecs() const {return m_relatedPropertySpecs;}
    InstanceFilteringParams const& GetInstanceFilteringParams() const {return m_instanceFilteringParams;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ContentSpecificationsHandler::AppendRelatedProperties(AppendRelatedPropertyParams const& params)
    {
    bvector<RelatedClassPath> allPaths;

    bvector<Utf8String> propertyNames;
    BeStringUtilities::Split(params.GetRelatedPropertySpec().GetPropertyNames().c_str(), ",", propertyNames);
    std::for_each(propertyNames.begin(), propertyNames.end(), [](Utf8StringR name){name.Trim();});
        
    Utf8String relatedClassNames = params.GetRelatedPropertySpec().GetRelatedClassNames();
    if (params.GetRelatedPropertySpec().IsPolymorphic())
        {
        bvector<RelatedClassPath> polymorphicallyRelatedClasses = GetContext().GetSchemaHelper().GetPolymorphicallyRelatedClassesWithInstances(params.GetRelatedClass(),
            params.GetRelatedPropertySpec().GetRelationshipClassNames(), (ECRelatedInstanceDirection)GetRelationshipDirection(params.GetRelatedPropertySpec()),
            params.GetRelatedPropertySpec().GetRelatedClassNames(), params.GetRelatedClassPath(), &params.GetInstanceFilteringParams());
        if (polymorphicallyRelatedClasses.empty())
            return bvector<RelatedClassPath>();
        relatedClassNames = CreateRelatedClassNamesList(polymorphicallyRelatedClasses);
        }

    ECSchemaHelper::RelationshipClassPathOptions options(params.GetRelatedClass(), GetRelationshipDirection(params.GetRelatedPropertySpec()), 0, 
        GetContext().GetRuleset().GetSupportedSchemas().c_str(), params.GetRelatedPropertySpec().GetRelationshipClassNames().c_str(), 
        relatedClassNames.c_str(), GetContext().GetRelationshipUseCounts());
    bvector<bpair<RelatedClassPath, bool>> paths = GetContext().GetSchemaHelper().GetRelationshipClassPaths(options);

    for (auto pair : paths)
        {
        BeAssert(pair.second); // assert this is an include path
        RelatedClassPath& path = pair.first;
        if (1 != path.size())
            {
            BeAssert(false);
            continue;
            }

        RelatedClass& relationshipInfo = path.back();

        RelatedClassPath propertyRelatedClassPath = params.GetRelatedClassPath();
        Utf8String targetClassAlias = propertyRelatedClassPath.empty() ? GetRelatedClassAlias(*relationshipInfo.GetSourceClass(), GetContext()) : params.GetRelatedClassAlias();
        propertyRelatedClassPath.push_back(RelatedClass(*relationshipInfo.GetTargetClass(), *relationshipInfo.GetSourceClass(), 
            *relationshipInfo.GetRelationship(), relationshipInfo.IsForwardRelationship(), 
            targetClassAlias.c_str(), relationshipInfo.GetRelationshipAlias(), relationshipInfo.IsPolymorphic()));

        // note: GetRelationshipClassPaths returns paths in opposite direction than we expect, so we have to reverse them
        relationshipInfo.SetIsForwardRelationship(!relationshipInfo.IsForwardRelationship());

        ECClassCP pathClass = relationshipInfo.GetTargetClass();
        Utf8String pathClassAlias = GetRelatedClassAlias(*pathClass, GetContext());
        relationshipInfo.SetTargetClassAlias(pathClassAlias);

        bvector<RelatedClass> navigationPropertiesPaths;
        bool appendPath = false;
        if (propertyNames.empty())
            {
            PropertyAppenderPtr appender = _CreatePropertyAppender(*pathClass, propertyRelatedClassPath, params.GetRelatedPropertySpec().GetRelationshipMeaning());
            ECPropertyIterable properties = pathClass->GetProperties(true);
            for (ECPropertyCP ecProperty : properties)
                appendPath |= AppendProperty(*appender, navigationPropertiesPaths, *ecProperty, pathClassAlias.c_str());
            }
        else if (1 == propertyNames.size() && propertyNames[0].EqualsI(NO_RELATED_PROPERTIES_KEYWORD))
            {
            // wip: log something
            }
        else
            {
            PropertyAppenderPtr appender = _CreatePropertyAppender(*pathClass, propertyRelatedClassPath, params.GetRelatedPropertySpec().GetRelationshipMeaning());
            for (Utf8StringCR propertyName : propertyNames)
                {
                ECPropertyCP ecProperty = pathClass->GetPropertyP(propertyName.c_str());
                if (nullptr != ecProperty)
                    appendPath |= AppendProperty(*appender, navigationPropertiesPaths, *ecProperty, pathClassAlias.c_str());
                }
            }

        AppendRelatedPropertiesParams nestedPropertyParams(propertyRelatedClassPath, *pathClass, pathClassAlias, 
            params.GetRelatedPropertySpec().GetNestedRelatedProperties(), params.GetInstanceFilteringParams());
        bvector<RelatedClassPath> relatedPaths = AppendRelatedProperties(nestedPropertyParams, true);
        if (!relatedPaths.empty())
            {
            for (RelatedClassPath& relatedPath : relatedPaths)
                {
                relatedPath.insert(relatedPath.begin(), relationshipInfo);
                allPaths.push_back(relatedPath);
                }
            }
        if (appendPath)
            {
            allPaths.push_back(path);
            for (RelatedClass const& navigationPropertyPath : navigationPropertiesPaths)
                allPaths.push_back({relationshipInfo, navigationPropertyPath});
            }
        }

    return allPaths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::AppendRelatedProperties(bvector<RelatedClassPath>& paths, AppendRelatedPropertiesParams const& params)
    {
    for (RelatedPropertiesSpecificationCP spec : params.GetRelatedPropertySpecs())
        {
        AppendRelatedPropertyParams specParams(params.GetRelatedClassPath(), params.GetRelatedClass(), params.GetRelatedClassAlias(), *spec, params.GetInstanceFilteringParams());
        bvector<RelatedClassPath> specPaths = AppendRelatedProperties(specParams);
        paths.insert(paths.end(), specPaths.begin(), specPaths.end());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ContentSpecificationsHandler::AppendRelatedProperties(AppendRelatedPropertiesParams const& params, bool isNested)
    {
    bvector<RelatedClassPath> paths;

    // appends from content rule
    AppendRelatedProperties(paths, params);

    if (!isNested)
        {
        // appends from content modifiers
        for (ContentModifierCP modifier : GetContext().GetRuleset().GetContentModifierRules())
            {
            if (params.GetRelatedClass().Is(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str()))
                {
                AppendRelatedPropertiesParams modifierParams(params.GetRelatedClassPath(), params.GetRelatedClass(),
                    params.GetRelatedClassAlias(), modifier->GetRelatedProperties(), params.GetInstanceFilteringParams());
                AppendRelatedProperties(paths, modifierParams);
                }
            }
        }

    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> GetRelatedClassPaths(ECSchemaHelper const& helper, ContentSpecificationsHandler::Context& context, ECClassCR nodeClass, 
    ContentRelatedInstancesSpecificationCR specification, PresentationRuleSetCR ruleset)
    {
    int skipRelatedLevel = specification.IsRecursive() ? -1 : specification.GetSkipRelatedLevel();
    ECSchemaHelper::RelationshipClassPathOptions options(nodeClass, GetRelationshipDirection(specification), 
        skipRelatedLevel, ruleset.GetSupportedSchemas().c_str(), specification.GetRelationshipClassNames().c_str(), 
        specification.GetRelatedClassNames().c_str(), context.GetRelationshipUseCounts());
    bvector<bpair<RelatedClassPath, bool>> relationshipClassPaths = helper.GetRelationshipClassPaths(options);

    bvector<RelatedClassPath> paths;
    for (bpair<RelatedClassPath, bool> const& pair : relationshipClassPaths)
        {
        BeAssert(true == pair.second && "Only included paths are supported");
        paths.push_back(pair.first);
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::AppendClass(ECClassCR ecClass, ContentSpecificationCR spec, bool isSpecificationPolymorphic, IParsedInput const* input, Utf8StringCR instanceFilter)
    {
    if (GetContext().IsClassHandled(ecClass))
        return;
    
    SelectClassInfo info(ecClass, isSpecificationPolymorphic);
    info.SetRelatedInstanceClasses(QueryBuilderHelpers::GetRelatedInstanceClasses(GetContext().GetSchemaHelper(), info.GetSelectClass(),
        spec.GetRelatedInstances(), GetContext().GetRelationshipUseCounts()));
    bvector<RelatedClass> navigationPropertiesPaths;
    PropertyAppenderPtr appender = _CreatePropertyAppender(ecClass, s_emptyRelationshipPath, RelationshipMeaning::SameInstance);
    ECPropertyIterable properties = ecClass.GetProperties(true);
    for (ECPropertyCP prop : properties)
        AppendProperty(*appender, navigationPropertiesPaths, *prop, "this");
    
    info.SetNavigationPropertyClasses(navigationPropertiesPaths);

    if (_ShouldIncludeRelatedProperties())
        {
        InstanceFilteringParams filteringParams(GetContext().GetConnection(), GetContext().GetSchemaHelper().GetECExpressionsCache(),
            input, info, nullptr, instanceFilter.c_str());
        AppendRelatedPropertiesParams params(s_emptyRelationshipPath, ecClass, "this", spec.GetRelatedProperties(), filteringParams);
        bvector<RelatedClassPath> relatedPropertyPaths = AppendRelatedProperties(params, false);
        info.SetRelatedPropertyPaths(relatedPropertyPaths);
        }

    _AppendClass(info);
    GetContext().SetClassHandled(ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::AppendClassPaths(bvector<RelatedClassPath> const& paths, ECClassCR nodeClass, ContentRelatedInstancesSpecificationCR spec, IParsedInput const& input)
    {
    InstanceFilteringParams::RecursiveQueryInfo const* recursiveInfo = nullptr;
    if (spec.IsRecursive())
        recursiveInfo = new InstanceFilteringParams::RecursiveQueryInfo(paths);

    for (RelatedClassPath path : paths)
        {
        bool isSelectPolymorphic = path.back().IsPolymorphic();
        path.Reverse("related", false);

        ECClassCR selectClass = *path.front().GetSourceClass();
        bvector<RelatedClass> navigationPropertiesPaths;
        if (!GetContext().IsClassHandled(selectClass))
            {
            PropertyAppenderPtr appender = _CreatePropertyAppender(selectClass, s_emptyRelationshipPath, RelationshipMeaning::SameInstance);
            ECPropertyIterable properties = selectClass.GetProperties(true);
            for (ECPropertyCP prop : properties)
                AppendProperty(*appender, navigationPropertiesPaths, *prop, "this");
            GetContext().AddNavigationPropertiesPaths(selectClass, navigationPropertiesPaths);
            GetContext().SetClassHandled(selectClass);
            }
        else
            {
            navigationPropertiesPaths = GetContext().GetNavigationPropertiesPaths(selectClass);
            }

        SelectClassInfo appendInfo(selectClass, isSelectPolymorphic);
        appendInfo.SetPathToPrimaryClass(path);
        appendInfo.SetRelatedInstanceClasses(QueryBuilderHelpers::GetRelatedInstanceClasses(GetContext().GetSchemaHelper(), appendInfo.GetSelectClass(),
            spec.GetRelatedInstances(), GetContext().GetRelationshipUseCounts()));
        appendInfo.SetNavigationPropertyClasses(navigationPropertiesPaths);
        
        if (_ShouldIncludeRelatedProperties())
            {
            InstanceFilteringParams filteringParams(GetContext().GetConnection(), GetContext().GetSchemaHelper().GetECExpressionsCache(),
                &input, appendInfo, recursiveInfo, spec.GetInstanceFilter().c_str());
            AppendRelatedPropertiesParams params(s_emptyRelationshipPath, selectClass, "this", spec.GetRelatedProperties(), filteringParams);
            bvector<RelatedClassPath> relatedPropertyPaths = AppendRelatedProperties(params, false);
            appendInfo.SetRelatedPropertyPaths(relatedPropertyPaths);
            }

        _AppendClass(appendInfo);
        }

    DELETE_AND_CLEAR(recursiveInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsECClassAccepted(SelectedNodeInstancesSpecificationCR specification, ECClassCR selectedClass)
    {
    if (!specification.GetAcceptableSchemaName().empty() && !specification.GetAcceptableSchemaName().Equals(selectedClass.GetSchema().GetName()))
        return false;

    if (!specification.GetAcceptableClassNames().empty())
        {
        bool didFindAccepted = false;
        bvector<Utf8String> classNames;
        BeStringUtilities::Split(specification.GetAcceptableClassNames().c_str(), ",", classNames);
        for (Utf8String className : classNames)
            {
            className.Trim();
            if (className.Equals(selectedClass.GetName())
                || specification.GetAcceptablePolymorphically() && selectedClass.Is(selectedClass.GetSchema().GetName().c_str(), className.c_str()))
                {
                didFindAccepted = true;
                break;
                }
            }
        return didFindAccepted;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ECClassCP> CollectContentModifiers(ECSchemaHelper const& helper, PresentationRuleSetCR ruleset)
    {
    bset<ECClassCP> classes;
    for (ContentModifierCP modifier : ruleset.GetContentModifierRules())
        {
        ECClassCP ecClass = helper.GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
        if (nullptr == ecClass)
            BeAssert(false);
        else
            classes.insert(ecClass); 
        }
    for (InstanceLabelOverrideCP labelOverride : ruleset.GetInstanceLabelOverrides())
        classes.insert(helper.GetECClass(labelOverride->GetClassName().c_str())); 
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessQueryClassesPolymorphically(bvector<SupportedEntityClassInfo>& infos, bset<ECClassCP> const& modifierClassList, SchemaManagerCR schemas)
    {
    bset<SupportedEntityClassInfo*> infosWithBasePrimaryClasses;
    bset<SupportedEntityClassInfo> infosToAppend;
    for (SupportedEntityClassInfo& info : infos)
        {
        if (!info.IsPolymorphic())
            continue;

        for (ECClassCP modifierClass : modifierClassList)
            {
            // check the primary select class
            if (modifierClass != &info.GetClass() && modifierClass->Is(&info.GetClass()))
                {
                // the rule wants to customize a subclass of the class and leave other subclasses unsorted -
                // this means we have to expand the ecClass into its subclasses
                bvector<SupportedEntityClassInfo> subclassInfos;
                subclassInfos.reserve(schemas.GetDerivedClasses(info.GetClass()).size());
                for (ECClassCP derived : schemas.GetDerivedClasses(info.GetClass()))
                    {
                    SupportedEntityClassInfo copy(*derived->GetEntityClassCP());
                    copy.SetFlags((int)(info.GetFlags() | CLASS_FLAG_Polymorphic));
                    subclassInfos.push_back(std::move(copy));
                    }

                ProcessQueryClassesPolymorphically(subclassInfos, modifierClassList, schemas);
                infosWithBasePrimaryClasses.insert(&info);
                infosToAppend.insert(subclassInfos.begin(), subclassInfos.end());
                }
            }
        }

    // the primary and related classes that were polymorphic and have been split into subclasses
    // should be changed to be selected non-polymorphically
    for (SupportedEntityClassInfo* selectInfo : infosWithBasePrimaryClasses)
        selectInfo->SetFlags(selectInfo->GetFlags() & ~CLASS_FLAG_Polymorphic);

    // don't want to append selects that are already in the input vector
    for (SupportedEntityClassInfo const& selectInfo : infos)
        {
        auto iter = infosToAppend.find(selectInfo);
        if (infosToAppend.end() != iter)
            infosToAppend.erase(iter);
        }

    // append the additional selects
    for (SupportedEntityClassInfo const& info : infosToAppend)
        infos.push_back(info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessRelationshipPathsPolymorphically(bvector<RelatedClassPath>& relatedPaths, bset<ECClassCP> const& modifierClassList, SchemaManagerCR schemas)
    {
    bvector<RelatedClassPath> childPaths;
    for (ECClassCP modifierClass : modifierClassList)
        {
        for (RelatedClassPath& related : relatedPaths)
            {
            if (!related.back().IsPolymorphic() || !modifierClass->Is(related.back().GetTargetClass()))
                continue;

            for (ECClassCP derived : schemas.GetDerivedClasses(*related.back().GetTargetClass()))
                {
                RelatedClassPath copy(related);
                copy.back().SetTargetClass(*derived->GetEntityClassCP());
                childPaths.push_back(copy);
                }
            related.back().SetIsPolymorphic(false);
            ProcessRelationshipPathsPolymorphically(childPaths, modifierClassList, schemas);
            }
        }
    for (RelatedClassPath const& relatedPath : childPaths)
        relatedPaths.push_back(relatedPath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ECClassCP> const& ContentSpecificationsHandler::GetModifierClasses() const
    {
    if (nullptr == m_modifierClasses)
        m_modifierClasses = new bset<ECClassCP>(CollectContentModifiers(GetContext().GetSchemaHelper(), GetContext().GetRuleset()));
    return *m_modifierClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::_OnBeforeAppendClassInfos(bvector<SupportedEntityClassInfo>& infos)
    {
    ProcessQueryClassesPolymorphically(infos, GetModifierClasses(), GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::_OnBeforeAppendClassPaths(bvector<RelatedClassPath>& paths)
    {
    ProcessRelationshipPathsPolymorphically(paths, GetModifierClasses(), GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::HandleSpecification(SelectedNodeInstancesSpecificationCR specification, IParsedInput const& input)
    {
    if (input.GetClasses().empty())
        return;
    
    bvector<SupportedEntityClassInfo> classInfos;
    for (ECClassCP inputClass : input.GetClasses())
        {
        if (inputClass->IsEntityClass())
            classInfos.push_back(SupportedEntityClassInfo(*inputClass->GetEntityClassCP()));
        }
    _OnBeforeAppendClassInfos(classInfos);
    
    for (SupportedEntityClassInfo const& classInfo : classInfos)
        {
        if (!IsECClassAccepted(specification, classInfo.GetClass()))
            continue;
        
        AppendClass(classInfo.GetClass(), specification, classInfo.IsPolymorphic(), &input, "");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::HandleSpecification(ContentRelatedInstancesSpecificationCR specification, IParsedInput const& input)
    {
    if (input.GetClasses().empty())
        return;

    for (ECClassCP ecClass : input.GetClasses())
        {
        bvector<RelatedClassPath> paths = GetRelatedClassPaths(GetContext().GetSchemaHelper(), GetContext(), *ecClass, specification, GetContext().GetRuleset());
        _OnBeforeAppendClassPaths(paths);
        AppendClassPaths(paths, *ecClass, specification, input);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::HandleSpecification(ContentInstancesOfSpecificClassesSpecificationCR specification)
    {
    SupportedEntityClassInfos classInfos = GetContext().GetSchemaHelper().GetECClassesFromClassList(specification.GetClassNames(), false);
    bvector<SupportedEntityClassInfo> classInfosVec;
    classInfosVec.reserve(classInfos.size());
    for (SupportedEntityClassInfo& classInfo : classInfos)
        {
        int flags = classInfo.GetFlags();
        classInfo.SetFlags(specification.GetArePolymorphic() ? (flags | CLASS_FLAG_Polymorphic) : (flags & ~CLASS_FLAG_Polymorphic));
        classInfosVec.push_back(std::move(classInfo));
        }
    _OnBeforeAppendClassInfos(classInfosVec);

    for (SupportedEntityClassInfo const& classInfo : classInfosVec)
        AppendClass(classInfo.GetClass(), specification, classInfo.IsPolymorphic(), nullptr, specification.GetInstanceFilter());
    }
