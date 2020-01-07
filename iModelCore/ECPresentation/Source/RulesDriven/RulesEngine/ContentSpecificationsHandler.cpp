/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ContentSpecificationsHandler.h"
#include "ECExpressionContextsProvider.h"
#include "LoggingHelper.h"

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
        if (prevSchema != &relatedClass.GetTargetClass().GetClass().GetSchema())
            {
            if (!list.empty())
                list.append(";");
            list.append(relatedClass.GetTargetClass().GetClass().GetSchema().GetName()).append(":");
            prevSchema = &relatedClass.GetTargetClass().GetClass().GetSchema();
            }
        if (!list.EndsWith(":"))
            list.append(",");
        list.append(relatedClass.GetTargetClass().GetClass().GetName());
        }
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* note: overrides need to be specified only if the appended property has them set specifically,
* e.g. as in 'related properties' specification. otherwise, when overrides are specified
* in content specification or content modifier, the overrides are taken care of by PropertyInfoStore.
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecificationsHandler::AppendProperty(PropertyAppender& appender, bvector<RelatedClass>& navigationPropertyPaths,
    ECPropertyCR prop, Utf8CP alias, PropertySpecificationCP overrides)
    {
    if (!appender.Supports(prop, overrides))
        return false;

    if (prop.GetIsNavigation())
        {
        RelatedClass navigationPropertyClass = GetContext().GetSchemaHelper().GetForeignKeyClass(prop);
        navigationPropertyClass.SetTargetClassAlias(GetNavigationClassAlias(navigationPropertyClass.GetTargetClass().GetClass(), GetContext()));
        navigationPropertyClass.SetRelationshipAlias(GetNavigationClassAlias(*navigationPropertyClass.GetRelationship(), GetContext()));
        navigationPropertyPaths.push_back(navigationPropertyClass);
        alias = navigationPropertyPaths.back().GetTargetClassAlias();
        }

    return appender.Append(prop, alias, overrides);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsHandler::AppendRelatedPropertyParams
{
private:
    RelatedClassPath const& m_pathFromSelectToSourceClass;
    ECClassCR m_sourceClass;
    Utf8String m_sourceClassAlias;
    RelatedPropertiesSpecificationCR m_relatedPropertySpec;
    InstanceFilteringParams const& m_instanceFilteringParams;
    PropertyCategorySpecificationsList const& m_scopeCategorySpecifications;
public:
    AppendRelatedPropertyParams(RelatedClassPath const& pathFromSelectToSourceClass, ECClassCR sourceClass,
        Utf8String sourceClassAlias, RelatedPropertiesSpecificationCR relatedPropertySpec, InstanceFilteringParams const& instanceFilteringParams,
        PropertyCategorySpecificationsList const& scopeCategorySpecifications)
        : m_pathFromSelectToSourceClass(pathFromSelectToSourceClass), m_sourceClass(sourceClass), m_sourceClassAlias(sourceClassAlias),
        m_relatedPropertySpec(relatedPropertySpec), m_instanceFilteringParams(instanceFilteringParams), m_scopeCategorySpecifications(scopeCategorySpecifications)
        {}
    RelatedClassPath const& GetPathFromSelectToSourceClass() const {return m_pathFromSelectToSourceClass;}
    ECClassCR GetSourceClass() const {return m_sourceClass;}
    Utf8StringCR GetSourceClassAlias() const {return m_sourceClassAlias;}
    RelatedPropertiesSpecificationCR GetRelatedPropertySpec() const {return m_relatedPropertySpec;}
    InstanceFilteringParams const& GetInstanceFilteringParams() const {return m_instanceFilteringParams;}
    PropertyCategorySpecificationsList const& GetScopeCategorySpecifications() const {return m_scopeCategorySpecifications;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsHandler::AppendRelatedPropertiesParams
{
private:
    RelatedClassPath const& m_pathFromSelectToSourceClass;
    ECClassCR m_sourceClass;
    Utf8String m_sourceClassAlias;
    RelatedPropertiesSpecificationList const& m_relatedPropertySpecs;
    InstanceFilteringParams const& m_instanceFilteringParams;
    PropertyCategorySpecificationsList const& m_scopeCategorySpecifications;
public:
    AppendRelatedPropertiesParams(RelatedClassPath const& pathFromSelectToSourceClass, ECClassCR sourceClass,
        Utf8String sourceClassAlias, RelatedPropertiesSpecificationList const& specs, InstanceFilteringParams const& instanceFilteringParams,
        PropertyCategorySpecificationsList const& scopeCategorySpecifications)
        : m_pathFromSelectToSourceClass(pathFromSelectToSourceClass), m_sourceClass(sourceClass), m_sourceClassAlias(sourceClassAlias),
        m_relatedPropertySpecs(specs), m_instanceFilteringParams(instanceFilteringParams), m_scopeCategorySpecifications(scopeCategorySpecifications)
        {}
    RelatedClassPath const& GetPathFromSelectToSourceClass() const {return m_pathFromSelectToSourceClass;}
    ECClassCR GetSourceClass() const {return m_sourceClass;}
    Utf8StringCR GetSourceClassAlias() const {return m_sourceClassAlias;}
    RelatedPropertiesSpecificationList const& GetRelatedPropertySpecs() const {return m_relatedPropertySpecs;}
    InstanceFilteringParams const& GetInstanceFilteringParams() const {return m_instanceFilteringParams;}
    PropertyCategorySpecificationsList const& GetScopeCategorySpecifications() const {return m_scopeCategorySpecifications;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ContentSpecificationsHandler::AppendRelatedProperties(AppendRelatedPropertyParams const& params)
    {
    bvector<RelatedClassPath> resultPathsFromSourceToPropertyClasses;

    Utf8String relatedClassNames = params.GetRelatedPropertySpec().GetRelatedClassNames();
    if (params.GetRelatedPropertySpec().IsPolymorphic())
        {
        bvector<RelatedClassPath> polymorphicallyRelatedClasses = GetContext().GetSchemaHelper().GetPolymorphicallyRelatedClassesWithInstances(params.GetSourceClass(),
            params.GetRelatedPropertySpec().GetRelationshipClassNames(), (ECRelatedInstanceDirection)GetRelationshipDirection(params.GetRelatedPropertySpec()),
            params.GetRelatedPropertySpec().GetRelatedClassNames(), params.GetPathFromSelectToSourceClass(), &params.GetInstanceFilteringParams());
        if (polymorphicallyRelatedClasses.empty())
            return bvector<RelatedClassPath>();
        relatedClassNames = CreateRelatedClassNamesList(polymorphicallyRelatedClasses);
        }

    ECSchemaHelper::RelationshipClassPathOptions options(params.GetSourceClass(), GetRelationshipDirection(params.GetRelatedPropertySpec()), 0,
        GetContext().GetRuleset().GetSupportedSchemas().c_str(), params.GetRelatedPropertySpec().GetRelationshipClassNames().c_str(),
        relatedClassNames.c_str(), true, GetContext().GetRelationshipUseCounts());
    bvector<RelatedClassPath> pathsFromSourceToPropertyClass = GetContext().GetSchemaHelper().GetRelationshipClassPaths(options);
    for (RelatedClassPath& pathFromSourceToPropertyClass : pathsFromSourceToPropertyClass)
        {
        if (1 != pathFromSourceToPropertyClass.size())
            {
            BeAssert(false);
            continue;
            }

        RelatedClass& relatedPropertyClass = pathFromSourceToPropertyClass.back();
        relatedPropertyClass.SetTargetClassAlias(GetRelatedClassAlias(relatedPropertyClass.GetTargetClass().GetClass(), GetContext()));

        RelatedClassPath pathFromSelectToPropertyClass = params.GetPathFromSelectToSourceClass();
        pathFromSelectToPropertyClass.push_back(relatedPropertyClass);

        bvector<RelatedClass> navigationPropertiesPaths;
        bool appendNavigationPropertyPaths = false;
        if (params.GetRelatedPropertySpec().GetIncludeNoProperties())
            {
            // wip: log something
            }
        else
            {
            PropertyAppenderPtr appender = _CreatePropertyAppender(relatedPropertyClass.GetTargetClass().GetClass(), pathFromSelectToPropertyClass,
                params.GetRelatedPropertySpec().GetRelationshipMeaning(), params.GetRelatedPropertySpec().ShouldAutoExpand(), &params.GetScopeCategorySpecifications());
            if (params.GetRelatedPropertySpec().GetIncludeAllProperties())
                {
                ECPropertyIterable properties = relatedPropertyClass.GetTargetClass().GetClass().GetProperties(true);
                for (ECPropertyCP ecProperty : properties)
                    appendNavigationPropertyPaths |= AppendProperty(*appender, navigationPropertiesPaths, *ecProperty, relatedPropertyClass.GetTargetClassAlias(), nullptr);
                }
            else
                {
                for (PropertySpecificationCP spec : params.GetRelatedPropertySpec().GetProperties())
                    {
                    ECPropertyCP ecProperty = relatedPropertyClass.GetTargetClass().GetClass().GetPropertyP(spec->GetPropertyName().c_str());
                    if (nullptr != ecProperty)
                        appendNavigationPropertyPaths |= AppendProperty(*appender, navigationPropertiesPaths, *ecProperty, relatedPropertyClass.GetTargetClassAlias(), spec);
                    }
                }
            }

        AppendRelatedPropertiesParams nestedPropertyParams(pathFromSelectToPropertyClass, relatedPropertyClass.GetTargetClass().GetClass(), relatedPropertyClass.GetTargetClassAlias(),
            params.GetRelatedPropertySpec().GetNestedRelatedProperties(), params.GetInstanceFilteringParams(), params.GetScopeCategorySpecifications());
        bvector<RelatedClassPath> nestedRelatedPropertyPaths = AppendRelatedProperties(nestedPropertyParams, true);
        for (RelatedClassPath& nestedRelatedPropertyPath : nestedRelatedPropertyPaths)
            {
            nestedRelatedPropertyPath.insert(nestedRelatedPropertyPath.begin(), relatedPropertyClass);
            resultPathsFromSourceToPropertyClasses.push_back(nestedRelatedPropertyPath);
            }
        if (appendNavigationPropertyPaths)
            {
            resultPathsFromSourceToPropertyClasses.push_back(pathFromSourceToPropertyClass);
            for (RelatedClass const& navigationPropertiesPath : navigationPropertiesPaths)
                resultPathsFromSourceToPropertyClasses.push_back({relatedPropertyClass, navigationPropertiesPath});
            }
        }

    return resultPathsFromSourceToPropertyClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::AppendRelatedProperties(bvector<RelatedClassPath>& paths, AppendRelatedPropertiesParams const& params)
    {
    for (RelatedPropertiesSpecificationCP spec : params.GetRelatedPropertySpecs())
        {
        AppendRelatedPropertyParams specParams(params.GetPathFromSelectToSourceClass(), params.GetSourceClass(), params.GetSourceClassAlias(), *spec,
            params.GetInstanceFilteringParams(), params.GetScopeCategorySpecifications());
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
            if (params.GetSourceClass().Is(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str()))
                {
                AppendRelatedPropertiesParams modifierParams(params.GetPathFromSelectToSourceClass(), params.GetSourceClass(),
                    params.GetSourceClassAlias(), modifier->GetRelatedProperties(), params.GetInstanceFilteringParams(),
                    modifier->GetPropertyCategories());
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
        specification.GetRelatedClassNames().c_str(), false, context.GetRelationshipUseCounts());
    return helper.GetRelationshipClassPaths(options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::AppendContent(ContentSource const& contentSource, ContentSpecificationCR spec, IParsedInput const* input,
    Utf8StringCR instanceFilter, InstanceFilteringParams::RecursiveQueryInfo const* recursiveFilteringInfo)
    {
    static const RelatedClassPath s_emptyRelationshipPath;

    SelectClass const& selectClass = contentSource.GetSelectClass();
    bvector<RelatedClass> navigationPropertiesPaths;
    if (!GetContext().IsClassHandled(selectClass.GetClass()))
        {
        PropertyAppenderPtr appender = _CreatePropertyAppender(selectClass.GetClass(), s_emptyRelationshipPath, RelationshipMeaning::SameInstance, false, &spec.GetPropertyCategories());
        ECPropertyIterable properties = contentSource.GetPropertiesSource().GetProperties(true);
        for (ECPropertyCP prop : properties)
            AppendProperty(*appender, navigationPropertiesPaths, *prop, "this", nullptr);
        GetContext().AddNavigationPropertiesPaths(selectClass.GetClass(), navigationPropertiesPaths);
        GetContext().SetClassHandled(selectClass.GetClass());
        }
    else
        {
        navigationPropertiesPaths = GetContext().GetNavigationPropertiesPaths(selectClass.GetClass());
        }

    SelectClassInfo appendInfo(contentSource.GetSelectClass());
    appendInfo.SetPathToInputClass(RelatedClassPath(contentSource.GetPathFromInputToSelectClass()).Reverse("related", false));
    appendInfo.SetRelatedInstanceClasses(QueryBuilderHelpers::GetRelatedInstanceClasses(GetContext().GetSchemaHelper(), selectClass.GetClass(),
        spec.GetRelatedInstances(), GetContext().GetRelationshipUseCounts()));
    appendInfo.SetNavigationPropertyClasses(navigationPropertiesPaths);

    if (_ShouldIncludeRelatedProperties())
        {
        InstanceFilteringParams filteringParams(GetContext().GetConnection(), GetContext().GetSchemaHelper().GetECExpressionsCache(),
            input, appendInfo, recursiveFilteringInfo, instanceFilter.c_str());
        AppendRelatedPropertiesParams params(s_emptyRelationshipPath, selectClass.GetClass(), "this", spec.GetRelatedProperties(), filteringParams, spec.GetPropertyCategories());
        bvector<RelatedClassPath> relatedPropertyPaths = AppendRelatedProperties(params, false);
        appendInfo.SetRelatedPropertyPaths(relatedPropertyPaths);
        }

    _AppendClass(appendInfo);
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
static bvector<RuleApplicationInfo> CollectCustomizationRuleInfos(ECSchemaHelper const& helper, PresentationRuleSetCR ruleset)
    {
    bvector<RuleApplicationInfo> infos;
    for (ContentModifierCP modifier : ruleset.GetContentModifierRules())
        {
        ECClassCP ecClass = helper.GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
        if (nullptr == ecClass)
            {
            BeAssert(false);
            continue;
            }
        infos.push_back(RuleApplicationInfo(*ecClass, true));
        }
    for (InstanceLabelOverrideCP labelOverride : ruleset.GetInstanceLabelOverrides())
        {
        ECClassCP ecClass = helper.GetECClass(labelOverride->GetClassName().c_str());
        if (nullptr == ecClass)
            {
            BeAssert(false);
            continue;
            }
        infos.push_back(RuleApplicationInfo(*ecClass, true));
        }
    return infos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RuleApplicationInfo> const& ContentSpecificationsHandler::GetCustomizationRuleInfos() const
    {
    if (nullptr == m_customizationRuleInfos)
        m_customizationRuleInfos = new bvector<RuleApplicationInfo>(CollectCustomizationRuleInfos(GetContext().GetSchemaHelper(), GetContext().GetRuleset()));
    return *m_customizationRuleInfos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::_BuildContentSource(bvector<SelectClass> const& selectClasses)
    {
    bvector<SelectClassSplitResult> result = QueryBuilderHelpers::ProcessSelectClassesBasedOnCustomizationRules(selectClasses, GetCustomizationRuleInfos(),
        GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    return ContainerHelpers::TransformContainer<bvector<ContentSource>>(result, [](SelectClassSplitResult const& r)
        {
        return ContentSource(r.GetSelectClass(), r.GetSplitPath().empty() ? nullptr : &r.GetSplitPath().front().GetClass());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::_BuildContentSource(bvector<RelatedClassPath> const& paths)
    {
    bvector<RelatedClassPath> result = QueryBuilderHelpers::ProcessRelationshipPathsBasedOnCustomizationRules(paths, GetCustomizationRuleInfos(),
        GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    return ContainerHelpers::TransformContainer<bvector<ContentSource>>(result, std::bind(&ContentSpecificationsHandler::CreateContentSource, this, std::placeholders::_1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSource ContentSpecificationsHandler::CreateContentSource(RelatedClassPath const& path) const
    {
    ContentSource source(path.back().GetTargetClass(), nullptr);
    source.SetPathFromInputToSelectClass(path);
    for (RelatedClass& rc : source.GetPathFromInputToSelectClass())
        rc.SetIsTargetOptional(false);
    return source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<InstanceFilteringParams::RecursiveQueryInfo const> CreateRecursiveFilteringInfo(bvector<ContentSource> const& contentSource, ContentRelatedInstancesSpecificationCR spec)
    {
    if (spec.IsRecursive())
        {
        return std::make_unique<InstanceFilteringParams::RecursiveQueryInfo const>(
            ContainerHelpers::TransformContainer<bvector<RelatedClassPath>>(contentSource, [](ContentSource const& s){return s.GetPathFromInputToSelectClass();}));
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::HandleSpecification(SelectedNodeInstancesSpecificationCR specification, IParsedInput const& input)
    {
    if (input.GetClasses().empty())
        return;

    bvector<SelectClass> selectClasses;
    for (ECClassCP inputClass : input.GetClasses())
        selectClasses.push_back(SelectClass(*inputClass, false));
    bvector<ContentSource> contentSource = _BuildContentSource(selectClasses);
    for (ContentSource const& src : contentSource)
        {
        if (IsECClassAccepted(specification, src.GetSelectClass().GetClass()))
            AppendContent(src, specification, &input, "", nullptr);
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
        bvector<ContentSource> contentSource = _BuildContentSource(paths);
        std::unique_ptr<InstanceFilteringParams::RecursiveQueryInfo const> recursiveInfo = CreateRecursiveFilteringInfo(contentSource, specification);
        for (ContentSource const& src : contentSource)
            AppendContent(src, specification, &input, specification.GetInstanceFilter(), recursiveInfo.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::HandleSpecification(ContentInstancesOfSpecificClassesSpecificationCR specification)
    {
    SupportedEntityClassInfos classInfos = GetContext().GetSchemaHelper().GetECClassesFromClassList(specification.GetClassNames(), false);
    bvector<SelectClass> selectClasses = ContainerHelpers::TransformContainer<bvector<SelectClass>>(classInfos, [&](SupportedEntityClassInfo const& ci)
        {
        return SelectClass(ci.GetClass(), ci.IsPolymorphic() && specification.ShouldHandleInstancesPolymorphically());
        });
    bvector<ContentSource> contentSource = _BuildContentSource(selectClasses);
    for (ContentSource const& src : contentSource)
        AppendContent(src, specification, nullptr, specification.GetInstanceFilter(), nullptr);
    }
