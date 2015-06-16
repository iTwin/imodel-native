/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbInternalTypes.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDbTypes.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeAssert.h>
#include "ECDbLogger.h"

ECDB_TYPEDEFS_PTR (ClassMap);
ECDB_TYPEDEFS_PTR (PropertyMap);
ECDB_TYPEDEFS(PropertyMapToColumn);
ECDB_TYPEDEFS_PTR(PropertyMapToInLineStruct);
ECDB_TYPEDEFS_PTR(PropertyMapToTable);
ECDB_TYPEDEFS_PTR(PropertyMapArrayOfPrimitives);
ECDB_TYPEDEFS_PTR(MappedTable);
ECDB_TYPEDEFS_PTR(RelationshipClassMap);
ECDB_TYPEDEFS_PTR(RelationshipClassEndTableMap);
ECDB_TYPEDEFS_PTR(RelationshipClassLinkTableMap);
ECDB_TYPEDEFS_PTR(ClassIndexInfo);
ECDB_TYPEDEFS_PTR(StandardKeySpecification);

//Schema analysis and map generator
ECDB_TYPEDEFS_PTR(ECCluster);
ECDB_TYPEDEFS_PTR(ECNode);
ECDB_TYPEDEFS_PTR(ECGraph);
ECDB_TYPEDEFS_PTR(ECClusterList);
ECDB_TYPEDEFS_PTR(ClusteringAlgorithm);
ECDB_TYPEDEFS_PTR(IECMapInfoProvider);
ECDB_TYPEDEFS(ClusteringAlgorithmConfiguration);
ECDB_TYPEDEFS2(bvector<ECNodeP>,ECNodeList);
ECDB_TYPEDEFS2(bvector<ECN::ECPropertyCP>, ECPropertyList);
//ECDbSchemaManager API
ECDB_TYPEDEFS(DbECSchemaInfo);
ECDB_TYPEDEFS(DbECClassInfo);
ECDB_TYPEDEFS(DbBaseClassInfo);
ECDB_TYPEDEFS(DbECPropertyInfo);
ECDB_TYPEDEFS(DbECRelationshipConstraintInfo);
ECDB_TYPEDEFS(DbECRelationshipConstraintClassInfo);
ECDB_TYPEDEFS(DbECRelationshipConstraintClassPropertyInfo);
ECDB_TYPEDEFS(DbCustomAttributeInfo);
ECDB_TYPEDEFS(DbECSchemaReferenceInfo);
ECDB_TYPEDEFS(DbBuffer);
ECDB_TYPEDEFS(DbECClassEntry);
ECDB_TYPEDEFS(DbECSchemaEntry);
ECDB_TYPEDEFS(ClassClause);
ECDB_TYPEDEFS(WhereClause);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define LOG (ECDbLogger::Get())

//=======================================================================================
//! ECSQL statement types
// @bsienum                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
enum class ECSqlType
    {
    Select, //!< ECSQL SELECT
    Insert, //!< ECSQL INSERT
    Update, //!< ECSQL UPDATE
    Delete //!< ECSQL DELETE
    };

