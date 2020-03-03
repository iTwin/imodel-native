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

#define ENUM_FLAG(enum_value)           (int)(enum_value)
#define DISPLAY_TYPES_EQUAL(lhs, rhs)   lhs == rhs || 0 == strcmp(lhs, rhs)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentSpecificationsHandler::GetDefaultContentFlags(Utf8CP displayType, ContentSpecificationCR spec)
    {
    int flags = 0;
    if (DISPLAY_TYPES_EQUAL(ContentDisplayType::Grid, displayType))
        flags |= ENUM_FLAG(ContentFlags::ShowLabels);
    else if (DISPLAY_TYPES_EQUAL(ContentDisplayType::PropertyPane, displayType))
        flags |= ENUM_FLAG(ContentFlags::MergeResults);
    else if (DISPLAY_TYPES_EQUAL(ContentDisplayType::Graphics, displayType))
        flags |= ENUM_FLAG(ContentFlags::NoFields) | ENUM_FLAG(ContentFlags::KeysOnly) | ENUM_FLAG(ContentFlags::SkipInstancesCheck);
    else if (DISPLAY_TYPES_EQUAL(ContentDisplayType::List, displayType))
        flags |= ENUM_FLAG(ContentFlags::NoFields) | ENUM_FLAG(ContentFlags::ShowLabels) | ENUM_FLAG(ContentFlags::SkipInstancesCheck);
    if (spec.GetShowImages())
        flags |= ENUM_FLAG(ContentFlags::ShowImages);
    return flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentSpecificationsHandler::_GetContentFlags(ContentSpecificationCR spec) const
    {
    int defaultFlags = GetDefaultContentFlags(GetContext().GetPreferredDisplayType(), spec);
    if (GetContext().GetContentFlagsCalculator())
        return GetContext().GetContentFlagsCalculator()(defaultFlags);
    return defaultFlags;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> GetPropertyPathsWithTargetInstances(RelatedClassPathCR pathFromInputToSelectClass, bvector<RelatedClassPath> const& pathsFromSelectToPropertyClass,
    InstanceFilteringParams const& filteringParams, ECSchemaHelper const& schemaHelper)
    {
    bvector<RelatedClassPath> filtered;
    for (RelatedClassPathCR propertyPath : pathsFromSelectToPropertyClass)
        {
        if (schemaHelper.DoesRelatedPropertyPathHaveInstances(pathFromInputToSelectClass, propertyPath, &filteringParams))
            filtered.push_back(propertyPath);
        }
    return filtered;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ContentSpecificationsHandler::CreateRelatedPropertyPaths(RelatedClassPathCR pathToSelectClass, ECClassCR selectClass,
    InstanceFilteringParams const& filteringParams, RelatedPropertiesSpecificationCR spec)
    {
    bool mergePolymorphicPaths = !spec.IsPolymorphic();
    bvector<RelatedClassPath> relatedPropertyPaths;
    if (nullptr == spec.GetPropertiesSource())
        {
        // deprecated
        ECSchemaHelper::RelationshipClassPathOptions options(selectClass, GetRelationshipDirection(spec), 0,
            GetContext().GetRuleset().GetSupportedSchemas().c_str(), spec.GetRelationshipClassNames().c_str(),
            spec.GetRelatedClassNames().c_str(), mergePolymorphicPaths, GetContext().GetRelationshipUseCounts());
        relatedPropertyPaths = GetContext().GetSchemaHelper().GetRelationshipClassPaths(options);
        }
    else
        {
        ECSchemaHelper::MultiRelationshipPathOptions options(selectClass, *spec.GetPropertiesSource(), mergePolymorphicPaths, GetContext().GetRelationshipUseCounts());
        relatedPropertyPaths = GetContext().GetSchemaHelper().GetRelationshipClassPaths(options);
        }
    if (spec.IsPolymorphic())
        relatedPropertyPaths = GetPropertyPathsWithTargetInstances(pathToSelectClass, relatedPropertyPaths, filteringParams, GetContext().GetSchemaHelper());
    return relatedPropertyPaths;
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
    bvector<RelatedClassPath> pathsFromSourceToPropertyClass = CreateRelatedPropertyPaths(params.GetPathFromSelectToSourceClass(), params.GetSourceClass(),
        params.GetInstanceFilteringParams(), params.GetRelatedPropertySpec());
    for (RelatedClassPath& pathFromSourceToPropertyClass : pathsFromSourceToPropertyClass)
        {
        pathFromSourceToPropertyClass.back().SetTargetClassAlias(GetRelatedClassAlias(pathFromSourceToPropertyClass.back().GetTargetClass().GetClass(), GetContext()));

        RelatedClassPath pathFromSelectToPropertyClass = params.GetPathFromSelectToSourceClass();
        ContainerHelpers::Push(pathFromSelectToPropertyClass, pathFromSourceToPropertyClass);

        ECClassCR targetClass = pathFromSelectToPropertyClass.back().GetTargetClass().GetClass();
        Utf8CP targetClassAlias = pathFromSelectToPropertyClass.back().GetTargetClassAlias();

        bvector<RelatedClass> navigationPropertiesPaths;
        bool appendNavigationPropertyPaths = false;
        if (params.GetRelatedPropertySpec().GetIncludeNoProperties())
            {
            // wip: log something
            }
        else
            {
            PropertyAppenderPtr appender = _CreatePropertyAppender(targetClass, pathFromSelectToPropertyClass,
                params.GetRelatedPropertySpec().GetRelationshipMeaning(), params.GetRelatedPropertySpec().ShouldAutoExpand(), &params.GetScopeCategorySpecifications());
            if (params.GetRelatedPropertySpec().GetIncludeAllProperties())
                {
                ECPropertyIterable properties = targetClass.GetProperties(true);
                for (ECPropertyCP ecProperty : properties)
                    appendNavigationPropertyPaths |= AppendProperty(*appender, navigationPropertiesPaths, *ecProperty, targetClassAlias, nullptr);
                }
            else
                {
                for (PropertySpecificationCP spec : params.GetRelatedPropertySpec().GetProperties())
                    {
                    ECPropertyCP ecProperty = targetClass.GetPropertyP(spec->GetPropertyName().c_str());
                    if (nullptr != ecProperty)
                        appendNavigationPropertyPaths |= AppendProperty(*appender, navigationPropertiesPaths, *ecProperty, targetClassAlias, spec);
                    }
                }
            }

        AppendRelatedPropertiesParams nestedPropertyParams(pathFromSelectToPropertyClass, targetClass, targetClassAlias,
            params.GetRelatedPropertySpec().GetNestedRelatedProperties(), params.GetInstanceFilteringParams(), params.GetScopeCategorySpecifications());
        bvector<RelatedClassPath> nestedRelatedPropertyPaths = AppendRelatedProperties(nestedPropertyParams, true);
        for (RelatedClassPath const& nestedRelatedPropertyPath : nestedRelatedPropertyPaths)
            {
            RelatedClassPath fullPathFromSourceToNestedPropertyClass(pathFromSourceToPropertyClass);
            ContainerHelpers::Push(fullPathFromSourceToNestedPropertyClass, nestedRelatedPropertyPath);
            resultPathsFromSourceToPropertyClasses.push_back(fullPathFromSourceToNestedPropertyClass);
            }
        if (appendNavigationPropertyPaths)
            {
            resultPathsFromSourceToPropertyClasses.push_back(pathFromSourceToPropertyClass);
            for (RelatedClass const& navigationPropertiesPath : navigationPropertiesPaths)
                {
                RelatedClassPath fullPathFromSourceToNavigationPropertyClass(pathFromSourceToPropertyClass);
                fullPathFromSourceToNavigationPropertyClass.push_back(navigationPropertiesPath);
                resultPathsFromSourceToPropertyClasses.push_back(fullPathFromSourceToNavigationPropertyClass);
                }
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
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> FilterOutAbstractTargetClasses(bvector<RelatedClassPath> const& paths)
    {
    bvector<RelatedClassPath> filteredPaths;
    for (RelatedClassPathCR path : paths)
        {
        if (path.empty())
            continue;

        ECClassCR targetClass = path.back().GetTargetClass().GetClass();
        if (targetClass.GetClassModifier() == ECClassModifier::Abstract)
            continue;

        filteredPaths.push_back(path);
        }
    return filteredPaths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> GetRelatedClassPaths(ContentSpecificationsHandler::Context& context, ECClassCR nodeClass,
    IParsedInput const& input, ContentRelatedInstancesSpecificationCR specification, bool mergePolymorphicPaths)
    {
    bvector<RelatedClassPath> paths;
    if (specification.GetRelationshipPaths().empty())
        {
        // deprecated
        int skipRelatedLevel = specification.IsRecursive() ? -1 : specification.GetSkipRelatedLevel();
        ECSchemaHelper::RelationshipClassPathOptions options(nodeClass, GetRelationshipDirection(specification),
            skipRelatedLevel, context.GetRuleset().GetSupportedSchemas().c_str(), specification.GetRelationshipClassNames().c_str(),
            specification.GetRelatedClassNames().c_str(), mergePolymorphicPaths, context.GetRelationshipUseCounts());
        paths = context.GetSchemaHelper().GetRelationshipClassPaths(options);
        }
    else
        {
        paths = context.GetSchemaHelper().GetRecursiveRelationshipClassPaths(nodeClass, input.GetInstanceIds(nodeClass),
            specification.GetRelationshipPaths(), context.GetRelationshipUseCounts(), mergePolymorphicPaths);
        }

    if (!mergePolymorphicPaths)
        {
        // need to reduce the number of paths...
        paths = FilterOutAbstractTargetClasses(paths);
        }

    return paths;
    }

#define COPY_INDEXED_ITEMS(target, source, indexes) \
    for (size_t index : indexes) \
        target.push_back(source[index]);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<size_t> ExecuteAndResetInstanceFilteringQuery(GenericQueryPtr& query, IConnectionCR connection)
    {
    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(),
        connection.GetDb(), query->ToString().c_str());
    if (stmt.IsNull())
        {
        BeAssert(false);
        query = nullptr;
        return bvector<size_t>();
        }
    query->BindValues(*stmt);

    bvector<size_t> indexes;
    while (BE_SQLITE_ROW == stmt->Step())
        indexes.push_back(stmt->GetValueInt(0));

    query = nullptr;
    return indexes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ContentSource> FilterContentSourcesWithInstances(bvector<ContentSource> const& sources, IParsedInput const& input, Utf8StringCR instanceFilter, ContentSpecificationsHandler::Context& context)
    {
    bvector<ContentSource> filteredSources;
    if (sources.empty())
        return filteredSources;

    GenericQueryPtr query;
    for (size_t i = 0; i < sources.size(); ++i)
        {
        ContentSource const& source = sources[i];

        SelectClassInfo selectInfo(source.GetSelectClass());
        selectInfo.SetPathFromInputToSelectClass(source.GetPathFromInputToSelectClass());
        selectInfo.SetRelatedInstancePaths(source.GetPathsFromSelectToRelatedInstanceClasses());

        InstanceFilteringParams filteringParams(context.GetConnection(), context.GetSchemaHelper().GetECExpressionsCache(),
            &input, selectInfo, nullptr, instanceFilter.c_str());

        ComplexGenericQueryPtr q = ComplexGenericQuery::Create();
        q->SelectContract(*SimpleQueryContract::Create(*PresentationQueryContractSimpleField::Create("Index", std::to_string(i).c_str(), false)));
        q->From(source.GetSelectClass(), "this");
        if (InstanceFilteringResult::NoResults == QueryBuilderHelpers::ApplyInstanceFilter(*q, filteringParams, RelatedClassPath()))
            continue;
        for (RelatedClassPathCR relatedInstancePath : source.GetPathsFromSelectToRelatedInstanceClasses())
            q->Join(relatedInstancePath);
        q->Limit(1);

        QueryBuilderHelpers::SetOrUnion<GenericQuery>(query, ComplexGenericQuery::Create()->SelectAll().From(*q));

        if ((i % 500) == 499)
            COPY_INDEXED_ITEMS(filteredSources, sources, ExecuteAndResetInstanceFilteringQuery(query, context.GetConnection()));
        }

    COPY_INDEXED_ITEMS(filteredSources, sources, ExecuteAndResetInstanceFilteringQuery(query, context.GetConnection()));

    return filteredSources;
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

    SelectClassInfo selectInfo(contentSource.GetSelectClass());
    selectInfo.SetPathFromInputToSelectClass(contentSource.GetPathFromInputToSelectClass());
    selectInfo.SetNavigationPropertyClasses(navigationPropertiesPaths);
    selectInfo.SetRelatedInstancePaths(contentSource.GetPathsFromSelectToRelatedInstanceClasses());

    if (_ShouldIncludeRelatedProperties())
        {
        InstanceFilteringParams filteringParams(GetContext().GetConnection(), GetContext().GetSchemaHelper().GetECExpressionsCache(),
            input, selectInfo, recursiveFilteringInfo, instanceFilter.c_str());
        AppendRelatedPropertiesParams params(s_emptyRelationshipPath, selectClass.GetClass(), "this", spec.GetRelatedProperties(), filteringParams, spec.GetPropertyCategories());
        bvector<RelatedClassPath> relatedPropertyPaths = AppendRelatedProperties(params, false);
        selectInfo.SetRelatedPropertyPaths(relatedPropertyPaths);
        }

    _AppendClass(selectInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static ECSchemaCP FindECSchemaOnBaseClasses(Utf8StringCR schemaName, ECClassCR ecClass)
    {
    if (ecClass.GetSchema().GetName().Equals(schemaName))
        return &ecClass.GetSchema();

    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        ECSchemaCP matchingBaseClassSchema = FindECSchemaOnBaseClasses(schemaName, *baseClass);
        if (nullptr != matchingBaseClassSchema)
            return matchingBaseClassSchema;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsECClassAccepted(SelectedNodeInstancesSpecificationCR specification, ECClassCR selectedClass)
    {
    ECSchemaCP acceptableSchema = nullptr;
    if (!specification.GetAcceptableSchemaName().empty())
        {
        if (specification.GetAcceptablePolymorphically())
            {
            acceptableSchema = FindECSchemaOnBaseClasses(specification.GetAcceptableSchemaName(), selectedClass);
            if (nullptr == acceptableSchema)
                return false;
            }
        else 
            {
            if (specification.GetAcceptableSchemaName().Equals(selectedClass.GetSchema().GetName()))
                acceptableSchema = &selectedClass.GetSchema();
            else
                return false;
            }
        }

    if (!specification.GetAcceptableClassNames().empty())
        {
        bool didFindAccepted = false;
        bvector<Utf8String> classNames;
        BeStringUtilities::Split(specification.GetAcceptableClassNames().c_str(), ",", classNames);
        for (Utf8String className : classNames)
            {
            className.Trim();
            if (className.Equals(selectedClass.GetName())
                || specification.GetAcceptablePolymorphically() && selectedClass.Is(selectedClass.GetSchema().GetName().c_str(), className.c_str())
                || specification.GetAcceptablePolymorphically() && nullptr != acceptableSchema && selectedClass.Is(acceptableSchema->GetName().c_str(), className.c_str()))
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
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static void JoinRelatedInstancePaths(bvector<ContentSource>& sources, bvector<RelatedClassPath> const& pathsFromSelectToRelatedInstanceClass)
    {
    bvector<ContentSource> newSources;
    for (ContentSource const& source : sources)
        {
        for (RelatedClassPath const& path : pathsFromSelectToRelatedInstanceClass)
            {
            ContentSource copy(source);
            copy.GetPathsFromSelectToRelatedInstanceClasses().push_back(path);
            newSources.push_back(copy);
            }
        }
    sources.swap(newSources);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ContentSource> CreateContentSourcesWithRelatedInstancePaths(ContentSource const& source,
    RelatedInstanceSpecificationList const& relatedInstanceSpecs, ContentSpecificationsHandler::Context const& context)
    {
    bvector<ContentSource> sources = {source};
    bmap<Utf8String, bvector<RelatedClassPath>> relatedInstancePaths = context.GetSchemaHelper().GetRelatedInstancePaths(
        source.GetSelectClass().GetClass(), relatedInstanceSpecs, context.GetRelationshipUseCounts());
    for (auto const& relatedInstancePathEntry : relatedInstancePaths)
        {
        // note: if a single related instance specification results in more than one path, we have to
        // multiply content sources to avoid having multiple paths based on the same specification being
        // assigned to the same content source
        JoinRelatedInstancePaths(sources, relatedInstancePathEntry.second);
        }
    return sources;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::CreateContentSources(SelectClassWithExcludes const& selectClass, ECClassCP propertiesSourceClass, ContentSpecificationCR spec) const
    {
    return CreateContentSourcesWithRelatedInstancePaths(ContentSource(selectClass, propertiesSourceClass),
        spec.GetRelatedInstances(), GetContext());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::_BuildContentSource(bvector<SelectClass> const& selectClasses, ContentSpecificationCR spec)
    {
    bvector<ContentSource> sources;
    bvector<SelectClassSplitResult> splitResults = QueryBuilderHelpers::ProcessSelectClassesBasedOnCustomizationRules(selectClasses, GetCustomizationRuleInfos(),
        GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    for (SelectClassSplitResult const& split : splitResults)
        {
        ECClassCP propertiesSourceClass = split.GetSplitPath().empty() ? nullptr : &split.GetSplitPath().front().GetClass();
        ContainerHelpers::Push(sources, CreateContentSources(split.GetSelectClass(), propertiesSourceClass, spec));
        }
    return sources;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::CreateContentSources(RelatedClassPath const& pathFromInputToSelectClass, ContentSpecificationCR spec) const
    {
    ContentSource source(pathFromInputToSelectClass.back().GetTargetClass(), nullptr);
    source.SetPathFromInputToSelectClass(pathFromInputToSelectClass);
    for (RelatedClass& rc : source.GetPathFromInputToSelectClass())
        rc.SetIsTargetOptional(false);
    return CreateContentSourcesWithRelatedInstancePaths(source, spec.GetRelatedInstances(), GetContext());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::_BuildContentSource(bvector<RelatedClassPath> const& paths, ContentSpecificationCR spec)
    {
    bvector<ContentSource> sources;
    bvector<RelatedClassPath> splitPaths = QueryBuilderHelpers::ProcessRelationshipPathsBasedOnCustomizationRules(paths, GetCustomizationRuleInfos(),
        GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    for (RelatedClassPathCR path : splitPaths)
        ContainerHelpers::Push(sources, CreateContentSources(path, spec));
    return sources;
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
    bvector<ContentSource> contentSource = _BuildContentSource(selectClasses, specification);
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

    bool skipContentWithInstancesCheck = (specification.IsRecursive() || 0 != (_GetContentFlags(specification) & (int)ContentFlags::SkipInstancesCheck));

    for (ECClassCP ecClass : input.GetClasses())
        {
        bvector<RelatedClassPath> paths = GetRelatedClassPaths(GetContext(), *ecClass, input, specification, skipContentWithInstancesCheck);
        bvector<ContentSource> contentSource = _BuildContentSource(paths, specification);
        if (!skipContentWithInstancesCheck)
            contentSource = FilterContentSourcesWithInstances(contentSource, input, specification.GetInstanceFilter(), GetContext());
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
    bvector<ContentSource> contentSource = _BuildContentSource(selectClasses, specification);
    for (ContentSource const& src : contentSource)
        AppendContent(src, specification, nullptr, specification.GetInstanceFilter(), nullptr);
    }
