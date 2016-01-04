/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistenceHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECClassEntry
    {
public:
    ECSchemaId m_ecSchemaId;
    ECClassId m_ecClassId;
    ECN::ECClassP m_cachedECClass;

    DbECClassEntry(ECN::ECSchemaId schemaId, ECN::ECClassCR ecClass) : m_ecSchemaId(schemaId), m_ecClassId(ecClass.GetId())
        {
        m_cachedECClass = const_cast<ECN::ECClassP> (&ecClass);
        }
    };

typedef int64_t ECEnumerationId;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
struct DbECEnumEntry
    {
public:
    ECSchemaId m_ecSchemaId;
    Utf8CP m_enumName;
    ECN::ECEnumerationP m_cachedECEnum;

    DbECEnumEntry(ECN::ECSchemaId schemaId, ECN::ECEnumerationCR ecEnum) : m_ecSchemaId(schemaId), m_enumName(ecEnum.GetName().c_str()) 
        {
        m_cachedECEnum = const_cast<ECN::ECEnumerationP> (&ecEnum);
        }
    };

//=======================================================================================
// @bsienum                                                Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
enum class ECPropertyKind
    {
    Primitive = 0,
    Struct = 1,
    PrimitiveArray = 2,
    StructArray = 3,
    Navigation = 4
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaPersistenceHelper
    {
private:
    ECDbSchemaPersistenceHelper();
    ~ECDbSchemaPersistenceHelper();

public:
    static bool ContainsECSchema(ECDbCR, ECSchemaId);
    static bool ContainsECClass(ECDbCR, ECClassCR);

    static ECSchemaId GetECSchemaId(ECDbCR, Utf8CP schemaName);
    static ECClassId GetECClassId(ECDbCR, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema);
    static ECPropertyId GetECPropertyId(ECDbCR, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName);

    static BentleyStatus GetECSchemaKeys(ECSchemaKeys&, ECDbCR);
    static BentleyStatus GetECClassKeys(ECClassKeys&, ECSchemaId, ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
