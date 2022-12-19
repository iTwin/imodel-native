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
static ECSchemaHelper::RelationshipPathsResponse FindRelatedPropertyPaths(ContentSpecificationsHandler::Context const& context, SelectClassWithExcludes<ECClass> const& sourceClass,
    InstanceFilteringParams const& contentInstanceFilteringParams, bvector<RelatedClassPath> const& relatedInstancePaths, bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>> const& relatedPropertyAppendInfos)
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
    return context.GetSchemaHelper().GetRelationshipPaths(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<std::unique_ptr<RelatedPropertySpecificationPaths>> ContentSpecificationsHandler::_GetRelatedPropertyPaths(RelatedPropertyPathsParams const& params) const
    {
    // first build a flat list of all related properties specifications
    auto flatSpecs = FlattenedRelatedPropertiesSpecification::Create(params.GetRelatedPropertySpecs(), RelatedPropertiesSpecificationScopeInfo(params.GetScopeCategorySpecifications()));
    for (ContentModifierCP modifier : GetContext().GetRulesPreprocessor().GetContentModifiers())
        {
        if (params.GetSourceClassInfo().GetSelectClass().GetClass().Is(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str()))
            {
            DiagnosticsHelpers::ReportRule(*modifier);
            ContainerHelpers::MovePush(flatSpecs, FlattenedRelatedPropertiesSpecification::Create(modifier->GetRelatedProperties(), RelatedPropertiesSpecificationScopeInfo(modifier->GetPropertyCategories())));
            }
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " flattened related property specs.", (uint64_t)flatSpecs.size()));

    // then determine actual related class paths for every specification
    auto paths = FindRelatedPropertyPaths(GetContext(), params.GetSourceClassInfo().GetSelectClass(), params.GetInstanceFilteringParams(), params.GetSourceClassInfo().GetRelatedInstancePaths(), flatSpecs);

    // finally, build the response
    bvector<std::unique_ptr<RelatedPropertySpecificationPaths>> result;
    for (size_t i = 0; i < flatSpecs.size(); ++i)
        {
        auto thisPaths = ContainerHelpers::MoveTransformContainer<bvector<RelatedPropertySpecificationPaths::Path>>(paths.GetPaths(i), [](auto&& p)
            {
            return RelatedPropertySpecificationPaths::Path(std::move(p.m_path), std::move(p.m_actualSourceClasses));
            });
        result.push_back(std::make_unique<RelatedPropertySpecificationPaths>(std::move(flatSpecs[i]), std::move(thisPaths)));
        }
    return result;
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
    RelatedPropertiesSpecificationList const& m_relatedPropertySpecs;
    PropertyCategorySpecificationsList const& m_scopeCategorySpecifications;
public:
    AppendRelatedPropertiesParams(SelectClassInfo const& sourceClassInfo,  IParsedInput const* specificationInput,
        Utf8StringCR inputInstanceFilter, RecursiveQueryInfo const* recursiveFilteringInfo,
        RelatedPropertiesSpecificationList const& specs, PropertyCategorySpecificationsList const& scopeCategorySpecifications)
        : m_sourceClassInfo(sourceClassInfo), m_relatedPropertySpecs(specs), m_scopeCategorySpecifications(scopeCategorySpecifications),
        m_specificationInput(specificationInput), m_inputInstanceFilter(inputInstanceFilter), m_recursiveFilteringInfo(recursiveFilteringInfo)
        {}
    SelectClassInfo const& GetSourceClassInfo() const {return m_sourceClassInfo;}
    IParsedInput const* GetSpecificationInput() const {return m_specificationInput;}
    Utf8StringCR GetInputInstanceFilter() const {return m_inputInstanceFilter;}
    RecursiveQueryInfo const* GetRecursiveFilteringInfo() const {return m_recursiveFilteringInfo;}
    RelatedPropertiesSpecificationList const& GetRelatedPropertySpecs() const {return m_relatedPropertySpecs;}
    PropertyCategorySpecificationsList const& GetScopeCategorySpecifications() const {return m_scopeCategorySpecifications;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ContentSpecificationsHandler::AppendRelatedProperties(AppendRelatedPropertiesParams const& params)
    {
    auto relatedPropertiesScope = Diagnostics::Scope::Create("Appending related properties");

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

    bvector<std::unique_ptr<RelatedPropertySpecificationPaths>> specPathsList = _GetRelatedPropertyPaths(RelatedPropertyPathsParams(params.GetSourceClassInfo(),
        filteringParams, params.GetRelatedPropertySpecs(), params.GetScopeCategorySpecifications()));
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

    auto query = ComplexQueryBuilder::Create();
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
        context.GetConnection().GetDb(), query->GetQuery()->GetQueryString().c_str());
    if (stmt.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Failed to prepare simple data sources query");

    query->GetQuery()->BindValues(*stmt);

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
static bvector<size_t> ExecuteAndResetInstanceFilteringQuery(PresentationQueryBuilderPtr& query, IConnectionCR connection)
    {
    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(),
        connection.GetDb(), query->GetQuery()->GetQueryString().c_str());
    if (stmt.IsNull())
        {
        query = nullptr;
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Failed to prepare instance filter query");
        }
    query->GetQuery()->BindValues(*stmt);

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
    PresentationQueryBuilderPtr query;
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

        auto q = ComplexQueryBuilder::Create();
        q->SelectContract(*SimpleQueryContract::Create(*PresentationQueryContractSimpleField::Create("Index", std::to_string(i).c_str(), false)));
        q->From(source.GetSelectClass());
        QueryBuilderHelpers::ApplyInstanceFilter(*q, filteringParams);
        for (RelatedClassPathCR relatedInstancePath : source.GetPathsFromSelectToRelatedInstanceClasses())
            q->Join(relatedInstancePath);
        q->Limit(1);

        QueryBuilderHelpers::SetOrUnion(query, ComplexQueryBuilder::Create()->SelectAll().From(*q));
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
    Utf8StringCR instanceFilter, RecursiveQueryInfo const* recursiveFilteringInfo)
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
        specificationInput, instanceFilter, recursiveFilteringInfo, spec.GetRelatedProperties(), spec.GetPropertyCategories())));

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
bvector<RuleApplicationInfo> const& ContentSpecificationsHandler::GetCustomizationRuleInfos() const
    {
    if (nullptr == m_customizationRuleInfos)
        {
        auto infos = new bvector<RuleApplicationInfo>();
        for (ContentModifierCP modifier : GetContext().GetRulesPreprocessor().GetContentModifiers())
            {
            ECClassCP ecClass = GetContext().GetSchemaHelper().GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
            if (nullptr == ecClass)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("ECClass '%s.%s' used in ContentModifier rule was not found", modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str())); // TODO: rule ref
                continue;
                }
            infos->push_back(RuleApplicationInfo(*ecClass, true));
            }
        for (InstanceLabelOverrideCP labelOverride : GetContext().GetRulesPreprocessor().GetInstanceLabelOverrides())
            {
            ECClassCP ecClass = GetContext().GetSchemaHelper().GetECClass(labelOverride->GetClassName().c_str());
            if (nullptr == ecClass)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("ECClass '%s' used in InstanceLabelOverride rule was not found", labelOverride->GetClassName().c_str())); // TODO: rule ref
                continue;
                }
            infos->push_back(RuleApplicationInfo(*ecClass, true));
            }
        for (SortingRuleCP sortingRule : GetContext().GetRulesPreprocessor().GetSortingRules())
            {
            ECClassCP ecClass = GetContext().GetSchemaHelper().GetECClass(sortingRule->GetSchemaName().c_str(), sortingRule->GetClassName().c_str());
            if (nullptr == ecClass)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("ECClass '%s.%s' used in sorting rule was not found", sortingRule->GetSchemaName().c_str(), sortingRule->GetClassName().c_str())); // TODO: rule ref
                continue;
                }
            infos->push_back(RuleApplicationInfo(*ecClass, sortingRule->GetIsPolymorphic()));
            }
        m_customizationRuleInfos = infos;
        }
    return *m_customizationRuleInfos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::CreateContentSources(SelectClassWithExcludes<ECClass> const& selectClass, ECClassCP propertiesSourceClass, ContentSpecificationCR spec) const
    {
    return CreateContentSourcesWithRelatedInstancePaths(ContentSource(selectClass, propertiesSourceClass),
        spec.GetRelatedInstances(), GetContext());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldSplitOnDerivedClasses(int flags)
    {
    return 0 == ((int)ContentFlags::KeysOnly & flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::_BuildContentSource(bvector<SelectClassWithExcludes<ECClass>> const& selectClasses, ContentSpecificationCR spec)
    {
    bvector<ContentSource> sources;
    bvector<SelectClassSplitResult> splitResults = QueryBuilderHelpers::ProcessSelectClassesBasedOnCustomizationRules(selectClasses,
        ShouldSplitOnDerivedClasses(_GetContentFlags(spec)) ? GetCustomizationRuleInfos() : bvector<RuleApplicationInfo>(),
        GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    for (SelectClassSplitResult const& split : splitResults)
        {
        ECClassCP propertiesSourceClass = split.GetSplitPath().empty() ? nullptr : &split.GetSplitPath().front().GetClass();
        ContainerHelpers::Push(sources, CreateContentSources(split.GetSelectClass(), propertiesSourceClass, spec));
        }
    return sources;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::CreateContentSources(RelatedClassPath const& pathFromInputToSelectClass, ContentSpecificationCR spec) const
    {
    ContentSource source(pathFromInputToSelectClass.back().GetTargetClass(), nullptr);
    source.GetSelectClass().SetAlias("this");
    source.SetPathFromInputToSelectClass(pathFromInputToSelectClass);
    for (RelatedClass& rc : source.GetPathFromInputToSelectClass())
        rc.SetIsTargetOptional(false);
    return CreateContentSourcesWithRelatedInstancePaths(source, spec.GetRelatedInstances(), GetContext());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentSource> ContentSpecificationsHandler::_BuildContentSource(bvector<RelatedClassPath> const& paths, ContentSpecificationCR spec)
    {
    bvector<ContentSource> sources;
    bvector<RelatedClassPath> splitPaths = QueryBuilderHelpers::ProcessRelationshipPathsBasedOnCustomizationRules(paths,
        ShouldSplitOnDerivedClasses(_GetContentFlags(spec)) ? GetCustomizationRuleInfos() : bvector<RuleApplicationInfo>(),
        GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
    for (RelatedClassPathCR path : splitPaths)
        ContainerHelpers::Push(sources, CreateContentSources(path, spec));
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

    bvector<ContentSource> contentSources = _BuildContentSource(selectClasses, specification);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " content sources.", (uint64_t)contentSources.size()));

    for (ContentSource const& src : contentSources)
        {
        ThrowIfCancelled(m_context.GetCancellationToken());
        if (!IsECClassAccepted(specification, src.GetSelectClass().GetClass()))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Skipping content source for `%s` - it doesn't match specification requirements.", src.GetSelectClass().GetClass().GetFullName()));
            continue;
            }
        AppendContent(src, specification, &input, "", nullptr);
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
    bool skipContentWithInstancesCheck = (specification.IsRecursive() || 0 != (contentFlags & (int)ContentFlags::SkipInstancesCheck));
    bool groupByInputKey = (0 != (contentFlags & (int)ContentFlags::IncludeInputKeys));

    for (ECClassCP ecClass : input.GetClasses())
        {
        auto classScope = Diagnostics::Scope::Create(Utf8PrintfString("Handling input class `%s`", ecClass->GetFullName()));

        bvector<RelatedClassPath> paths = GetRelatedClassPaths(GetContext(), *ecClass, input, specification, skipContentWithInstancesCheck, groupByInputKey);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Input class has %" PRIu64 " related classes.", (uint64_t)paths.size()));

        bvector<ContentSource> contentSources = _BuildContentSource(paths, specification);
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
            AppendContent(src, specification, &input, specification.GetInstanceFilter(), recursiveInfo.get());
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
        auto unionQuery = UnionQueryBuilder::Create({});
        size_t offset = iteration * MAX_COMPOUND_STATEMENTS_COUNT;
        size_t end = offset + MAX_COMPOUND_STATEMENTS_COUNT;
        if (end > inputSelectClasses.size())
            end = inputSelectClasses.size();
        for (size_t i = offset; i < end; ++i)
            {
            SelectClassWithExcludes<ECClass> const& selectClass = inputSelectClasses[i];

            auto query = ComplexQueryBuilder::Create();
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
            context.GetConnection().GetECDb().Schemas(), context.GetConnection().GetDb(), unionQuery->GetQuery()->GetQueryString().c_str());
        if (statement.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Failed to prepare actual classes with instances query");

        unionQuery->GetQuery()->BindValues(*statement);

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
    SupportedClassInfos classInfos = GetContext().GetSchemaHelper().GetECClassesFromClassList(specification.GetClasses(), false);
    SupportedClassInfos excludedClassInfos = GetContext().GetSchemaHelper().GetECClassesFromClassList(specification.GetExcludedClasses(), true);

    auto const excludedClasses = ContainerHelpers::TransformContainer<bvector<SelectClass<ECClass>>>(excludedClassInfos, [&](auto const& eci)
        {
        return SelectClass<ECClass>(eci.GetClass(), "", eci.IsPolymorphic());
        });

    auto selectClasses = ContainerHelpers::TransformContainer<bvector<SelectClassWithExcludes<ECClass>>>(classInfos, [&](auto const& ci)
        {
        SelectClassWithExcludes<ECClass> selectClass(ci.GetClass(), "this", ci.IsPolymorphic());
        selectClass.GetDerivedExcludedClasses() = excludedClasses;
        return selectClass;
        });
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " select classes.", (uint64_t)selectClasses.size()));

    if (specification.ShouldHandlePropertiesPolymorphically())
        {
        selectClasses = FindActualSelectClassesWithInstances(selectClasses, specification.GetInstanceFilter(), GetContext());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " select classes after filtering-out classes without instances.", (uint64_t)selectClasses.size()));
        }

    bvector<ContentSource> contentSources = _BuildContentSource(selectClasses, specification);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " content sources.", (uint64_t)contentSources.size()));

    for (ContentSource const& src : contentSources)
        {
        ThrowIfCancelled(m_context.GetCancellationToken());
        AppendContent(src, specification, nullptr, specification.GetInstanceFilter(), nullptr);
        }

    _OnContentAppended();
    }
