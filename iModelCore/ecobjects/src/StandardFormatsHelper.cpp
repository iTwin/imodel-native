/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandardFormatsHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"


BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define FORMATS_SCHEMA_NAME "Formats"

//*********************** StandardUnitsSchemaHolder *************************************

//--------------------------------------------------------------------------------------
// @bsiclass
// Helper class to hold the schema
//                                              Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
struct StandardFormatsSchemaHolder;
typedef RefCountedPtr<StandardFormatsSchemaHolder> StandardFormatsSchemaHolderPtr;

struct StandardFormatsSchemaHolder : RefCountedBase
    {
    private:
        ECSchemaPtr                         m_schema;
        static StandardFormatsSchemaHolderPtr s_schemaHolder;

        StandardFormatsSchemaHolder();
        ECSchemaPtr _GetSchema() {return m_schema;}

    public:

        static StandardFormatsSchemaHolderPtr GetHolder();
        static ECSchemaPtr GetSchema() {return GetHolder()->_GetSchema();}
    };

StandardFormatsSchemaHolderPtr StandardFormatsSchemaHolder::s_schemaHolder;

static const uint32_t s_formatsVersionRead = 1;
static const uint32_t s_formatsVersionWrite = 0;
static const uint32_t s_formatsVersionMinor = 0;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
StandardFormatsSchemaHolder::StandardFormatsSchemaHolder()
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey key(FORMATS_SCHEMA_NAME, s_formatsVersionRead, s_formatsVersionWrite, s_formatsVersionMinor);
    m_schema = ECSchema::LocateSchema(key, *schemaContext);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
// static
StandardFormatsSchemaHolderPtr StandardFormatsSchemaHolder::GetHolder()
    {
    if (s_schemaHolder.IsNull())
        s_schemaHolder = new StandardFormatsSchemaHolder();

    return s_schemaHolder;
    }

//*********************** StandardFormatsHelper *************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECUnitCP StandardFormatsHelper::GetFormat(Utf8CP unitName)
    {
    return StandardFormatsSchemaHolder::GetSchema()->GetUnitCP(unitName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
ECSchemaPtr StandardFormatsHelper::GetSchema()
    {
    return StandardFormatsSchemaHolder::GetSchema();
    }

END_BENTLEY_ECOBJECT_NAMESPACE
