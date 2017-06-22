/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaPersistenceHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECDBMETA_PROP_ECEnumerator_IntValue "IntValue"
#define ECDBMETA_PROP_ECEnumerator_StringValue "StringValue"
#define ECDBMETA_PROP_ECEnumerator_DisplayLabel "DisplayLabel"

//=======================================================================================
// @bsienum                                                Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
enum class PropertyKind
    {
    Primitive = 0,
    Struct = 1,
    PrimitiveArray = 2,
    StructArray = 3,
    Navigation = 4
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct Utf8StringVirtualSet final : VirtualSet
    {
private:
    std::function<bool(Utf8CP)> m_predicate;

    bool _IsInSet(int nVals, DbValue const* values) const override
        {
        BeAssert(nVals == 1);
        return m_predicate(values[0].GetValueText());
        }

public:
    explicit Utf8StringVirtualSet(std::function<bool(Utf8CP)> const& predicate) : VirtualSet(), m_predicate(predicate) {}

    ~Utf8StringVirtualSet() {}
    };

//---------------------------------------------------------------------------------------
//!@remarks Only call these methods if you need to get the information from the DB. 
//! If you want to leverage caches (which can speed up performance), then use SchemaManager::GetReader()
// @bsiclass                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+------
struct SchemaPersistenceHelper final
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
    SchemaPersistenceHelper();
    ~SchemaPersistenceHelper();

public:
    static ECN::ECSchemaId GetSchemaId(ECDbCR, Utf8CP schemaNameOrAlias, SchemaLookupMode);
    
    //!@p schemaNameCsvList List of comma separated schema names to be appended to the WHERE clause used to retrieve the ids
    static std::vector<ECN::ECSchemaId> GetSchemaIds(ECDbCR, Utf8StringVirtualSet const& schemaNames);
    static Utf8String GetSchemaName(ECDbCR, ECN::ECSchemaId);
    static ECN::ECClassId GetClassId(ECDbCR, ECN::ECSchemaId, Utf8CP className);
    static ECN::ECClassId GetClassId(ECDbCR, Utf8CP schemaNameOrAlias, Utf8CP className, SchemaLookupMode);
    static ECN::ECEnumerationId GetEnumerationId(ECDbCR, Utf8CP schemaNameOrAlias, Utf8CP enumName, SchemaLookupMode);
    static ECN::KindOfQuantityId GetKindOfQuantityId(ECDbCR, Utf8CP schemaNameOrAlias, Utf8CP koqName, SchemaLookupMode);
    static ECN::PropertyCategoryId GetPropertyCategoryId(ECDbCR, Utf8CP schemaNameOrAlias, Utf8CP catName, SchemaLookupMode);
    static ECN::ECPropertyId GetPropertyId(ECDbCR, ECN::ECClassId, Utf8CP propertyName);
    static ECN::ECPropertyId GetPropertyId(ECDbCR, Utf8CP schemaNameOrAlias, Utf8CP className, Utf8CP propertyName, SchemaLookupMode);

    static bool TryGetSchemaKey(ECN::SchemaKey&, ECDbCR, Utf8CP schemaName);
    static bool TryGetSchemaKeyAndId(ECN::SchemaKey&, ECN::ECSchemaId&, ECDbCR, Utf8CP schemaName);

    static BentleyStatus SerializeEnumerationValues(Utf8StringR jsonStr, ECN::ECEnumerationCR);
    static BentleyStatus DeserializeEnumerationValues(ECN::ECEnumerationR, Utf8CP jsonStr);

    static BentleyStatus SerializeKoqPresentationUnits(Utf8StringR jsonStr, ECDbCR, ECN::KindOfQuantityCR);
    static BentleyStatus DeserializeKoqPresentationUnits(ECN::KindOfQuantityR, Utf8CP jsonStr);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
