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
    Utf8String        m_columnName;
    ECN::PrimitiveType m_columnType;
    bool              m_nullable;
    bool              m_unique;
    ECDbSqlColumn::Constraint::Collate           m_collate;
    bool              m_virtual;
    int               m_priority;
    void InitializeFromHint(ECN::IECInstanceCP ecdbPropertyHint, ECN::ECPropertyCR ecProperty);

protected:
    ColumnInfo (Utf8CP columnName, ECN::PrimitiveType primitiveType, bool nullable, bool unique, ECDbSqlColumn::Constraint::Collate collate);

public:
    ColumnInfo (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, ECN::IECInstanceCP ecdbPropertyHint);
    virtual ~ColumnInfo() {}
    int               GetPriority () const  { return m_priority; }
    void              SetPriority (int priority) { m_priority = priority; }
     //! @return ecProperty's name if no explicit info was provided for column name. Never returns nullptr.
    Utf8CP            GetName() const;
    ECN::PrimitiveType GetColumnType() const;
    bool              IsVirtual() const { return m_virtual;}
    void              MakeVirtual() { m_virtual = true;};
    bool              GetNullable() const;
    void              SetNullable(bool nullable){m_nullable = nullable;}
    void              SetUnique(bool unique){m_unique = unique;}
    void              SetColumnType (ECN::PrimitiveType columnType) { m_columnType = columnType; }
    bool              GetUnique() const;
    ECDbSqlColumn::Constraint::Collate           GetCollate ()const { return m_collate; }
    static DbResult   LoadMappingInformationFromDb(ColumnInfo& columnInfo, ECN::ECPropertyCR ecProperty, BeSQLite::Db& db);
    void SetColumnName (Utf8CP columnName);
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
