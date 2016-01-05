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
