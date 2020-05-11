/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "QueryExecutor.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* Responsible for creating a nodes from a ECSql statement.
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct ContentReader : IQueryResultReader<ContentSetItemPtr>
{
private:
    ContentQueryCR m_query;
    IECPropertyFormatter const* m_propertyFormatter;
    ECPresentation::UnitSystem m_unitSystem;
    bset<ECInstanceKey> m_readKeys;

protected:
    ECPRESENTATION_EXPORT QueryResultReaderStatus _ReadRecord(ContentSetItemPtr&, ECSqlStatementCR) override;

public:
    ContentReader(ContentQueryCR query)
        : m_query(query), m_propertyFormatter(nullptr), m_unitSystem(ECPresentation::UnitSystem::Undefined)
        {}
    void SetPropertyFormatter(IECPropertyFormatter const& formatter) {m_propertyFormatter = &formatter;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}
    void SetUnitSystem(ECPresentation::UnitSystem unitSystem) {m_unitSystem = unitSystem;}
    ECPresentation::UnitSystem GetUnitSystem() const {return m_unitSystem;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
struct DistinctValuesReader : IQueryResultReader<bpair<Utf8String, ECValue>>
{
private:
    ContentDescriptor::Field const& m_field;
    IECPropertyFormatter const* m_propertyFormatter;
    ECPresentation::UnitSystem m_unitSystem;
    std::deque<bpair<Utf8String, ECValue>> m_currRecords;

private:
    QueryResultReaderStatus PopRecord(bpair<Utf8String, ECValue>&);
    void ReadNavigationPropertyRecord(ContentDescriptor::ECPropertiesField const&, ECSqlStatementCR);
    void ReadPrimitivePropertyRecord(ContentDescriptor::ECPropertiesField const&, ECSqlStatementCR);

protected:
    ECPRESENTATION_EXPORT QueryResultReaderStatus _ReadRecord(bpair<Utf8String, ECValue>&, ECSqlStatementCR) override;

public:
    DistinctValuesReader(ContentDescriptor::Field const& field)
        : m_field(field), m_propertyFormatter(nullptr), m_unitSystem(ECPresentation::UnitSystem::Undefined)
        {}
    void SetPropertyFormatter(IECPropertyFormatter const& formatter) {m_propertyFormatter = &formatter;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}
    void SetUnitSystem(ECPresentation::UnitSystem unitSystem) {m_unitSystem = unitSystem;}
    ECPresentation::UnitSystem GetUnitSystem() const {return m_unitSystem;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
