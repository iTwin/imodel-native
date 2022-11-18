/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include "ContentClassesLocater.h"
#include "PropertyInfoStore.h"
#include "../Shared/NodeLabelCalculator.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ECClassCP> CollectRuleConditionClasses(ECSchemaHelper const& schemaHelper, IRulesPreprocessorR rules, ECExpressionsCache& expressionsCache)
    {
    ECExpressionsHelper expressionsHelper(expressionsCache);
    bset<ECClassCP> classes;
    for (ContentRuleCP rule : rules.GetContentRules())
        {
        DiagnosticsHelpers::ReportRule(*rule);
        Utf8StringCR condition = rule->GetCondition();
        bvector<Utf8String> classNames =  expressionsHelper.GetUsedClasses(condition);
        for (Utf8StringCR name : classNames)
            {
            Utf8String schemaName, className;
            if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, name))
                continue;

            if (!schemaName.empty())
                {
                classes.insert(schemaHelper.GetECClass(schemaName.c_str(), className.c_str()));
                }
            else
                {
                bvector<ECClassCP> classesWithName = schemaHelper.GetECClassesByName(className.c_str());
                classes.insert(classesWithName.begin(), classesWithName.end());
                }
            }
        }
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TList>
static void SplitPolymorphicClassesList(bvector<ECClassCP>& result, TList const& searchList, bset<ECClassCP> const& splitClasses, SchemaManagerCR schemas)
    {
    for (ECClassCP ecClass : searchList)
        {
        bool checkDerivedClasses = false;
        for (ECClassCP splitClass : splitClasses)
            {
            if (ecClass == splitClass)
                result.push_back(ecClass);
            if (splitClass != ecClass && splitClass->Is(ecClass))
                {
                checkDerivedClasses = true;
                break;
                }
            }
        if (checkDerivedClasses)
            SplitPolymorphicClassesList(result, schemas.GetDerivedClasses(*ecClass), splitClasses, schemas);
        }
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ClassInput : IParsedInput
{
private:
    bvector<ECClassCP> m_classes;
protected:
    bvector<ECClassCP> const& _GetClasses() const override
        {
        return m_classes;
        }
    bvector<ECInstanceId> const& _GetInstanceIds(ECClassCR) const override
        {
        static const bvector<ECInstanceId> s_empty;
        return s_empty;
        }
public:
    ClassInput(NavNodeKeyListCR keys, SchemaManagerCR schemas)
        {
        bset<ECClassCP> classes;
        for (NavNodeKeyCPtr const& key : keys)
            {
            if (nullptr == key->AsECInstanceNodeKey())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_TRACE, LOG_INFO, "Input node is not an ECInstance-based node - ignoring");
                continue;
                }
            for (ECClassInstanceKeyCR instanceKey : key->AsECInstanceNodeKey()->GetInstanceKeys())
                {
                if (classes.end() != classes.find(instanceKey.GetClass()))
                    continue;

                m_classes.push_back(instanceKey.GetClass());
                classes.insert(instanceKey.GetClass());
                }
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ForEachDerivedClass(SchemaManagerCR schemas, ECClassCR base, std::function<void(ECClassCR)> const& derivedClassCallback)
    {
    for (ECClassCP derived : schemas.GetDerivedClasses(base))
        {
        derivedClassCallback(*derived);
        ForEachDerivedClass(schemas, *derived, derivedClassCallback);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassCR GetRootClass(ECClassCR in)
    {
    for (ECClassCP base : in.GetBaseClasses())
        {
        if (base->IsMixin())
            continue;
        return GetRootClass(*base);
        }
    return in;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ClassesLocaterPropertyAppender : ContentSpecificationsHandler::PropertyAppender
{
private:
    PropertyInfoStore m_propertyInfos;
    ECClassCR m_propertyClass;
    ContentSpecificationsHandler::Context m_context;
    bset<bpair<ECRelationshipClassCP, ECClassCP>> m_handledNavigationPropRelationships;
protected:
    bool _Supports(ECPropertyCR prop, PropertySpecificationsList const& ovr) override
        {
        return m_propertyInfos.ShouldDisplay(prop, m_propertyClass, [this]() { return CreateExpressionContext(m_context); }, ovr);
        }
    ContentSpecificationsHandler::PropertyAppendResult _Append(ECPropertyCR prop, Utf8CP, PropertySpecificationsList const&) override
        {
        ContentSpecificationsHandler::PropertyAppendResult result(true);
        if (prop.GetIsNavigation())
            {
            RelatedClass navigationPropRelatedClass = m_context.GetSchemaHelper().GetForeignKeyClass(prop);
            ECRelationshipClassCR rootRelationship = *GetRootClass(navigationPropRelatedClass.GetRelationship().GetClass()).GetRelationshipClassCP();
            ECClassCR rootTarget = GetRootClass(navigationPropRelatedClass.GetTargetClass().GetClass());
            auto key = make_bpair(&rootRelationship, &rootTarget);
            if (m_handledNavigationPropRelationships.find(key) == m_handledNavigationPropRelationships.end())
                {
                m_handledNavigationPropRelationships.insert(key);
                navigationPropRelatedClass.SetSourceClass(m_propertyClass);
                navigationPropRelatedClass.GetRelationship().SetClass(rootRelationship);
                navigationPropRelatedClass.GetTargetClass().SetClass(rootTarget);
                result.GetAppendedNavigationPropertyPaths().push_back(navigationPropRelatedClass);
                }
            }
        return result;
        }
public:
    ClassesLocaterPropertyAppender(ContentSpecificationsHandler::Context const& context, bvector<ContentModifierCP> const& contentModifiers, ContentSpecificationCR spec, ECClassCR propertyClass)
        : m_propertyInfos(context.GetSchemaHelper(), contentModifiers, &spec), m_propertyClass(propertyClass), m_context(context)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentClassesLocaterImpl : ContentSpecificationsHandler, ContentSpecificationsVisitor
{
private:
    bvector<SelectClassInfo> m_classes;
    uint32_t m_handledSpecifications;
    ContentSpecificationCP m_currentSpecification;
    bvector<ContentModifierCP> m_contentModifiers;

private:
    Context const& GetContext() const {return static_cast<Context const&>(ContentSpecificationsHandler::GetContext());}

protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool _VisitImplementation(SelectedNodeInstancesSpecificationCR specification) override
        {
        m_currentSpecification = &specification;
        HandleSpecification(specification, *GetInput());
        m_currentSpecification = nullptr;
        m_handledSpecifications++;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool _VisitImplementation(ContentInstancesOfSpecificClassesSpecificationCR specification) override
        {
        m_currentSpecification = &specification;
        HandleSpecification(specification);
        m_currentSpecification = nullptr;
        m_handledSpecifications++;
        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyAppenderPtr _CreatePropertyAppender(std::unordered_set<ECClassCP> const&, RelatedClassPathCR, ECClassCR propertyClass, bvector<RelatedPropertiesSpecification const*> const&, PropertyCategorySpecificationsList const*) override
        {
        return new ClassesLocaterPropertyAppender(GetContext(), m_contentModifiers, *m_currentSpecification, propertyClass);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyAppendResult _OnPropertiesAppended(PropertyAppender& appender, ECClassCR propertyClass, Utf8StringCR propertyClassAlias) override
        {
        ContentSpecificationsHandler::PropertyAppendResult result(false);
        ForEachDerivedClass(GetContext().GetConnection().GetECDb().Schemas(), propertyClass, [&](ECClassCR derivedClass)
            {
            for (auto prop : derivedClass.GetProperties(false))
                result.MergeWith(AppendProperty(appender, *prop, propertyClassAlias.c_str(), {}));
            });
        return result;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::unique_ptr<RelatedPropertyPathsCache> _CreateRelatedPropertyPathsCache(bvector<SelectClassWithExcludes<ECClass>> const&, ContentInstancesOfSpecificClassesSpecificationCR, RelatedInstancePathsCache const&) const override
        {
        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<std::shared_ptr<RelatedPropertySpecificationPaths>> _GetRelatedPropertyPaths(RelatedPropertyPathsParams const& params) const override
        {
        auto const& propertyClass = params.GetSourceClassInfo().GetSelectClass();

        // first build a flat list of all related properties specifications
        auto flatSpecs = GetRelatedPropertySpecifications(params.GetSourceClassInfo().GetSelectClass(), params.GetRelatedPropertySpecs(), params.GetScopeCategorySpecifications(), true);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Got %" PRIu64 " flattened related property specs.", (uint64_t)flatSpecs.size()));

        // then, build the response
        return ContainerHelpers::MoveTransformContainer<bvector<std::shared_ptr<RelatedPropertySpecificationPaths>>>(flatSpecs, [this, &propertyClass](auto&& spec)
            {
            bvector<RelatedClassPath> paths;
            auto propertiesSource = spec->GetFlattened().GetPropertiesSource();
            if (propertiesSource)
                {
                paths = GetContext().GetSchemaHelper().BuildRelationshipPathsFromStepSpecifications(propertyClass.GetClass(), propertiesSource->GetSteps(), {}, true);
                }
            else
                {
                Utf8String relationshipClassNames = spec->GetFlattened().GetRelationshipClassNames();
                if (relationshipClassNames.empty())
                    {
                    auto relationships = GetContext().GetSchemaHelper().GetPossibleRelationships(propertyClass.GetClass(), spec->GetFlattened().GetRequiredRelationDirection(), spec->GetFlattened().GetRelatedClassNames());
                    relationshipClassNames = GetContext().GetSchemaHelper().CreateClassListString(relationships);
                    }
                RelationshipStepSpecification step(relationshipClassNames, spec->GetFlattened().GetRequiredRelationDirection(), spec->GetFlattened().GetRelatedClassNames());
                paths = GetContext().GetSchemaHelper().BuildRelationshipPathsFromStepSpecifications(propertyClass.GetClass(), { &step }, {}, true);
                }
            return std::make_unique<RelatedPropertySpecificationPaths>(std::move(spec), ContainerHelpers::MoveTransformContainer<bvector<RelatedPropertySpecificationPaths::Path>>(paths, [](auto&& path)
                {
                return RelatedPropertySpecificationPaths::Path(std::move(path), {});
                }));
            });
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AppendClass(SelectClassInfo const& classInfo) override
        {
        m_classes.push_back(classInfo);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Appended: `%s`", classInfo.GetSelectClass().GetClass().GetFullName()));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<ContentSource> _BuildContentSource(bvector<SelectClassWithExcludes<ECClass>> const& selectClasses, RelatedInstancePathsCache const&, bvector<RuleApplicationInfo> const&) override
        {
        return ContainerHelpers::TransformContainer<bvector<ContentSource>>(selectClasses, [](auto const& sc)
            {
            // classes locater should always treat input classes as polymorphic
            return ContentSource(SelectClassWithExcludes<ECClass>(sc.GetClass(), sc.GetAlias(), true));
            });
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<ContentSource> _BuildContentSource(bvector<RelatedClassPath> const& paths, RelatedInstancePathsCache const&, bvector<RuleApplicationInfo> const&) override
        {
        return ContainerHelpers::TransformContainer<bvector<ContentSource>>(paths, [](auto const& path)
            {
            ContentSource source(SelectClassWithExcludes<ECClass>(path.back().GetTargetClass().GetClass(), "this", true), nullptr);
            source.SetPathFromInputToSelectClass(path);
            for (RelatedClass& rc : source.GetPathFromInputToSelectClass())
                rc.SetIsTargetOptional(false);
            return source;
            });
        }

public:
    ContentClassesLocaterImpl(ContentClassesLocater::Context& context)
        : ContentSpecificationsHandler(context), ContentSpecificationsVisitor(), m_currentSpecification(nullptr)
        {
        m_contentModifiers = context.GetRulesPreprocessor().GetContentModifiers();
        }
    bvector<SelectClassInfo> const& GetClasses() const {return m_classes;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavNodeKeyCPtr> ContentClassesLocater::GetClassKeys(IRulesPreprocessor& rules, bvector<ECClassCP> const& input) const
    {
    bvector<ECClassCP> lookup = input;
    bset<ECClassCP> ruleConditionClasses = CollectRuleConditionClasses(m_context.GetSchemaHelper(), rules, m_context.GetSchemaHelper().GetECExpressionsCache());
    SplitPolymorphicClassesList(lookup, input, ruleConditionClasses, m_context.GetSchemaHelper().GetConnection().GetECDb().Schemas());

    bvector<NavNodeKeyCPtr> keys;
    for (ECClassCP ecClass : lookup)
        keys.push_back(ECInstancesNodeKey::Create(ECClassInstanceKey(ecClass, ECInstanceId()), "", bvector<Utf8String>()));
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SelectClassInfo> ContentClassesLocater::Locate(bvector<ECClassCP> const& classes) const
    {
    auto scope = Diagnostics::Scope::Create("Get content classes from given input classes");
    DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Total input classes: %" PRIu64, (uint64_t)classes.size()));

    NodeLabelCalculator nodeLabelCalculator(m_context.GetSchemaHelper(), m_context.GetConnections(), m_context.GetConnection(), m_context.GetRuleset().GetRuleSetId(),
        m_context.GetRulesPreprocessor(), m_context.GetRulesetVariables(), m_context.GetSchemaHelper().GetECExpressionsCache(), m_context.GetNavNodeFactory());
    IRulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(GetClassKeys(m_context.GetRulesPreprocessor(), classes)),
        m_context.GetPreferredDisplayType(), nullptr, nodeLabelCalculator, &m_context.GetNodesLocater());
    ContentRuleInputKeysContainer ruleSpecs = m_context.GetRulesPreprocessor().GetContentSpecifications(params);
    DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Total content rules matching the input: %" PRIu64, (uint64_t)ruleSpecs.size()));

    ContentClassesLocaterImpl locater(m_context);
    for (ContentRuleInputKeys const& rule : ruleSpecs)
        {
        DiagnosticsHelpers::ReportRule(rule.GetRule());
        auto ruleScope = Diagnostics::Scope::Create(Utf8PrintfString("Handling %s", DiagnosticsHelpers::CreateRuleIdentifier(rule.GetRule()).c_str()));
        ClassInput input(rule.GetMatchingNodeKeys(), m_context.GetSchemaHelper().GetConnection().GetECDb().Schemas());
        locater.SetCurrentInput(&input);
        for (ContentSpecificationCP spec : rule.GetRule().GetSpecifications())
            {
            auto specificationScope = Diagnostics::Scope::Create(Utf8PrintfString("Handling %s", DiagnosticsHelpers::CreateRuleIdentifier(*spec).c_str()));
            spec->Accept(locater);
            }
        locater.SetCurrentInput(nullptr);
        }

    return locater.GetClasses();
    }
