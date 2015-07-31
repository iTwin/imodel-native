/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ColumnInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/NonCopyableClass.h>
#include "ECDbInternalTypes.h"
#include "ECDbMap.h"
#include "ECDbSql.h"

ECDB_TYPEDEFS(ColumnInfo);
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

///*=================================================================================**//**
//* @bsiClass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======*/
struct ColumnInfo
{
private:
    bool m_isValid;
    Utf8String m_columnName;
    ECN::PrimitiveType m_columnType;
    bool m_nullable;
    bool m_unique;
    ECDbSqlColumn::Constraint::Collation m_collation;

    ColumnInfo();
    BentleyStatus Initialize(ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString);

public:
    static ColumnInfo Create(ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString);
    bool IsValid() const { return m_isValid; }

    void SetColumnType(ECN::PrimitiveType columnType) { BeAssert(IsValid()); m_columnType = columnType; }

    Utf8CP GetName() const { BeAssert(IsValid()); return m_columnName.c_str(); }
    ECN::PrimitiveType GetColumnType() const { BeAssert(IsValid()); return m_columnType; }
    bool IsNullable() const { BeAssert(IsValid()); return m_nullable; };
    bool IsUnique() const { BeAssert(IsValid()); return m_unique; };
    ECDbSqlColumn::Constraint::Collation GetCollation() const { BeAssert(IsValid()); return m_collation; }
    };

///*=================================================================================**//**
//* @bsiclass                                                     Affan.Khan      11/2011
//+===============+===============+===============+===============+===============+======*/
struct DbMetaDataHelper
    {
    enum class ObjectType
        {
        None,
        Table,
        View,
        Index,
        };
    static ObjectType GetObjectType (Db& db, Utf8CP name)
        {
        BeSQLite::CachedStatementPtr stmt;
        db.GetCachedStatement (stmt, "SELECT type FROM sqlite_master WHERE name =?");
        stmt->BindText (1, name, BeSQLite::Statement::MakeCopy::No);
        if (stmt->Step () == BE_SQLITE_ROW)
            {
            if (BeStringUtilities::Stricmp (stmt->GetValueText (0), "table") == 0)
                return ObjectType::Table;
            if (BeStringUtilities::Stricmp (stmt->GetValueText (0), "view") == 0)
                return ObjectType::View;
            if (BeStringUtilities::Stricmp (stmt->GetValueText (0), "index") == 0)
                return ObjectType::Index;
            }
        return ObjectType::None;
        }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
