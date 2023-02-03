/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Rules/PresentationRules.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include <Bentley/Logging.h>
#include "../Shared/Queries/QueryBuilderHelpers.h"
#include "../Shared/ECSchemaHelper.h"
#include "../Shared/ECExpressions/ECExpressionContextsProvider.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentSource
{
private:
    SelectClassWithExcludes<ECClass> m_selectClass;
    ECClassCP m_propertiesSourceOverride;
    RelatedClassPath m_pathFromInputToSelectClass;
    bvector<RelatedClassPath> m_pathsFromSelectToRelatedInstanceClasses;

public:
    ContentSource(SelectClassWithExcludes<ECClass> selectClass) : m_selectClass(selectClass), m_propertiesSourceOverride(nullptr) {}
    ContentSource(SelectClassWithExcludes<ECClass> selectClass, ECClassCP propertiesSourceOverride) : m_selectClass(selectClass), m_propertiesSourceOverride(propertiesSourceOverride) {}

    SelectClassWithExcludes<ECClass> const& GetSelectClass() const {return m_selectClass;}
    SelectClassWithExcludes<ECClass>& GetSelectClass() {return m_selectClass;}

    ECClassCR GetPropertiesSource() const {return m_propertiesSourceOverride ? *m_propertiesSourceOverride : m_selectClass.GetClass();}
    void SetPropertiesSourceOverride(ECClassCP ovr) {m_propertiesSourceOverride = ovr;}

    RelatedClassPath const& GetPathFromInputToSelectClass() const {return m_pathFromInputToSelectClass;}
    RelatedClassPath& GetPathFromInputToSelectClass() {return m_pathFromInputToSelectClass;}
    void SetPathFromInputToSelectClass(RelatedClassPath path) {m_pathFromInputToSelectClass = path;}

    bvector<RelatedClassPath> const& GetPathsFromSelectToRelatedInstanceClasses() const {return m_pathsFromSelectToRelatedInstanceClasses;}
    bvector<RelatedClassPath>& GetPathsFromSelectToRelatedInstanceClasses() {return m_pathsFromSelectToRelatedInstanceClasses;}
    void SetPathsFromSelectToRelatedInstanceClasses(bvector<RelatedClassPath> paths) {m_pathsFromSelectToRelatedInstanceClasses = paths;}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedPropertiesSpecificationScopeInfo
{
private:
    PropertyCategorySpecificationsList const& m_categories;
public:
    RelatedPropertiesSpecificationScopeInfo(PropertyCategorySpecificationsList const& categories)
        : m_categories(categories)
        {}
    PropertyCategorySpecificationsList const& GetCategories() const {return m_categories;}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct FlattenedRelatedPropertiesSpecification
{
private:
    std::shared_ptr<RelatedPropertiesSpecification> m_flatSpecification;
    bvector<RelatedPropertiesSpecification const*> m_specificationsStack;
    RelatedPropertiesSpecificationScopeInfo m_scope;
private:
    bvector<RelatedPropertiesSpecification const*>& GetSpecificationsStack() {return m_specificationsStack;}
public:
    FlattenedRelatedPropertiesSpecification(std::shared_ptr<RelatedPropertiesSpecification> specification, RelatedPropertiesSpecificationScopeInfo scope)
        : m_flatSpecification(std::move(specification)), m_scope(scope)
        {}
    RelatedPropertiesSpecification& GetFlattened() const {return *m_flatSpecification;}
    bvector<RelatedPropertiesSpecification const*> const& GetSource() const {return m_specificationsStack;}
    RelatedPropertiesSpecificationScopeInfo const& GetScope() const {return m_scope;}
    static bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>> Create(RelatedPropertiesSpecificationCR, RelatedPropertiesSpecificationScopeInfo const&);
    static bvector<std::unique_ptr<FlattenedRelatedPropertiesSpecification>> Create(bvector<RelatedPropertiesSpecificationP> const&, RelatedPropertiesSpecificationScopeInfo const&);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RelatedPropertyPathsParams
{
private:
    SelectClassInfo const& m_sourceClassInfo;
    InstanceFilteringParams const& m_instanceFilteringParams;
    RelatedPropertiesSpecificationList const& m_relatedPropertySpecs;
    PropertyCategorySpecificationsList const& m_scopeCategorySpecifications;
public:
    RelatedPropertyPathsParams(SelectClassInfo const& sourceClassInfo, InstanceFilteringParams const& instanceFilteringParams,
        RelatedPropertiesSpecificationList const& specs, PropertyCategorySpecificationsList const& scopeCategorySpecifications)
        : m_sourceClassInfo(sourceClassInfo), m_relatedPropertySpecs(specs), m_scopeCategorySpecifications(scopeCategorySpecifications),
        m_instanceFilteringParams(instanceFilteringParams)
        {}
    SelectClassInfo const& GetSourceClassInfo() const {return m_sourceClassInfo;}
    InstanceFilteringParams const& GetInstanceFilteringParams() const {return m_instanceFilteringParams;}
    RelatedPropertiesSpecificationList const& GetRelatedPropertySpecs() const {return m_relatedPropertySpecs;}
    PropertyCategorySpecificationsList const& GetScopeCategorySpecifications() const {return m_scopeCategorySpecifications;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RelatedPropertySpecificationPaths
{
    struct Path : RelatedClassPath
    {
    private:
        std::unordered_set<ECClassCP> m_actualSourceClasses;
    public:
        Path() {}
        Path(Path&& other)
            : RelatedClassPath(std::move(other)), m_actualSourceClasses(std::move(other.m_actualSourceClasses))
            {}
        Path(Path const& other)
            : RelatedClassPath(other), m_actualSourceClasses(other.m_actualSourceClasses)
            {}
        Path(RelatedClassPath&& source, std::unordered_set<ECClassCP>&& actualSourceClasses)
            : RelatedClassPath(std::move(source)), m_actualSourceClasses(std::move(actualSourceClasses))
            {}
        Path(RelatedClassPath const& source, std::unordered_set<ECClassCP> const& actualSourceClasses)
            : RelatedClassPath(source), m_actualSourceClasses(actualSourceClasses)
            {}
        Path& operator=(Path&& other)
            {
            RelatedClassPath::operator=(std::move(other));
            m_actualSourceClasses = std::move(other.m_actualSourceClasses);
            return *this;
            }
        Path& operator=(Path const& other)
            {
            RelatedClassPath::operator=(other);
            m_actualSourceClasses = other.m_actualSourceClasses;
            return *this;
            }
        std::unordered_set<ECClassCP> const& GetActualSourceClasses() const {return m_actualSourceClasses;}
    };

private:
    std::unique_ptr<FlattenedRelatedPropertiesSpecification> m_specification;
    bvector<Path> m_paths;
public:
    RelatedPropertySpecificationPaths(std::unique_ptr<FlattenedRelatedPropertiesSpecification> spec, bvector<Path> paths)
        : m_specification(std::move(spec)), m_paths(paths)
        {}
    FlattenedRelatedPropertiesSpecification const& GetSpecification() const {return *m_specification;}
    bvector<Path> const& GetPaths() const {return m_paths;}
};

/*=================================================================================**//**
* Base abstract class for content specification handlers.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsHandler
{
    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct Context
    {
    private:
        IConnectionManagerCR m_connections;
        IConnectionCR m_connection;
        ICancelationTokenCP m_cancellationToken;
        IRulesPreprocessorR m_rulesPreprocessor;
        PresentationRuleSetCR m_ruleset;
        RulesetVariables const& m_rulesetVariables;
        Utf8CP m_preferredDisplayType;
        ECSchemaHelper const& m_helper;
        INavNodeKeysContainerCPtr m_inputKeys;
        bmap<ECClassCP, size_t> m_classCounter;
        bset<ECClassCP> m_handledClasses;
        bmap<ECSchemaCP, bmap<ECClassCP, RelatedPropertiesSpecificationList>> m_relatedPropertySpecifications;
        mutable ECClassUseCounter m_relationshipUseCounts;
        bmap<ECClassCP, bvector<RelatedClass>> m_handledNavigationPropertiesPaths;
        std::function<int(int)> m_contentFlagsCalculator;
    private:
        size_t GetClassCount(ECClassCR ecClass) {return m_classCounter[&ecClass]++;}
    public:
        Context(ECSchemaHelper const& helper, IConnectionManagerCR connections, IConnectionCR connection, ICancelationTokenCP cancellationToken,
            IRulesPreprocessorR rulesPreprocessor, PresentationRuleSetCR ruleset,
            RulesetVariables const& variables, Utf8CP preferredDisplayType, INavNodeKeysContainerCR inputKeys)
            : m_helper(helper), m_connections(connections), m_connection(connection), m_cancellationToken(cancellationToken),
            m_rulesPreprocessor(rulesPreprocessor), m_ruleset(ruleset), m_rulesetVariables(variables),
            m_preferredDisplayType(preferredDisplayType), m_inputKeys(&inputKeys)
            {}
        IConnectionManagerCR GetConnections() const {return m_connections;}
        IConnectionCR GetConnection() const {return m_connection;}
        ICancelationTokenCP GetCancellationToken() const {return m_cancellationToken;}
        IRulesPreprocessorR GetRulesPreprocessor() const {return m_rulesPreprocessor;}
        PresentationRuleSetCR GetRuleset() const {return m_ruleset;}
        RulesetVariables const& GetRulesetVariables() const {return m_rulesetVariables;}
        Utf8CP GetPreferredDisplayType() const {return m_preferredDisplayType;}
        void SetPreferredDisplayType(Utf8CP value) {m_preferredDisplayType = value;}
        INavNodeKeysContainerCR GetInputKeys() const {return *m_inputKeys;}
        ECSchemaHelper const& GetSchemaHelper() const {return m_helper;}
        ECClassUseCounter& GetRelationshipUseCounts() const {return m_relationshipUseCounts;}
        bool IsClassHandled(ECClassCR ecClass) const {return m_handledClasses.end() != m_handledClasses.find(&ecClass);}
        void SetClassHandled(ECClassCR ecClass) {m_handledClasses.insert(&ecClass);}
        void AddNavigationPropertiesPaths(ECClassCR ecClass, bvector<RelatedClass> navigationPropertiesPaths) {m_handledNavigationPropertiesPaths[&ecClass] = navigationPropertiesPaths;}
        bvector<RelatedClass> GetNavigationPropertiesPaths(ECClassCR ecClass) {return m_handledNavigationPropertiesPaths[&ecClass];}
        std::function<int(int)> const& GetContentFlagsCalculator() const {return m_contentFlagsCalculator;}
        void SetContentFlagsCalculator(std::function<int(int)> func) {m_contentFlagsCalculator = func;}
        Utf8String CreateRelatedClassAlias(ECClassCR);
        Utf8String CreateNavigationClassAlias(ECClassCR);
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct PropertyAppendResult
    {
        struct ReplacedRelationshipPath
            {
            RelatedClassPath prev;
            RelatedClassPath curr;
            ReplacedRelationshipPath(RelatedClassPath p, RelatedClassPath c) : prev(p), curr(c) {}
            };
    private:
        bool m_didAppend;
        bvector<RelatedClass> m_appendedNavigationPropertyPaths;
        std::unique_ptr<ReplacedRelationshipPath> m_replacedSelectToPropertyPath;
    public:
        PropertyAppendResult(bool didAppend) : m_didAppend(didAppend) {}
        PropertyAppendResult(bool didAppend, std::unique_ptr<ReplacedRelationshipPath>&& replacedPath)
            : m_didAppend(didAppend), m_replacedSelectToPropertyPath(std::move(replacedPath))
            {}
        bool DidAppend() const {return m_didAppend;}
        bvector<RelatedClass> const& GetAppendedNavigationPropertyPaths() const {return m_appendedNavigationPropertyPaths;}
        bvector<RelatedClass>& GetAppendedNavigationPropertyPaths() {return m_appendedNavigationPropertyPaths;}
        ReplacedRelationshipPath const* GetReplacedSelectToPropertyPath() const {return m_replacedSelectToPropertyPath.get();}
        PropertyAppendResult& MergeWith(PropertyAppendResult const& other)
            {
            m_didAppend |= other.DidAppend();
            ContainerHelpers::Push(m_appendedNavigationPropertyPaths, other.GetAppendedNavigationPropertyPaths());
            if (other.GetReplacedSelectToPropertyPath())
                {
                if (!m_replacedSelectToPropertyPath)
                    m_replacedSelectToPropertyPath = std::make_unique<ReplacedRelationshipPath>(*other.GetReplacedSelectToPropertyPath());
                else
                    m_replacedSelectToPropertyPath->curr = other.GetReplacedSelectToPropertyPath()->curr;
                }
            return *this;
            }
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct PropertyAppender : RefCountedBase
    {
    private:
        ExpressionContextPtr m_expressionContext = nullptr;
    protected:
        virtual bool _Supports(ECPropertyCR, PropertySpecificationsList const&) = 0;
        virtual PropertyAppendResult _Append(ECPropertyCR, Utf8CP, PropertySpecificationsList const&) = 0;
        ExpressionContextPtr CreateExpressionContext(Context const& context)
            {
            if (m_expressionContext != nullptr)
                return m_expressionContext;

            ECExpressionContextsProvider::ContextParametersBase params(context.GetConnection(), context.GetRulesetVariables(), nullptr);
            return m_expressionContext = ECExpressionContextsProvider::GetRulesEngineRootContext(params);
            }
    public:
        bool Supports(ECPropertyCR ecProperty, PropertySpecificationsList const& overrides) {return _Supports(ecProperty, overrides);}
        PropertyAppendResult Append(ECPropertyCR ecProperty, Utf8CP propertyClassAlias, PropertySpecificationsList const& overrides) {return _Append(ecProperty, propertyClassAlias, overrides);}
    };
    typedef RefCountedPtr<PropertyAppender> PropertyAppenderPtr;

    struct AppendRelatedPropertiesParams;

private:
    Context& m_context;
    mutable bvector<RuleApplicationInfo> const* m_customizationRuleInfos;

private:
    void AppendContent(ContentSource const&, ContentSpecificationCR, IParsedInput const*, Utf8StringCR instanceFilter, RecursiveQueryInfo const*);
    PropertyAppendResult AppendRelatedProperties(PropertySpecificationsList const&, PropertyAppender&, ECClassCR propertiesClass, Utf8StringCR propertiesClassAlias);
    bvector<RelatedClassPath> AppendRelatedProperties(AppendRelatedPropertiesParams const&);
    bvector<RuleApplicationInfo> const& GetCustomizationRuleInfos() const;

protected:
    ECPRESENTATION_EXPORT virtual int _GetContentFlags(ContentSpecificationCR) const;
    virtual PropertyAppenderPtr _CreatePropertyAppender(std::unordered_set<ECClassCP> const& actualSourceClasses, RelatedClassPathCR pathFromSelectToPropertyClass, ECClassCR propertyClass,
        bvector<RelatedPropertiesSpecification const*> const& relatedPropertyStack, PropertyCategorySpecificationsList const*) = 0;
    virtual bvector<std::unique_ptr<RelatedPropertySpecificationPaths>> _GetRelatedPropertyPaths(RelatedPropertyPathsParams const&) const;
    virtual void _AppendClass(SelectClassInfo const&) = 0;
    virtual PropertyAppendResult _OnPropertiesAppended(PropertyAppender&, ECClassCR, Utf8StringCR) {return PropertyAppendResult(false);}
    virtual void _OnContentAppended() {}
    ECPRESENTATION_EXPORT virtual bvector<ContentSource> _BuildContentSource(bvector<SelectClassWithExcludes<ECClass>> const&, ContentSpecificationCR);
    ECPRESENTATION_EXPORT virtual bvector<ContentSource> _BuildContentSource(bvector<RelatedClassPath> const&, ContentSpecificationCR);
    bvector<ContentSource> CreateContentSources(SelectClassWithExcludes<ECClass> const& selectClass, ECClassCP propertiesSourceClass, ContentSpecificationCR) const;
    bvector<ContentSource> CreateContentSources(RelatedClassPath const& pathFromInputToSelectClass, ContentSpecificationCR) const;
    PropertyAppendResult AppendProperty(PropertyAppender&, ECPropertyCR, Utf8CP alias, PropertySpecificationsList const& overrides);
    static int GetDefaultContentFlags(Utf8CP displayType, ContentSpecificationCR spec);

protected:
    ContentSpecificationsHandler(Context& context) : m_context(context), m_customizationRuleInfos(nullptr) {}
    ContentSpecificationsHandler(ContentSpecificationsHandler const& other) : m_context(other.m_context), m_customizationRuleInfos(nullptr) {}
    virtual ~ContentSpecificationsHandler() {DELETE_AND_CLEAR(m_customizationRuleInfos);}
    Context& GetContext() {return m_context;}
    Context const& GetContext() const {return m_context;}
    ECPRESENTATION_EXPORT void HandleSpecification(SelectedNodeInstancesSpecificationCR, IParsedInput const&);
    ECPRESENTATION_EXPORT void HandleSpecification(ContentRelatedInstancesSpecificationCR, IParsedInput const&);
    ECPRESENTATION_EXPORT void HandleSpecification(ContentInstancesOfSpecificClassesSpecificationCR);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsVisitor : PresentationRuleSpecificationVisitor
{
private:
    IParsedInput const* m_input;
    bool m_hasAlreadyVisited;
protected:
    IParsedInput const* GetInput() {return m_input;}
public:
    ContentSpecificationsVisitor() : m_input(nullptr), m_hasAlreadyVisited(false) {}
    virtual ~ContentSpecificationsVisitor() {}
    void SetCurrentInput(IParsedInput const* input) {m_input = input;}

    virtual bool _VisitImplementation(SelectedNodeInstancesSpecificationCR specification) {return false;};
    void _Visit(SelectedNodeInstancesSpecificationCR specification) override
        {
        if (m_hasAlreadyVisited && specification.GetOnlyIfNotHandled())
            return;

        m_hasAlreadyVisited |= _VisitImplementation(specification);
        }

    virtual bool _VisitImplementation(ContentInstancesOfSpecificClassesSpecificationCR specification) {return false;};
    void _Visit(ContentInstancesOfSpecificClassesSpecificationCR specification) override
        {
        if (m_hasAlreadyVisited && specification.GetOnlyIfNotHandled())
            return;

        m_hasAlreadyVisited |= _VisitImplementation(specification);
        }

    virtual bool _VisitImplementation(ContentRelatedInstancesSpecificationCR specification) {return false;};
    void _Visit(ContentRelatedInstancesSpecificationCR specification) override
        {
        if (m_hasAlreadyVisited && specification.GetOnlyIfNotHandled())
            return;

        m_hasAlreadyVisited |= _VisitImplementation(specification);
        }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
