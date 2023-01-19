/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include "HierarchiesFiltering.h"
#include "NavNodesHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ChildNodeSpecificationCP> GetDirectChildNodeSpecifications(NavNodeCP parentNode, IRulesPreprocessorR rulesPreprocessor)
    {
    bvector<ChildNodeSpecificationCP> nodeSpecs;
    if (!parentNode)
        {
        IRulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
        ContainerHelpers::TransformContainer(
            nodeSpecs,
            rulesPreprocessor.GetRootNodeSpecifications(params),
            [](auto const& ruleSpec){return &ruleSpec.GetSpecification();});
        }
    else
        {
        IRulesPreprocessor::ChildNodeRuleParameters params(*parentNode, TargetTree_MainTree);
        ContainerHelpers::TransformContainer(
            nodeSpecs,
            rulesPreprocessor.GetChildNodeSpecifications(params),
            [](auto const& ruleSpec){return &ruleSpec.GetSpecification();});
        }
    return nodeSpecs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ChildNodeSpecificationCP> GetDirectChildNodeSpecifications(bvector<NavNodeCPtr> const& parentNodes, IRulesPreprocessorR rulesPreprocessor)
    {
    VectorSet<ChildNodeSpecificationCP> childSpecs;
    for (auto const& parentNode : parentNodes)
        ContainerHelpers::Push(childSpecs, GetDirectChildNodeSpecifications(parentNode.get(), rulesPreprocessor));
    return ContainerHelpers::TransformContainer<bvector<ChildNodeSpecificationCP>>(childSpecs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECClassCP> GetQuerySpecificationClasses(SearchResultInstanceNodesSpecificationCR specification, ECSchemaHelper const& schemaHelper)
    {
    VectorSet<ECClassCP> queryClasses;
    for (auto const& querySpec : specification.GetQuerySpecifications())
        {
        ECClassCP queryClass = schemaHelper.GetECClass(querySpec->GetSchemaName().c_str(), querySpec->GetClassName().c_str());
        if (queryClass)
            queryClasses.push_back(queryClass);
        }
    return ContainerHelpers::TransformContainer<bvector<ECClassCP>>(queryClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECClassCP> GetRelationshipPathTargetClasses(RepeatableRelationshipPathSpecification const& specification, ECSchemaHelper const& schemaHelper)
    {
    if (specification.GetSteps().empty())
        return {};

    auto const& lastStep = specification.GetSteps().back();
    auto relationshipClass = schemaHelper.GetECClass(lastStep->GetRelationshipClassName().c_str());
    if (!relationshipClass || !relationshipClass->IsRelationshipClass())
        return {};

    VectorSet<ECClassCP> targetClasses;
    auto targetClass = lastStep->GetTargetClassName().size() ? schemaHelper.GetECClass(lastStep->GetTargetClassName().c_str()) : nullptr;
    if (targetClass)
        {
        targetClasses.push_back(targetClass);
        }
    else
        {
        if (lastStep->GetRelationDirection() == RequiredRelationDirection_Both || lastStep->GetRelationDirection() == RequiredRelationDirection_Forward)
            ContainerHelpers::Push(targetClasses, schemaHelper.GetRelationshipConstraintClasses(*relationshipClass->GetRelationshipClassCP(), ECRelatedInstanceDirection::Forward, ""));
        if (lastStep->GetRelationDirection() == RequiredRelationDirection_Both || lastStep->GetRelationDirection() == RequiredRelationDirection_Backward)
            ContainerHelpers::Push(targetClasses, schemaHelper.GetRelationshipConstraintClasses(*relationshipClass->GetRelationshipClassCP(), ECRelatedInstanceDirection::Backward, ""));
        }
    return ContainerHelpers::TransformContainer<bvector<ECClassCP>>(targetClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void TraverseChildNodeSpecifications(
    PresentationRuleSpecificationVisitorR visitor, 
    CustomNodeSpecificationCR specification, 
    TraverseHierarchyRulesProps const& props
)
    {
    auto fakeParentNode = props.GetNodesFactory().CreateCustomNode(props.GetSchemaHelper().GetConnection(), specification.GetHash(), nullptr,
        *LabelDefinition::FromString(specification.GetLabel().c_str()), specification.GetDescription().c_str(), specification.GetImageId().c_str(),
        specification.GetNodeType().c_str(), nullptr);
    auto childSpecs = GetDirectChildNodeSpecifications(fakeParentNode.get(), props.GetRulesPreprocessor());
    for (auto const& childSpec : childSpecs)
        childSpec->Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void TraverseChildNodeSpecifications(
    PresentationRuleSpecificationVisitorR visitor, 
    InstanceNodesOfSpecificClassesSpecificationCR specification, 
    TraverseHierarchyRulesProps const& props
)
    {
    auto targetClasses = props.GetSchemaHelper().GetECClassesFromClassList(specification.GetClasses(), false);
    auto fakeParentNodes = ContainerHelpers::TransformContainer<bvector<NavNodeCPtr>>(targetClasses, [&](auto const& targetClass)
        {
        return props.GetNodesFactory().CreateECInstanceNode(props.GetSchemaHelper().GetConnection(), specification.GetHash(), nullptr, targetClass.GetClass().GetId(), ECInstanceId(), *LabelDefinition::Create());
        });
    auto childSpecs = GetDirectChildNodeSpecifications(fakeParentNodes, props.GetRulesPreprocessor());
    for (auto const& childSpec : childSpecs)
        childSpec->Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void TraverseChildNodeSpecifications(
    PresentationRuleSpecificationVisitorR visitor, 
    RelatedInstanceNodesSpecificationCR specification, 
    RepeatableRelationshipPathSpecification const& pathSpecification, 
    TraverseHierarchyRulesProps const& props
)
    {
    auto targetClasses = GetRelationshipPathTargetClasses(pathSpecification, props.GetSchemaHelper());
    auto fakeParentNodes = ContainerHelpers::TransformContainer<bvector<NavNodeCPtr>>(targetClasses, [&](auto const& targetClass)
        {
        return props.GetNodesFactory().CreateECInstanceNode(props.GetSchemaHelper().GetConnection(), specification.GetHash(), nullptr, targetClass->GetId(), ECInstanceId(), *LabelDefinition::Create());
        });
    auto childSpecs = GetDirectChildNodeSpecifications(fakeParentNodes, props.GetRulesPreprocessor());
    for (auto const& childSpec : childSpecs)
        childSpec->Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void TraverseChildNodeSpecifications(
    PresentationRuleSpecificationVisitorR visitor, 
    SearchResultInstanceNodesSpecificationCR specification, 
    TraverseHierarchyRulesProps const& props
)
    {
    auto queryClasses = GetQuerySpecificationClasses(specification, props.GetSchemaHelper());
    auto fakeParentNodes = ContainerHelpers::TransformContainer<bvector<NavNodeCPtr>>(queryClasses, [&](auto const& queryClass)
        {
        return props.GetNodesFactory().CreateECInstanceNode(props.GetSchemaHelper().GetConnection(), specification.GetHash(), nullptr, queryClass->GetId(), ECInstanceId(), *LabelDefinition::Create());
        });
    auto childSpecs = GetDirectChildNodeSpecifications(fakeParentNodes, props.GetRulesPreprocessor());
    for (auto const& childSpec : childSpecs)
        childSpec->Accept(visitor);
    }

#define REPORT_ISSUE(issue) \
    { \
    m_hasIssues = true; \
    if (m_issues) \
        m_issues->push_back(issue); \
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DirectUnfilterableSpecificationsDetector : PresentationRuleSpecificationVisitor
{
private:
    bvector<Utf8String>* m_issues;
    bool m_hasIssues;
private:
    void HandleCommonAttributes(ChildNodeSpecificationCR spec)
        {
        if (!spec.GetHideExpression().empty())
            REPORT_ISSUE(Utf8PrintfString("%s does not support filtering due to hide expression attribute", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        }
public:
    void _Visit(AllInstanceNodesSpecification const& spec) override
        {
        REPORT_ISSUE(Utf8PrintfString("%s does not support filtering", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        }
    void _Visit(AllRelatedInstanceNodesSpecification const& spec) override
        {
        REPORT_ISSUE(Utf8PrintfString("%s does not support filtering", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        }
    void _Visit(CustomNodeSpecification const&) override
        {
        }
    void _Visit(InstanceNodesOfSpecificClassesSpecification const& spec) override
        {
        HandleCommonAttributes(spec);
        }
    void _Visit(RelatedInstanceNodesSpecification const& spec) override
        {
        HandleCommonAttributes(spec);
        if (spec.GetSkipRelatedLevel() != 0)
            REPORT_ISSUE(Utf8PrintfString("%s does not support filtering due to deprecated \"skip related level\" attribute", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        if (!spec.GetSupportedSchemas().empty())
            REPORT_ISSUE(Utf8PrintfString("%s does not support filtering due to deprecated \"supported schemas\" attribute", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        if (!spec.GetRelationshipClassNames().empty())
            REPORT_ISSUE(Utf8PrintfString("%s does not support filtering due to deprecated \"relationship class names\" attribute", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        if (!spec.GetRelatedClassNames().empty())
            REPORT_ISSUE(Utf8PrintfString("%s does not support filtering due to deprecated \"related class names\" attribute", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        }
    void _Visit(SearchResultInstanceNodesSpecification const& spec) override
        {
        HandleCommonAttributes(spec);
        }
public:
    DirectUnfilterableSpecificationsDetector(bvector<Utf8String>* issues)
        : m_issues(issues), m_hasIssues(false)
        {}
    bool LogsIssues() const {return nullptr != m_issues;}
    bool DidFindAnyUnfilterableSpecifications() const {return m_hasIssues;}
};

#define ENSURE_NOT_VISITED(set, specCR) \
    { \
    if (set.end() != set.find(&specCR)) \
        return; \
    set.insert(&specCR); \
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchicalUnfilterableSpecificationsDetector : PresentationRuleSpecificationVisitor
{
private:
    TraverseHierarchyRulesProps const& m_props;
    DirectUnfilterableSpecificationsDetector m_directDetector;
    bset<ChildNodeSpecificationCP> m_visitedSpecs;
private:
    bool ShouldTraverseChildNodeSpecifications(ChildNodeSpecificationCR spec) const
        {
        if (m_directDetector.DidFindAnyUnfilterableSpecifications() && !m_directDetector.LogsIssues())
            {
            // no need to traverse deeper if we already found unsupported specs and we're not logging issues
            return false;
            }
        return spec.GetHideNodesInHierarchy();
        }
protected:
    void _Visit(AllInstanceNodesSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        m_directDetector._Visit(specification);
        }
    void _Visit(AllRelatedInstanceNodesSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        m_directDetector._Visit(specification);
        }
    void _Visit(CustomNodeSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        m_directDetector._Visit(specification);
        if (ShouldTraverseChildNodeSpecifications(specification))
            TraverseChildNodeSpecifications(*this, specification, m_props);
        }
    void _Visit(InstanceNodesOfSpecificClassesSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        m_directDetector._Visit(specification);
        if (ShouldTraverseChildNodeSpecifications(specification))
            TraverseChildNodeSpecifications(*this, specification, m_props);
        }
    void _Visit(RelatedInstanceNodesSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        m_directDetector._Visit(specification);
        if (ShouldTraverseChildNodeSpecifications(specification))
            {
            for (auto const& pathSpec : specification.GetRelationshipPaths())
                TraverseChildNodeSpecifications(*this, specification, *pathSpec, m_props);
            }
        }
    void _Visit(SearchResultInstanceNodesSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        m_directDetector._Visit(specification);
        if (ShouldTraverseChildNodeSpecifications(specification))
            TraverseChildNodeSpecifications(*this, specification, m_props);
        }
public:
    HierarchicalUnfilterableSpecificationsDetector(TraverseHierarchyRulesProps const& props, bvector<Utf8String>* issues)
        : m_props(props), m_directDetector(issues)
        {
        }
    bool DidFindAnyUnfilterableSpecifications() const
        {
        return m_directDetector.DidFindAnyUnfilterableSpecifications();
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool HierarchiesFilteringHelper::SupportsFiltering(ChildNodeSpecificationCR spec, bvector<Utf8String>* reasons)
    {
    DirectUnfilterableSpecificationsDetector detector(reasons);
    spec.Accept(detector);
    return !detector.DidFindAnyUnfilterableSpecifications();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool HierarchiesFilteringHelper::SupportsFiltering(NavNodeCP parentNode, TraverseHierarchyRulesProps const& props, bvector<Utf8String>* reasons)
    {
    HierarchicalUnfilterableSpecificationsDetector detector(props, reasons);
    bvector<ChildNodeSpecificationCP> nodeSpecs = GetDirectChildNodeSpecifications(parentNode, props.GetRulesPreprocessor());
    for (auto const& spec : nodeSpecs)
        spec->Accept(detector);
    return !detector.DidFindAnyUnfilterableSpecifications();
    }

//===================================================================================
// @bsiclass
//===================================================================================
struct HierarchySpecsToContentRulesetConverter : PresentationRuleSpecificationVisitor
{
    struct ChainedSpecificationsContext
    {
    private:
        std::function<void()> m_restoreState;
    private:
        static std::function<void()> CreateStateRestoreFunc(HierarchySpecsToContentRulesetConverter& converter)
            {
            return [
                &converter,
                wasInputReset = converter.m_inputReset,
                wasPathPrefix = converter.m_pathPrefix
            ]()
                {
                converter.m_inputReset = wasInputReset;
                converter.m_pathPrefix = wasPathPrefix;
                };
            }
    public:
        ChainedSpecificationsContext(HierarchySpecsToContentRulesetConverter& converter, InstanceNodesOfSpecificClassesSpecification const&)
            : m_restoreState(CreateStateRestoreFunc(converter))
            {
            converter.m_inputReset = true;
            converter.m_pathPrefix.clear();
            }
        ChainedSpecificationsContext(HierarchySpecsToContentRulesetConverter& converter, SearchResultInstanceNodesSpecification const&)
            : m_restoreState(CreateStateRestoreFunc(converter))
            {
            converter.m_inputReset = true;
            converter.m_pathPrefix.clear();
            }
        ChainedSpecificationsContext(HierarchySpecsToContentRulesetConverter& converter, RelatedInstanceNodesSpecification const& spec, RepeatableRelationshipPathSpecification const& path)
            : m_restoreState(CreateStateRestoreFunc(converter))
            {
            converter.m_pathPrefix.push_back(&path);
            }
        ~ChainedSpecificationsContext()
            {
            if (m_restoreState)
                m_restoreState();
            }
    };

private:
    TraverseHierarchyRulesProps const& m_props;

    PresentationRuleSetPtr m_ruleset;
    ContentRuleP m_contentRule;
    bool m_inputReset;
    bvector<RepeatableRelationshipPathSpecification const*> m_pathPrefix;
    bset<ChildNodeSpecificationCP> m_visitedSpecs;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentRuleR GetInProgressContentRule()
        {
        if (m_ruleset.IsNull())
            {
            // will set the id when returning the ruleset
            m_ruleset = PresentationRuleSet::CreateInstance("");
            }
        if (!m_contentRule)
            {
            // ruleset takes ownership of the rule
            m_contentRule = new ContentRule();
            m_ruleset->AddPresentationRule(*m_contentRule);
            }
        return *m_contentRule;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(AllInstanceNodesSpecification const& specification) override
        {
        throw FilteringNotSupportedException({
            Utf8PrintfString("%s does not support filtering", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()),
            });
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(AllRelatedInstanceNodesSpecification const& specification) override
        {
        throw FilteringNotSupportedException({
            Utf8PrintfString("%s does not support filtering", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()),
            });
        }

    /*---------------------------------------------------------------------------------**//**
    * Custom node itself doesn't represent any instances, but we may need to handle its child
    * specifications if the custom node specification has 'hide nodes in hierarchy' flag
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(CustomNodeSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        ENSURE_SUPPORTS_FILTERING(specification);
        if (specification.GetHideNodesInHierarchy())
            TraverseChildNodeSpecifications(*this, specification, m_props);

        // custom node rule always gets filtered-out as it doesn't represent an instance, so
        // no handling is necessary
        }

    /*---------------------------------------------------------------------------------**//**
    * When the specification has 'hide nodes in hierarchy' flag, we want to find target classes,
    * find child node specifications for them and handle each of them individually. Otherwise
    * we can just translate `InstanceNodesOfSpecificClassesSpecification` to `ContentInstancesOfSpecificClassesSpecification`.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(InstanceNodesOfSpecificClassesSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        ENSURE_SUPPORTS_FILTERING(specification);

        if (specification.GetHideNodesInHierarchy())
            {
            ChainedSpecificationsContext chained(*this, specification);
            TraverseChildNodeSpecifications(*this, specification, m_props);
            return;
            }

        auto contentSpec = new ContentInstancesOfSpecificClassesSpecification(
            1000,
            false,
            specification.GetInstanceFilter(),
            ContainerHelpers::TransformContainer<bvector<MultiSchemaClass*>>(specification.GetClasses(), [](auto srcP){return new MultiSchemaClass(*srcP);}),
            ContainerHelpers::TransformContainer<bvector<MultiSchemaClass*>>(specification.GetExcludedClasses(), [](auto srcP){return new MultiSchemaClass(*srcP);}),
            true
            );
        for (auto const& relatedInstanceSpec : specification.GetRelatedInstances())
            contentSpec->AddRelatedInstance(*new RelatedInstanceSpecification(*relatedInstanceSpec));
        GetInProgressContentRule().AddSpecification(*contentSpec);
        }

    /*---------------------------------------------------------------------------------**//**
    * This is the most complex case caused by the 'hide nodes in hierarchy' flag.
    *
    * If it's not  present, then we can simply convert `RelatedInstanceNodesSpecification`
    * to `ContentRelatedInstancesSpecification`.
    *
    * If it there, we have to find the specifications that are producing visible nodes. If all hidden
    * hierarchy levels was made using the `RelatedInstanceNodesSpecification`, we can simply create a
    * merged relationship path - that allows us to filter targets based on input keys. If there's either
    * `InstanceNodesOfSpecificClassesSpecification` or `SearchResultInstanceNodesSpecification` - we can't
    * use the keys anymore, at which point we only look at the target classes and create
    * `ContentInstancesOfSpecificClassesSpecification` rather than `ContentRelatedInstancesSpecification`.
    *
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(RelatedInstanceNodesSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        ENSURE_SUPPORTS_FILTERING(specification);

        if (m_inputReset)
            {
            // we get here if either `InstanceNodesOfSpecificClassesSpecification` or `SearchResultInstanceNodesSpecification` was detected
            // when traversing hidden hierarchy levels
            for (auto const& pathSpec : specification.GetRelationshipPaths())
                {
                if (specification.GetHideNodesInHierarchy())
                    {
                    // if the nodes are hidden - fake a parent node, find child specifications and recurse
                    ChainedSpecificationsContext chained(*this, specification, *pathSpec);
                    TraverseChildNodeSpecifications(*this, specification, *pathSpec, m_props);
                    }
                else
                    {
                    // create a `ContentInstancesOfSpecificClassesSpecification` for the target classes
                    auto targetClasses = GetRelationshipPathTargetClasses(*pathSpec, m_props.GetSchemaHelper());
                    auto contentSpec = new ContentInstancesOfSpecificClassesSpecification(
                        1000,
                        false,
                        specification.GetInstanceFilter(),
                        ContainerHelpers::TransformContainer<bvector<MultiSchemaClass*>>(targetClasses, [](auto targetClass)
                            {
                            return new MultiSchemaClass(targetClass->GetSchema().GetName(), true, { targetClass->GetName() });
                            }),
                        bvector<MultiSchemaClass*>(),
                        true
                        );
                    for (auto const& relatedInstanceSpec : specification.GetRelatedInstances())
                        contentSpec->AddRelatedInstance(*new RelatedInstanceSpecification(*relatedInstanceSpec));
                    GetInProgressContentRule().AddSpecification(*contentSpec);
                    }
                }
            }

        // In case we had a chain of hidden `RelatedInstanceNodesSpecification` specifications, we accumulate their paths. At this point
        // we have to merge the steps into a single path prefix.
        bvector<RepeatableRelationshipStepSpecification> prefixSteps;
        for (auto pathPrefixCP : m_pathPrefix)
            {
            for (auto const& step : pathPrefixCP->GetSteps())
                prefixSteps.push_back(*step);
            }

        // The specification may have multiple paths - we have to prefix all of them
        bvector<RepeatableRelationshipPathSpecification*> paths;
        ContainerHelpers::TransformContainer(
            paths,
            specification.GetRelationshipPaths(),
            [&](auto const& srcPath)
                {
                bvector<RepeatableRelationshipStepSpecification*> steps;
                for (auto const& prefixStep : prefixSteps)
                    steps.push_back(new RepeatableRelationshipStepSpecification(prefixStep));
                for (auto const& pathStep : srcPath->GetSteps())
                    steps.push_back(new RepeatableRelationshipStepSpecification(*pathStep));
                return new RepeatableRelationshipPathSpecification(steps);
                }
            );

        auto contentSpec = new ContentRelatedInstancesSpecification(1000, false, specification.GetInstanceFilter(), paths);
        for (auto const& relatedInstanceSpec : specification.GetRelatedInstances())
            contentSpec->AddRelatedInstance(*new RelatedInstanceSpecification(*relatedInstanceSpec));
        GetInProgressContentRule().AddSpecification(*contentSpec);
        }

    /*---------------------------------------------------------------------------------**//**
    * Similar to `InstanceNodesOfSpecificClassesSpecification`
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(SearchResultInstanceNodesSpecification const& specification) override
        {
        ENSURE_NOT_VISITED(m_visitedSpecs, specification);
        ENSURE_SUPPORTS_FILTERING(specification);

        if (specification.GetHideNodesInHierarchy())
            {
            ChainedSpecificationsContext chained(*this, specification);
            TraverseChildNodeSpecifications(*this, specification, m_props);
            return;
            }

        auto contentSpec = new ContentInstancesOfSpecificClassesSpecification(
            1000,
            false,
            "",
            ContainerHelpers::TransformContainer<bvector<MultiSchemaClass*>>(
                GetQuerySpecificationClasses(specification, m_props.GetSchemaHelper()),
                [](auto queryClass)
                    {
                    return new MultiSchemaClass(queryClass->GetSchema().GetName(), true, { queryClass->GetName() });
                    }
                ),
            bvector<MultiSchemaClass*>(),
            true
        );
        for (auto const& relatedInstanceSpec : specification.GetRelatedInstances())
            contentSpec->AddRelatedInstance(*new RelatedInstanceSpecification(*relatedInstanceSpec));
        GetInProgressContentRule().AddSpecification(*contentSpec);
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    HierarchySpecsToContentRulesetConverter(TraverseHierarchyRulesProps const& props)
        : m_contentRule(nullptr), m_props(props), m_inputReset(false)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationRuleSetPtr GetRuleset()
        {
        GetInProgressContentRule(); // just to make sure we have the ruleset
        PresentationRuleSetPtr ruleset = m_ruleset;
        m_ruleset->SetRuleSetId(Utf8String("NodesDescriptor:").append(m_contentRule->GetHash()));
        m_ruleset = nullptr;
        m_contentRule = nullptr;
        return ruleset;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr HierarchiesFilteringHelper::CreateHierarchyLevelDescriptorRuleset(NavNodeCP parentNode, TraverseHierarchyRulesProps const& props)
    {
    bvector<ChildNodeSpecificationCP> nodeSpecs = GetDirectChildNodeSpecifications(parentNode, props.GetRulesPreprocessor());
    HierarchySpecsToContentRulesetConverter contentRulesetFactory(props);
    for (auto spec : nodeSpecs)
        spec->Accept(contentRulesetFactory);
    return contentRulesetFactory.GetRuleset();
    }
