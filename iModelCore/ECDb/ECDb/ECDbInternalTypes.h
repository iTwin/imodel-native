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

ECDB_TYPEDEFS(ECDbMap);

ECDB_TYPEDEFS_PTR(ECDbSchemaWriter);
ECDB_TYPEDEFS_PTR(ECDbSchemaReader);
ECDB_TYPEDEFS_PTR (ClassMap);
ECDB_TYPEDEFS_PTR (PropertyMap);
ECDB_TYPEDEFS_PTR(PropertyMapToInLineStruct);
ECDB_TYPEDEFS_PTR(PropertyMapToTable);
ECDB_TYPEDEFS_PTR(MappedTable);
ECDB_TYPEDEFS_PTR(RelationshipClassMap);
ECDB_TYPEDEFS_PTR(RelationshipClassEndTableMap);
ECDB_TYPEDEFS_PTR(RelationshipClassLinkTableMap);
ECDB_TYPEDEFS_PTR(ClassIndexInfo);
ECDB_TYPEDEFS_PTR(StandardKeySpecification);

#define USING_NAMESPACE_BENTLEY_EC using namespace BentleyApi::ECN;

//#define ENABLE_TRIGGER_DEBUGGING

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

enum class ECContainerType
    {
    Schema = 1,
    Class = 2,
    Property = 3,
    RelationshipConstraintSource = 4,
    RelationshipConstraintTarget = 5
    };

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

//#define ECDbKnownColumns::DataColumn  0x0U
//#define ECDbKnownColumns::ECInstanceId  0x1U
//#define ECDbKnownColumns::ECClassId  0x2U
//#define ECDbKnownColumns::ParentECInstanceId  0x4U
//#define ECDbKnownColumns::ECPropertyPathId  0x8U
//#define ECArraryIndex  0x10U
//
//#define ECDbSystemColumns  (ECDbKnownColumns::ECInstanceId | ECDbKnownColumns::ECClassId | ECDbKnownColumns::ParentECInstanceId | ECDbKnownColumns::ECPropertyPathId | ECArraryIndex)
//

END_BENTLEY_SQLITE_EC_NAMESPACE
