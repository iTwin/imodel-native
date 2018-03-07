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
        ECSchemaPtr _GetSchema() {return m_schema;}

    public:

        static StandardUnitsSchemaHolderPtr GetHolder();
        static ECSchemaPtr GetSchema() {return GetHolder()->_GetSchema();}
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

//*********************** StandardUnitsHelper *************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECUnitCP StandardUnitsHelper::GetUnit(Utf8CP unitName)
    {
    return StandardUnitsSchemaHolder::GetSchema()->GetUnitCP(unitName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECUnitCP StandardUnitsHelper::GetInvertedUnit(Utf8CP unitName)
    {
    return StandardUnitsSchemaHolder::GetSchema()->GetInvertedUnitCP(unitName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECUnitCP StandardUnitsHelper::GetConstant(Utf8CP unitName) 
    {
    return StandardUnitsSchemaHolder::GetSchema()->GetConstantCP(unitName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
PhenomenonCP StandardUnitsHelper::GetPhenomenon(Utf8CP phenomName)
    {
    return StandardUnitsSchemaHolder::GetSchema()->GetPhenomenonCP(phenomName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
UnitSystemCP StandardUnitsHelper::GetUnitSystem(Utf8CP systemName)
    {
    return StandardUnitsSchemaHolder::GetSchema()->GetUnitSystemCP(systemName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECSchemaPtr StandardUnitsHelper::GetSchema()
    {
    return StandardUnitsSchemaHolder::GetSchema();
    }

END_BENTLEY_ECOBJECT_NAMESPACE
