/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaPersistenceHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECDBMETA_PROP_ECEnumerator_Name "Name"
#define ECDBMETA_PROP_ECEnumerator_IntValue "IntValue"
#define ECDBMETA_PROP_ECEnumerator_StringValue "StringValue"
#define ECDBMETA_PROP_ECEnumerator_DisplayLabel "DisplayLabel"
#define ECDBMETA_PROP_ECEnumerator_Description "Description"

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
    SchemaPersistenceHelper() = delete;
    ~SchemaPersistenceHelper() = delete;

public:
    static ECN::ECSchemaId GetSchemaId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, SchemaLookupMode);
    
    //!@p schemaNameCsvList List of comma separated schema names to be appended to the WHERE clause used to retrieve the ids
    static std::vector<ECN::ECSchemaId> GetSchemaIds(ECDbCR, DbTableSpace const&, Utf8StringVirtualSet const& schemaNames);
    static Utf8String GetSchemaName(ECDbCR, DbTableSpace const&, ECN::ECSchemaId);
    static ECN::ECClassId GetClassId(ECDbCR, DbTableSpace const&, ECN::ECSchemaId, Utf8CP className);
    static ECN::ECClassId GetClassId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, Utf8CP className, SchemaLookupMode);
    static ECN::ECEnumerationId GetEnumerationId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, Utf8CP enumName, SchemaLookupMode);
    static ECN::KindOfQuantityId GetKindOfQuantityId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, Utf8CP koqName, SchemaLookupMode);
    static ECN::PropertyCategoryId GetPropertyCategoryId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, Utf8CP catName, SchemaLookupMode);
    static ECN::UnitSystemId GetUnitSystemId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, Utf8CP unitSystemName, SchemaLookupMode);
    static ECN::PhenomenonId GetPhenomenonId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, Utf8CP phenomenonName, SchemaLookupMode);
    static ECN::UnitId GetUnitId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, Utf8CP unitName, SchemaLookupMode);
    static ECN::FormatId GetFormatId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, Utf8CP formatName, SchemaLookupMode);
    static ECN::ECPropertyId GetPropertyId(ECDbCR, DbTableSpace const&, ECN::ECClassId, Utf8CP propertyName);
    static ECN::ECPropertyId GetPropertyId(ECDbCR, DbTableSpace const&, Utf8CP schemaNameOrAlias, Utf8CP className, Utf8CP propertyName, SchemaLookupMode);

    static bool TryGetSchemaKey(ECN::SchemaKey&, ECDbCR, DbTableSpace const&, Utf8CP schemaName);

    static BentleyStatus SerializeEnumerationValues(Utf8StringR jsonStr, ECN::ECEnumerationCR, bool isEC32AvailableInFile);
    static BentleyStatus DeserializeEnumerationValues(ECN::ECEnumerationR, ECDbCR, Utf8CP jsonStr);

    static BentleyStatus SerializeKoqPresentationFormats(Utf8StringR jsonStr, ECDbCR, ECN::KindOfQuantityCR, bool isEC32AvailableInFile);
    static BentleyStatus SerializeKoqPresentationFormats(Utf8StringR jsonStr, bvector<Utf8String> const& presFormats);

    static Utf8String SerializeNumericSpec(Formatting::NumericFormatSpecCR);
    static Utf8String SerializeCompositeSpecWithoutUnits(Formatting::CompositeValueSpecCR);

    //!Safe method to cast an integer value to the ECClassType enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<ECN::ECClassType> ToClassType(int val)
        {
        if (val == Enum::ToInt(ECN::ECClassType::CustomAttribute) || val == Enum::ToInt(ECN::ECClassType::Entity) ||
            val == Enum::ToInt(ECN::ECClassType::Relationship) || val == Enum::ToInt(ECN::ECClassType::Struct))
            return Enum::FromInt<ECN::ECClassType>(val);

        return Nullable<ECN::ECClassType>();
        };

    //!Safe method to cast an integer value to the ECClassModifier enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<ECN::ECClassModifier> ToClassModifier(int val)
        {
        if (val == Enum::ToInt(ECN::ECClassModifier::Abstract) || val == Enum::ToInt(ECN::ECClassModifier::None) || val == Enum::ToInt(ECN::ECClassModifier::Sealed))
            return Enum::FromInt<ECN::ECClassModifier>(val);

        return Nullable<ECN::ECClassModifier>();
        };

    //!Safe method to cast an integer value to the PropertyKind enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<PropertyKind> ToPropertyKind(int val)
        {
        if (val >= 0 && val <= 4)
            return Enum::FromInt<PropertyKind>(val);

        return Nullable<PropertyKind>();
        };

    //!Safe method to cast an integer value to the PrimitiveType enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<ECN::PrimitiveType> ToPrimitiveType(int val)
        {
        if (val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_Binary) || val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_Boolean) ||
            val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_DateTime) || val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_Double) ||
            val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_IGeometry) || val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_Integer) ||
            val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_Long) || val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_Point2d) ||
            val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_Point3d) || val == Enum::ToInt(ECN::PrimitiveType::PRIMITIVETYPE_String))
            return Enum::FromInt<ECN::PrimitiveType>(val);

        return Nullable<ECN::PrimitiveType>();
        };

    //!Safe method to cast an integer value to the StrengthType enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<ECN::StrengthType> ToStrengthType(int val)
        {
        if (val == Enum::ToInt(ECN::StrengthType::Embedding) || val == Enum::ToInt(ECN::StrengthType::Holding) || val == Enum::ToInt(ECN::StrengthType::Referencing))
            return Enum::FromInt<ECN::StrengthType>(val);

        return Nullable<ECN::StrengthType>();
        };

    //!Safe method to cast an integer value to the ECRelatedInstanceDirection enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<ECN::ECRelatedInstanceDirection> ToECRelatedInstanceDirection(int val)
        {
        if (val == Enum::ToInt(ECN::ECRelatedInstanceDirection::Backward) || val == Enum::ToInt(ECN::ECRelatedInstanceDirection::Forward))
            return Enum::FromInt<ECN::ECRelatedInstanceDirection>(val);

        return Nullable<ECN::ECRelatedInstanceDirection>();
        };
 
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
