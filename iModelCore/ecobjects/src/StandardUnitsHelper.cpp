/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandardUnitsHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"


BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define UNITS_SCHEMA_NAME "Units"

//*********************** StandardUnitsSchemaHolder *************************************

//--------------------------------------------------------------------------------------
// @bsiclass
// Helper class to hold the schema
//                                              Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
struct StandardUnitsSchemaHolder;
typedef RefCountedPtr<StandardUnitsSchemaHolder> StandardUnitsSchemaHolderPtr;

struct StandardUnitsSchemaHolder : RefCountedBase
    {
    private:
        ECSchemaPtr                         m_schema;
        static StandardUnitsSchemaHolderPtr s_schemaHolder;

        StandardUnitsSchemaHolder();
        ECSchemaPtr _GetSchema();

    public:

        static StandardUnitsSchemaHolderPtr GetHolder();
        static ECSchemaPtr GetSchema();
    };

StandardUnitsSchemaHolderPtr StandardUnitsSchemaHolder::s_schemaHolder;

static const uint32_t s_unitsVersionRead = 1;
static const uint32_t s_unitsVersionWrite = 0;
static const uint32_t s_unitsVersionMinor = 0;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
StandardUnitsSchemaHolder::StandardUnitsSchemaHolder()
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey key(UNITS_SCHEMA_NAME, s_unitsVersionRead, s_unitsVersionWrite, s_unitsVersionMinor);
    m_schema = ECSchema::LocateSchema(key, *schemaContext);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
// static
StandardUnitsSchemaHolderPtr StandardUnitsSchemaHolder::GetHolder()
    {
    if (s_schemaHolder.IsNull())
        s_schemaHolder = new StandardUnitsSchemaHolder();

    return s_schemaHolder;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECSchemaPtr StandardUnitsSchemaHolder::_GetSchema()
    {
    return m_schema;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
// static
ECSchemaPtr StandardUnitsSchemaHolder::GetSchema()
    {
    return GetHolder()->_GetSchema();
    }

//*********************** StandardUnitsHelper *************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus StandardUnitsHelper::GetUnit (ECUnitCP& unit, Utf8CP unitName) 
    {
    unit = StandardUnitsSchemaHolder::GetSchema()->GetUnitCP(unitName);
    if(nullptr != unit)
        return ECObjectsStatus::Success;
    return ECObjectsStatus::NotFound;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus StandardUnitsHelper::GetInvertedUnit (ECUnitCP& unit, Utf8CP unitName) 
    {
    unit = StandardUnitsSchemaHolder::GetSchema()->GetInvertedUnitCP(unitName);
    if(nullptr != unit)
        return ECObjectsStatus::Success;
    return ECObjectsStatus::NotFound;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus StandardUnitsHelper::GetConstant (ECUnitCP& unit, Utf8CP unitName) 
    {
    unit = StandardUnitsSchemaHolder::GetSchema()->GetConstantCP(unitName);
    if(nullptr != unit)
        return ECObjectsStatus::Success;
    return ECObjectsStatus::NotFound;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus StandardUnitsHelper::GetPhenomenon(PhenomenonCP& phenom, Utf8CP phenomName) 
    {
    phenom = StandardUnitsSchemaHolder::GetSchema()->GetPhenomenonCP(phenomName);
    if(nullptr != phenom)
        return ECObjectsStatus::Success;
    return ECObjectsStatus::NotFound;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus StandardUnitsHelper::GetUnitSystem(UnitSystemCP& system, Utf8CP systemName) 
    {
    system = StandardUnitsSchemaHolder::GetSchema()->GetUnitSystemCP(systemName);
    if(nullptr != system)
        return ECObjectsStatus::Success;
    return ECObjectsStatus::NotFound;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
