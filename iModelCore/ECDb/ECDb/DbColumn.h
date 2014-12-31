/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbColumn.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

ECDB_TYPEDEFS_PTR(DbColumn);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

typedef bvector<DbColumnCP>             DbColumnList;
typedef bvector<DbColumnPtr>            DbColumnPtrList;

/*=================================================================================**//**
* @bsiProperty                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct ColumnInfo
{
private:
    Utf8String        m_columnName;
    ECN::PrimitiveType m_columnType;
    bool              m_nullable;
    bool              m_unique;
    Collate           m_collate;
    bool              m_virtual;
    int               m_priority;
    void InitializeFromHint(ECN::IECInstanceCP ecdbPropertyHint, ECN::ECPropertyCR ecProperty);

protected:
    ColumnInfo (Utf8CP columnName, ECN::PrimitiveType primitiveType, bool nullable, bool unique, Collate collate);

public:
    ColumnInfo (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, ECN::IECInstanceCP ecdbPropertyHint);
    virtual ~ColumnInfo () {}
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
    Collate           GetCollate()const { return m_collate; }
    static DbResult   LoadMappingInformationFromDb(ColumnInfoR columnInfo, ECN::ECPropertyCR ecProperty, BeSQLite::Db& db);
    void SetColumnName (Utf8CP columnName);
    };

//! DbColumn is not intended as a general representation of a database column, but
//! is specific to mapping ECSchema to db schema, and therefore it has rigid rules, 
//! e.g. for column creation, they can only be created from ECProperties or for ECInstanceIds or ECClassIds, etc.
struct DbColumn : public RefCountedBase, public ColumnInfo
{
private:
    DbColumn(DbColumnCR); // hide copy constructor
    DbColumn(Utf8CP columnName, ECN::PrimitiveType primitiveType, bool nullable, bool unique, Collate collate);

public:
    ~DbColumn () {}
    //! Specify a DbColumn for creating a new table or representing and existing column in a table
    //! @param  columnName
    //! @param  primitiveType  A standard type that maps well to EC, ADO.NET, and ODBC
    //! @param  nullable (defaults to true)
    //! @param  unique   (defaults to false)
    //! @return 
    static DbColumnPtr Create(Utf8CP columnName, ECN::PrimitiveType primitiveType, bool nullable = true, bool unique = false, Collate collate = Collate::Default);
    static DbColumnPtr CreatePrimaryKey ();
    static DbColumnPtr CreatePrimaryKey(Utf8CP column);
    static DbColumnPtr CreateClassId();
    bool IsCompatibleWith(ECN::PrimitiveType expectedColumnType) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
