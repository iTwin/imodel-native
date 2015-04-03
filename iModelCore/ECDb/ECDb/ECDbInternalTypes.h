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

ECDB_TYPEDEFS(RelationshipClassMapInfo);
ECDB_TYPEDEFS_PTR (ClassMap);
ECDB_TYPEDEFS_PTR (PropertyMap);
ECDB_TYPEDEFS(PropertyMapToColumn);
ECDB_TYPEDEFS_PTR(PropertyMapToInLineStruct);
ECDB_TYPEDEFS_PTR(PropertyMapToTable);
ECDB_TYPEDEFS_PTR(PropertyMapArrayOfPrimitives);
ECDB_TYPEDEFS_PTR(ClassMapInfo);
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
enum class SqlBinaryOperator
    {
    //WIP_ECSQL: use pascal casing
    PLUS,
    MINUS,
    MODULUS,
    DIVIDE,
    MULTIPLY,
    SHIFT_LEFT,
    SHIFT_RIGHT,
    BITWISE_OR,
    BITWISE_AND,
    BITWISE_XOR,
    CONCAT
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class SqlBooleanOperator
    {
    //WIP_ECSQL: use pascal casing
    LE,
    GE,
    LT,
    GT,
    EQ,
    NE,
    OR,
    AND,
    IS,
    IS_NOT,
    IN,
    NOT_IN,
    BETWEEN,
    NOT_BETWEEN,
    LIKE,
    NOT_LIKE
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
enum class SqlUnaryOperator
    {
    //WIP_ECSQL: use pascal casing
    MINUS, PLUS, BITWISE_NOT
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class SqlBooleanUnaryOperator
    {
    //WIP_ECSQL: use pascal casing
    MINUS, PLUS, NOT, BITWISE_NOT
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

enum CreateTableStatus
    {
    CREATE_ECTABLE_Success                      = SUCCESS,
    CREATE_ECTABLE_AlreadyExists                = 1,
    CREATE_ECTABLE_IsEmpty                      = 2,
    CREATE_ECTABLE_Error                        = 3, // Anything greater or equal to this is an error
    CREATE_ECTABLE_SqlFailed                    = 4,
    CREATE_ECTABLE_MapNotFound                  = 5,
    CREATE_ECTABLE_MissingMappedTable           = 6,
    };

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

//It follow SqlLite collate ordering. User can specify this using ECDbPropertyHint.
 enum class Collate
     {
     Default, // Default is really Binary in sqlite. But we will not provide collate for property to sqlite in this case and assume sqlite default.
     Binary, // Compares string data using memcmp(), regardless of text encoding
     NoCase, // The same as binary, except the 26 upper case characters of ASCII are folded to their lower case equivalents before the comparison is performed. Note that only ASCII characters are case folded. SQLite does not attempt to do full UTF case folding due to the size of the tables required.
     RTrim,  // The same as binary, except that trailing space characters are ignored.
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

// BSCA = Bentley_Standard_CustomAttributes (ECSchema)
//    C = ECClass
//    P = ECProperty
//    V = ECPropertyValue
#define BSCAC_ECDbClassHint             L"ECDbClassHint"
#define BSCAC_ECDbClassHasCurrentTimeStamp     L"ClassHasCurrentTimeStampProperty"
#define BSCAC_ECDbSchemaHint            L"ECDbSchemaHint"
#define BSCAP_TablePrefix               L"TablePrefix"
#define BSCAP_DefaultClassMapStrategy   L"DefaultClassMapStrategy"

#define BSCAP_MapStrategy               L"MapStrategy" 
#define BSCAP_MapStrategyOption         L"MapStrategyOption" 
#define BSCAP_TableName                 L"TableName"
#define BSCAP_ECInstanceIdColumn        L"ECIdColumn"
#define BSCAP_Indexes                   L"Indexes"
#define BSCAP_IsUnique                  L"IsUnique"
#define BSCAP_IsNullable                L"IsNullable"
#define BSCAP_Collate                   L"Collate"
#define BSCAP_Collate_Binary            L"Binary"
#define BSCAP_Collate_NoCase            L"NoCase"
#define BSCAP_Collate_RTrim             L"RTrim"

#define BSCAP_Name                      L"Name"
#define BSCAP_Where                     L"Where"
#define BSCAP_Properties                L"Properties"
#define BSCAP_ECDBNOTNULL               "ECDB_NOTNULL"
#define BSCAP_AllowDuplicateRelationships L"AllowDuplicateRelationships"
#define BSCAV_NoHint                    L"NoHint"
#define BSCAV_DoNotMap                  L"DoNotMap"
#define BSCAV_DoNotMapHierarchy         L"DoNotMapHierarchy"
#define BSCAV_InParentTable             L"InParentTable"
#define BSCAV_TablePerHierarchy         L"TablePerHierarchy"
#define BSCAV_TableForThisClass         L"TableForThisClass"
#define BSCAV_TablePerClass             L"TablePerClass"
#define BSCAV_RelationshipSourceTable   L"RelationshipSourceTable"
#define BSCAV_RelationshipTargetTable   L"RelationshipTargetTable"
#define BSCAV_SharedTableForThisClass   L"SharedTableForThisClass"

#define BSCAP_SourceECInstanceIdColumn  L"SourceECIdColumn"
#define BSCAP_TargetECInstanceIdColumn  L"TargetECIdColumn"
#define BSCAP_SourceECClassIdColumn     L"SourceECClassIdColumn"
#define BSCAP_TargetECClassIdColumn     L"TargetECClassIdColumn"

#define BSCAC_ECDbRelationshipClassHint L"ECDbRelationshipClassHint"
#define BSCAP_PreferredDirection        L"PreferredDirection"
#define BSCAV_SourceToTarget            L"SourceToTarget"
#define BSCAV_Bidirectional             L"Bidirectional"
#define BSCAV_TargetToSource            L"TargetToSource"
#define BSCAP_MapToExistingTable        L"MapToExistingTable"
#define BSCAP_ReplaceEmptyTableWithEmptyView        L"ReplaceEmptyTableWithEmptyView"
#define BSCAP_ExcludeFromColumnsReuse L"ExcludeFromColumnsReuse"
#define BSCAC_ECDbPropertyHint          L"ECDbPropertyHint"
#define BSCAP_ColumnName                L"ColumnName"
#define BSCAP_Blob                      L"Blob"

#define ECDB_COL_ECInstanceId           "ECInstanceId"
#define ECDB_COL_ECClassId              "ECClassId"
#define ECDB_COL_ECPropertyPathId       "ECPropertyPathId"
#define ECDB_COL_ParentECInstanceId     "ParentECInstanceId"
#define ECDB_COL_ECArrayIndex           "ECArrayIndex"

#define UTF8_Stricmp(S1,S2)  BeStringUtilities::Stricmp(S1, S2) 

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
