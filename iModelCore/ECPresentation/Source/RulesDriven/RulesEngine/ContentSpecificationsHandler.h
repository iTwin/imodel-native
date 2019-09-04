/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct IParsedInput
{
protected:
    virtual bvector<ECClassCP> const& _GetClasses() const = 0;
    virtual bvector<BeSQLite::EC::ECInstanceId> const& _GetInstanceIds(ECClassCR) const = 0;
public:
    virtual ~IParsedInput() {}
    bvector<ECClassCP> const& GetClasses() const {return _GetClasses();}
    bvector<BeSQLite::EC::ECInstanceId> const& GetInstanceIds(ECClassCR selectClass) const {return _GetInstanceIds(selectClass);}
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
        bmap<ECRelationshipClassCP, int> m_relationshipUseCounts;
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
        bmap<ECRelationshipClassCP, int>& GetRelationshipUseCounts() {return m_relationshipUseCounts;}
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
        virtual bool _Supports(ECPropertyCR) = 0;
        virtual bool _Append(ECPropertyCR, Utf8CP) = 0;
    public:
        bool Supports(ECPropertyCR ecProperty) {return _Supports(ecProperty);}
        bool Append(ECPropertyCR ecProperty, Utf8CP propertyClassAlias) {return _Append(ecProperty, propertyClassAlias);}
    };
    typedef RefCountedPtr<PropertyAppender> PropertyAppenderPtr;

    struct AppendRelatedPropertyParams;
    struct AppendRelatedPropertiesParams;

private:    
    Context& m_context;
    mutable bset<ECClassCP> const* m_modifierClasses;

private:
    void AppendClass(ECClassCR, ContentSpecificationCR, bool isSpecificationPolymorphic, IParsedInput const*, Utf8StringCR instanceFilter);
    void AppendClassPaths(bvector<RelatedClassPath> const&, ECClassCR primaryClass, ContentRelatedInstancesSpecificationCR, IParsedInput const&);
    bvector<RelatedClassPath> AppendRelatedProperties(AppendRelatedPropertyParams const&);
    bvector<RelatedClassPath> AppendRelatedProperties(AppendRelatedPropertiesParams const&, bool isNested);
    void AppendRelatedProperties(bvector<RelatedClassPath>&, AppendRelatedPropertiesParams const&);
    bool AppendProperty(PropertyAppender&, bvector<RelatedClass>&, ECPropertyCR, Utf8CP defaultAlias);
    bset<ECClassCP> const& GetModifierClasses() const;

protected:
    virtual PropertyAppenderPtr _CreatePropertyAppender(ECClassCR propertyClass, RelatedClassPath const& pathToSelectClass, RelationshipMeaning, bool expandNestedFields) = 0;
    virtual bool _ShouldIncludeRelatedProperties() const {return true;}
    virtual void _AppendClass(SelectClassInfo const&) = 0;
    ECPRESENTATION_EXPORT virtual void _OnBeforeAppendClassInfos(bvector<SupportedEntityClassInfo>&);
    ECPRESENTATION_EXPORT virtual void _OnBeforeAppendClassPaths(bvector<RelatedClassPath>&);

protected:
    ContentSpecificationsHandler(Context& context) : m_context(context), m_modifierClasses(nullptr) {}
    ContentSpecificationsHandler(ContentSpecificationsHandler const& other) : m_context(other.m_context), m_modifierClasses(nullptr) {}
    virtual ~ContentSpecificationsHandler() {DELETE_AND_CLEAR(m_modifierClasses);}
    Context& GetContext() {return m_context;}
    Context const& GetContext() const {return m_context;}
    ECPRESENTATION_EXPORT void HandleSpecification(SelectedNodeInstancesSpecificationCR, IParsedInput const&);
    ECPRESENTATION_EXPORT void HandleSpecification(ContentRelatedInstancesSpecificationCR, IParsedInput const&);
    ECPRESENTATION_EXPORT void HandleSpecification(ContentInstancesOfSpecificClassesSpecificationCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
