/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryBuilder.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <Logging/bentleylogging.h>
#include "JsonNavNode.h"
#include "NavNodesCache.h"
#include "NavigationQuery.h"
#include "ECSchemaHelper.h"
#include "QueryContracts.h"
#include "ContentSpecificationsHandler.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define RULES_ENGINE_LOCAL_STATE_NAMESPACE                  "RulesEngine"
#define RULES_ENGINE_ACTIVE_GROUPS_LOCAL_STATE_NAMESPACE    RULES_ENGINE_LOCAL_STATE_NAMESPACE "_ActiveGroups"

#define SEARCH_QUERY_FIELD_ECInstanceId "RulesEngine_ECInstanceId"
#define SEARCH_QUERY_FIELD_ECClassId    "RulesEngine_ECClassId"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct IUsedClassesListener
    {
    virtual ~IUsedClassesListener() {}
    virtual void _OnClassUsed(ECClassCR, bool polymorphically) = 0;
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
struct UsedClassesHelper
{
private:
    UsedClassesHelper() {}
public:
    static void NotifyListenerWithUsedClasses(IUsedClassesListener&, ECSchemaHelper const&, ECExpressionsCache&, Utf8StringCR ecexpression);
    static void NotifyListenerWithRulesetClasses(IUsedClassesListener&, ECSchemaHelper const&, ECExpressionsCache&, PresentationRuleSetCR);
    static void NotifyListenerWithUsedClasses(IECDbUsedClassesListener&, ECSchemaHelper const&, ECExpressionsCache&, Utf8StringCR ecexpression);
    static void NotifyListenerWithRulesetClasses(IECDbUsedClassesListener&, ECSchemaHelper const&, ECExpressionsCache&, PresentationRuleSetCR);
};

/*=============================================================================**//**
* @bsiclass                                     Grigas.Petraitis            07/2016
+===============+===============+===============+===============+===============+==*/
struct ECDbUsedClassesListenerWrapper : IUsedClassesListener
    {
    BeSQLite::EC::ECDbCR m_db;
    IECDbUsedClassesListener& m_wrappedListener;
    ECDbUsedClassesListenerWrapper(BeSQLite::EC::ECDbCR db, IECDbUsedClassesListener& wrappedListener)
        : m_db(db), m_wrappedListener(wrappedListener)
        {}
    void _OnClassUsed(ECClassCR ecClass, bool polymorphically) override {m_wrappedListener.NotifyClassUsed(m_db, ecClass, polymorphically);}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderParameters
{
private:
    ECSchemaHelper const& m_schemaHelper;
    PresentationRuleSetCP m_ruleset;
    IUserSettings const& m_userSettings;
    ECExpressionsCache& m_ecexpressionsCache;
    IJsonLocalState const* m_localState;

public:
    QueryBuilderParameters(ECSchemaHelper const& schemaHelper, PresentationRuleSetCR ruleset, IUserSettings const& userSettings, 
        ECExpressionsCache& ecexpressionsCache, IJsonLocalState const* localState = nullptr) 
        : m_schemaHelper(schemaHelper), m_ruleset(&ruleset), m_userSettings(userSettings), m_ecexpressionsCache(ecexpressionsCache), m_localState(localState)
        {}
    void SetRuleset(PresentationRuleSetCR ruleset) {m_ruleset = &ruleset;}
    void SetLocalState(IJsonLocalState const& localState) {m_localState = &localState;}
    ECSchemaHelper const& GetSchemaHelper() const {return m_schemaHelper;}
    PresentationRuleSetCR GetRuleset() const {return *m_ruleset;}
    IUserSettings const& GetUserSettings() const {return m_userSettings;}
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    IJsonLocalState const* GetLocalState() const {return m_localState;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilderParameters : QueryBuilderParameters
{
private:
    IHierarchyCacheCR m_nodesCache;
    IUsedClassesListener* m_usedClassesListener;
    IUsedUserSettingsListener* m_usedSettingsListener;
public:
    NavigationQueryBuilderParameters(ECSchemaHelper const& schemaHelper, PresentationRuleSetCR ruleset, 
        IUserSettings const& userSettings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache, 
        IHierarchyCacheCR nodesCache, IJsonLocalState const* localState = nullptr) 
        : QueryBuilderParameters(schemaHelper, ruleset, userSettings, ecexpressionsCache, localState), m_nodesCache(nodesCache), 
        m_usedClassesListener(nullptr), m_usedSettingsListener(settingsListener)
        {}
    IHierarchyCacheCR GetNodesCache() const {return m_nodesCache;}
    void SetUsedClassesListener(IUsedClassesListener* listener) {m_usedClassesListener = listener;}
    IUsedClassesListener* GetUsedClassesListener() const {return m_usedClassesListener;}
    IUsedUserSettingsListener* GetUsedSettingsListener() const {return m_usedSettingsListener;}
};

/*=================================================================================**//**
* Responsible for creating navigation queries based on presentation rules.
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilder
{
    struct SpecificationsVisitor;
    struct RelatedPathsCache;
    
private:
    NavigationQueryBuilderParameters m_params;
    RelatedPathsCache* m_relatedPathsCache;

private:
    template<typename SpecificationType> Utf8String GetSupportedSchemas(SpecificationType const& specification) const;

    // the real specification handlers
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, AllInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, InstanceNodesOfSpecificClassesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, RelatedInstanceNodesSpecification const& specification, Utf8StringCR specificationHash, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, SearchResultInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;

    // called by SpecificationsVisitor:
    bvector<NavigationQueryPtr> GetQueries(AllInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(AllRelatedInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(RelatedInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(InstanceNodesOfSpecificClassesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(SearchResultInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, AllInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, AllRelatedInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, RelatedInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, InstanceNodesOfSpecificClassesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, SearchResultInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    
public:
    ECPRESENTATION_EXPORT NavigationQueryBuilder(NavigationQueryBuilderParameters params);
    ECPRESENTATION_EXPORT ~NavigationQueryBuilder();
    NavigationQueryBuilderParameters const& GetParameters() const {return m_params;}
    NavigationQueryBuilderParameters& GetParameters() {return m_params;}
    ECPRESENTATION_EXPORT bvector<NavigationQueryPtr> GetQueries(RootNodeRuleCR, ChildNodeSpecificationCR) const;
    ECPRESENTATION_EXPORT bvector<NavigationQueryPtr> GetQueries(ChildNodeRuleCR, ChildNodeSpecificationCR, NavNodeCR) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct ParsedSelectionInfo : IParsedSelectionInfo
{
private:
    bvector<ECClassCP> m_orderedClasses;
    bmap<ECClassCP, bvector<BeSQLite::EC::ECInstanceId>> m_classSelection;
private:
    void GetNodeClasses(NavNodeKeyCR, INavNodeLocater const&, ECSchemaHelper const&);
    void Parse(NavNodeKeyListCR, INavNodeLocater const&, ECSchemaHelper const&);
protected:
    bvector<ECClassCP> const& _GetClasses() const override {return m_orderedClasses;}
    bvector<BeSQLite::EC::ECInstanceId> const& _GetInstanceIds(ECClassCR selectClass) const override;
public:
    ParsedSelectionInfo(NavNodeKeyListCR nodeKeys, INavNodeLocater const& nodesLocater, ECSchemaHelper const& helper);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorBuilder
{
    struct Context : ContentSpecificationsHandler::Context
    {
    private:
        IPropertyCategorySupplierR m_categorySupplier;
        IECPropertyFormatter const* m_propertyFormatter;
        ILocalizationProvider const* m_localizationProvider;
    public:
        Context(ECSchemaHelper const& helper, PresentationRuleSetCR ruleset, Utf8CP preferredDisplayType, 
            IPropertyCategorySupplierR categorySupplier, IECPropertyFormatter const* propertyFormatter, ILocalizationProvider const* localizationProvider) 
            : ContentSpecificationsHandler::Context(helper, ruleset, preferredDisplayType), m_categorySupplier(categorySupplier), 
            m_propertyFormatter(propertyFormatter), m_localizationProvider(localizationProvider)
            {}
        IPropertyCategorySupplierR GetCategorySupplier() const {return m_categorySupplier;}
        IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}
        ILocalizationProvider const* GetLocalizationProvider() const {return m_localizationProvider;}
    };
    struct CreateDescriptorContext;

private:
    Context& m_context;
    
public:
    ContentDescriptorBuilder(Context& context) : m_context(context) {}
    Context& GetContext() {return m_context;}
    Context const& GetContext() const {return m_context;}
    ECPRESENTATION_EXPORT ContentDescriptorPtr CreateDescriptor(SelectedNodeInstancesSpecificationCR, IParsedSelectionInfo const&) const;
    ECPRESENTATION_EXPORT ContentDescriptorPtr CreateDescriptor(ContentRelatedInstancesSpecificationCR, IParsedSelectionInfo const&) const;
    ECPRESENTATION_EXPORT ContentDescriptorPtr CreateDescriptor(ContentInstancesOfSpecificClassesSpecificationCR) const;
    ECPRESENTATION_EXPORT ContentDescriptorPtr CreateDescriptor(ContentDescriptor::NestedContentField const&) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct ContentQueryBuilderParameters : QueryBuilderParameters
{
private:
    INavNodeLocaterCR m_nodesLocater;
    ILocalizationProvider const* m_localizationProvider;
    IPropertyCategorySupplierR m_categorySupplier;
    IECPropertyFormatter const* m_formatter;
public:
    ContentQueryBuilderParameters(ECSchemaHelper const& schemaHelper, INavNodeLocaterCR nodesLocater, PresentationRuleSetCR ruleset, 
        IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache, IPropertyCategorySupplierR categorySupplier, 
        IECPropertyFormatter const* formatter, IJsonLocalState const* localState = nullptr, ILocalizationProvider const* localizationProvider = nullptr)
        : QueryBuilderParameters(schemaHelper, ruleset, userSettings, ecexpressionsCache, localState), m_categorySupplier(categorySupplier), 
        m_formatter(formatter), m_nodesLocater(nodesLocater), m_localizationProvider(localizationProvider)
        {}
    void SetLoacalizationProvider(ILocalizationProvider const* localizationProvider) {m_localizationProvider = localizationProvider;}
    INavNodeLocaterCR GetNodesLocater() const {return m_nodesLocater;}
    ILocalizationProvider const* GetLocalizationProvider() const {return m_localizationProvider;}
    IPropertyCategorySupplierR GetCategorySupplier() const {return m_categorySupplier;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_formatter;}
    };

/*=================================================================================**//**
* Responsible for creating content queries based on presentation rules.
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct ContentQueryBuilder
{    
private:
    ContentQueryBuilderParameters m_params;
    uint64_t m_contractIdsCounter;

public:
    ContentQueryBuilder(ContentQueryBuilderParameters params) : m_params(params), m_contractIdsCounter(0) {}
    ContentQueryBuilder(ContentQueryBuilder const& other) : m_params(other.m_params), m_contractIdsCounter(0) {}

    ContentQueryBuilderParameters const& GetParameters() const {return m_params;}
    ContentQueryBuilderParameters& GetParameters() {return m_params;}

    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(SelectedNodeInstancesSpecificationCR, ContentDescriptorCR, IParsedSelectionInfo const&);
    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ContentRelatedInstancesSpecificationCR, ContentDescriptorCR, IParsedSelectionInfo const&);
    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ContentInstancesOfSpecificClassesSpecificationCR, ContentDescriptorCR);
    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ContentDescriptor::NestedContentField const&);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderHelpers
{
private:
    QueryBuilderHelpers() {}
    static void ProcessQueryClassesBasedOnCustomizationRules(ECClassSet& classes, bmap<ECClassCP, bool> const& customizationClasses);

public:
    template<typename T> static void SetOrUnion(RefCountedPtr<T>& target, T& source);
    template<typename T> static void Where(RefCountedPtr<T>& query, Utf8CP clause, BoundQueryValuesListCR bindings);
    template<typename T> static void Order(T& query, Utf8CP clause);
    template<typename T> static void Limit(T& query, uint64_t limit, uint64_t offset = 0);
    static Utf8String Escape(Utf8String input);

    template<typename T> static RefCountedPtr<ComplexPresentationQuery<T>> CreateNestedQuery(T& innerQuery);
    template<typename T> static bool NeedsNestingToUseAlias(T const& query, bvector<Utf8CP> const& aliases);
    template<typename T> static RefCountedPtr<T> CreateNestedQueryIfNecessary(T& query, bvector<Utf8CP> const& aliases);
    template<typename T> static RefCountedPtr<ComplexPresentationQuery<T>> CreateComplexNestedQueryIfNecessary(T& query, bvector<Utf8CP> const& aliases);
    
    static Utf8String CreateFieldName(ContentDescriptor::ECPropertiesField const&);
    static void ApplyDescriptorOverrides(RefCountedPtr<ContentQuery>& query, ContentDescriptorCR ovr, ECExpressionsCache&);
    static void ApplyPagingOptions(RefCountedPtr<ContentQuery>& query, PageOptionsCR opts);
    static void ApplyDefaultContentFlags(ContentDescriptorR descriptor, Utf8CP displayType, ContentSpecificationCR);
    static void AddCalculatedFields(ContentDescriptorR, CalculatedPropertiesSpecificationList const&, ILocalizationProvider const*, PresentationRuleSetCR, ECClassCP);
    static void Aggregate(ContentDescriptorPtr& aggregateDescriptor, ContentDescriptorR inputDescriptor);
    static ContentQueryPtr CreateMergedResultsQuery(ContentQueryR, ContentDescriptorR);

    static BeSQLite::IdSet<BeSQLite::EC::ECInstanceId> CreateIdSetFromJsonArray(RapidJsonValueCR);
    static ECValue CreateECValueFromJson(RapidJsonValueCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
