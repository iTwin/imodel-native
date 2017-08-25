/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryBuilder.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentationRules/PresentationRules.h>
#include <Logging/bentleylogging.h>
#include "JsonNavNode.h"
#include "NavNodesCache.h"
#include "NavigationQuery.h"
#include "ECSchemaHelper.h"
#include "QueryContracts.h"

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
    virtual void _OnClassUsed(ECN::ECClassCR, bool polymorphically) = 0;
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
struct UsedClassesHelper
{
private:
    UsedClassesHelper() {}
public:
    static void NotifyListenerWithUsedClasses(IUsedClassesListener&, BeSQLite::EC::ECDbCR, ECExpressionsCache&, Utf8StringCR ecexpression);
    static void NotifyListenerWithRulesetClasses(IUsedClassesListener&, BeSQLite::EC::ECDbCR, ECExpressionsCache&, ECN::PresentationRuleSetCR);
    static void NotifyListenerWithUsedClasses(IECDbUsedClassesListener&, BeSQLite::EC::ECDbCR, ECExpressionsCache&, Utf8StringCR ecexpression);
    static void NotifyListenerWithRulesetClasses(IECDbUsedClassesListener&, BeSQLite::EC::ECDbCR, ECExpressionsCache&, ECN::PresentationRuleSetCR);
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
    void _OnClassUsed(ECN::ECClassCR ecClass, bool polymorphically) override {m_wrappedListener.NotifyClassUsed(m_db, ecClass, polymorphically);}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderParameters
{
private:
    ECSchemaHelper const& m_schemaHelper;
    ECN::PresentationRuleSetCP m_ruleset;
    IUserSettings const& m_userSettings;
    ECExpressionsCache& m_ecexpressionsCache;
    IJsonLocalState const* m_localState;

public:
    QueryBuilderParameters(ECSchemaHelper const& schemaHelper, ECN::PresentationRuleSetCR ruleset, IUserSettings const& userSettings, 
        ECExpressionsCache& ecexpressionsCache, IJsonLocalState const* localState = nullptr) 
        : m_schemaHelper(schemaHelper), m_ruleset(&ruleset), m_userSettings(userSettings), m_ecexpressionsCache(ecexpressionsCache), m_localState(localState)
        {}
    void SetRuleset(ECN::PresentationRuleSetCR ruleset) {m_ruleset = &ruleset;}
    void SetLocalState(IJsonLocalState const& localState) {m_localState = &localState;}
    ECSchemaHelper const& GetSchemaHelper() const {return m_schemaHelper;}
    ECN::PresentationRuleSetCR GetRuleset() const {return *m_ruleset;}
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
    NavigationQueryBuilderParameters(ECSchemaHelper const& schemaHelper, ECN::PresentationRuleSetCR ruleset, 
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
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, ECN::AllInstanceNodesSpecification const& specification, ECN::ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, ECN::InstanceNodesOfSpecificClassesSpecification const& specification, ECN::ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, ECN::RelatedInstanceNodesSpecification const& specification, int specificationId, ECN::ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCP parentNode, ECN::SearchResultInstanceNodesSpecification const& specification, ECN::ChildNodeRuleCR rule) const;

    // called by SpecificationsVisitor:
    bvector<NavigationQueryPtr> GetQueries(ECN::AllInstanceNodesSpecification const& specification, ECN::RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(ECN::AllRelatedInstanceNodesSpecification const& specification, ECN::RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(ECN::RelatedInstanceNodesSpecification const& specification, ECN::RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(ECN::InstanceNodesOfSpecificClassesSpecification const& specification, ECN::RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(ECN::SearchResultInstanceNodesSpecification const& specification, ECN::RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, ECN::AllInstanceNodesSpecification const& specification, ECN::ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, ECN::AllRelatedInstanceNodesSpecification const& specification, ECN::ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, ECN::RelatedInstanceNodesSpecification const& specification, ECN::ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, ECN::InstanceNodesOfSpecificClassesSpecification const& specification, ECN::ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(NavNodeCR parentNode, ECN::SearchResultInstanceNodesSpecification const& specification, ECN::ChildNodeRuleCR rule) const;
    
public:
    ECPRESENTATION_EXPORT NavigationQueryBuilder(NavigationQueryBuilderParameters params);
    ECPRESENTATION_EXPORT ~NavigationQueryBuilder();
    NavigationQueryBuilderParameters const& GetParameters() const {return m_params;}
    NavigationQueryBuilderParameters& GetParameters() {return m_params;}
    ECPRESENTATION_EXPORT bvector<NavigationQueryPtr> GetQueries(ECN::RootNodeRuleCR, ECN::ChildNodeSpecificationCR) const;
    ECPRESENTATION_EXPORT bvector<NavigationQueryPtr> GetQueries(ECN::ChildNodeRuleCR, ECN::ChildNodeSpecificationCR, NavNodeCR) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct ContentQueryBuilderParameters : QueryBuilderParameters
{
private:
    Utf8CP m_preferredDisplayType;
    INavNodeLocaterCR m_nodesLocater;
    ILocalizationProvider const* m_localizationProvider;
    IPropertyCategorySupplierR m_categorySupplier;
    IECPropertyFormatter const* m_formatter;
public:
    ContentQueryBuilderParameters(ECSchemaHelper const& schemaHelper, INavNodeLocaterCR nodesLocater, ECN::PresentationRuleSetCR ruleset, 
        Utf8CP preferredDisplayType, IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache,
        IPropertyCategorySupplierR categorySupplier, IECPropertyFormatter const* formatter, IJsonLocalState const* localState = nullptr, ILocalizationProvider const* localizationProvider = nullptr)
        : QueryBuilderParameters(schemaHelper, ruleset, userSettings, ecexpressionsCache, localState), m_preferredDisplayType(preferredDisplayType), 
        m_categorySupplier(categorySupplier), m_formatter(formatter), m_nodesLocater(nodesLocater), m_localizationProvider(localizationProvider)
        {}
    void SetPreferredDisplayType(Utf8CP value) {m_preferredDisplayType = value;}
    void SetLoacalizationProvider(ILocalizationProvider const* localizationProvider) {m_localizationProvider = localizationProvider;}
    Utf8CP GetPreferredDisplayType() const {return m_preferredDisplayType;}
    INavNodeLocaterCR GetNodesLocater() const {return m_nodesLocater;}
    ILocalizationProvider const* GetLocalizationProvider() const {return m_localizationProvider;}
    IPropertyCategorySupplierR GetCategorySupplier() const {return m_categorySupplier;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_formatter;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct IParsedSelectionInfo
{
protected:
    virtual bvector<ECN::ECClassCP> const& _GetClasses() const = 0;
    virtual bvector<BeSQLite::EC::ECInstanceId> const& _GetInstanceIds(ECN::ECClassCR) const = 0;
public:
    virtual ~IParsedSelectionInfo() {}
    bvector<ECN::ECClassCP> const& GetClasses() const {return _GetClasses();}
    bvector<BeSQLite::EC::ECInstanceId> const& GetInstanceIds(ECN::ECClassCR selectClass) const {return _GetInstanceIds(selectClass);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct ParsedSelectionInfo : IParsedSelectionInfo
{
private:
    bvector<ECN::ECClassCP> m_orderedClasses;
    bmap<ECN::ECClassCP, bvector<BeSQLite::EC::ECInstanceId>> m_classSelection;
private:
    void GetNodeClasses(NavNodeKeyCR, INavNodeLocater const&, ECSchemaHelper const&);
    void Parse(NavNodeKeyListCR, INavNodeLocater const&, ECSchemaHelper const&);
protected:
    bvector<ECN::ECClassCP> const& _GetClasses() const override {return m_orderedClasses;}
    bvector<BeSQLite::EC::ECInstanceId> const& _GetInstanceIds(ECN::ECClassCR selectClass) const override;
public:
    ParsedSelectionInfo(NavNodeKeyListCR nodeKeys, INavNodeLocater const& nodesLocater, ECSchemaHelper const& helper);
};

/*=================================================================================**//**
* Responsible for creating content queries based on presentation rules.
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct ContentQueryBuilder
{
    struct SpecificationsVisitor;
    
private:
    ContentQueryBuilderParameters m_params;

public:
    ContentQueryBuilder(ContentQueryBuilderParameters params) : m_params(params) {}
    ContentQueryBuilder(ContentQueryBuilder const& other) : m_params(other.m_params) {}

    ContentQueryBuilderParameters const& GetParameters() const {return m_params;}
    ContentQueryBuilderParameters& GetParameters() {return m_params;}

    ECPRESENTATION_EXPORT ContentDescriptorPtr CreateDescriptor(ECN::SelectedNodeInstancesSpecificationCR, IParsedSelectionInfo const&);
    ECPRESENTATION_EXPORT ContentDescriptorPtr CreateDescriptor(ECN::ContentRelatedInstancesSpecificationCR, IParsedSelectionInfo const&);
    ECPRESENTATION_EXPORT ContentDescriptorPtr CreateDescriptor(ECN::ContentInstancesOfSpecificClassesSpecificationCR);

    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ECN::SelectedNodeInstancesSpecificationCR, ContentDescriptorCR, IParsedSelectionInfo const&);
    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ECN::ContentRelatedInstancesSpecificationCR, ContentDescriptorCR, IParsedSelectionInfo const&);
    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ECN::ContentInstancesOfSpecificClassesSpecificationCR, ContentDescriptorCR);
    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ContentDescriptor::NestedContentField const&);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderHelpers
{
private:
    QueryBuilderHelpers() {}
    static void ProcessQueryClassesBasedOnCustomizationRules(ECClassSet& classes, bmap<ECN::ECClassCP, bool> const& customizationClasses);

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
    static void ApplyDefaultContentFlags(ContentDescriptorR descriptor, Utf8CP displayType, ECN::ContentSpecificationCR);
    static void AddCalculatedFields(ContentDescriptorR, ECN::CalculatedPropertiesSpecificationList const&, ILocalizationProvider const*, ECN::PresentationRuleSetCR, ECN::ECClassCP);
    static void Aggregate(ContentDescriptorPtr& aggregateDescriptor, ContentDescriptorR inputDescriptor);
    static ContentQueryPtr CreateMergedResultsQuery(ContentQueryR, ContentDescriptorR);

    static BeSQLite::IdSet<BeSQLite::EC::ECInstanceId> CreateIdSetFromJsonArray(RapidJsonValueCR);
    static ECN::ECValue CreateECValueFromJson(RapidJsonValueCR);

    static void Reverse(RelatedClassPath&, Utf8CP firstTargetClassAlias, bool isFirstTargetPolymorphic);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
