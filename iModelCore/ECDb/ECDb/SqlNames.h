/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SqlNames.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define TABLE_ECIndex "ec_Index"
#define TABLE_ECIndexColumn "ec_IndexColumn"

#define TABLE_ClassHierarchyCache "ec_cache_ClassHierarchy"
#define TABLE_ClassHasTablesCache "ec_cache_ClassHasTables"

#define COL_ECClassId "ECClassId"
#define COL_Overflow "scoverflow"

#define SQLFUNC_BlobToBase64 "BlobToBase64"
#define SQLFUNC_Base64ToBlob "Base64ToBlob"

//The SQLVAL_xxx defines are meant to be only used in SQL strings. Potential redundancy
//is ok to avoid the expensiveness of compose strings at runtime using expensive methods like Utf8String.Sprintf
//although the SQL string is actually literal.
//Example:
//stmt.Prepare(ecdb,"SELECT Id FROM ec_Column WHERE ColumnKind=" SQLVAL_DbColumnKind_SharedDataColumn);
//In this example the SQL string is constant at compile time,
//as opposed to the expensive way:
//Utf8String sql;
//sql.Sprintf("SELECT Id FROM ec_Column WHERE ColumnKind=%d", DbColumn::Kind::SharedDataColumn);
//stmt.Prepare(ecdb, sql.c_str());

//All these redundant defines must be safe-guarded by static_assert to monitor when the values in the API changes.

#define SQLVAL_True "1"
#define SQLVAL_False "0"

//The defines must match the respective enum declaration. The redundancy is taken into account
//to avoid the burden to compose strings at runtime using expensive methods like Utf8String.Sprintf
//although the SQL string is actually literal. The static_assert are safe-guards to monitor
//when enum values are changed

