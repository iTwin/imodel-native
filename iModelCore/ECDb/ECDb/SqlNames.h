/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SqlNames.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define TABLE_ECIndex "ec_Index"
#define TABLE_ECIndexColumn "ec_IndexColumn"

#define TABLE_ClassHierarchyCache "ec_cache_ClassHierarchy"
#define TABLE_ClassHasTablesCache "ec_cache_ClassHasTables"

#define COL_ECInstanceId "ECInstanceId"
#define COL_ECClassId "ECClassId"

#define SQLVAL_INT_True "1"
#define SQLVAL_INT_False "0"

//The defines are meant to be only used in SQL strings.  
//NOTE: Every define here must be backed by a static_assert in DbSchemaPersistenceManager.cpp (bottom)
//The defines must match the respective enum declaration. The redundancy is taken into account
//to avoid the burden to compose strings at runtime using expensive methods like Utf8String.Sprintf
//although the SQL string is actually literal. The static_assert are safe-guards to monitor
//when enum values are changed
#define SQLVAL_INT_DbColumn_Kind_Unknown "0"
#define SQLVAL_INT_DbColumn_Kind_ECInstanceId "1"
#define SQLVAL_INT_DbColumn_Kind_ECClassId "2"
#define SQLVAL_INT_DbColumn_Kind_SourceECInstanceId "32"
#define SQLVAL_INT_DbColumn_Kind_SourceECClassId "64"
#define SQLVAL_INT_DbColumn_Kind_TargetECInstanceId "128"
#define SQLVAL_INT_DbColumn_Kind_TargetECClassId "256"
#define SQLVAL_INT_DbColumn_Kind_DataColumn "512"
#define SQLVAL_INT_DbColumn_Kind_SharedDataColumn "1024"
#define SQLVAL_INT_DbColumn_Kind_RelECClassId "2048"

#define SQLVAL_INT_DbTable_Type_Primary "0"
#define SQLVAL_INT_DbTable_Type_Joined "1"
#define SQLVAL_INT_DbTable_Type_Existing "2"

#define SQLVAL_INT_ECClassType_Entity "0"
#define SQLVAL_INT_ECClassType_Relationship "1"
#define SQLVAL_INT_ECClassType_Struct "2"
#define SQLVAL_INT_ECClassType_CustomAttribute "3"

#define SQLVAL_INT_JoinedTableInfo_None "0"
#define SQLVAL_INT_JoinedTableInfo_JoinedTable "1"
#define SQLVAL_INT_JoinedTableInfo_ParentOfJoinedTable "2"

#define SQLVAL_INT_MapStrategy_NotMapped "0"
#define SQLVAL_INT_MapStrategy_OwnTable "1"
#define SQLVAL_INT_MapStrategy_TablePerHierarchy "2"
#define SQLVAL_INT_MapStrategy_ExistingTable "3"
#define SQLVAL_INT_MapStrategy_SharedTable "4"
#define SQLVAL_INT_MapStrategy_ForeignKeyRelationshipInSourceTable "100"
#define SQLVAL_INT_MapStrategy_ForeignKeyRelationshipInTargetTable "101"


END_BENTLEY_SQLITE_EC_NAMESPACE