//*** ECSQL / SQL Operators ***
//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
enum class BinarySqlOperator
    {
    Plus,
    Minus,
    Divide,
    Multiply,
    Modulo,
    ShiftLeft,
    ShiftRight,
    BitwiseOr,
    BitwiseAnd,
    BitwiseXOr,
    Concat
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class BooleanSqlOperator
    {
    EqualTo,
    NotEqualTo,
    LessThan,
    LessThanOrEqualTo,
    GreaterThan,
    GreaterThanOrEqualTo,
    Is,
    IsNot,
    In,
    NotIn,
    Between,
    NotBetween,
    Like,
    NotLike,
    Or,
    And,
    Match,
    NotMatch
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
enum class UnarySqlOperator
    {
    Minus,
    Plus,
    BitwiseNot
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
enum class SqlCompareListType
    {
    All,
    Any,
    Some
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class SqlSetQuantifier
    {
    NotSpecified,
    Distinct,
    All,
    };

typedef int64_t ECContainerId;

enum class MapStatus 
    {
    Success                         = SUCCESS,
    AlreadyMapped                   = 1,
    BaseClassesNotMapped            = 2,    // We have temporarily stopped mapping a given branch of the class hierarchy because
                                                // we haven't mapped one or more of its base classes. This can happen in the case 
                                                // of multiple inheritance, where we attempt to map a child class for which 
                                                // not all parent classes have been mapped
    Error                           = 666
    };

enum SqlUpdateActions
    {
    SQLUPDATE_None,
    SQLUPDATE_Rollback,
    SQLUPDATE_Abort,
    SQLUPDATE_Replace,
    SQLUPDATE_Fail,
    SQLUPDATE_Ignore,
    };

enum SqlInsertActions
    {
    SQLINSERT_None,
    SQLINSERT_Rollback,
    SQLINSERT_Abort,
    SQLINSERT_Replace,
    SQLINSERT_Fail,
    SQLINSERT_Ignore,
    };

enum ECContainerType
    {
    ECONTAINERTYPE_Schema = 1,
    ECONTAINERTYPE_Class = 2,
    ECONTAINERTYPE_Property = 3,
    ECONTAINERTYPE_RelationshipConstraintSource = 4,
    ECONTAINERTYPE_RelationshipConstraintTarget = 5,
    };

// In SQLite, there is something "special" about a primary key declared as "INTEGER", so we want to use "INTEGER" for primary keys
// In reality it will be a 64 bit integer
// However, we do not wish to use "INTEGER" SQLite type for "Long" ECProperties, because it ends up mapping to Int32 via Micrsoft ADO.NET Provider for ODBC
// Even if we end up changing this, it seems reasonable to distinguish "key" values from data values.
#define PRIMITIVETYPE_DbKey   PrimitiveType(0xCEE) /* A "Fake" PrimitiveType to distinguish columns used as keys */
#define PRIMITIVETYPE_Unknown PrimitiveType(0x000) /* A "Fake" PrimitiveType  */

#define ECDB_COL_ECInstanceId           "ECInstanceId"
#define ECDB_COL_ECClassId              "ECClassId"
#define ECDB_COL_ECPropertyPathId       "ECPropertyPathId"
#define ECDB_COL_ParentECInstanceId     "ParentECInstanceId"
#define ECDB_COL_ECArrayIndex           "ECArrayIndex"

//=======================================================================================
// For case-sensitive UTF-8 string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareUtf8
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return strcmp(s1, s2) < 0;}
    };


//=======================================================================================
// For case-insensitive UTF-8 string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareIUtf8
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return BeStringUtilities::Stricmp (s1, s2) < 0;}

    bool operator()(Utf8StringCR s1, Utf8StringCR s2) const
        {
        return BeStringUtilities::Stricmp (s1.c_str (), s2.c_str ()) < 0;
        }
    };

//=======================================================================================
// For case-sensitive WChar string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareWChar
    {
    bool operator()(WCharCP s1, WCharCP s2) const { return (wcscmp(s1, s2) < 0);}
    };

//=======================================================================================
// For case-insensitive WChar string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareIWChar
    {
    bool operator()(WCharCP s1, WCharCP s2) const { return (BeStringUtilities::Wcsicmp(s1, s2) < 0);}
    };

//=======================================================================================
// @bsienum                                                
// @remarks See @ref ECDbSchemaPersistence to find how these enum values map to actual 
// persisted values in the Db. 
//+===============+===============+===============+===============+===============+======
//enum class MapStrategy
//    {
//    // This first group of strategies no ramifications for subclasses
//    NoHint = 0,         // Use default rules, which may include inheriting strategy of parent
//    DoNotMap,           // Skip this one, but child ECClasses may still be mapped
//    TableForThisClass,  // Put this class in a table, but do not pass the strategy along to child ECClasses 
//    // Only DoNotMap and TableForThisClass are valid default strategies
//
//    // These strategies are directly inherited, except for TablePerHierarchy, which causes its children to use InParentTable
//    // They are listed in order of priority (when it comes to conflicts with/among base ECClasses)
//    TablePerHierarchy,  // This class and all child ECClasses stored in one table
//    InParentTable,      // Assigned by system for subclasses of ECClasses using TablePerHierarchy
//    TablePerClass,      // Put each class in its own table (including child ECClasses
//    DoNotMapHierarchy,  // Also don't map children (unless they are reached by a different inheritance pathway) 
//    SharedTableForThisClass, // TableName must be provided. 
//    // These strategies are applicable only to relationships
//    RelationshipSourceTable,     // Store the relationship in the table in which the source class(es) are stored 
//    RelationshipTargetTable,     // Store the relationship in the table in which the target class(es) are stored 
//    };


#define ECDbDataColumn  0x0U
#define ECDbSystemColumnECInstanceId  0x1U
#define ECDbSystemColumnECClassId  0x2U
#define ECDbSystemColumnParentECInstanceId  0x4U
#define ECDbSystemColumnECPropertyPathId  0x8U
#define ECDbSystemColumnECArraryIndex  0x10U

#define ECDbSystemColumns  (ECDbSystemColumnECInstanceId | ECDbSystemColumnECClassId | ECDbSystemColumnParentECInstanceId | ECDbSystemColumnECPropertyPathId | ECDbSystemColumnECArraryIndex)


END_BENTLEY_SQLITE_EC_NAMESPACE
