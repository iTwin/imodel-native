/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistenceHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define METASCHEMA_ECENUMERATOR_PROPERTY_IntValue "IntValue"
#define METASCHEMA_ECENUMERATOR_PROPERTY_StringValue "StringValue"
#define METASCHEMA_ECENUMERATOR_PROPERTY_DisplayLabel "DisplayLabel"

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
    static ECN::ECSchemaId GetECSchemaId(ECDbCR, ECN::ECSchemaCR);
    static ECN::ECSchemaId GetECSchemaId(ECDbCR, Utf8CP schemaName);
    static ECN::ECClassId GetECClassId(ECDbCR, ECN::ECClassCR);
    static ECN::ECClassId GetECClassId(ECDbCR, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema);
    static ECN::ECEnumerationId GetECEnumerationId(ECDbCR, ECN::ECEnumerationCR);
    static ECN::ECEnumerationId GetECEnumerationId(ECDbCR, Utf8CP schemaName, Utf8CP enumName);
    static ECN::KindOfQuantityId GetKindOfQuantityId(ECDbCR, ECN::KindOfQuantityCR);
    static ECN::KindOfQuantityId GetKindOfQuantityId(ECDbCR, Utf8CP schemaName, Utf8CP koqName);
    static ECN::ECPropertyId GetECPropertyId(ECDbCR, ECN::ECPropertyCR);
    static ECN::ECPropertyId GetECPropertyId(ECDbCR, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName);

    static bool TryGetECSchemaKey(SchemaKey&, ECDbCR, Utf8CP schemaName);
    static bool TryGetECSchemaKeyAndId(SchemaKey&, ECN::ECSchemaId& schemaId, ECDbCR, Utf8CP schemaName);

    static BentleyStatus SerializeRelationshipKeyProperties(Utf8StringR jsonStr, bvector<Utf8String> const& keyPropNames);
    static BentleyStatus DeserializeRelationshipKeyProperties(ECN::ECRelationshipConstraintClassR, Utf8CP jsonStr);
    static BentleyStatus SerializeECEnumerationValues(Utf8StringR jsonStr, ECN::ECEnumerationCR);
    static BentleyStatus DeserializeECEnumerationValues(ECN::ECEnumerationR, Utf8CP jsonStr);

    static BentleyStatus SerializeKoqAlternativePresentationUnits(Utf8StringR jsonStr, ECN::KindOfQuantityCR);
    static BentleyStatus DeserializeKoqAlternativePresentationUnits(ECN::KindOfQuantityR, Utf8CP jsonStr);

    static bool ContainsECSchemaWithNamespacePrefix(ECDbCR, Utf8CP namespacePrefix);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
