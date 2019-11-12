/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECUnits/ECUnitsClassLocater.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

ECUnitsClassLocaterPtr ECUnitsClassLocater::s_unitsECClassLocaterPtr;
static Utf8CP const  UNITS_SCHEMA                      = "Units_Schema";
static Utf8CP const  KOQ_SCHEMA                        = "KindOfQuantity_Schema";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECUnitsClassLocaterPtr ECUnitsClassLocater::Create()
    {
    if (s_unitsECClassLocaterPtr.IsNull())
        s_unitsECClassLocaterPtr = new ECUnitsClassLocater();
    return s_unitsECClassLocaterPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsClassLocater::LoadUnitsSchemas (ECSchemaReadContextR context)
    {
    SchemaKey keyKoqSchema (KOQ_SCHEMA, 1, 0);
    ECSchemaPtr koqSchema = context.LocateSchema (keyKoqSchema, SchemaMatchType::LatestWriteCompatible);
    POSTCONDITION (koqSchema.IsValid() && "Cannot load KOQ_SCHEMA", false);

    SchemaKey keyUnitsSchema (UNITS_SCHEMA, 1, 0);
    ECSchemaPtr unitsSchema = context.LocateSchema (keyUnitsSchema, SchemaMatchType::LatestWriteCompatible);
    POSTCONDITION (unitsSchema.IsValid() && "Cannot load UNITS_SCHEMA", false);

    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsClassLocater::Initialize()
    {
    if (!m_context.IsNull())
        return true;
    m_context = ECSchemaReadContext::CreateContext (nullptr, false);

    if (!LoadUnitsSchemas (*m_context))
        {
        m_context = NULL;
        return false;
        }
    
    SchemaKey keyKoqSchema (KOQ_SCHEMA, 1, 0);
    m_koqSchema = m_context->GetFoundSchema (keyKoqSchema, SchemaMatchType::LatestWriteCompatible);

    SchemaKey keyUnitsSchema (UNITS_SCHEMA, 1, 0);
    m_unitsSchema = m_context->LocateSchema (keyUnitsSchema, SchemaMatchType::LatestWriteCompatible);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECUnitsClassLocater::_LocateClass (Utf8CP schemaName, Utf8CP className)
    {
    if (!Initialize())
        return NULL;

    if (0 == strcmp(UNITS_SCHEMA, schemaName))
        return m_unitsSchema->GetClassCP(className);
    
    if (0 == strcmp(KOQ_SCHEMA, schemaName))
        return m_koqSchema->GetClassCP(className);

    return nullptr;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
