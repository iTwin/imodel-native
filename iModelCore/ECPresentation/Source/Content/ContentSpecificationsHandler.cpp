/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include "ContentSpecificationsHandler.h"
#include "../Shared/ECExpressions/ECExpressionContextsProvider.h"
#include "../Shared/Queries/QueryExecutor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentSpecificationsHandler::Context::CreateRelatedClassAlias(ECClassCR ecClass)
    {
    return RULES_ENGINE_RELATED_CLASS_ALIAS(ecClass, GetClassCount(ecClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentSpecificationsHandler::Context::CreateNavigationClassAlias(ECClassCR ecClass)
    {
    return RULES_ENGINE_NAV_CLASS_ALIAS(ecClass, GetClassCount(ecClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentSpecificationsHandler::_GetContentFlags(ContentSpecificationCR spec) const
    {
    int defaultFlags = GetDefaultContentFlags(GetContext().GetPreferredDisplayType(), spec);
    if (GetContext().GetContentFlagsCalculator())
        return GetContext().GetContentFlagsCalculator()(defaultFlags);
    return defaultFlags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionContextPtr CreateContentSpecificationInstanceFilterContext(ContentSpecificationsHandler::Context const& params)
    {
    ECExpressionContextsProvider::ContentSpecificationInstanceFilterContextParameters contextParams(params.GetConnection(),
        params.GetInputKeys(), params.GetRulesetVariables(), nullptr);
    return ECExpressionContextsProvider::GetContentSpecificationInstanceFilterContext(contextParams);
    }

/*---------------------------------------------------------------------------------**//**
* note: overrides need to be specified only if the appended property has them set specifically,
* e.g. as in 'related properties' specification. otherwise, when overrides are specified
* in content specification or content modifier, the overrides are taken care of by PropertyInfoStore.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationsHandler::PropertyAppendResult ContentSpecificationsHandler::AppendProperty(PropertyAppender& appender, ECPropertyCR prop, Utf8CP alias, PropertySpecificationsList const& overrides)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Append property `%s`", prop.GetName().c_str()));
    if (!appender.Supports(prop, overrides))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Appender skipped the property.");
        return false;
        }

    ContentSpecificationsHandler::PropertyAppendResult result = appender.Append(prop, alias, overrides);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Did append: %s", result.DidAppend() ? "TRUE" : "FALSE"));

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PropertySpecificationsList FindSpecificationsForProperty(PropertySpecificationsList const& specs, Utf8StringCR propertyName)
    {
    return ContainerHelpers::FindSubset<PropertySpecificationP>(specs, [&](auto const spec) {
        return spec->GetPropertyName().Equals(propertyName)
            || spec->GetPropertyName().Equals(INCLUDE_ALL_PROPERTIES_SPEC); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelationshipStepSpecification*> GetRelationshipStepSpecifications(RelatedPropertiesSpecificationCR relatedPropertiesSpec)
    {
    bvector<RelationshipStepSpecification*> stepSpecifications;
    if (nullptr != relatedPropertiesSpec.GetPropertiesSource())
        {
        for (auto const& step : relatedPropertiesSpec.GetPropertiesSource()->GetSteps())
            stepSpecifications.push_back(new RelationshipStepSpecification(*step));
        }
    else
        {
        // deprecated case:
        stepSpecifications.push_back(new RelationshipStepSpecification(relatedPropertiesSpec.GetRelationshipClassNames(), relatedPropertiesSpec.GetRequiredRelationDirection(), relatedPropertiesSpec.GetRelatedClassNames()));
        }
    return stepSpecifications;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static RelationshipPathSpecification* CreateCombinedRelationshipPathSpecification(RelatedPropertiesSpecificationCR lhs, RelatedPropertiesSpecificationCR rhs)
    {
    auto steps = GetRelationshipStepSpecifications(lhs);
    ContainerHelpers::Push(steps, GetRelationshipStepSpecifications(rhs));
    return new RelationshipPathSpecification(steps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>> FlattenedRelatedPropertiesSpecification::Create(RelatedPropertiesSpecificationCR spec, RelatedPropertiesSpecificationScopeInfo const& scope)
    {
    bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>> specs;
    auto currSpecInfo = std::make_unique<FlattenedRelatedPropertiesSpecification>(std::make_unique<RelatedPropertiesSpecification>(spec), scope);
    currSpecInfo->GetSpecificationsStack().push_back(&spec);
    specs.push_back(std::move(currSpecInfo));
    ContainerHelpers::MoveTransformContainer(specs, Create(spec.GetNestedRelatedProperties(), scope), [&spec](auto&& nestedSpec)
        {
        auto combinedPath = CreateCombinedRelationshipPathSpecification(spec, nestedSpec->GetFlattened());
        nestedSpec->GetFlattened().SetPropertiesSource(combinedPath);
        nestedSpec->GetSpecificationsStack().insert(nestedSpec->GetSpecificationsStack().begin(), &spec);
        return std::move(nestedSpec);
        });
    return specs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>> FlattenedRelatedPropertiesSpecification::Create(bvector<RelatedPropertiesSpecificationP> const& specPtrs, RelatedPropertiesSpecificationScopeInfo const& scope)
    {
    bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>> specs;
    for (auto const& spec : specPtrs)
        ContainerHelpers::MovePush(specs, Create(*spec, scope));
    return specs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<std::shared_ptr<RelatedPropertySpecificationPaths>> FindRelatedPropertyPaths(ContentSpecificationsHandler::Context const& context, SelectClassWithExcludes<ECClass> const& sourceClass,
    InstanceFilteringParams const& contentInstanceFilteringParams, bvector<RelatedClassPath> const& relatedInstancePaths, bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>>&& relatedPropertyAppendInfos)
    {
    // transform property specs to path specs
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs;
    bvector<std::unique_ptr<RelationshipPathSpecification>> tempPropertySources;
    for (size_t i = 0; i < relatedPropertyAppendInfos.size(); ++i)
        {
        FlattenedRelatedPropertiesSpecification const& appendInfo = *relatedPropertyAppendInfos[i];
        RelatedPropertiesSpecification const& propertySpec = appendInfo.GetFlattened();
        RelationshipPathSpecification const* propertiesSource = propertySpec.GetPropertiesSource();
        if (propertiesSource)
            {
            bvector<Utf8String> stepTargetInstanceFilters;
            stepTargetInstanceFilters.reserve(propertiesSource->GetSteps().size());
            for (auto const& stackSpec : appendInfo.GetSource())
                {
                if (stackSpec->GetPropertiesSource())
                    {
                    for (size_t specIndex = 1; specIndex < stackSpec->GetPropertiesSource()->GetSteps().size(); ++specIndex)
                        stepTargetInstanceFilters.push_back("");
                    }
                stepTargetInstanceFilters.push_back(stackSpec->GetInstanceFilter());
                }

            pathSpecs.push_back(ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification((int)i, *propertiesSource, propertySpec.IsPolymorphic(), stepTargetInstanceFilters));
            continue;
            }

        // support for the deprecated case:
        Utf8String relationshipClassNames = propertySpec.GetRelationshipClassNames();
        if (relationshipClassNames.empty())
            {
            auto relationships = context.GetSchemaHelper().GetPossibleRelationships(sourceClass.GetClass(), propertySpec.GetRequiredRelationDirection(), propertySpec.GetRelatedClassNames());
            relationshipClassNames = context.GetSchemaHelper().CreateClassListString(relationships);
            }

        tempPropertySources.push_back(std::make_unique<RelationshipPathSpecification>(*new RelationshipStepSpecification(
            relationshipClassNames, propertySpec.GetRequiredRelationDirection(), propertySpec.GetRelatedClassNames())));
        pathSpecs.push_back(ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification((int)i, *tempPropertySources.back().get(), propertySpec.IsPolymorphic(), {}));
        }

    // find all paths that match each spec (associated by index)
    auto countTargets = !(context.GetContentFlagsCalculator()(0) & (int)ContentFlags::DescriptorOnly);
    ECSchemaHelper::RelationshipPathsRequestParams params(sourceClass, pathSpecs, &contentInstanceFilteringParams, relatedInstancePaths, context.GetRelationshipUseCounts(), countTargets);
    auto paths = context.GetSchemaHelper().GetRelationshipPaths(params);

    // finally, build the response
    bvector<std::shared_ptr<RelatedPropertySpecificationPaths>> result;
    for (size_t i = 0; i < relatedPropertyAppendInfos.size(); ++i)
        {
        auto thisPaths = ContainerHelpers::MoveTransformContainer<bvector<RelatedPropertySpecificationPaths::Path>>(paths.GetPaths(i), [](auto&& p)
            {
            return RelatedPropertySpecificationPaths::Path(std::move(p.m_path), std::move(p.m_actualSourceClasses));
            });
        result.push_back(std::make_unique<RelatedPropertySpecificationPaths>(std::move(relatedPropertyAppendInfos[i]), std::move(thisPaths)));
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* Creates related properties paths' cache for given content specification. Note that only
* `ContentInstancesOfSpecificClassesSpecification` is supported - that's because it's the only
* specification that doesn't depend on input keys. When input keys are involved, creating the
* cache looses its benefits.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ContentSpecificationsHandler::RelatedPropertyPathsCache> ContentSpecificationsHandler::_CreateRelatedPropertyPathsCache(
    bvector<SelectClassWithExcludes<ECClass>> const& selectClasses,
    ContentInstancesOfSpecificClassesSpecificationCR specification,
    RelatedInstancePathsCache const& relatedInstancePaths
) const
    {
    // don't create the cache if we only want keys or specifically don't want any fields
    if (0 != (_GetContentFlags(specification) & (ENUM_FLAG(ContentFlags::KeysOnly) | ENUM_FLAG(ContentFlags::NoFields))))
        return nullptr;

    // find specs that apply for given content specification
    auto relatedPropertySpecsFromContentSpecification = FlattenedRelatedPropertiesSpecification::Create(specification.GetRelatedProperties(), RelatedPropertiesSpecificationScopeInfo(specification.GetPropertyCategories()));

    // find specs from content modifiers that apply for given select classes
    bvector<std::pair<ECClassCP, bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>>>> relatedPropertySpecsFromModifiers;
    for (ContentModifierCP modifier : GetContext().GetRulesPreprocessor().GetContentModifiers())
        {
        if (modifier->GetRelatedProperties().empty())
            {
            // only interested in the ones with related property specs
            continue;
            }

        ECClassCP modifierClass = GetContext().GetSchemaHelper().GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
        if (modifierClass == nullptr)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Content modifier %s specifies non-existing class: %s.%s.",
                DiagnosticsHelpers::CreateRuleIdentifier(*modifier).c_str(), modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str()));
            continue;
            }

        bool doesModifierApply =
            // handle situation where we select from derived class and the modifier is for its base class
            ContainerHelpers::Contains(selectClasses, [&](auto const& selectClass){return selectClass.GetClass().Is(modifierClass);})
            // handle situation where we select from base class polymorphically and the modifier is for derived class
            || ContainerHelpers::Contains(selectClasses, [&](auto const& selectClass){return selectClass.IsSelectPolymorphic() && modifierClass->Is(&selectClass.GetClass());});
        if (!doesModifierApply)
            continue;

        auto specs = FlattenedRelatedPropertiesSpecification::Create(modifier->GetRelatedProperties(), RelatedPropertiesSpecificationScopeInfo(modifier->GetPropertyCategories()));
        auto iter = std::find_if(relatedPropertySpecsFromModifiers.begin(), relatedPropertySpecsFromModifiers.end(), [&modifierClass](auto const& pair){return pair.first == modifierClass;});
        if (iter != relatedPropertySpecsFromModifiers.end())
            ContainerHelpers::MovePush(iter->second, std::move(specs));
        else
            relatedPropertySpecsFromModifiers.push_back(std::make_pair(modifierClass, std::move(specs)));
        }

    // find actual paths
    auto relatedPropertyPaths = std::make_unique<RelatedPropertyPathsCache>();
    auto filteringExpressionContext = CreateContentSpecificationInstanceFilterContext(GetContext());
    InstanceFilteringParams filteringParams(GetContext().GetSchemaHelper().GetECExpressionsCache(), filteringExpressionContext.get(), specification.GetInstanceFilter().c_str(), nullptr);

    // handle related property specs from content specification
    bvector<std::shared_ptr<RelatedPropertySpecificationPaths>> contentSpecificationRelatedPropertyPaths;
    for (auto const& selectClass : selectClasses)
        {
        ContainerHelpers::MovePush(contentSpecificationRelatedPropertyPaths, FindRelatedPropertyPaths(GetContext(), selectClass,
            filteringParams, relatedInstancePaths.GetMergedPathsForSelectClass(selectClass.GetClass()), std::move(relatedPropertySpecsFromContentSpecification)));
        }
    relatedPropertyPaths->push_back(std::make_pair(nullptr, std::move(contentSpecificationRelatedPropertyPaths)));

    // handle related property specs from content modifiers
    for (auto& entry : relatedPropertySpecsFromModifiers)
        {
        bool wasHandled = false;
        for (auto const& selectClass : selectClasses)
            {
            if (!selectClass.GetClass().Is(entry.first))
                continue;

            relatedPropertyPaths->push_back(std::make_pair(&selectClass.GetClass(), FindRelatedPropertyPaths(GetContext(), selectClass,
                filteringParams, relatedInstancePaths.GetMergedPathsForSelectClass(selectClass.GetClass()), std::move(entry.second))));
            wasHandled = true;
            }
        if (!wasHandled)
            {
            SelectClass<ECClass> selectClass(*entry.first, "this", true);
            relatedPropertyPaths->push_back(std::make_pair(&selectClass.GetClass(), FindRelatedPropertyPaths(GetContext(), selectClass,
                filteringParams, relatedInstancePaths.GetMergedPathsForSelectClass(selectClass.GetClass()), std::move(entry.second))));
            }
        }

    return relatedPropertyPaths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>> ContentSpecificationsHandler::GetRelatedPropertySpecifications(
    SelectClass<ECClass> const& propertyClass,
    RelatedPropertiesSpecificationList const& relatedPropertySpecs,
    PropertyCategorySpecificationsList const& scopeCategorySpecifications,
    bool includeSubclassModifiers
) const
    {
    // first, handle related property specs specified in the content specification
    auto flatSpecs = FlattenedRelatedPropertiesSpecification::Create(relatedPropertySpecs, RelatedPropertiesSpecificationScopeInfo(scopeCategorySpecifications));

    // then, look at content modifiers
    for (ContentModifierCP modifier : GetContext().GetRulesPreprocessor().GetContentModifiers())
        {
        if (modifier->GetRelatedProperties().empty())
            {
            // only interested in the ones with related property specs
            continue;
            }

        ECClassCP modifierClass = GetContext().GetSchemaHelper().GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
        if (modifierClass == nullptr)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Content modifier %s specifies non-existing class: %s.%s.",
                DiagnosticsHelpers::CreateRuleIdentifier(*modifier).c_str(), modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str()));
            continue;
            }

        bool propertyClassMatchesModifierClass = propertyClass.GetClass().Is(modifierClass);
        bool modifierClassMatchesPropertyClass = includeSubclassModifiers && propertyClass.IsSelectPolymorphic() && modifierClass->Is(&propertyClass.GetClass());
        if (propertyClassMatchesModifierClass || modifierClassMatchesPropertyClass)
            {
            DiagnosticsHelpers::ReportRule(*modifier);
            ContainerHelpers::MovePush(flatSpecs, FlattenedRelatedPropertiesSpecification::Create(modifier->GetRelatedProperties(), RelatedPropertiesSpecificationScopeInfo(modifier->GetPropertyCategories())));
            }
        }

    return flatSpecs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<std::shared_ptr<RelatedPropertySpecificationPaths>> ContentSpecificationsHandler::_GetRelatedPropertyPaths(RelatedPropertyPathsParams const& params) const
    {
    auto flatSpecs = GetRelatedPropertySpecifications(params.GetSourceClassInfo().GetSelectClass(), params.GetRelatedPropertySpecs(), params.GetScopeCategorySpecifications(), false);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " flattened related property specs.", (uint64_t)flatSpecs.size()));

    // then determine actual related class paths for every specification
    return FindRelatedPropertyPaths(GetContext(), params.GetSourceClassInfo().GetSelectClass(), params.GetInstanceFilteringParams(),
        params.GetSourceClassInfo().GetRelatedInstancePaths(), std::move(flatSpecs));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationsHandler::PropertyAppendResult ContentSpecificationsHandler::AppendRelatedProperties(PropertySpecificationsList const& propertySpecList, PropertyAppender& appender,
    ECClassCR propertiesClass, Utf8StringCR propertiesClassAlias)
    {
    ContentSpecificationsHandler::PropertyAppendResult propertyAppendResult(false);
    auto propertiesAppendScope = Diagnostics::Scope::Create("Appending properties");
    for (PropertySpecificationCP propertySpec : propertySpecList)
        {
        bvector<ECPropertyCP> properties;
        if (propertySpec->GetPropertyName() == INCLUDE_ALL_PROPERTIES_SPEC)
            {
            for (ECPropertyCP ecProperty : propertiesClass.GetProperties(true))
                properties.push_back(ecProperty);
            }
        else
            {
            ECPropertyCP ecProperty = propertiesClass.GetPropertyP(propertySpec->GetPropertyName().c_str());
            if (nullptr == ecProperty)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Requested property `%s` was not found - skipping.", propertySpec->GetPropertyName().c_str()));
                continue;
                }
            properties.push_back(ecProperty);
            }

        for (ECPropertyCP ecProperty : properties)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Appending `%s`.", ecProperty->GetName().c_str()));
            PropertySpecificationsList overrides = FindSpecificationsForProperty(propertySpecList, ecProperty->GetName());
            propertyAppendResult.MergeWith(AppendProperty(appender, *ecProperty, propertiesClassAlias.c_str(), overrides));
            }
        }

    return propertyAppendResult;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool UpdatePaths(ContentSpecificationsHandler::PropertyAppendResult const& propertyAppendResult, bvector<RelatedClassPath>& paths, RelatedClassPathR pathFromSelectToPropertyClass)
    {
    bool shouldIncludePath = propertyAppendResult.DidAppend();

    if (propertyAppendResult.GetReplacedSelectToPropertyPath())
        {
        // relationship path in related content field was replaced while appending the field - have
        // to also replace the path in our paths list
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Replaced path from `%s` to `%s`.", DiagnosticsHelpers::CreateRelatedClassPathStr(propertyAppendResult.GetReplacedSelectToPropertyPath()->prev).c_str(),
            DiagnosticsHelpers::CreateRelatedClassPathStr(propertyAppendResult.GetReplacedSelectToPropertyPath()->curr).c_str()));

        auto replaceIter = std::find_if(paths.begin(), paths.end(), [&](RelatedClassPathCR replacedPath) {return replacedPath == propertyAppendResult.GetReplacedSelectToPropertyPath()->prev;});
        if (replaceIter != paths.end())
            *replaceIter = propertyAppendResult.GetReplacedSelectToPropertyPath()->curr;
        pathFromSelectToPropertyClass = propertyAppendResult.GetReplacedSelectToPropertyPath()->curr;
        shouldIncludePath = true;
        }

    if (shouldIncludePath)
        {
        for (RelatedClass const& navigationPropertiesPath : propertyAppendResult.GetAppendedNavigationPropertyPaths())
            {
            paths.push_back(RelatedClassPath::Combine(pathFromSelectToPropertyClass, navigationPropertiesPath));
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Appended navigation property relationship path `%s`.", DiagnosticsHelpers::CreateRelatedClassPathStr(paths.back()).c_str()));
            }
        }

    return shouldIncludePath;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsHandler::AppendRelatedPropertiesParams
{
private:
    SelectClassInfo const& m_sourceClassInfo;
    IParsedInput const* m_specificationInput;
    Utf8StringCR m_inputInstanceFilter;
    RecursiveQueryInfo const* m_recursiveFilteringInfo;
    ContentSpecification const& m_specification;
    RelatedPropertyPathsCache const* m_relatedPropertyPathsCache;
public:
    AppendRelatedPropertiesParams(SelectClassInfo const& sourceClassInfo,  IParsedInput const* specificationInput,
        Utf8StringCR inputInstanceFilter, RecursiveQueryInfo const* recursiveFilteringInfo,
        ContentSpecification const& contentSpecification, RelatedPropertyPathsCache const* relatedPropertyPathsCache)
        : m_sourceClassInfo(sourceClassInfo), m_specification(contentSpecification),
        m_specificationInput(specificationInput), m_inputInstanceFilter(inputInstanceFilter), m_recursiveFilteringInfo(recursiveFilteringInfo),
        m_relatedPropertyPathsCache(relatedPropertyPathsCache)
        {}
    SelectClassInfo const& GetSourceClassInfo() const {return m_sourceClassInfo;}
    IParsedInput const* GetSpecificationInput() const {return m_specificationInput;}
    Utf8StringCR GetInputInstanceFilter() const {return m_inputInstanceFilter;}
    RecursiveQueryInfo const* GetRecursiveFilteringInfo() const {return m_recursiveFilteringInfo;}
    ContentSpecification const& GetSpecification() const {return m_specification;}
    RelatedPropertyPathsCache const* GetRelatedPropertyPathsCache() const {return m_relatedPropertyPathsCache;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ContentSpecificationsHandler::AppendRelatedProperties(AppendRelatedPropertiesParams const& params)
    {
    auto relatedPropertiesScope = Diagnostics::Scope::Create("Appending related properties");

    bvector<std::shared_ptr<RelatedPropertySpecificationPaths>> specPathsList;
    if (params.GetRelatedPropertyPathsCache())
        {
        auto const& selectClass = params.GetSourceClassInfo().GetSelectClass();
        for (auto const& cacheEntry : *params.GetRelatedPropertyPathsCache())
            {
            // - nullptr ECClass in cache entry means the paths are based on content specification (always apply)
            // - we want to always use the paths if select class derives from cache entry class
            // - we want to use the paths if select is polymorphic and cache entry class derives from select class
            if (nullptr == cacheEntry.first || selectClass.GetClass().Is(cacheEntry.first) || selectClass.IsSelectPolymorphic() && cacheEntry.first->Is(&selectClass.GetClass()))
                {
                for (auto const& pathsEntry : cacheEntry.second)
                    {
                    bvector<RelatedPropertySpecificationPaths::Path> pathsForThisSelectClass;
                    for (auto const& path : pathsEntry->GetPaths())
                        {
                        auto pathClass = ContainerHelpers::FindFirst<ECClassCP>(path.GetActualSourceClasses(), [&](ECClassCP pathClass){return pathClass->Is(&selectClass.GetClass());}, nullptr);
                        if (nullptr != pathClass)
                            {
                            RelatedPropertySpecificationPaths::Path copy(path);
                            if (!copy.front().GetSourceClass()->Is(&selectClass.GetClass()))
                                copy.front().SetSourceClass(selectClass.GetClass());
                            pathsForThisSelectClass.push_back(copy);
                            }
                        }
                    if (!pathsForThisSelectClass.empty())
                        specPathsList.push_back(std::make_unique<RelatedPropertySpecificationPaths>(pathsEntry->GetSpecificationPtr(), pathsForThisSelectClass));
                    }
                }
            }
        }
    else
        {
        std::unique_ptr<InputFilteringParams> inputFilteringParams;
        if (params.GetSpecificationInput())
            {
            inputFilteringParams = QueryBuilderHelpers::CreateInputFilter(GetContext().GetConnection(), params.GetSourceClassInfo(), params.GetRecursiveFilteringInfo(), *params.GetSpecificationInput());
            if (!inputFilteringParams)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Input filter results in no content. No need to handle related properties.");
                return bvector<RelatedClassPath>();
                }
            }
        auto filteringExpressionContext = CreateContentSpecificationInstanceFilterContext(GetContext());
        InstanceFilteringParams filteringParams(GetContext().GetSchemaHelper().GetECExpressionsCache(), filteringExpressionContext.get(), params.GetInputInstanceFilter().c_str(), inputFilteringParams.get());
        specPathsList = _GetRelatedPropertyPaths(RelatedPropertyPathsParams(params.GetSourceClassInfo(), filteringParams, params.GetSpecification().GetRelatedProperties(), params.GetSpecification().GetPropertyCategories()));
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " related property paths", (uint64_t)specPathsList.size()));
    ThrowIfCancelled(m_context.GetCancellationToken());

    // then iterate over every path and append properties
    bvector<RelatedClassPath> paths;
    for (auto const& specPaths : specPathsList)
        {
        auto appendScope = Diagnostics::Scope::Create(Utf8PrintfString("Append related properties from %s", DiagnosticsHelpers::CreateRuleIdentifier(*specPaths->GetSpecification().GetSource().back()).c_str()));
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " related property paths.", (uint64_t)specPaths->GetPaths().size()));

        RelatedPropertiesSpecificationCR spec = specPaths->GetSpecification().GetFlattened();
        for (auto const& path : specPaths->GetPaths())
            {
            ThrowIfCancelled(m_context.GetCancellationToken());
            auto pathScope = Diagnostics::Scope::Create(Utf8PrintfString("Append path: `%s`", DiagnosticsHelpers::CreateRelatedClassPathStr(path).c_str()));

            RelatedClassPath pathFromSelectToPropertyClass(path);
            bool shouldIncludePath = false;

                {
                ECClassCR relationshipClass = pathFromSelectToPropertyClass.back().GetRelationship().GetClass();
                Utf8StringCR relationshipClassAlias = pathFromSelectToPropertyClass.back().GetRelationship().GetAlias();

                PropertyAppenderPtr relationshipPropertyAppender = _CreatePropertyAppender(path.GetActualSourceClasses(), pathFromSelectToPropertyClass, relationshipClass, specPaths->GetSpecification().GetSource(),
                    &specPaths->GetSpecification().GetScope().GetCategories());
                if (!relationshipPropertyAppender.IsNull())
                    {
                    PropertyAppendResult relationshipPropertyAppendResult(false);
                    relationshipPropertyAppendResult.MergeWith(AppendRelatedProperties(spec.GetRelationshipProperties(), *relationshipPropertyAppender, relationshipClass, relationshipClassAlias));
                    relationshipPropertyAppendResult.MergeWith(_OnPropertiesAppended(*relationshipPropertyAppender, relationshipClass, relationshipClassAlias));
                    bool shouldIncludePathForRelationship = UpdatePaths(relationshipPropertyAppendResult, paths, pathFromSelectToPropertyClass);
                    shouldIncludePath |= shouldIncludePathForRelationship;
                    }
                }

                {
                ECClassCR targetClass = pathFromSelectToPropertyClass.back().GetTargetClass().GetClass();
                Utf8StringCR targetClassAlias = pathFromSelectToPropertyClass.back().GetTargetClass().GetAlias();

                PropertyAppenderPtr appender = _CreatePropertyAppender(path.GetActualSourceClasses(), pathFromSelectToPropertyClass, targetClass, specPaths->GetSpecification().GetSource(),
                    &specPaths->GetSpecification().GetScope().GetCategories());
                if (appender.IsNull())
                    {
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Properties appender was not created for requested path, continue to other paths");
                    continue;
                    }

                PropertyAppendResult propertyAppendResult(false);
                propertyAppendResult.MergeWith(AppendRelatedProperties(spec.GetProperties(), *appender, targetClass, targetClassAlias));
                propertyAppendResult.MergeWith(_OnPropertiesAppended(*appender, targetClass, targetClassAlias));
                bool shouldIncludePathForTargetClass = UpdatePaths(propertyAppendResult, paths, pathFromSelectToPropertyClass);
                shouldIncludePath |= shouldIncludePathForTargetClass;
                }

            if (!shouldIncludePath)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Did not append any properties. As a result, not including navigation properties and relationship path.");
                continue;
                }

            // append this path
            paths.push_back(pathFromSelectToPropertyClass);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Appended related properties relationship path `%s`.", DiagnosticsHelpers::CreateRelatedClassPathStr(pathFromSelectToPropertyClass).c_str()));
            }
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> FilterOutAbstractTargetClasses(bvector<RelatedClassPath> const& paths)
    {
    bvector<RelatedClassPath> filteredPaths;
    for (RelatedClassPathCR path : paths)
        {
        if (path.empty())
            continue;

        SelectClass<ECClass> const& targetClass = path.back().GetTargetClass();
        if (targetClass.GetClass().GetClassModifier() == ECClassModifier::Abstract && !targetClass.IsSelectPolymorphic())
            continue;

        filteredPaths.push_back(path);
        }
    return filteredPaths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> GetRelatedClassPaths(ContentSpecificationsHandler::Context& context, ECClassCR nodeClass,
    IParsedInput const& input, ContentRelatedInstancesSpecificationCR specification, bool mergePolymorphicPaths, bool groupByInputKey)
    {
    bvector<RelatedClassPath> paths;
    if (specification.GetRelationshipPaths().empty())
        {
        // deprecated
        int skipRelatedLevel = specification.IsRecursive() ? -1 : specification.GetSkipRelatedLevel();
        ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(nodeClass, GetRelationshipDirection(specification),
            skipRelatedLevel, context.GetRuleset().GetSupportedSchemas().c_str(), specification.GetRelationshipClassNames().c_str(),
            specification.GetRelatedClassNames().c_str(), mergePolymorphicPaths, context.GetRelationshipUseCounts());
        paths = context.GetSchemaHelper().GetRelationshipClassPathsDeprecated(options);
        }
    else
        {
        paths = context.GetSchemaHelper().GetRecursiveRelationshipClassPaths(nodeClass, input.GetInstanceIds(nodeClass),
            specification.GetRelationshipPaths(), context.GetRelationshipUseCounts(), mergePolymorphicPaths, groupByInputKey);
        }

    // reduce the number of paths...
    paths = FilterOutAbstractTargetClasses(paths);

    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ContentSource const*> FindSimpleContentSources(bvector<ContentSource> const& sources)
    {
    bset<ContentSource const*> simpleContentSources;
    RelatedClassPath const* firstSimplePathFromInputToSelectClass = nullptr;
    for (ContentSource const& source : sources)
        {
        if (source.GetSelectClass().IsSelectPolymorphic())
            continue;

        if (!source.GetPathsFromSelectToRelatedInstanceClasses().empty())
            continue;

        bool hasMissingRelationships = std::any_of(source.GetPathFromInputToSelectClass().begin(), source.GetPathFromInputToSelectClass().end(),
            [](RelatedClassCR rc){return !rc.GetRelationship().IsValid();});
        if (hasMissingRelationships)
            continue;

        if (nullptr == firstSimplePathFromInputToSelectClass)
            {
            firstSimplePathFromInputToSelectClass = &source.GetPathFromInputToSelectClass();
            }
        else
            {
            if (source.GetPathFromInputToSelectClass().size() != firstSimplePathFromInputToSelectClass->size())
                continue;

            bool beginningMatches = true;
            for (size_t i = 0; i < firstSimplePathFromInputToSelectClass->size() - 1; ++i)
                {
                if (source.GetPathFromInputToSelectClass()[i] != firstSimplePathFromInputToSelectClass->at(i))
                    {
                    beginningMatches = false;
                    break;
                    }
                }
            if (!beginningMatches)
                continue;

            RelatedClassCR lastLhs = source.GetPathFromInputToSelectClass().back();
            RelatedClassCR lastRhs = firstSimplePathFromInputToSelectClass->back();
            if (lastLhs.GetSourceClass() != lastRhs.GetSourceClass() || lastLhs.GetRelationship() != lastRhs.GetRelationship() || lastLhs.IsForwardRelationship() != lastRhs.IsForwardRelationship())
                continue;
            }

        simpleContentSources.insert(&source);
        }
    return simpleContentSources;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void FilterSimpleContentSources(bvector<ContentSource>& filteredSources, bset<ContentSource const*> const& simpleContentSources, IParsedInput const& specificationInput, ContentSpecificationsHandler::Context& context)
    {
    if (simpleContentSources.empty())
        return;

    ContentSource const& firstContentSource = **simpleContentSources.begin();
    RelatedClassPath pathPrefix;
    if (firstContentSource.GetPathFromInputToSelectClass().size() > 1)
        std::copy(firstContentSource.GetPathFromInputToSelectClass().begin(), firstContentSource.GetPathFromInputToSelectClass().end() - 1, std::back_inserter(pathPrefix));
    bset<ECClassId> targetClassIds = ContainerHelpers::TransformContainer<bset<ECClassId>, bset<ContentSource const*>>(simpleContentSources, [](ContentSource const* cs){return cs->GetSelectClass().GetClass().GetId();});
    bool isForward = (firstContentSource.GetPathFromInputToSelectClass().back().IsForwardRelationship());
    bvector<ECInstanceId> const& inputIds = specificationInput.GetInstanceIds(*firstContentSource.GetPathFromInputToSelectClass().front().GetSourceClass());
    Utf8String relationshipJoinTarget = pathPrefix.empty() ? "related" : pathPrefix.back().GetTargetClass().GetAlias();

    ComplexGenericQueryPtr query = ComplexGenericQuery::Create();
    query->SelectContract(*SimpleQueryContract::Create(*PresentationQueryContractSimpleField::Create("ClassId",
        Utf8PrintfString("[relationship].[%sECClassId]", isForward ? "Target" : "Source").c_str(), false)));
    query->From(SelectClass<ECClass>(*firstContentSource.GetPathFromInputToSelectClass().front().GetSourceClass(), "related", false));
    query->Where(ValuesFilteringHelper(inputIds).Create("[related].[ECInstanceId]", false));
    if (!pathPrefix.empty())
        query->Join(pathPrefix);
    query->Join(SelectClass<ECClass>(firstContentSource.GetPathFromInputToSelectClass().back().GetRelationship().GetClass(), "relationship"),
        QueryClauseAndBindings(Utf8PrintfString("[relationship].[%sECInstanceId] = [%s].[ECInstanceId]", isForward ? "Source" : "Target", relationshipJoinTarget.c_str())), false);
    query->Where(ValuesFilteringHelper(targetClassIds).Create(Utf8PrintfString("[relationship].[%sECClassId]", isForward ? "Target" : "Source").c_str(), false));

    CachedECSqlStatementPtr stmt = context.GetConnection().GetStatementCache().GetPreparedStatement(context.GetConnection().GetECDb().Schemas(),
        context.GetConnection().GetDb(), query->ToString().c_str());
    if (stmt.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Failed to prepare simple data sources query");

    query->BindValues(*stmt);

    bset<ECClassId> filteredTargetClassIds;
    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*stmt))
        filteredTargetClassIds.insert(stmt->GetValueId<ECClassId>(0));

    for (ContentSource const* source : simpleContentSources)
        {
        if (filteredTargetClassIds.end() != filteredTargetClassIds.find(source->GetSelectClass().GetClass().GetId()))
            filteredSources.push_back(*source);
        }
    }

#define COPY_INDEXED_ITEMS(target, sourcePtrs, indexes) \
    for (size_t index : indexes) \
        target.push_back(*sourcePtrs[index]);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<size_t> ExecuteAndResetInstanceFilteringQuery(GenericQueryPtr& query, IConnectionCR connection)
    {
    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(),
        connection.GetDb(), query->ToString().c_str());
    if (stmt.IsNull())
        {
        query = nullptr;
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Failed to prepare instance filter query");
        }
    query->BindValues(*stmt);

    bvector<size_t> indexes;
    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*stmt))
        indexes.push_back(stmt->GetValueInt(0));

    query = nullptr;
    return indexes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void FilterComplexContentSources(bvector<ContentSource>& filteredSources, bvector<ContentSource const*> const& complexContentSources, IParsedInput const& specificationInput,
    Utf8StringCR instanceFilter, ContentSpecificationsHandler::Context& context)
    {
    GenericQueryPtr query;
    uint64_t unionSize = 0;
    for (size_t i = 0; i < complexContentSources.size(); ++i)
        {
        ContentSource const& source = *complexContentSources[i];

        SelectClassInfo selectInfo(source.GetSelectClass());
        selectInfo.SetPathFromInputToSelectClass(source.GetPathFromInputToSelectClass());
        selectInfo.SetRelatedInstancePaths(source.GetPathsFromSelectToRelatedInstanceClasses());

        auto inputFilter = QueryBuilderHelpers::CreateInputFilter(context.GetConnection(), selectInfo, nullptr, specificationInput);
        if (nullptr == inputFilter)
            continue;

        auto filteringExpressionContext = CreateContentSpecificationInstanceFilterContext(context);
        InstanceFilteringParams filteringParams(context.GetSchemaHelper().GetECExpressionsCache(), filteringExpressionContext.get(),
            instanceFilter.c_str(), inputFilter.get());

        ComplexGenericQueryPtr q = ComplexGenericQuery::Create();
        q->SelectContract(*SimpleQueryContract::Create(*PresentationQueryContractSimpleField::Create("Index", std::to_string(i).c_str(), false)));
        q->From(source.GetSelectClass());
        QueryBuilderHelpers::ApplyInstanceFilter(*q, filteringParams);
        for (RelatedClassPathCR relatedInstancePath : source.GetPathsFromSelectToRelatedInstanceClasses())
            q->Join(relatedInstancePath);
        q->Limit(1);

        QueryBuilderHelpers::SetOrUnion<GenericQuery>(query, ComplexGenericQuery::Create()->SelectAll().From(*q));
        ++unionSize;

        if ((unionSize % 500) == 499)
            {
            COPY_INDEXED_ITEMS(filteredSources, complexContentSources, ExecuteAndResetInstanceFilteringQuery(query, context.GetConnection()));
            unionSize = 0;
            }
        }

    if (query.IsValid())
        COPY_INDEXED_ITEMS(filteredSources, complexContentSources, ExecuteAndResetInstanceFilteringQuery(query, context.GetConnection()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ContentSource> FilterContentSourcesWithInstances(bvector<ContentSource> const& sources, IParsedInput const& specificationInput, Utf8StringCR instanceFilter, ContentSpecificationsHandler::Context& context)
    {
    bvector<ContentSource> filteredSources;
    if (sources.empty())
        return filteredSources;

    // cover majority of cases with a single query. we can do that if:
    // - there's no instance filter
    // - select is non-polymorphic
    // - only the target class of 'input to select' path is different
    bset<ContentSource const*> simpleContentSources = instanceFilter.empty() ? FindSimpleContentSources(sources) : bset<ContentSource const*>();
    FilterSimpleContentSources(filteredSources, simpleContentSources, specificationInput, context);

    // find all uncovered cases and handle them separately
    bvector<ContentSource const*> complexContentSources;
    for (ContentSource const& source : sources)
        {
        if (simpleContentSources.end() == simpleContentSources.find(&source))
            complexContentSources.push_back(&source);
        }
    FilterComplexContentSources(filteredSources, complexContentSources, specificationInput, instanceFilter, context);

    return filteredSources;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::AppendContent(ContentSource const& contentSource, ContentSpecificationCR spec, IParsedInput const* specificationInput,
    Utf8StringCR instanceFilter, RecursiveQueryInfo const* recursiveFilteringInfo, RelatedPropertyPathsCache const* relatedPropertyPathsCache)
    {
    SelectClass<ECClass> const& selectClass = contentSource.GetSelectClass();
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Append content for `%s`", selectClass.GetClass().GetFullName()));

    static const RelatedClassPath s_emptyPath;
    static const std::unordered_set<ECClassCP> s_emptyClassesSet;
    PropertyAppenderPtr appender;
    if (!GetContext().IsClassHandled(selectClass.GetClass()))
        {
        auto classPropertiesScope = Diagnostics::Scope::Create("Appending class properties");
        bvector<RelatedPropertiesSpecification const*> specsStack;
        appender = _CreatePropertyAppender(s_emptyClassesSet, s_emptyPath, selectClass.GetClass(), specsStack, &spec.GetPropertyCategories());
        if (appender.IsValid())
            {
            PropertyAppendResult appendResult(false);
            ECPropertyIterable properties = contentSource.GetPropertiesSource().GetProperties(true);
            for (ECPropertyCP prop : properties)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Appending `%s`", prop->GetName().c_str()));
                appendResult.MergeWith(AppendProperty(*appender, *prop, "this", PropertySpecificationsList()));
                }
            appendResult.MergeWith(_OnPropertiesAppended(*appender, selectClass.GetClass(), "this"));
            GetContext().AddNavigationPropertiesPaths(selectClass.GetClass(), appendResult.GetAppendedNavigationPropertyPaths());
            }
        GetContext().SetClassHandled(selectClass.GetClass());
        }

    SelectClassInfo selectInfo(contentSource.GetSelectClass());
    selectInfo.SetPathFromInputToSelectClass(contentSource.GetPathFromInputToSelectClass());
    selectInfo.SetNavigationPropertyClasses(GetContext().GetNavigationPropertiesPaths(selectClass.GetClass()));
    selectInfo.SetRelatedInstancePaths(contentSource.GetPathsFromSelectToRelatedInstanceClasses());
    selectInfo.SetRelatedPropertyPaths(AppendRelatedProperties(AppendRelatedPropertiesParams(selectInfo,
        specificationInput, instanceFilter, recursiveFilteringInfo, spec, relatedPropertyPathsCache)));

    _AppendClass(selectInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RuleApplicationInfo> ContentSpecificationsHandler::CreateCustomizationRuleInfos(ContentSpecificationCR specification) const
    {
    bvector<RuleApplicationInfo> infos;

    auto contentFlags = _GetContentFlags(specification);
    if (0 != (contentFlags & (ENUM_FLAG(ContentFlags::KeysOnly) | ENUM_FLAG(ContentFlags::NoFields))))
        return infos;

    for (ContentModifierCP modifier : GetContext().GetRulesPreprocessor().GetContentModifiers())
        {
        ECClassCP ecClass = GetContext().GetSchemaHelper().GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
        if (nullptr == ecClass)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("ECClass '%s.%s' used in ContentModifier rule was not found", modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str())); // TODO: rule ref
            continue;
            }
        infos.push_back(RuleApplicationInfo(*ecClass, true));
        }

    for (InstanceLabelOverrideCP labelOverride : GetContext().GetRulesPreprocessor().GetInstanceLabelOverrides())
        {
        ECClassCP ecClass = GetContext().GetSchemaHelper().GetECClass(labelOverride->GetClassName().c_str());
        if (nullptr == ecClass)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("ECClass '%s' used in InstanceLabelOverride rule was not found", labelOverride->GetClassName().c_str())); // TODO: rule ref
            continue;
            }
        infos.push_back(RuleApplicationInfo(*ecClass, true));
        }

    if (0 == (contentFlags & ENUM_FLAG(ContentFlags::DescriptorOnly)))
        {
        // don't care about sorting rules if we're not going to request content
        for (SortingRuleCP sortingRule : GetContext().GetRulesPreprocessor().GetSortingRules())
            {
            ECClassCP ecClass = GetContext().GetSchemaHelper().GetECClass(sortingRule->GetSchemaName().c_str(), sortingRule->GetClassName().c_str());
            if (nullptr == ecClass)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("ECClass '%s.%s' used in sorting rule was not found", sortingRule->GetSchemaName().c_str(), sortingRule->GetClassName().c_str())); // TODO: rule ref
                continue;
                }
            infos.push_back(RuleApplicationInfo(*ecClass, sortingRule->GetIsPolymorphic()));
            }
        }

    return infos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassCR GetRootBaseClass(ECClassCR ecClass)
    {
    ECClassCP base = &ecClass;
    while (true)
        {
        bool didChange = false;
        for (auto const& curr : base->GetBaseClasses())
            {
            if (curr->IsEntityClass())
                {
                base = curr;
                didChange = true;
                break;
                }
            }
        if (!didChange)
            break;
        }
    return *base;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationsHandler::RelatedInstancePathsCache ContentSpecificationsHandler::_CreateRelatedInstancePathsCache(
    bvector<SelectClassWithExcludes<ECClass>> const& selectClasses,
    ContentSpecificationCR contentSpecification
) const
    {
    bvector<ECClassCP> baseClasses;
    for (auto const& selectClass : selectClasses)
        {
        ECClassCR base = GetRootBaseClass(selectClass.GetClass());
        if (!ContainerHelpers::Contains(baseClasses, &base))
            baseClasses.push_back(&base);
        }

    RelatedInstancePathsCache cache;
    for (auto const& selectClass : baseClasses)
        {
        bmap<Utf8String, bvector<RelatedClassPath>> relatedInstancePaths = GetContext().GetSchemaHelper().GetRelatedInstancePaths(
            *selectClass, contentSpecification.GetRelatedInstances(), GetContext().GetRelationshipUseCounts());
        cache.push_back(std::make_pair(selectClass, relatedInstancePaths));
        }
    return cache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void JoinRelatedInstancePaths(bvector<ContentSource>& sources, bvector<RelatedClassPath> const& pathsFromSelectToRelatedInstanceClass)
    {
    if (pathsFromSelectToRelatedInstanceClass.empty())
        return;

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ContentSource> CreateContentSourcesWithRelatedInstancePaths(ContentSource const& source, ContentSpecificationsHandler::RelatedInstancePathsCache const& relatedInstancePaths)
    {
    bvector<ContentSource> sources = {source};
    auto const& pathsForThisSource = relatedInstancePaths.GetPathsForSelectClass(source.GetSelectClass().GetClass());
    for (auto const& entry : pathsForThisSource)
        {
        // note: if a single related instance specification results in more than one path, we have to
        // multiply content sources to avoid having multiple paths based on the same specification being
        // assigned to the same content source
        JoinRelatedInstancePaths(sources, entry.second);
        }
    return sources;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::CreateContentSources(SelectClassWithExcludes<ECClass> const& selectClass, ECClassCP propertiesSourceClass, RelatedInstancePathsCache const& relatedInstancePaths) const
    {
    return CreateContentSourcesWithRelatedInstancePaths(ContentSource(selectClass, propertiesSourceClass), relatedInstancePaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::_BuildContentSource(bvector<SelectClassWithExcludes<ECClass>> const& selectClasses, RelatedInstancePathsCache const& relatedInstancePaths, bvector<RuleApplicationInfo> const& customizationRules)
    {
    bvector<ContentSource> sources;
    bvector<SelectClassSplitResult> splitResults = QueryBuilderHelpers::ProcessSelectClassesBasedOnCustomizationRules(selectClasses,
        customizationRules, GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    for (SelectClassSplitResult const& split : splitResults)
        {
        ECClassCP propertiesSourceClass = split.GetSplitPath().empty() ? nullptr : &split.GetSplitPath().front().GetClass();
        ContainerHelpers::Push(sources, CreateContentSources(split.GetSelectClass(), propertiesSourceClass, relatedInstancePaths));
        }
    return sources;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::CreateContentSources(RelatedClassPath const& pathFromInputToSelectClass, RelatedInstancePathsCache const& relatedInstancePaths) const
    {
    ContentSource source(pathFromInputToSelectClass.back().GetTargetClass(), nullptr);
    source.GetSelectClass().SetAlias("this");
    source.SetPathFromInputToSelectClass(pathFromInputToSelectClass);
    for (RelatedClass& rc : source.GetPathFromInputToSelectClass())
        rc.SetIsTargetOptional(false);
    return CreateContentSourcesWithRelatedInstancePaths(source, relatedInstancePaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::_BuildContentSource(bvector<RelatedClassPath> const& paths, RelatedInstancePathsCache const& relatedInstancePaths, bvector<RuleApplicationInfo> const& customizationRules)
    {
    bvector<ContentSource> sources;
    bvector<RelatedClassPath> splitPaths = QueryBuilderHelpers::ProcessRelationshipPathsBasedOnCustomizationRules(paths,
        customizationRules, GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    for (RelatedClassPathCR path : splitPaths)
        ContainerHelpers::Push(sources, CreateContentSources(path, relatedInstancePaths));
    return sources;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<RecursiveQueryInfo const> CreateRecursiveFilteringInfo(bvector<ContentSource> const& contentSource, ContentRelatedInstancesSpecificationCR spec)
    {
    if (spec.IsRecursive())
        {
        return std::make_unique<RecursiveQueryInfo const>(
            ContainerHelpers::TransformContainer<bvector<RelatedClassPath>>(contentSource, [](ContentSource const& s){return s.GetPathFromInputToSelectClass();}));
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::HandleSpecification(SelectedNodeInstancesSpecificationCR specification, IParsedInput const& input)
    {
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " input classes.", (uint64_t)input.GetClasses().size()));
    if (input.GetClasses().empty())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, "Specification has no effect due to empty input.");
        return;
        }

    bvector<SelectClassWithExcludes<ECClass>> selectClasses;
    for (ECClassCP inputClass : input.GetClasses())
        {
        // the class should be handled polymorphically if this is a "class" request rather than "instance" request
        selectClasses.push_back(SelectClassWithExcludes<ECClass>(*inputClass, "this", false));
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " select classes.", (uint64_t)selectClasses.size()));

    auto customizationRules = CreateCustomizationRuleInfos(specification);
    auto relatedInstancePaths = _CreateRelatedInstancePathsCache(selectClasses, specification);

    bvector<ContentSource> contentSources = _BuildContentSource(selectClasses, relatedInstancePaths, customizationRules);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " content sources.", (uint64_t)contentSources.size()));

    for (ContentSource const& src : contentSources)
        {
        ThrowIfCancelled(m_context.GetCancellationToken());
        if (!IsECClassAccepted(specification, src.GetSelectClass().GetClass()))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Skipping content source for `%s` - it doesn't match specification requirements.", src.GetSelectClass().GetClass().GetFullName()));
            continue;
            }
        AppendContent(src, specification, &input, "", nullptr, nullptr);
        }
    _OnContentAppended();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::HandleSpecification(ContentRelatedInstancesSpecificationCR specification, IParsedInput const& input)
    {
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " input classes.", (uint64_t)input.GetClasses().size()));
    if (input.GetClasses().empty())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, "Specification has no effect due to empty input.");
        return;
        }

    int contentFlags = _GetContentFlags(specification);
    bool skipContentWithInstancesCheck = (specification.IsRecursive() || 0 != (contentFlags & ENUM_FLAG(ContentFlags::SkipInstancesCheck)));
    bool groupByInputKey = (0 != (contentFlags & ENUM_FLAG(ContentFlags::IncludeInputKeys)));

    auto customizationRules = CreateCustomizationRuleInfos(specification);

    for (ECClassCP ecClass : input.GetClasses())
        {
        auto classScope = Diagnostics::Scope::Create(Utf8PrintfString("Handling input class `%s`", ecClass->GetFullName()));

        bvector<RelatedClassPath> paths = GetRelatedClassPaths(GetContext(), *ecClass, input, specification, skipContentWithInstancesCheck, groupByInputKey);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Input class has %" PRIu64 " related classes.", (uint64_t)paths.size()));

        auto relatedInstancePaths = _CreateRelatedInstancePathsCache(ContainerHelpers::TransformContainer<bvector<SelectClassWithExcludes<ECClass>>>(paths, [](auto const& path)
            {
            return path[path.size() - 1].GetTargetClass();
            }), specification);

        bvector<ContentSource> contentSources = _BuildContentSource(paths, relatedInstancePaths, customizationRules);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " content sources.", (uint64_t)contentSources.size()));

        if (!skipContentWithInstancesCheck)
            {
            contentSources = FilterContentSourcesWithInstances(contentSources, input, specification.GetInstanceFilter(), GetContext());
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " content sources after filtering-out content sources without instances", (uint64_t)contentSources.size()));
            }

        std::unique_ptr<RecursiveQueryInfo const> recursiveInfo = CreateRecursiveFilteringInfo(contentSources, specification);
        for (ContentSource const& src : contentSources)
            {
            ThrowIfCancelled(m_context.GetCancellationToken());
            AppendContent(src, specification, &input, specification.GetInstanceFilter(), recursiveInfo.get(), nullptr);
            }
        }
    _OnContentAppended();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<SelectClassWithExcludes<ECClass>> FindActualSelectClassesWithInstances(bvector<SelectClassWithExcludes<ECClass>> const& inputSelectClasses,
    Utf8StringCR instanceFilter, ContentSpecificationsHandler::Context const& context)
    {
    bvector<SelectClassWithExcludes<ECClass>> result;

    // sqlite has a limit of 500 statements in a union - need to split the query
    size_t iterationsCount = inputSelectClasses.size() / MAX_COMPOUND_STATEMENTS_COUNT + 1;
    for (size_t iteration = 0; iteration < iterationsCount; ++iteration)
        {
        UnionGenericQueryPtr unionQuery = UnionGenericQuery::Create({});
        size_t offset = iteration * MAX_COMPOUND_STATEMENTS_COUNT;
        size_t end = offset + MAX_COMPOUND_STATEMENTS_COUNT;
        if (end > inputSelectClasses.size())
            end = inputSelectClasses.size();
        for (size_t i = offset; i < end; ++i)
            {
            SelectClassWithExcludes<ECClass> const& selectClass = inputSelectClasses[i];

            ComplexGenericQueryPtr query = ComplexGenericQuery::Create();
            auto contract = SimpleQueryContract::Create(*PresentationQueryContractSimpleField::Create("ECClassId", "ECClassId"));
            query->SelectContract(*contract);
            query->From(selectClass);
            query->GroupByContract(*contract);

            auto filteringExpressionContext = CreateContentSpecificationInstanceFilterContext(context);
            InstanceFilteringParams params(context.GetSchemaHelper().GetECExpressionsCache(), filteringExpressionContext.get(),
                instanceFilter.c_str(), nullptr);
            QueryBuilderHelpers::ApplyInstanceFilter(*query, params);

            unionQuery->AddQuery(*query);
            }

        CachedECSqlStatementPtr statement = context.GetConnection().GetStatementCache().GetPreparedStatement(
            context.GetConnection().GetECDb().Schemas(), context.GetConnection().GetDb(), unionQuery->ToString().c_str());
        if (statement.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Failed to prepare actual classes with instances query");

        unionQuery->BindValues(*statement);

        while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*statement))
            {
            auto classId = statement->GetValueId<ECClassId>(0);
            auto ecclass = context.GetConnection().GetECDb().Schemas().GetClass(classId);
            if (nullptr == ecclass)
                {
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default,
                    Utf8PrintfString("Failed to load ECClass with id - %s while getting actual classes with instances.", classId.ToString().c_str()));
                }
            result.push_back(SelectClass<ECClass>(*ecclass, "this", false));
            }
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecificationsHandler::HandleSpecification(ContentInstancesOfSpecificClassesSpecificationCR specification)
    {
    SupportedClassInfos excludedClassInfos = GetContext().GetSchemaHelper().GetECClassesFromClassList(specification.GetExcludedClasses(), true);
    auto const excludedClasses = ContainerHelpers::TransformContainer<bvector<SelectClass<ECClass>>>(excludedClassInfos, [&](auto const& eci)
        {
        return SelectClass<ECClass>(eci.GetClass(), "", eci.IsPolymorphic());
        });

    SupportedClassInfos classInfos = GetContext().GetSchemaHelper().GetECClassesFromClassList(specification.GetClasses(), false);
    auto selectClasses = ContainerHelpers::TransformContainer<bvector<SelectClassWithExcludes<ECClass>>>(classInfos, [&](auto const& ci)
        {
        SelectClassWithExcludes<ECClass> selectClass(ci.GetClass(), "this", ci.IsPolymorphic());
        selectClass.GetDerivedExcludedClasses() = excludedClasses;
        return selectClass;
        });
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " select classes.", (uint64_t)selectClasses.size()));

    auto customizationRules = CreateCustomizationRuleInfos(specification);
    auto relatedInstancePaths = _CreateRelatedInstancePathsCache(selectClasses, specification);
    auto relatedPropertyPaths = _CreateRelatedPropertyPathsCache(selectClasses, specification, relatedInstancePaths);

    if (specification.ShouldHandlePropertiesPolymorphically())
        {
        selectClasses = FindActualSelectClassesWithInstances(selectClasses, specification.GetInstanceFilter(), GetContext());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " select classes after filtering-out classes without instances.", (uint64_t)selectClasses.size()));
        }

    bvector<ContentSource> contentSources = _BuildContentSource(selectClasses, relatedInstancePaths, customizationRules);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " content sources.", (uint64_t)contentSources.size()));

    for (ContentSource const& src : contentSources)
        {
        ThrowIfCancelled(m_context.GetCancellationToken());
        AppendContent(src, specification, nullptr, specification.GetInstanceFilter(), nullptr, relatedPropertyPaths.get());
        }

    _OnContentAppended();
    }
