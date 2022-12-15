/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Shared/Queries/QueryBuilder.h"
#include "ContentQueryContracts.h"
#include "ContentSpecificationsHandler.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorBuilder
{
    struct Context : ContentSpecificationsHandler::Context
    {
    private:
        IPropertyCategorySupplierCR m_categorySupplier;
        IECPropertyFormatter const* m_propertyFormatter;
        ECPresentation::UnitSystem m_unitSystem;
        SelectionInfo const* m_selectionInfo;
    public:
        Context(ECSchemaHelper const& helper, IConnectionManagerCR connections, IConnectionCR connection, ICancelationTokenCP cancellationToken,
            IRulesPreprocessorR rulesPreprocessor, PresentationRuleSetCR ruleset, Utf8CP preferredDisplayType,
            RulesetVariables const& rulesetVariables, IPropertyCategorySupplierCR categorySupplier, IECPropertyFormatter const* propertyFormatter, ECPresentation::UnitSystem unitSystem,
            INavNodeKeysContainerCR input, SelectionInfo const* selection)
            : ContentSpecificationsHandler::Context(helper, connections, connection, cancellationToken, rulesPreprocessor, ruleset, rulesetVariables, preferredDisplayType, input),
            m_categorySupplier(categorySupplier), m_propertyFormatter(propertyFormatter), m_unitSystem(unitSystem), m_selectionInfo(selection)
            {}
        IPropertyCategorySupplierCR GetCategorySupplier() const {return m_categorySupplier;}
        IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}
        ECPresentation::UnitSystem GetUnitSystem() const {return m_unitSystem;}
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentQueryBuilderParameters : QueryBuilderParameters
{
private:
    INavNodeLocaterCR m_nodesLocater;
    IPropertyCategorySupplierCR m_categorySupplier;
    IECPropertyFormatter const* m_formatter;
    PageOptions m_pageOptions;
    bool m_skipCompositePropertyFields;
    bool m_skipXToManyRelatedContentFields;
    bool m_disableOmittingFilteredOutQueries;
public:
    ContentQueryBuilderParameters(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, INavNodeLocaterCR nodesLocater, IConnectionCR connection, ICancelationTokenCP cancellationToken,
        IRulesPreprocessorR rulesPreprocessor, PresentationRuleSetCR ruleset, RulesetVariables const& rulesetVariables, ECExpressionsCache& ecexpressionsCache, IUsedRulesetVariablesListener* variablesListener,
        IPropertyCategorySupplierCR categorySupplier, bool skipCompositePropertyFields, bool skipXToManyRelatedContentFields, IECPropertyFormatter const* formatter,
        IJsonLocalState const* localState = nullptr)
        : QueryBuilderParameters(schemaHelper, connections, connection, cancellationToken, rulesPreprocessor, ruleset, rulesetVariables, ecexpressionsCache, variablesListener, localState),
        m_categorySupplier(categorySupplier), m_formatter(formatter), m_nodesLocater(nodesLocater),
        m_skipCompositePropertyFields(skipCompositePropertyFields), m_skipXToManyRelatedContentFields(skipXToManyRelatedContentFields), m_disableOmittingFilteredOutQueries(false)
        {}
    INavNodeLocaterCR GetNodesLocater() const {return m_nodesLocater;}
    IPropertyCategorySupplierCR GetCategorySupplier() const {return m_categorySupplier;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_formatter;}
    void SetPageOptions(PageOptions value) {m_pageOptions = value;}
    PageOptionsCR GetPageOptions() const {return m_pageOptions;}
    bool ShouldSkipCompositePropertyFields() const {return m_skipCompositePropertyFields;}
    bool ShouldSkipXToManyRelatedContentFields() const {return m_skipXToManyRelatedContentFields;}
    bool ShouldOmitFilteredOutQueries() const {return !m_disableOmittingFilteredOutQueries;}
    void SetDisableOmittingFilteredOutQueries(bool value) {m_disableOmittingFilteredOutQueries = value;}
    };

/*=================================================================================**//**
* Responsible for creating content queries based on presentation rules.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentQueryBuilder
{
private:
    ContentQueryBuilderParameters m_params;
    uint64_t m_contractIdsCounter;

private:
    ContentQueryContractPtr CreateContract(ContentDescriptorCR, SelectClassInfo const&, IQueryInfoProvider const&);

public:
    ContentQueryBuilder(ContentQueryBuilderParameters params) : m_params(params), m_contractIdsCounter(0) {}
    ContentQueryBuilder(ContentQueryBuilder const& other) : m_params(other.m_params), m_contractIdsCounter(0) {}

    ContentQueryBuilderParameters const& GetParameters() const {return m_params;}
    ContentQueryBuilderParameters& GetParameters() {return m_params;}

    ECPRESENTATION_EXPORT QuerySet CreateQuerySet(SelectedNodeInstancesSpecificationCR, ContentDescriptorCR, IParsedInput const&);
    ECPRESENTATION_EXPORT QuerySet CreateQuerySet(ContentRelatedInstancesSpecificationCR, ContentDescriptorCR, IParsedInput const&);
    ECPRESENTATION_EXPORT QuerySet CreateQuerySet(ContentInstancesOfSpecificClassesSpecificationCR, ContentDescriptorCR);
    ECPRESENTATION_EXPORT QuerySet CreateQuerySet(ContentDescriptor::NestedContentField const&);
};

/*=================================================================================**//**
* Responsible for creating a single unioned content query based on given presentation rules,
* also taking into account MAX_COMPOUND_STATEMENTS_COUNT
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiContentQueryBuilder
{
private:
    std::unique_ptr<ContentQueryBuilder> m_builder;
    ContentDescriptorCPtr m_descriptor;
    PageOptions m_pageOptions;
    bool m_adjustmentsApplied;
    QuerySet m_unions;

private:
    static ContentQueryBuilderParameters ClearPageOptions(ContentQueryBuilderParameters params)
        {
        params.SetPageOptions(PageOptions());
        return params;
        }

public:
    MultiContentQueryBuilder(ContentQueryBuilderParameters params, ContentDescriptorCR descriptor)
        : m_builder(std::make_unique<ContentQueryBuilder>(ClearPageOptions(params))), m_descriptor(&descriptor), m_pageOptions(params.GetPageOptions()),
        m_adjustmentsApplied(false)
        {}
    ECPRESENTATION_EXPORT bool Accept(SelectedNodeInstancesSpecificationCR, IParsedInput const&);
    ECPRESENTATION_EXPORT bool Accept(ContentRelatedInstancesSpecificationCR, IParsedInput const&);
    ECPRESENTATION_EXPORT bool Accept(ContentInstancesOfSpecificClassesSpecificationCR);
    ECPRESENTATION_EXPORT QuerySet const& GetQuerySet();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
