/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
#include "UsedClassesListener.h"
#include "QueryBuilderHelpers.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define SEARCH_QUERY_Alias              "/search/"
#define SEARCH_QUERY_FIELD_ECInstanceId "/ECInstanceId/"
#define SEARCH_QUERY_FIELD_ECClassId    "/ECClassId/"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderParameters
{
private:
    ECSchemaHelper const& m_schemaHelper;
    IConnectionManagerCR m_connections;
    IConnectionCR m_connection;
    PresentationRuleSetCP m_ruleset;
    Utf8String m_locale;
    IUserSettings const& m_userSettings;
    ECExpressionsCache& m_ecexpressionsCache;
    IJsonLocalState const* m_localState;

public:
    QueryBuilderParameters(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset, 
        Utf8String locale, IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache, IJsonLocalState const* localState = nullptr) 
        : m_schemaHelper(schemaHelper), m_connections(connections), m_connection(connection), m_ruleset(&ruleset), m_locale(locale),
        m_userSettings(userSettings), m_ecexpressionsCache(ecexpressionsCache), m_localState(localState)
        {}
    void SetRuleset(PresentationRuleSetCR ruleset) {m_ruleset = &ruleset;}
    void SetLocalState(IJsonLocalState const& localState) {m_localState = &localState;}
    ECSchemaHelper const& GetSchemaHelper() const {return m_schemaHelper;}
    IConnectionManagerCR GetConnections() const {return m_connections;}
    IConnectionCR GetConnection() const {return m_connection;}
    PresentationRuleSetCR GetRuleset() const {return *m_ruleset;}
    Utf8StringCR GetLocale() const {return m_locale;}
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
    NavigationQueryBuilderParameters(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset, 
        Utf8String locale, IUserSettings const& userSettings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache, 
        IHierarchyCacheCR nodesCache, IJsonLocalState const* localState = nullptr) 
        : QueryBuilderParameters(schemaHelper, connections, connection, ruleset, locale, userSettings, ecexpressionsCache, localState), m_nodesCache(nodesCache), 
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
    bvector<NavigationQueryPtr> GetQueries(JsonNavNodeCP parentNode, AllInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(JsonNavNodeCP parentNode, InstanceNodesOfSpecificClassesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(JsonNavNodeCP parentNode, RelatedInstanceNodesSpecification const& specification, Utf8StringCR specificationHash, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(JsonNavNodeCP parentNode, SearchResultInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;

    // called by SpecificationsVisitor:
    bvector<NavigationQueryPtr> GetQueries(AllInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(AllRelatedInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(RelatedInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(InstanceNodesOfSpecificClassesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(SearchResultInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(JsonNavNodeCR parentNode, AllInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(JsonNavNodeCR parentNode, AllRelatedInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(JsonNavNodeCR parentNode, RelatedInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(JsonNavNodeCR parentNode, InstanceNodesOfSpecificClassesSpecification const& specification, ChildNodeRuleCR rule) const;
    bvector<NavigationQueryPtr> GetQueries(JsonNavNodeCR parentNode, SearchResultInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const;
    
public:
    ECPRESENTATION_EXPORT NavigationQueryBuilder(NavigationQueryBuilderParameters params);
    ECPRESENTATION_EXPORT ~NavigationQueryBuilder();
    NavigationQueryBuilderParameters const& GetParameters() const {return m_params;}
    NavigationQueryBuilderParameters& GetParameters() {return m_params;}
    ECPRESENTATION_EXPORT bvector<NavigationQueryPtr> GetQueries(RootNodeRuleCR, ChildNodeSpecificationCR) const;
    ECPRESENTATION_EXPORT bvector<NavigationQueryPtr> GetQueries(ChildNodeRuleCR, ChildNodeSpecificationCR, JsonNavNodeCR) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct ParsedInput : IParsedInput
{
private:
    bvector<ECClassCP> m_orderedClasses;
    bmap<ECClassCP, bvector<ECInstanceId>> m_classInput;
private:
    void GetNodeClasses(ECInstanceKeyCR, INavNodeLocater const&, IConnectionCR, ECSchemaHelper const&);
    void Parse(bvector<ECInstanceKey> const&, INavNodeLocater const&, IConnectionCR, ECSchemaHelper const&);
protected:
    bvector<ECClassCP> const& _GetClasses() const override {return m_orderedClasses;}
    bvector<ECInstanceId> const& _GetInstanceIds(ECClassCR selectClass) const override;
public:
    ParsedInput(bvector<ECInstanceKey> const&, INavNodeLocater const&, IConnectionCR, ECSchemaHelper const&);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorBuilder
{
    struct Context : ContentSpecificationsHandler::Context
    {
    private:
        int m_contentFlags;
        IPropertyCategorySupplierCR m_categorySupplier;
        IECPropertyFormatter const* m_propertyFormatter;
        ILocalizationProvider const* m_localizationProvider;
        INavNodeKeysContainerCPtr m_inputKeys;
        SelectionInfo const* m_selectionInfo;
    public:
        Context(ECSchemaHelper const& helper, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset, Utf8CP preferredDisplayType, int contentFlags,
            IPropertyCategorySupplierCR categorySupplier, IECPropertyFormatter const* propertyFormatter, ILocalizationProvider const* localizationProvider, Utf8String locale,
            INavNodeKeysContainerCR input, SelectionInfo const* selection) 
            : ContentSpecificationsHandler::Context(helper, connections, connection, ruleset, locale, preferredDisplayType), m_contentFlags(contentFlags), m_categorySupplier(categorySupplier),
            m_propertyFormatter(propertyFormatter), m_localizationProvider(localizationProvider), m_inputKeys(&input), m_selectionInfo(selection)
            {}
        int GetContentFlags() const {return m_contentFlags;}
        IPropertyCategorySupplierCR GetCategorySupplier() const {return m_categorySupplier;}
        IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}
        ILocalizationProvider const* GetLocalizationProvider() const {return m_localizationProvider;}
        INavNodeKeysContainerCR GetInputKeys() const {return *m_inputKeys;}
        SelectionInfo const* GetSelectionInfo() const {return m_selectionInfo;}
    };
    struct CreateDescriptorContext;

private:
    Context& m_context;
    
public:
    ContentDescriptorBuilder(Context& context) : m_context(context) {}
    Context& GetContext() {return m_context;}
    Context const& GetContext() const {return m_context;}
    ECPRESENTATION_EXPORT ContentDescriptorPtr CreateDescriptor(SelectedNodeInstancesSpecificationCR, IParsedInput const&) const;
    ECPRESENTATION_EXPORT ContentDescriptorPtr CreateDescriptor(ContentRelatedInstancesSpecificationCR, IParsedInput const&) const;
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
    IPropertyCategorySupplierCR m_categorySupplier;
    IECPropertyFormatter const* m_formatter;
public:
    ContentQueryBuilderParameters(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, INavNodeLocaterCR nodesLocater, IConnectionCR connection, 
        PresentationRuleSetCR ruleset, Utf8String locale, IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache, IPropertyCategorySupplierCR categorySupplier,
        IECPropertyFormatter const* formatter, IJsonLocalState const* localState = nullptr, ILocalizationProvider const* localizationProvider = nullptr)
        : QueryBuilderParameters(schemaHelper, connections, connection, ruleset, locale, userSettings, ecexpressionsCache, localState), 
        m_categorySupplier(categorySupplier), m_formatter(formatter), m_nodesLocater(nodesLocater), m_localizationProvider(localizationProvider)
        {}
    void SetLoacalizationProvider(ILocalizationProvider const* localizationProvider) {m_localizationProvider = localizationProvider;}
    INavNodeLocaterCR GetNodesLocater() const {return m_nodesLocater;}
    ILocalizationProvider const* GetLocalizationProvider() const {return m_localizationProvider;}
    IPropertyCategorySupplierCR GetCategorySupplier() const {return m_categorySupplier;}
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

    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(SelectedNodeInstancesSpecificationCR, ContentDescriptorCR, IParsedInput const&);
    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ContentRelatedInstancesSpecificationCR, ContentDescriptorCR, IParsedInput const&);
    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ContentInstancesOfSpecificClassesSpecificationCR, ContentDescriptorCR);
    ECPRESENTATION_EXPORT ContentQueryPtr CreateQuery(ContentDescriptor::NestedContentField const&);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
