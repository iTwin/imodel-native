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
#include "QueryBuilderHelpers.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct ContentSource
{
private:
    SelectClassWithExcludes m_selectClass;
    ECClassCP m_propertiesSourceOverride;
    RelatedClassPath m_pathFromInputToSelectClass;
public:
    ContentSource(SelectClassWithExcludes selectClass) : m_selectClass(selectClass), m_propertiesSourceOverride(nullptr) {}
    ContentSource(SelectClassWithExcludes selectClass, ECClassCP propertiesSourceOverride) : m_selectClass(selectClass), m_propertiesSourceOverride(propertiesSourceOverride) {}

    SelectClassWithExcludes const& GetSelectClass() const {return m_selectClass;}
    SelectClassWithExcludes& GetSelectClass() {return m_selectClass;}

    ECClassCR GetPropertiesSource() const {return m_propertiesSourceOverride ? *m_propertiesSourceOverride : m_selectClass.GetClass();}
    void SetPropertiesSourceOverride(ECClassCP ovr) {m_propertiesSourceOverride = ovr;}

    RelatedClassPath const& GetPathFromInputToSelectClass() const {return m_pathFromInputToSelectClass;}
    RelatedClassPath& GetPathFromInputToSelectClass() {return m_pathFromInputToSelectClass;}
    void SetPathFromInputToSelectClass(RelatedClassPath path) {m_pathFromInputToSelectClass = path;}
};

/*=================================================================================**//**
* Base abstract class for content specification handlers.
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsHandler
{
    /*=================================================================================**//**
    * @bsiclass                                     Grigas.Petraitis                10/2017
    +===============+===============+===============+===============+===============+======*/
    struct Context
    {
    private:
        IConnectionManagerCR m_connections;
        IConnectionCR m_connection;
        PresentationRuleSetCR m_ruleset;
        Utf8String m_locale;
        Utf8CP m_preferredDisplayType;
        ECSchemaHelper const& m_helper;
        bmap<ECClassCP, size_t> m_classCounter;
        bset<ECClassCP> m_handledClasses;
        bmap<ECSchemaCP, bmap<ECClassCP, RelatedPropertiesSpecificationList>> m_relatedPropertySpecifications;
        ECClassUseCounter m_relationshipUseCounts;
        bmap<ECClassCP, bvector<RelatedClass>> m_handledNavigationPropertiesPaths;

    public:
        Context(ECSchemaHelper const& helper, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset, Utf8String locale, Utf8CP preferredDisplayType)
            : m_helper(helper), m_connections(connections), m_connection(connection), m_ruleset(ruleset), m_locale(locale), m_preferredDisplayType(preferredDisplayType)
            {}
        IConnectionManagerCR GetConnections() const {return m_connections;}
        IConnectionCR GetConnection() const {return m_connection;}
        PresentationRuleSetCR GetRuleset() const {return m_ruleset;}
        Utf8StringCR GetLocale() const {return m_locale;}
        Utf8CP GetPreferredDisplayType() const {return m_preferredDisplayType;}
        void SetPreferredDisplayType(Utf8CP value) {m_preferredDisplayType = value;}
        ECSchemaHelper const& GetSchemaHelper() const {return m_helper;}
        size_t GetClassCount(ECClassCR ecClass) {return m_classCounter[&ecClass]++;}
        ECClassUseCounter& GetRelationshipUseCounts() {return m_relationshipUseCounts;}
        bool IsClassHandled(ECClassCR ecClass) const {return m_handledClasses.end() != m_handledClasses.find(&ecClass);}
        void SetClassHandled(ECClassCR ecClass) {m_handledClasses.insert(&ecClass);}
        void AddNavigationPropertiesPaths(ECClassCR ecClass, bvector<RelatedClass> navigationPropertiesPaths) {m_handledNavigationPropertiesPaths[&ecClass] = navigationPropertiesPaths;}
        bvector<RelatedClass> GetNavigationPropertiesPaths(ECClassCR ecClass) {return m_handledNavigationPropertiesPaths[&ecClass];}
    };

    /*=================================================================================**//**
    * @bsiclass                                     Grigas.Petraitis                10/2017
    +===============+===============+===============+===============+===============+======*/
    struct PropertyAppender : RefCountedBase
    {
    protected:
        virtual bool _Supports(ECPropertyCR, PropertySpecificationCP) = 0;
        virtual bool _Append(ECPropertyCR, Utf8CP, PropertySpecificationCP) = 0;
    public:
        bool Supports(ECPropertyCR ecProperty, PropertySpecificationCP overrides) {return _Supports(ecProperty, overrides);}
        bool Append(ECPropertyCR ecProperty, Utf8CP propertyClassAlias, PropertySpecificationCP overrides) {return _Append(ecProperty, propertyClassAlias, overrides);}
    };
    typedef RefCountedPtr<PropertyAppender> PropertyAppenderPtr;

    struct AppendRelatedPropertyParams;
    struct AppendRelatedPropertiesParams;

private:
    Context& m_context;
    mutable bvector<RuleApplicationInfo> const* m_customizationRuleInfos;

private:
    void AppendContent(ContentSource const&, ContentSpecificationCR, IParsedInput const*, Utf8StringCR instanceFilter, InstanceFilteringParams::RecursiveQueryInfo const*);
    bvector<RelatedClassPath> CreateRelatedPropertyPaths(RelatedClassPathCR pathFromSelectToSourceClass, ECClassCR sourceClass, InstanceFilteringParams const&, RelatedPropertiesSpecificationCR);
    bvector<RelatedClassPath> AppendRelatedProperties(AppendRelatedPropertyParams const&);
    bvector<RelatedClassPath> AppendRelatedProperties(AppendRelatedPropertiesParams const&, bool isNested);
    void AppendRelatedProperties(bvector<RelatedClassPath>&, AppendRelatedPropertiesParams const&);
    bool AppendProperty(PropertyAppender&, bvector<RelatedClass>&, ECPropertyCR, Utf8CP defaultAlias, PropertySpecificationCP overrides);
    bvector<RuleApplicationInfo> const& GetCustomizationRuleInfos() const;

protected:
    virtual PropertyAppenderPtr _CreatePropertyAppender(ECClassCR propertyClass, RelatedClassPath const& pathToSelectClass, RelationshipMeaning,
        bool expandNestedFields, PropertyCategorySpecificationsList const*) = 0;
    virtual bool _ShouldIncludeRelatedProperties() const {return true;}
    virtual void _AppendClass(SelectClassInfo const&) = 0;
    ECPRESENTATION_EXPORT virtual bvector<ContentSource> _BuildContentSource(bvector<SelectClass> const&);
    ECPRESENTATION_EXPORT virtual bvector<ContentSource> _BuildContentSource(bvector<RelatedClassPath> const&);
    ContentSource CreateContentSource(RelatedClassPath const& path) const;

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

END_BENTLEY_ECPRESENTATION_NAMESPACE