//** Enum DbColumn::Kind
#define SQLVAL_DbColumn_Kind_Unknown "0"
static_assert(0 == (int) DbColumn::Kind::Unknown, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_ECInstanceId "1"
static_assert(1 == (int) DbColumn::Kind::ECInstanceId, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_ECClassId "2"
static_assert(2 == (int) DbColumn::Kind::ECClassId, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_SourceECInstanceId "4"
static_assert(4 == (int) DbColumn::Kind::SourceECInstanceId, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_SourceECClassId "8"
static_assert(8 == (int) DbColumn::Kind::SourceECClassId, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_TargetECInstanceId "16"
static_assert(16 == (int) DbColumn::Kind::TargetECInstanceId, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_TargetECClassId "32"
static_assert(32 == (int) DbColumn::Kind::TargetECClassId, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_DataColumn "64"
static_assert(64 == (int) DbColumn::Kind::DataColumn, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_SharedDataColumn "128"
static_assert(128 == (int) DbColumn::Kind::SharedDataColumn, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_RelECClassId "256"
static_assert(256 == (int) DbColumn::Kind::RelECClassId, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_Overflow "512"
static_assert(512 == (int) DbColumn::Kind::Overflow, "Persisted enum DbColumn::Kind has changed");
#define SQLVAL_DbColumn_Kind_OverflowSlave "1024"
static_assert(1024 == (int) DbColumn::Kind::OverflowSlave, "Persisted enum DbColumn::Kind has changed");

//** Enum DbTable::Type
#define SQLVAL_DbTable_Type_Primary "0"
static_assert(0 == (int) DbTable::Type::Primary, "Persisted enum DbTable::Type has changed");
#define SQLVAL_DbTable_Type_Joined "1"
static_assert(1 == (int) DbTable::Type::Joined, "Persisted enum DbTable::Type has changed");
#define SQLVAL_DbTable_Type_Existing "2"
static_assert(2 == (int) DbTable::Type::Existing, "Persisted enum DbTable::Type has changed");

//** Enum ECClassType
#define SQLVAL_ECClassType_Entity "0"
static_assert(0 == (int) ECN::ECClassType::Entity, "Persisted enum ECN::ECClassType has changed");
#define SQLVAL_ECClassType_Relationship "1"
static_assert(1 == (int) ECN::ECClassType::Relationship, "Persisted enum ECN::ECClassType has changed");
#define SQLVAL_ECClassType_Struct "2"
static_assert(2 == (int) ECN::ECClassType::Struct, "Persisted enum ECN::ECClassType has changed");
#define SQLVAL_ECClassType_CustomAttribute "3"
static_assert(3 == (int) ECN::ECClassType::CustomAttribute, "Persisted enum ECN::ECClassType has changed");
#define SQLVAL_ECClassType_Interface "4"
static_assert(4 == (int) ECN::ECClassType::Interface, "Persisted enum ECN::ECClassType has changed");

//** Enum JoinedTableInfo
#define SQLVAL_JoinedTableInfo_None "0"
static_assert(0 == (int) JoinedTableInfo::None, "Persisted enum JoinedTableInfo has changed");
#define SQLVAL_JoinedTableInfo_JoinedTable "1"
static_assert(1 == (int) JoinedTableInfo::JoinedTable, "Persisted enum JoinedTableInfo has changed");
#define SQLVAL_JoinedTableInfo_ParentOfJoinedTable "2"
static_assert(2 == (int) JoinedTableInfo::ParentOfJoinedTable, "Persisted enum JoinedTableInfo has changed");

//** Enum ShareColumnsMode
static_assert(0 == (int) TablePerHierarchyInfo::ShareColumnsMode::No, "Persisted enum TablePerHierarchyInfo::ShareColumnsMode has changed");
static_assert(1 == (int) TablePerHierarchyInfo::ShareColumnsMode::Yes, "Persisted enum TablePerHierarchyInfo::ShareColumnsMode has changed");
static_assert(2 == (int) TablePerHierarchyInfo::ShareColumnsMode::ApplyToSubclassesOnly, "Persisted enum TablePerHierarchyInfo::ShareColumnsMode has changed");

//** Enum MapStrategy
#define SQLVAL_MapStrategy_NotMapped "0"
static_assert(0 == (int) MapStrategy::NotMapped, "Persisted enum MapStrategy has changed");
#define SQLVAL_MapStrategy_OwnTable "1"
static_assert(1 == (int) MapStrategy::OwnTable, "Persisted enum MapStrategy has changed");
#define SQLVAL_MapStrategy_TablePerHierarchy "2"
static_assert(2 == (int) MapStrategy::TablePerHierarchy, "Persisted enum MapStrategy has changed");
#define SQLVAL_MapStrategy_ExistingTable "3"
static_assert(3 == (int) MapStrategy::ExistingTable, "Persisted enum MapStrategy has changed");
#define SQLVAL_MapStrategy_SharedTable "4"
static_assert(4 == (int) MapStrategy::SharedTable, "Persisted enum MapStrategy has changed");
#define SQLVAL_MapStrategy_ForeignKeyRelationshipInSourceTable "5"
static_assert(5 == (int) MapStrategy::ForeignKeyRelationshipInSourceTable, "Persisted enum MapStrategy has changed");
#define SQLVAL_MapStrategy_ForeignKeyRelationshipInTargetTable "6"
static_assert(6 == (int) MapStrategy::ForeignKeyRelationshipInTargetTable, "Persisted enum MapStrategy has changed");

//List here all other enums whose values are persisted to detect enum changes.
static_assert((int) ECN::CustomAttributeContainerType::Any == 30655 &&
(int) ECN::CustomAttributeContainerType::AnyClass == 62 &&
(int) ECN::CustomAttributeContainerType::AnyProperty == 6016 &&
(int) ECN::CustomAttributeContainerType::AnyRelationshipConstraint == 24576 &&
(int) ECN::CustomAttributeContainerType::PrimitiveArrayProperty == 512 &&
(int) ECN::CustomAttributeContainerType::CustomAttributeClass == 4 &&
(int) ECN::CustomAttributeContainerType::EntityClass == 2 &&
(int) ECN::CustomAttributeContainerType::NavigationProperty == 4096 &&
(int) ECN::CustomAttributeContainerType::PrimitiveProperty == 128 &&
(int) ECN::CustomAttributeContainerType::RelationshipClass == 16 &&
(int) ECN::CustomAttributeContainerType::Schema == 1 &&
(int) ECN::CustomAttributeContainerType::SourceRelationshipConstraint == 8192 &&
(int) ECN::CustomAttributeContainerType::StructProperty == 256 &&
(int) ECN::CustomAttributeContainerType::StructArrayProperty == 1024 &&
(int) ECN::CustomAttributeContainerType::StructClass == 8 &&
(int) ECN::CustomAttributeContainerType::InterfaceClass == 32 &&
(int) ECN::CustomAttributeContainerType::TargetRelationshipConstraint == 16384, "Persisted Enum has changed: ECN::CustomAttributeContainerType.");


static_assert((int) DbColumn::Type::Any == 0 &&
(int) DbColumn::Type::Blob == 2 &&
(int) DbColumn::Type::Boolean == 1 &&
(int) DbColumn::Type::Integer == 5 &&
(int) DbColumn::Type::Real == 4 &&
(int) DbColumn::Type::Text == 6 &&
(int) DbColumn::Type::TimeStamp == 3, "Persisted Enum has changed: DbColumn::Type.");


static_assert((int) ECN::ECClassModifier::Abstract == 1 &&
(int) ECN::ECClassModifier::None == 0 &&
(int) ECN::ECClassModifier::Sealed == 2, "Persisted Enum has changed: ECN::ECClassModifier.");

static_assert((int) ECN::ECClassType::CustomAttribute == 3 &&
(int) ECN::ECClassType::Entity == 0 &&
(int) ECN::ECClassType::Relationship == 1 &&
(int) ECN::ECClassType::Struct == 2 &&
(int) ECN::ECClassType::Interface == 4, "Persisted Enum has changed: ECN::ECClassType.");


static_assert((int) ECPropertyKind::Navigation == 4 &&
(int) ECPropertyKind::Primitive == 0 &&
(int) ECPropertyKind::PrimitiveArray == 2 &&
(int) ECPropertyKind::Struct == 1 &&
(int) ECPropertyKind::StructArray == 3, "Persisted Enum has changed: ECPropertyKind.");

static_assert((int) ECN::StrengthType::Embedding == 2 &&
(int) ECN::StrengthType::Holding == 1 &&
(int) ECN::StrengthType::Referencing == 0, "Persisted Enum has changed: ECN::StrengthType.");

static_assert((int) ECN::ECRelatedInstanceDirection::Backward == 2 &&
(int) ECN::ECRelatedInstanceDirection::Forward == 1, "Persisted Enum has changed: ECN::ECRelatedInstanceDirection.");

static_assert((int) ECN::ECRelationshipEnd::ECRelationshipEnd_Source == 0 &&
(int) ECN::ECRelationshipEnd::ECRelationshipEnd_Target == 1, "Persisted Enum has changed: ECN::ECRelationshipEnd.");

static_assert((int) ForeignKeyDbConstraint::ActionType::Cascade == 1 &&
(int) ForeignKeyDbConstraint::ActionType::NoAction == 2 &&
(int) ForeignKeyDbConstraint::ActionType::NotSpecified == 0 &&
(int) ForeignKeyDbConstraint::ActionType::Restrict == 5 &&
(int) ForeignKeyDbConstraint::ActionType::SetDefault == 4 &&
(int) ForeignKeyDbConstraint::ActionType::SetNull == 3, "Persisted Enum has changed: ForeignKeyDbConstraint::ActionType.");

static_assert((int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class == 62 &&
(int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property == 6016 &&
(int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema == 1 &&
(int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint == 8192 &&
(int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint == 16384, "Persisted Enum has changed: ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType.");

static_assert((int) ECN::PrimitiveType::PRIMITIVETYPE_Binary == 0x101 &&
(int) ECN::PrimitiveType::PRIMITIVETYPE_Boolean == 0x201 &&
(int) ECN::PrimitiveType::PRIMITIVETYPE_DateTime == 0x301 &&
(int) ECN::PrimitiveType::PRIMITIVETYPE_Double == 0x401 &&
(int) ECN::PrimitiveType::PRIMITIVETYPE_IGeometry == 0xa01 &&
(int) ECN::PrimitiveType::PRIMITIVETYPE_Integer == 0x501 &&
(int) ECN::PrimitiveType::PRIMITIVETYPE_Long == 0x601 &&
(int) ECN::PrimitiveType::PRIMITIVETYPE_Point2d == 0x701 &&
(int) ECN::PrimitiveType::PRIMITIVETYPE_Point3d == 0x801 &&
(int) ECN::PrimitiveType::PRIMITIVETYPE_String == 0x901, "Persisted Enum has changed: ECN::PrimitiveType.");





END_BENTLEY_SQLITE_EC_NAMESPACE