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
    Enumeration = 4,
    Navigation = 5
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaPersistenceHelper
    {
public:
    //!This enum generalizes ECObjects' ones and is used to persist CA instances along with the container type
    //!in the table ec_CustomAttribute. In that table we don't need to store the exact container type, so
    //!the extra enum is for type-safety and to be robust against changes in the ECObjects enum (enforced through
    //!separate static asserts)
    enum class GeneralizedCustomAttributeContainerType : std::underlying_type<ECN::CustomAttributeContainerType>::type
        {
        Schema = std::underlying_type<ECN::CustomAttributeContainerType>::type(ECN::CustomAttributeContainerType::Schema),
        Class = std::underlying_type<ECN::CustomAttributeContainerType>::type(ECN::CustomAttributeContainerType::AnyClass),
        Property = std::underlying_type<ECN::CustomAttributeContainerType>::type(ECN::CustomAttributeContainerType::AnyProperty),
        SourceRelationshipConstraint = std::underlying_type<ECN::CustomAttributeContainerType>::type(ECN::CustomAttributeContainerType::SourceRelationshipConstraint),
        TargetRelationshipConstraint = std::underlying_type<ECN::CustomAttributeContainerType>::type(ECN::CustomAttributeContainerType::TargetRelationshipConstraint)
        };

private:
    ECDbSchemaPersistenceHelper();
    ~ECDbSchemaPersistenceHelper();

public:
    static bool ContainsECSchema(ECDbCR, ECSchemaId);
    static bool ContainsECClass(ECDbCR, ECClassCR);

    static ECSchemaId GetECSchemaId(ECDbCR, Utf8CP schemaName);
    static ECClassId GetECClassId(ECDbCR, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema);
    static ECEnumerationId GetECEnumerationId(ECDbCR, Utf8CP schemaName, Utf8CP enumName);
    static ECPropertyId GetECPropertyId(ECDbCR, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName);

    static BentleyStatus SerializeRelationshipKeyProperties(Utf8StringR jsonStr, bvector<Utf8String> const& keyPropNames);
    static BentleyStatus DeserializeRelationshipKeyProperties(ECRelationshipConstraintClassR, Utf8CP jsonStr);
    static BentleyStatus SerializeECEnumerationValues(Utf8StringR jsonStr, ECEnumerationCR);
    static BentleyStatus DeserializeECEnumerationValues(ECEnumerationR, Utf8CP jsonStr);
    static bool ContainsECSchemaWithNamespacePrefix(ECDbCR db, Utf8CP namespacePrefix);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
