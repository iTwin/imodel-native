/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ContentItemBuilder.h"
#include "../Shared/Queries/QueryExecutor.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* Responsible for creating a content items from a ECSql statement.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentReader : IQueryResultReader<ContentSetItemPtr>
{
    enum class Mode
        {
        Skip, //!< Don't initialize properties for the item
        Read, //!< Full read the item
        };

private:
    SchemaManagerCR m_schemaManager;
    IContractProvider<ContentQueryContract> const& m_contracts;
    IECPropertyFormatter const* m_propertyFormatter;
    ECPresentation::UnitSystem m_unitSystem;
    Mode m_mode;
    std::shared_ptr<ContentItemBuilder::Context> m_builderContext;
    std::unique_ptr<ContentItemBuilder> m_inProgress;
    bmap<ECClassInstanceKey, ContentSetItemPtr> m_createdItems;

protected:
    ECPRESENTATION_EXPORT QueryResultReaderStatus _ReadRecord(ContentSetItemPtr&, ECSqlStatementCR) override;
    ECPRESENTATION_EXPORT bool _Complete(ContentSetItemPtr&, ECSqlStatementCR) override;
    ContentItemBuilder& GetItemInProgress();

public:
    ContentReader(SchemaManagerCR schemaManager, IContractProvider<ContentQueryContract> const& contracts)
        : m_schemaManager(schemaManager), m_contracts(contracts), m_propertyFormatter(nullptr), m_unitSystem(ECPresentation::UnitSystem::Undefined),
        m_mode(Mode::Read)
        {
        m_builderContext = std::make_shared<ContentItemBuilder::Context>();
        }
    void SetPropertyFormatter(IECPropertyFormatter const& formatter) {m_propertyFormatter = &formatter;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}
    void SetUnitSystem(ECPresentation::UnitSystem unitSystem) {m_unitSystem = unitSystem;}
    ECPresentation::UnitSystem GetUnitSystem() const {return m_unitSystem;}
    void SetMode(Mode mode) {m_mode = mode;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DistinctValuesAccumulator final : IQueryResultAccumulator
{
private:
    SchemaManagerCR m_schemaManager;
    ContentDescriptor::Field const& m_field;
    IECPropertyFormatter const* m_propertyFormatter;
    ECPresentation::UnitSystem m_unitSystem;
    bmap<Utf8String, DisplayValueGroupPtr> m_values;

private:
    DisplayValueGroupR GetOrCreateDisplayValueGroup(Utf8StringCR displayValue);
    void ReadNavigationPropertyRecord(ContentDescriptor::ECPropertiesField const&, ECSqlStatementCR);
    void ReadPrimitivePropertyRecord(ContentDescriptor::ECPropertiesField const&, ECSqlStatementCR);

public:
    DistinctValuesAccumulator(ContentDescriptor::Field const& field, SchemaManagerCR schemaManager)
        : m_schemaManager(schemaManager), m_field(field), m_propertyFormatter(nullptr), m_unitSystem(ECPresentation::UnitSystem::Undefined)
        {}

    void SetPropertyFormatter(IECPropertyFormatter const& formatter) {m_propertyFormatter = &formatter;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}

    void SetUnitSystem(ECPresentation::UnitSystem unitSystem) {m_unitSystem = unitSystem;}
    ECPresentation::UnitSystem GetUnitSystem() const {return m_unitSystem;}

    QueryResultAccumulatorStatus _ReadRow(ECSqlStatementCR statement) override;

    bmap<Utf8String, DisplayValueGroupPtr> GetValues() const {return m_values;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
