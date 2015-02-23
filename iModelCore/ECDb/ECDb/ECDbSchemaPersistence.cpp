/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistence.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=================================================================================
//* @bsiclass                                                    Krischan.Eberle  02/2015
//+===============+===============+===============+===============+===============+======
struct SqlClauseBuilder
    {
private:
    Utf8String m_clause;
    Utf8String m_delimiter;

public:
    explicit SqlClauseBuilder (Utf8CP delimiter)
        : m_delimiter (delimiter)
        {}

    //! Adds the specified item to the clause.
    //! If there are already other items in the clause, the 
    //! delimiter specified to the constructor is injected, too.
    //! Escaping tokens has to be done by the caller.
    void AddItem (Utf8CP item)
        {
        if (!m_clause.empty ())
            m_clause.append (m_delimiter).append (" ");
        
        m_clause.append (item);
        }

    bool IsEmpty () const { return m_clause.empty (); }

    Utf8CP ToString () const
        {
        return m_clause.c_str ();
        }
    };


//***************************************************************************************
// ECDbSchemaPersistence
//***************************************************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECSchemaInfo (BeSQLite::Db& db, DbECSchemaInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_Schema (ECSchemaId, Name, DisplayLabel, Description, NamespacePrefix, VersionMajor, VersionMinor, SchemaType) VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECSchemaInfo::COL_ECSchemaId     ) stmt->BindInt64 (1,  info.m_ecSchemaId);
    if (info.ColsInsert & DbECSchemaInfo::COL_Name           ) stmt->BindText (2,  info.m_name, Statement::MakeCopy::No);        
    if (info.ColsInsert & DbECSchemaInfo::COL_DisplayLabel   ) stmt->BindText (3,  info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_Description    ) stmt->BindText (4,  info.m_description, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_NamespacePrefix) stmt->BindText (5,  info.m_namespacePrefix, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_VersionMajor   ) stmt->BindInt (6,  info.m_versionMajor);
    if (info.ColsInsert & DbECSchemaInfo::COL_VersionMinor   ) stmt->BindInt (7,  info.m_versionMinor);
    if (info.ColsInsert & DbECSchemaInfo::COL_SchemaType     ) stmt->BindInt (10, info.m_schemaType);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECSchemaInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECSchemaInfoCR spec)
    {
    SqlClauseBuilder selectClause (",");

    //prepare select list
    if (spec.ColsSelect & DbECSchemaInfo::COL_ECSchemaId) selectClause.AddItem ("ECSchemaId");
    if (spec.ColsSelect & DbECSchemaInfo::COL_Name) selectClause.AddItem ("[Name]");
    if (spec.ColsSelect & DbECSchemaInfo::COL_DisplayLabel) selectClause.AddItem ("DisplayLabel");
    if (spec.ColsSelect & DbECSchemaInfo::COL_Description) selectClause.AddItem ("Description");
    if (spec.ColsSelect & DbECSchemaInfo::COL_NamespacePrefix) selectClause.AddItem ("NamespacePrefix");
    if (spec.ColsSelect & DbECSchemaInfo::COL_VersionMajor) selectClause.AddItem ("VersionMajor");
    if (spec.ColsSelect & DbECSchemaInfo::COL_VersionMinor) selectClause.AddItem ("VersionMinor");
    if (spec.ColsSelect & DbECSchemaInfo::COL_SchemaType) selectClause.AddItem ("SchemaType");
    BeAssert (!selectClause.IsEmpty ());

    Utf8String sql ("SELECT ");
    sql.append (selectClause.ToString ()).append (" FROM ec_Schema");

    //prepare where
    SqlClauseBuilder whereClause ("AND");
    if (spec.ColsWhere & DbECSchemaInfo::COL_ECSchemaId) whereClause.AddItem ("ECSchemaId=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_Name) whereClause.AddItem ("[Name]=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix) whereClause.AddItem ("NamespacePrefix=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMajor) whereClause.AddItem ("VersionMajor=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMinor) whereClause.AddItem ("VersionMinor=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_SchemaType) whereClause.AddItem ("SchemaType=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (spec.ColsWhere & DbECSchemaInfo::COL_ECSchemaId     ) stmt->BindInt64 (nCol++, spec.m_ecSchemaId);  
    if (spec.ColsWhere & DbECSchemaInfo::COL_Name           ) stmt->BindText (nCol++, spec.m_name, Statement::MakeCopy::No);        
    if (spec.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix) stmt->BindText (nCol++, spec.m_namespacePrefix, Statement::MakeCopy::No);    
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMajor   ) stmt->BindInt (nCol++, spec.m_versionMajor);
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMinor   ) stmt->BindInt (nCol++, spec.m_versionMinor);
    if (spec.ColsWhere & DbECSchemaInfo::COL_SchemaType     ) stmt->BindInt (nCol++, spec.m_schemaType);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECSchemaInfoR info,  BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECSchemaInfo::COL_ECSchemaId  )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_ECSchemaId;      nCol++; } else info.m_ecSchemaId   = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_Name        )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_Name;            nCol++; } else info.m_name         = stmt.GetValueText (nCol++);        
        if (info.ColsSelect & DbECSchemaInfo::COL_DisplayLabel)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_DisplayLabel;    nCol++; } else info.m_displayLabel = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_Description )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_Description;     nCol++; } else info.m_description  = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_NamespacePrefix    )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_NamespacePrefix; nCol++; } else info.m_namespacePrefix = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_VersionMajor)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_VersionMajor;    nCol++; } else info.m_versionMajor = stmt.GetValueInt  (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_VersionMinor)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_VersionMinor;    nCol++; } else info.m_versionMinor = stmt.GetValueInt  (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_SchemaType  )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_SchemaType;       nCol++; } else info.m_schemaType =  (PersistedSchemaType)stmt.GetValueInt  (nCol++) ;
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECClassInfo (BeSQLite::Db& db, DbECClassInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_Class (ECClassId, ECSchemaId, Name, DisplayLabel, Description, IsDomainClass, IsStruct, IsCustomAttribute, RelationStrength, RelationStrengthDirection, IsRelationship) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECClassInfo::COL_ECClassId                ) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbECClassInfo::COL_ECSchemaId               ) stmt->BindInt64 (2, info.m_ecSchemaId);
    if (info.ColsInsert & DbECClassInfo::COL_Name                     ) stmt->BindText (3, info.m_name, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECClassInfo::COL_DisplayLabel             ) stmt->BindText (4, info.m_displayLabel, Statement::MakeCopy::No); 
    if (info.ColsInsert & DbECClassInfo::COL_Description              ) stmt->BindText (5, info.m_description, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECClassInfo::COL_IsDomainClass            ) stmt->BindInt (6, info.m_isDomainClass? 1 : 0);
    if (info.ColsInsert & DbECClassInfo::COL_IsStruct                 ) stmt->BindInt (7, info.m_isStruct? 1 : 0);
    if (info.ColsInsert & DbECClassInfo::COL_IsCustomAttribute        ) stmt->BindInt (8, info.m_isCustomAttribute? 1 : 0);
    if (info.ColsInsert & DbECClassInfo::COL_RelationStrength         ) stmt->BindInt (9, info.m_relationStrength);
    if (info.ColsInsert & DbECClassInfo::COL_RelationStrengthDirection) stmt->BindInt (10, ToInt (info.m_relationStrengthDirection));
    if (info.ColsInsert & DbECClassInfo::COL_IsRelationship           ) stmt->BindInt (11, info.m_isRelationship? 1 : 0 );

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECClassInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECClassInfoCR info)
    {
    //prepare select list
    SqlClauseBuilder selectClause (",");
    if (info.ColsSelect & DbECClassInfo::COL_ECClassId) selectClause.AddItem ("ECClassId");
    if (info.ColsSelect & DbECClassInfo::COL_ECSchemaId) selectClause.AddItem ("ECSchemaId");
    if (info.ColsSelect & DbECClassInfo::COL_Name) selectClause.AddItem ("[Name]");
    if (info.ColsSelect & DbECClassInfo::COL_DisplayLabel) selectClause.AddItem ("DisplayLabel");
    if (info.ColsSelect & DbECClassInfo::COL_Description) selectClause.AddItem ("Description");
    if (info.ColsSelect & DbECClassInfo::COL_IsDomainClass) selectClause.AddItem ("IsDomainClass");
    if (info.ColsSelect & DbECClassInfo::COL_IsStruct) selectClause.AddItem ("IsStruct");
    if (info.ColsSelect & DbECClassInfo::COL_IsCustomAttribute) selectClause.AddItem ("IsCustomAttribute");
    if (info.ColsSelect & DbECClassInfo::COL_RelationStrength) selectClause.AddItem ("RelationStrength");
    if (info.ColsSelect & DbECClassInfo::COL_RelationStrengthDirection) selectClause.AddItem ("RelationStrengthDirection");
    if (info.ColsSelect & DbECClassInfo::COL_IsRelationship) selectClause.AddItem ("IsRelationship");

    BeAssert (!selectClause.IsEmpty ());

    Utf8String sql ("SELECT ");
    sql.append (selectClause.ToString ()).append (" FROM ec_Class");

    //prepare where
    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECClassInfo::COL_ECClassId) whereClause.AddItem ("ECClassId=?");
    if (info.ColsWhere & DbECClassInfo::COL_ECSchemaId) whereClause.AddItem ("ECSchemaId=?");
    if (info.ColsWhere & DbECClassInfo::COL_Name) whereClause.AddItem ("[Name]=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass) whereClause.AddItem ("IsDomainClass=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct) whereClause.AddItem ("IsStruct=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute) whereClause.AddItem ("IsCustomAttribute=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECClassInfo::COL_ECClassId                ) stmt->BindInt64      (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECClassInfo::COL_ECSchemaId               ) stmt->BindInt64      (nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECClassInfo::COL_Name                     ) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_Description              ) stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass            ) stmt->BindInt        (nCol++, info.m_isDomainClass? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct                 ) stmt->BindInt        (nCol++, info.m_isStruct? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute        ) stmt->BindInt        (nCol++, info.m_isCustomAttribute? 1 : 0);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECClassInfoR info,  BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECClassInfo::COL_ECClassId                )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_ECClassId;                 nCol++; } else info.m_ecClassId                 = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_ECSchemaId               )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_ECSchemaId;                nCol++; } else info.m_ecSchemaId                = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_Name                     )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_Name;                      nCol++; } else info.m_name                      = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_DisplayLabel             )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_DisplayLabel;              nCol++; } else info.m_displayLabel              = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_Description              )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_Description;               nCol++; } else info.m_description               = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_IsDomainClass            )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_IsDomainClass;             nCol++; } else info.m_isDomainClass             = stmt.GetValueInt  (nCol++) == 1;
        if (info.ColsSelect & DbECClassInfo::COL_IsStruct                 )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_IsStruct;                  nCol++; } else info.m_isStruct                  = stmt.GetValueInt  (nCol++) == 1;
        if (info.ColsSelect & DbECClassInfo::COL_IsCustomAttribute        )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_IsCustomAttribute;         nCol++; } else info.m_isCustomAttribute         = stmt.GetValueInt  (nCol++) == 1;
        if (info.ColsSelect & DbECClassInfo::COL_RelationStrength         )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_RelationStrength;          nCol++; } else info.m_relationStrength          = (StrengthType)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_RelationStrengthDirection)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_RelationStrengthDirection; nCol++; }  else info.m_relationStrengthDirection= ToECRelatedInstanceDirection (stmt.GetValueInt (nCol++));
        if (info.ColsSelect & DbECClassInfo::COL_IsRelationship           )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_IsRelationship;            nCol++; } else info.m_isRelationship            = stmt.GetValueInt  (nCol++) == 1;
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECSchemaInfo (BeSQLite::Db& db, DbECSchemaInfoCR info)
    {
    Utf8String sql ("UPDATE ec_Schema SET ");

    SqlClauseBuilder setClause (",");
    if (info.ColsUpdate & DbECSchemaInfo::COL_ECSchemaId) setClause.AddItem ("ECSchemaId=?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_Name) setClause.AddItem ("[Name]=?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_DisplayLabel) setClause.AddItem ("DisplayLabel=?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_Description) setClause.AddItem ("Description=?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_NamespacePrefix) setClause.AddItem ("NamespacePrefix=?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_VersionMajor) setClause.AddItem ("VersionMajor=?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_VersionMinor) setClause.AddItem ("VersionMinor=?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_SchemaType) setClause.AddItem ("SchemaType=?");

    BeAssert (!setClause.IsEmpty ());
    sql.append (setClause.ToString ());

    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECSchemaInfo::COL_ECSchemaId) whereClause.AddItem ("ECSchemaId=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_Name) whereClause.AddItem ("[Name]=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix) whereClause.AddItem ("NamespacePrefix=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_VersionMajor) whereClause.AddItem ("VersionMajor=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_VersionMinor) whereClause.AddItem ("VersionMinor=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_SchemaType) whereClause.AddItem ("SchemaType=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;

    if (info.ColsUpdate & DbECSchemaInfo::COL_ECSchemaId)
        if (info.ColsNull & DbECSchemaInfo::COL_ECSchemaId) nCol++; else stmt->BindInt64 (nCol++, info.m_ecSchemaId);
    if (info.ColsUpdate & DbECSchemaInfo::COL_Name)
        if (info.ColsNull & DbECSchemaInfo::COL_Name) nCol++; else stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);  
    if (info.ColsUpdate & DbECSchemaInfo::COL_DisplayLabel)
        if (info.ColsNull & DbECSchemaInfo::COL_DisplayLabel) nCol++; else stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECSchemaInfo::COL_Description)
        if (info.ColsNull & DbECSchemaInfo::COL_Description) nCol++; else stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECSchemaInfo::COL_NamespacePrefix)
        if (info.ColsNull & DbECSchemaInfo::COL_NamespacePrefix) nCol++; else stmt->BindText (nCol++, info.m_namespacePrefix, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECSchemaInfo::COL_VersionMajor)
        if (info.ColsNull & DbECSchemaInfo::COL_VersionMajor) nCol++; else stmt->BindInt (nCol++, info.m_versionMajor);
    if (info.ColsUpdate & DbECSchemaInfo::COL_VersionMinor)
        if (info.ColsNull & DbECSchemaInfo::COL_VersionMinor) nCol++; else stmt->BindInt (nCol++, info.m_versionMinor);
    if (info.ColsUpdate & DbECSchemaInfo::COL_SchemaType)
        if (info.ColsNull & DbECSchemaInfo::COL_SchemaType) nCol++; else stmt->BindInt (nCol++, info.m_schemaType);
 
    if (info.ColsWhere & DbECSchemaInfo::COL_ECSchemaId) stmt->BindInt64 (nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECSchemaInfo::COL_Name) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);        
    if (info.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix) stmt->BindText (nCol++, info.m_namespacePrefix, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECSchemaInfo::COL_VersionMajor) stmt->BindInt (nCol++, info.m_versionMajor);
    if (info.ColsWhere & DbECSchemaInfo::COL_VersionMinor) stmt->BindInt (nCol++, info.m_versionMinor);
    if (info.ColsWhere & DbECSchemaInfo::COL_SchemaType) stmt->BindInt (nCol++, info.m_schemaType);
    
    stat = stmt->Step();
    BeAssert (stat != BE_SQLITE_DONE || db.GetModifiedRowCount () > 0);
    return stat;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECClassInfo (BeSQLite::Db& db, DbECClassInfoCR info)
    {
    Utf8String sql ("UPDATE ec_Class SET ");

    SqlClauseBuilder setClause (",");
    if (info.ColsUpdate & DbECClassInfo::COL_ECClassId) setClause.AddItem ("ECClassId=?");
    if (info.ColsUpdate & DbECClassInfo::COL_ECSchemaId) setClause.AddItem ("ECSchemaId=?");
    if (info.ColsUpdate & DbECClassInfo::COL_Name) setClause.AddItem ("[Name]=?");
    if (info.ColsUpdate & DbECClassInfo::COL_DisplayLabel) setClause.AddItem ("DisplayLabel=?");
    if (info.ColsUpdate & DbECClassInfo::COL_Description) setClause.AddItem ("Description=?");
    if (info.ColsUpdate & DbECClassInfo::COL_IsDomainClass) setClause.AddItem ("IsDomainClass=?");
    if (info.ColsUpdate & DbECClassInfo::COL_IsStruct) setClause.AddItem ("IsStruct=?");
    if (info.ColsUpdate & DbECClassInfo::COL_IsCustomAttribute) setClause.AddItem ("IsCustomAttribute=?");
    if (info.ColsUpdate & DbECClassInfo::COL_RelationStrength) setClause.AddItem ("RelationStrength=?");
    if (info.ColsUpdate & DbECClassInfo::COL_RelationStrengthDirection) setClause.AddItem ("RelationStrengthDirection=?");
    if (info.ColsUpdate & DbECClassInfo::COL_IsRelationship) setClause.AddItem ("IsRelationship=?");
    BeAssert (!setClause.IsEmpty ());
    sql.append (setClause.ToString ());

    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECClassInfo::COL_ECClassId) whereClause.AddItem ("ECClassId=?");
    if (info.ColsWhere & DbECClassInfo::COL_ECSchemaId) whereClause.AddItem ("ECSchemaId=?");
    if (info.ColsWhere & DbECClassInfo::COL_Name) whereClause.AddItem ("[Name]=?");
    if (info.ColsWhere & DbECClassInfo::COL_DisplayLabel) whereClause.AddItem ("DisplayLabel=?");
    if (info.ColsWhere & DbECClassInfo::COL_Description) whereClause.AddItem ("Description=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass) whereClause.AddItem ("IsDomainClass=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct) whereClause.AddItem ("IsStruct=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute) whereClause.AddItem ("IsCustomAttribute=?");
    if (info.ColsWhere & DbECClassInfo::COL_RelationStrength) whereClause.AddItem ("RelationStrength=?");
    if (info.ColsWhere & DbECClassInfo::COL_RelationStrengthDirection) whereClause.AddItem ("RelationStrengthDirection=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsRelationship) whereClause.AddItem ("IsRelationship=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;

    if (info.ColsUpdate & DbECClassInfo::COL_ECClassId                )
        if (info.ColsNull & DbECClassInfo::COL_ECClassId) nCol++; else stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsUpdate & DbECClassInfo::COL_ECSchemaId               )
        if (info.ColsNull & DbECClassInfo::COL_ECSchemaId) nCol++; else stmt->BindInt64(nCol++, info.m_ecSchemaId);
    if (info.ColsUpdate & DbECClassInfo::COL_Name                     )
        if (info.ColsNull & DbECClassInfo::COL_Name) nCol++; else stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECClassInfo::COL_DisplayLabel             )
        if (info.ColsNull & DbECClassInfo::COL_DisplayLabel) nCol++; else stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No); 
    if (info.ColsUpdate & DbECClassInfo::COL_Description              )
        if (info.ColsNull & DbECClassInfo::COL_Description) nCol++; else stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECClassInfo::COL_IsDomainClass            )
        if (info.ColsNull & DbECClassInfo::COL_IsDomainClass) nCol++; else stmt->BindInt (nCol++, info.m_isDomainClass? 1 : 0);
    if (info.ColsUpdate & DbECClassInfo::COL_IsStruct                 )
        if (info.ColsNull & DbECClassInfo::COL_IsStruct) nCol++; else stmt->BindInt (nCol++, info.m_isStruct? 1 : 0);
    if (info.ColsUpdate & DbECClassInfo::COL_IsCustomAttribute        )
        if (info.ColsNull & DbECClassInfo::COL_IsCustomAttribute) nCol++; else stmt->BindInt (nCol++, info.m_isCustomAttribute? 1 : 0);
    if (info.ColsUpdate & DbECClassInfo::COL_RelationStrength         )
        if (info.ColsNull & DbECClassInfo::COL_RelationStrength) nCol++; else stmt->BindInt (nCol++, info.m_relationStrength);       
    if (info.ColsUpdate & DbECClassInfo::COL_RelationStrengthDirection)
        if (info.ColsNull & DbECClassInfo::COL_RelationStrengthDirection) nCol++; else stmt->BindInt (nCol++, ToInt (info.m_relationStrengthDirection));     
    if (info.ColsUpdate & DbECClassInfo::COL_IsRelationship           )
        if (info.ColsNull & DbECClassInfo::COL_IsRelationship) nCol++; else stmt->BindInt (nCol++, info.m_isRelationship? 1 : 0 );

    if (info.ColsWhere & DbECClassInfo::COL_ECClassId                ) stmt->BindInt64      (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECClassInfo::COL_ECSchemaId               ) stmt->BindInt64      (nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECClassInfo::COL_Name                     ) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_DisplayLabel             ) stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_Description              ) stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass            ) stmt->BindInt        (nCol++, info.m_isDomainClass? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct                 ) stmt->BindInt        (nCol++, info.m_isStruct? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute        ) stmt->BindInt        (nCol++, info.m_isCustomAttribute? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_RelationStrength         ) stmt->BindInt        (nCol++, info.m_relationStrength);       
    if (info.ColsWhere & DbECClassInfo::COL_RelationStrengthDirection) stmt->BindInt        (nCol++, ToInt (info.m_relationStrengthDirection));     
    if (info.ColsWhere & DbECClassInfo::COL_IsRelationship           ) stmt->BindInt        (nCol++, info.m_isRelationship? 1 : 0 );

    stat = stmt->Step();
    BeAssert (stat != BE_SQLITE_DONE || db.GetModifiedRowCount () > 0);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertBaseClass (BeSQLite::Db& db, DbBaseClassInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_BaseClass (ECClassId, BaseECClassId, ECIndex) VALUES (?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbBaseClassInfo::COL_ECClassId) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbBaseClassInfo::COL_BaseECClassId) stmt->BindInt64 (2, info.m_baseECClassId);
    if (info.ColsInsert & DbBaseClassInfo::COL_ECIndex) stmt->BindInt (3, info.m_ecIndex);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindBaseClassInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbBaseClassInfoCR info)
    {
    //prepare select list
    SqlClauseBuilder selectClause (",");
    if (info.ColsSelect & DbBaseClassInfo::COL_ECClassId) selectClause.AddItem ("ECClassId");
    if (info.ColsSelect & DbBaseClassInfo::COL_BaseECClassId) selectClause.AddItem ("BaseECClassId");
    if (info.ColsSelect & DbBaseClassInfo::COL_ECIndex) selectClause.AddItem ("ECIndex");
    BeAssert (!selectClause.IsEmpty ());

    Utf8String sql ("SELECT ");
    sql.append (selectClause.ToString ()).append (" FROM ec_BaseClass");

    //prepare where
    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbBaseClassInfo::COL_ECClassId) whereClause.AddItem ("ECClassId=?");
    if (info.ColsWhere & DbBaseClassInfo::COL_BaseECClassId) whereClause.AddItem ("BaseECClassId=?");
    if (info.ColsWhere & DbBaseClassInfo::COL_ECIndex) whereClause.AddItem ("ECIndex=?");
    
    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    sql.append (" ORDER BY ECIndex");

    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;
    
    int nCol = 1;
    if (info.ColsWhere & DbBaseClassInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbBaseClassInfo::COL_BaseECClassId) stmt->BindInt64 (nCol++, info.m_baseECClassId);
    if (info.ColsWhere & DbBaseClassInfo::COL_ECIndex) stmt->BindInt (nCol++, info.m_ecIndex);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbBaseClassInfoR info , BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbBaseClassInfo::COL_ECClassId) info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbBaseClassInfo::COL_BaseECClassId) info.m_baseECClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbBaseClassInfo::COL_ECIndex) info.m_ecIndex = stmt.GetValueInt (nCol++);
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECSchemaReferenceInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECSchemaReferenceInfoCR info)
    {
    Utf8String sql ("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause (",");
    if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ECSchemaId) selectClause.AddItem ("ECSchemaId");
    if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ReferenceECSchemaId) selectClause.AddItem ("ReferenceECSchemaId");

    BeAssert (!selectClause.IsEmpty ());

    sql.append (selectClause.ToString ()).append (" FROM ec_SchemaReference");

    //prepare where
    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ECSchemaId) whereClause.AddItem ("ECSchemaId=?");
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ReferenceECSchemaId) whereClause.AddItem ("ReferenceECSchemaId=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ECSchemaId) stmt->BindInt64 (nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ReferenceECSchemaId) stmt->BindInt64 (nCol++, info.m_referenceECSchemaId);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECSchemaReferenceInfoR info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step ()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ECSchemaId) info.m_ecSchemaId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ReferenceECSchemaId) info.m_referenceECSchemaId = stmt.GetValueInt64 (nCol++);
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECPropertyInfo (BeSQLite::Db& db, DbECPropertyInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_Property (ECClassId, ECPropertyId, Name, DisplayLabel, Description, IsArray, TypeCustom, TypeECPrimitive, TypeGeometry, TypeECStruct, ECIndex, IsReadOnly, MinOccurs, MaxOccurs) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECPropertyInfo::COL_ECClassId) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbECPropertyInfo::COL_ECPropertyId) stmt->BindInt64 (2, info.m_ecPropertyId);
    if (info.ColsInsert & DbECPropertyInfo::COL_Name) stmt->BindText (3, info.m_name, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_DisplayLabel) stmt->BindText (4, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_Description) stmt->BindText (5, info.m_description, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_IsArray) stmt->BindInt (6, info.m_isArray ? 1 : 0);
    if (info.ColsInsert & DbECPropertyInfo::COL_TypeCustom) stmt->BindText (7, info.m_typeCustom, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_TypeECPrimitive) stmt->BindInt (8, info.m_typeECPrimitive);
    if (info.ColsInsert & DbECPropertyInfo::COL_TypeGeometry) stmt->BindText (9, info.m_typeGeometry, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_TypeECStruct) stmt->BindInt64 (10, info.m_typeECStruct);
    if (info.ColsInsert & DbECPropertyInfo::COL_ECIndex) stmt->BindInt (11, info.m_ecIndex);
    if (info.ColsInsert & DbECPropertyInfo::COL_IsReadOnly) stmt->BindInt (12, info.m_isReadOnly ? 1 : 0);

    if (info.m_isArray)
        {
        if (info.ColsInsert & DbECPropertyInfo::COL_MinOccurs) stmt->BindInt (13, (int32_t) info.m_minOccurs);
        if (info.ColsInsert & DbECPropertyInfo::COL_MaxOccurs) stmt->BindInt (14, (int32_t) info.m_maxOccurs);
        }

    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECPropertyInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECPropertyInfoCR info)
    {
    Utf8String sql ("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause (",");
    if (info.ColsSelect & DbECPropertyInfo::COL_ECClassId) selectClause.AddItem ("ECClassId");
    if (info.ColsSelect & DbECPropertyInfo::COL_ECPropertyId) selectClause.AddItem ("ECPropertyId");
    if (info.ColsSelect & DbECPropertyInfo::COL_Name) selectClause.AddItem ("[Name]");
    if (info.ColsSelect & DbECPropertyInfo::COL_DisplayLabel) selectClause.AddItem ("DisplayLabel");
    if (info.ColsSelect & DbECPropertyInfo::COL_Description) selectClause.AddItem ("Description");
    if (info.ColsSelect & DbECPropertyInfo::COL_IsArray) selectClause.AddItem ("IsArray");
    if (info.ColsSelect & DbECPropertyInfo::COL_TypeCustom) selectClause.AddItem ("TypeCustom");
    if (info.ColsSelect & DbECPropertyInfo::COL_TypeECPrimitive) selectClause.AddItem ("TypeECPrimitive");
    if (info.ColsSelect & DbECPropertyInfo::COL_TypeGeometry) selectClause.AddItem ("TypeGeometry");
    if (info.ColsSelect & DbECPropertyInfo::COL_TypeECStruct) selectClause.AddItem ("TypeECStruct");
    if (info.ColsSelect & DbECPropertyInfo::COL_ECIndex) selectClause.AddItem ("ECIndex");
    if (info.ColsSelect & DbECPropertyInfo::COL_IsReadOnly) selectClause.AddItem ("IsReadOnly");
    if (info.ColsSelect & DbECPropertyInfo::COL_MinOccurs) selectClause.AddItem ("MinOccurs");
    if (info.ColsSelect & DbECPropertyInfo::COL_MaxOccurs) selectClause.AddItem ("MaxOccurs");

    BeAssert (!selectClause.IsEmpty ());
    sql.append (selectClause.ToString ()).append (" FROM ec_Property");

    //prepare where
    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECPropertyInfo::COL_ECClassId) whereClause.AddItem ("ECClassId=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_ECPropertyId) whereClause.AddItem ("ECPropertyId=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) whereClause.AddItem ("[Name]=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    sql.append (" ORDER BY ECIndex");

    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (stat != BE_SQLITE_OK)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECPropertyInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECPropertyInfo::COL_ECPropertyId) stmt->BindInt64 (nCol++, info.m_ecPropertyId);
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step(DbECPropertyInfoR info , BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECPropertyInfo::COL_ECClassId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_ECClassId;       nCol++; } else info.m_ecClassId = stmt.GetValueInt64 (nCol++);            
        if (info.ColsSelect & DbECPropertyInfo::COL_ECPropertyId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_ECPropertyId;    nCol++; } else info.m_ecPropertyId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_Name)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_Name;            nCol++; } else info.m_name = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_DisplayLabel)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_DisplayLabel;    nCol++; } else info.m_displayLabel = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_Description)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_Description;     nCol++; } else info.m_description = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_IsArray)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_IsArray;         nCol++; } else info.m_isArray = stmt.GetValueInt  (nCol++) == 1;
        if (info.ColsSelect & DbECPropertyInfo::COL_TypeCustom)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_TypeCustom;      nCol++; } else info.m_typeCustom = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_TypeECPrimitive)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_TypeECPrimitive; nCol++; } else info.m_typeECPrimitive = (ECN::PrimitiveType)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_TypeGeometry)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_TypeGeometry;    nCol++; } else info.m_typeGeometry = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_TypeECStruct)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_TypeECStruct;    nCol++; } else info.m_typeECStruct = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_ECIndex)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_ECIndex;         nCol++; } else info.m_ecIndex = stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_IsReadOnly)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_IsReadOnly;      nCol++; } else info.m_isReadOnly = stmt.GetValueInt (nCol++) == 1;
        if (info.ColsSelect & DbECPropertyInfo::COL_MinOccurs)
            if (stmt.IsColumnNull (nCol)) { info.ColsNull |= DbECPropertyInfo::COL_MinOccurs;      nCol++; } else info.m_minOccurs = (uint32_t)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_MaxOccurs)
            if (stmt.IsColumnNull (nCol)) { info.ColsNull |= DbECPropertyInfo::COL_MaxOccurs;      nCol++; } else info.m_maxOccurs = (uint32_t)stmt.GetValueInt (nCol++);

        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECPropertyInfo (BeSQLite::Db& db, DbECPropertyInfoCR info)
    {
    Utf8String sql ("UPDATE ec_Property SET ");

    SqlClauseBuilder setClause (",");
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECClassId) setClause.AddItem ("ECClassId=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECPropertyId) setClause.AddItem ("ECPropertyId=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_Name) setClause.AddItem ("[Name]=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_DisplayLabel) setClause.AddItem ("DisplayLabel=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_Description) setClause.AddItem ("Description=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_IsArray) setClause.AddItem ("IsArray=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeCustom) setClause.AddItem ("TypeCustom=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeECPrimitive) setClause.AddItem ("TypeECPrimitive=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeGeometry) setClause.AddItem ("TypeGeometry=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeECStruct) setClause.AddItem ("TypeECStruct=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECIndex) setClause.AddItem ("ECIndex=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_IsReadOnly) setClause.AddItem ("IsReadOnly=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_MinOccurs) setClause.AddItem ("MinOccurs=?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_MaxOccurs) setClause.AddItem ("MaxOccurs=?");

    BeAssert (!setClause.IsEmpty ());
    sql.append (setClause.ToString ());

    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECPropertyInfo::COL_ECClassId) whereClause.AddItem ("ECClassId=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_ECPropertyId) whereClause.AddItem ("ECPropertyId=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) whereClause.AddItem ("[Name]=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_DisplayLabel) whereClause.AddItem ("DisplayLabel=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_Description) whereClause.AddItem ("Description=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_IsArray) whereClause.AddItem ("IsArray=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeCustom) whereClause.AddItem ("TypeCustom=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeECPrimitive) whereClause.AddItem ("TypeECPrimitive=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeGeometry) whereClause.AddItem ("TypeGeometry=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeECStruct) whereClause.AddItem ("TypeECStruct=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_ECIndex) whereClause.AddItem ("ECIndex=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_IsReadOnly) whereClause.AddItem ("IsReadOnly=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECClassId)
        if (info.ColsNull & DbECPropertyInfo::COL_ECClassId)  nCol++; else stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECPropertyId)
        if (info.ColsNull & DbECPropertyInfo::COL_ECPropertyId)  nCol++; else stmt->BindInt64 (nCol++, info.m_ecPropertyId);
    if (info.ColsUpdate & DbECPropertyInfo::COL_Name)
        if (info.ColsNull & DbECPropertyInfo::COL_Name) nCol++; else stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_DisplayLabel)
        if (info.ColsNull & DbECPropertyInfo::COL_DisplayLabel) nCol++; else stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_Description)
        if (info.ColsNull & DbECPropertyInfo::COL_Description)  nCol++; else stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_IsArray)
        if (info.ColsNull & DbECPropertyInfo::COL_IsArray) nCol++; else stmt->BindInt (nCol++, info.m_isArray ? 1 : 0);
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeCustom)
        if (info.ColsNull & DbECPropertyInfo::COL_TypeCustom) nCol++; else stmt->BindText (nCol++, info.m_typeCustom, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeECPrimitive)
        if (info.ColsNull & DbECPropertyInfo::COL_TypeECPrimitive) nCol++; else stmt->BindInt (nCol++, info.m_typeECPrimitive);
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeGeometry)
        if (info.ColsNull & DbECPropertyInfo::COL_TypeGeometry) nCol++; else stmt->BindText (nCol++, info.m_typeGeometry, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeECStruct)
        if (info.ColsNull & DbECPropertyInfo::COL_TypeECStruct) nCol++; else stmt->BindInt64 (nCol++, info.m_typeECStruct);
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECIndex)
        if (info.ColsNull & DbECPropertyInfo::COL_ECIndex) nCol++; else stmt->BindInt (nCol++, info.m_ecIndex);
    if (info.ColsUpdate & DbECPropertyInfo::COL_IsReadOnly)
        if (info.ColsNull & DbECPropertyInfo::COL_IsReadOnly) nCol++; else stmt->BindInt (nCol++, info.m_isReadOnly ? 1 : 0);
    if (info.ColsUpdate & DbECPropertyInfo::COL_MinOccurs)
        if (info.ColsNull & DbECPropertyInfo::COL_MinOccurs) nCol++; else stmt->BindInt (nCol++, (int32_t) info.m_minOccurs);
    if (info.ColsUpdate & DbECPropertyInfo::COL_MaxOccurs)
        if (info.ColsNull & DbECPropertyInfo::COL_MaxOccurs) nCol++; else stmt->BindInt (nCol++, (int32_t) info.m_maxOccurs);

    if (info.ColsWhere & DbECPropertyInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECPropertyInfo::COL_ECPropertyId) stmt->BindInt64 (nCol++, info.m_ecPropertyId);
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_DisplayLabel) stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_Description) stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_IsArray) stmt->BindInt (nCol++, info.m_isArray ? 1 : 0);
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeCustom) stmt->BindText (nCol++, info.m_typeCustom, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeECPrimitive) stmt->BindInt (nCol++, info.m_typeECPrimitive);
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeGeometry) stmt->BindText (nCol++, info.m_typeGeometry, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeECStruct) stmt->BindInt64 (nCol++, info.m_typeECStruct);
    if (info.ColsWhere & DbECPropertyInfo::COL_ECIndex) stmt->BindInt (nCol++, info.m_ecIndex);
    if (info.ColsWhere & DbECPropertyInfo::COL_IsReadOnly) stmt->BindInt (nCol++, info.m_isReadOnly ? 1 : 0);

    stat = stmt->Step ();
    BeAssert (stat != BE_SQLITE_DONE || db.GetModifiedRowCount () > 0);
    return stat;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECRelationConstraintInfo (BeSQLite::Db& db, DbECRelationshipConstraintInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_RelationshipConstraint (ECClassId, ECRelationshipEnd, CardinalityLowerLimit, CardinalityUpperLimit, RoleLabel, IsPolymorphic) VALUES (?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_ECClassId) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) stmt->BindInt (2, info.m_ecRelationshipEnd);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) stmt->BindInt (3, info.m_cardinalityLowerLimit);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) stmt->BindInt (4, info.m_cardinalityUpperLimit);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_RoleLabel) stmt->BindText (5, info.m_roleLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_IsPolymorphic) stmt->BindInt (6, info.m_isPolymorphic ? 1 : 0);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECRelationConstraintInfo (BeSQLite::Db& db, DbECRelationshipConstraintInfoCR info)
    {
    Utf8String sql ("UPDATE ec_RelationshipConstraint SET ");

    SqlClauseBuilder setClause (",");

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_ECClassId) 
        setClause.AddItem ("ECClassId=?");
    
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) 
        setClause.AddItem ("ECRelationshipEnd=?");
    
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_RoleLabel) 
        setClause.AddItem ("RoleLabel=?");
    
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_IsPolymorphic) 
        setClause.AddItem ("IsPolymorphic=?");
    
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) 
        setClause.AddItem ("CardinalityLowerLimit=?");

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) 
        setClause.AddItem ("CardinalityUpperLimit=?");

    BeAssert (!setClause.IsEmpty ());
    sql.append (setClause.ToString ());

    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECClassId)
        whereClause.AddItem ("ECClassId=?");

    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd)
        whereClause.AddItem ("ECRelationshipEnd=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol= 1;
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_ECClassId)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_ECClassId) 
            nCol++; 
        else 
            stmt->BindInt64 (nCol++, info.m_ecClassId);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) 
            nCol++; 
        else 
            stmt->BindInt (nCol++, info.m_ecRelationshipEnd);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_RoleLabel)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_RoleLabel) 
            nCol++; 
        else 
            stmt->BindText (nCol++, info.m_roleLabel.c_str(), Statement::MakeCopy::No);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_IsPolymorphic)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_IsPolymorphic) 
            nCol++; 
        else 
            stmt->BindInt (nCol++, info.m_isPolymorphic);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) 
            nCol++; 
        else 
            stmt->BindInt (nCol++, info.m_cardinalityLowerLimit);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) 
            nCol++; 
        else 
            stmt->BindInt (nCol++, info.m_cardinalityUpperLimit);

    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECClassId)
        stmt->BindInt64 (nCol++, info.m_ecClassId);

    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) 
        stmt->BindInt   (nCol++, info.m_ecRelationshipEnd);

    stat = stmt->Step();
    BeAssert (stat != BE_SQLITE_DONE || db.GetModifiedRowCount () > 0);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECRelationshipConstraintInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECRelationshipConstraintInfoCR info)
    {
    Utf8String sql ("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause (",");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_ECClassId) selectClause.AddItem ("ECClassId");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) selectClause.AddItem ("ECRelationshipEnd");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) selectClause.AddItem ("CardinalityLowerLimit");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) selectClause.AddItem ("CardinalityUpperLimit");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RoleLabel) selectClause.AddItem ("RoleLabel");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_IsPolymorphic) selectClause.AddItem ("IsPolymorphic");
    
    BeAssert (!selectClause.IsEmpty ());
    sql.append (selectClause.ToString ()).append (" FROM ec_RelationshipConstraint");


    //prepare where
    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECClassId) whereClause.AddItem ("ECClassId=?");
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) whereClause.AddItem ("ECRelationshipEnd=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) stmt->BindInt (nCol++, info.m_ecRelationshipEnd);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECRelationshipConstraintInfoR info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step ()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_ECClassId) info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) info.m_ecRelationshipEnd = (ECN::ECRelationshipEnd)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) info.m_cardinalityLowerLimit = stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) info.m_cardinalityUpperLimit = stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RoleLabel)
            if (stmt.IsColumnNull (nCol)) { info.ColsNull |= DbECRelationshipConstraintInfo::COL_RoleLabel; nCol++; }
            else info.m_roleLabel = stmt.GetValueText (nCol++);
            if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_IsPolymorphic) info.m_isPolymorphic = stmt.GetValueInt (nCol++) == 1;
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECRelationConstraintClassInfo (BeSQLite::Db& db, DbECRelationshipConstraintClassInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_RelationshipConstraintClass (ECClassId, ECRelationshipEnd, RelationECClassId) VALUES (?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECRelationshipConstraintClassInfo::COL_ECClassId) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) stmt->BindInt (2, info.m_ecRelationshipEnd);
    if (info.ColsInsert & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) stmt->BindInt64 (3, info.m_relationECClassId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECRelationConstraintClassInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECRelationshipConstraintClassInfoCR info)
    {
    Utf8String sql ("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause (",");
    if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ECClassId) selectClause.AddItem ("ECClassId");
    if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) selectClause.AddItem ("ECRelationshipEnd");
    if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) selectClause.AddItem ("RelationECClassId");

    BeAssert (!selectClause.IsEmpty ());
    sql.append (selectClause.ToString ()).append (" FROM ec_RelationshipConstraintClass");

    //prepare where
    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECClassId) whereClause.AddItem ("ECClassId=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) whereClause.AddItem ("ECRelationshipEnd=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) whereClause.AddItem ("RelationECClassId=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) stmt->BindInt (nCol++, info.m_ecRelationshipEnd);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) stmt->BindInt64 (nCol++, info.m_relationECClassId);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECRelationshipConstraintClassInfoR info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step ()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ECClassId) info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) info.m_ecRelationshipEnd = (ECRelationshipEnd) stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) info.m_relationECClassId = stmt.GetValueInt64 (nCol++);
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECRelationshipConstraintClass (BeSQLite::Db& db, DbECRelationshipConstraintClassInfoCR info)
    {
    Utf8String sql ("UPDATE ec_RelationshipConstraintClass SET ");

    BeAssert (false && "UpdateECRelationshipConstraintClass seems to be not implemented correctly: No SET clause defined for SQL UPDATE statement.");
    SqlClauseBuilder setClause (",");

    BeAssert (!setClause.IsEmpty ());
    sql.append (setClause.ToString ());

    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECClassId) whereClause.AddItem ("ECClassId=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) whereClause.AddItem ("ECRelationshipEnd=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) whereClause.AddItem ("RelationECClassId=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) stmt->BindInt (nCol++, info.m_ecRelationshipEnd);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) stmt->BindInt64 (nCol++, info.m_relationECClassId);

    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertCustomAttributeInfo (BeSQLite::Db& db, DbCustomAttributeInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_CustomAttribute (ContainerId, ContainerType, ECClassId, [Index], Instance) VALUES(?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbCustomAttributeInfo::COL_ContainerId) stmt->BindInt64 (1, info.m_containerId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_ContainerType) stmt->BindInt (2, info.m_containerType);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_ECClassId) stmt->BindInt64 (3, info.m_ecClassId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_Index) stmt->BindInt (4, info.m_index);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_Instance)
        {
        DbResult stat = stmt->BindText (5, info.GetCaInstanceXml (), Statement::MakeCopy::No);
        POSTCONDITION (stat == BE_SQLITE_OK, stat);
        }

    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindCustomAttributeInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbCustomAttributeInfoCR info)
    {
    Utf8String sql ("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause (",");

    //prepare select list
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerId) selectClause.AddItem ("ContainerId");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerType) selectClause.AddItem ("ContainerType");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ECClassId) selectClause.AddItem ("ECClassId");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_Index) selectClause.AddItem ("[Index]");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_Instance) selectClause.AddItem ("Instance");

    BeAssert (!selectClause.IsEmpty ());
    sql.append (selectClause.ToString ()).append (" FROM ec_CustomAttribute");

    //prepare where
    SqlClauseBuilder whereClause ("AND");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId)
        whereClause.AddItem ("ContainerId=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType)
        whereClause.AddItem ("ContainerType=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ECClassId)
        whereClause.AddItem ("ECClassId=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_Index)
        whereClause.AddItem ("[Index]=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    sql.append (" ORDER BY [Index]");

    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (stat != BE_SQLITE_OK)
        return stat;

    int nCol = 1;
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId)            stmt->BindInt64 (nCol++, info.m_containerId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType)          stmt->BindInt (nCol++, info.m_containerType);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ECClassId)              stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_Index)                  stmt->BindInt (nCol++, info.m_index);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateCustomAttributeInfo (BeSQLite::Db& db, DbCustomAttributeInfoCR info)
    {
    Utf8String sql ("UPDATE ec_CustomAttribute SET ");

    SqlClauseBuilder setClause (",");
    if (info.ColsUpdate & DbCustomAttributeInfo::COL_Instance) setClause.AddItem ("Instance=?");

    BeAssert (!setClause.IsEmpty ());
    sql.append (setClause.ToString ());

    SqlClauseBuilder whereClause ("AND");

    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId)
        whereClause.AddItem ("ContainerId=?");

    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType)
        whereClause.AddItem ("ContainerType=?");

    if (info.ColsWhere & DbCustomAttributeInfo::COL_ECClassId)
        whereClause.AddItem ("ECClassId=?");

    if (info.ColsWhere & DbCustomAttributeInfo::COL_Index)
        whereClause.AddItem ("[Index]=?");

    if (!whereClause.IsEmpty ())
        sql.append (" WHERE ").append (whereClause.ToString ());

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;

    if (~info.ColsNull & info.ColsUpdate & DbCustomAttributeInfo::COL_Instance)
        {
        stat = stmt->BindText (nCol++, info.GetCaInstanceXml (), Statement::MakeCopy::No);
        POSTCONDITION (stat == BE_SQLITE_OK, stat);
        }
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId) stmt->BindInt64 (nCol++, info.m_containerId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType) stmt->BindInt (nCol++, info.m_containerType);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_Index) stmt->BindInt (nCol++, info.m_index);

    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbCustomAttributeInfoR info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_ContainerId; nCol++; } else info.m_containerId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerType) 
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_ContainerType; nCol++; } else info.m_containerType = (ECContainerType)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_ECClassId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_ECClassId; nCol++; } else info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_Index) 
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_Index; nCol++; } else info.m_index = stmt.GetValueInt(nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_Instance)
            if (stmt.IsColumnNull(nCol)) 
                { 
                info.ColsNull |= DbCustomAttributeInfo::COL_Instance; 
                nCol++; 
                } 
            else
                {
                //TODO: is it ensured that the info object is not used after the next call to ECDbSchemaPersistence::Step?
                Utf8CP caInstanceXml = stmt.GetValueText (nCol);
                info.SetCaInstanceXml (caInstanceXml);
                }
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECRelationConstraintClassPropertyInfo (BeSQLite::Db& db, DbECRelationshipConstraintClassPropertyInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_RelationshipConstraintClassProperty (ECClassId, ECRelationshipEnd, RelationECClassId, RelationECPropertyId) VALUES (?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, info.m_ecClassId);
    stmt->BindInt (2, info.m_ecRelationshipEnd);
    stmt->BindInt64 (3, info.m_relationECClassId);
    stmt->BindInt64 (4, info.m_relationECPropertyId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECSchemaReferenceInfo (BeSQLite::Db& db, DbECSchemaReferenceInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_SchemaReference (ECSchemaId, ReferenceECSchemaId) VALUES(?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, info.m_ecSchemaId);
    stmt->BindInt64 (2, info.m_referenceECSchemaId);
    return stmt->Step ();
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECClass (BeSQLite::Db& db, ECClassCR ecClass)
    {
    if (ecClass.HasId()) //This is unsafe but since we donot delete ecclass any class that hasId does exist in db
        return true;

    auto classId = GetECClassIdBySchemaName(db , 
                        Utf8String(ecClass.GetSchema().GetName().c_str()).c_str(),
                        Utf8String(ecClass.GetName().c_str()).c_str());

    return classId != 0;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECSchemaReference (BeSQLite::Db& db, ECSchemaId ecPrimarySchemaId, ECSchemaId ecReferenceSchemaId)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECSchemaId FROM ec_SchemaReference WHERE ECSchemaId = ? AND ReferenceECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, ecPrimarySchemaId);
    stmt->BindInt64 (2, ecReferenceSchemaId);
    return stmt->Step () == BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECClassId (Utf8StringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, Utf8StringR className, ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT s.Name, s.VersionMajor, s.VersionMinor, c.Name  FROM ec_Class c INNER JOIN ec_Schema s ON s.ECSchemaId = c.ECSchemaId WHERE ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecClassId);

    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return stat;

    schemaName = stmt->GetValueText (0);
    versionMajor = (uint32_t) stmt->GetValueInt (1);
    versionMinor = (uint32_t) stmt->GetValueInt (2);
    className = stmt->GetValueText (3);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECClassId (DbECClassEntryR key, ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT Name, ECSchemaId  FROM ec_Class WHERE ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecClassId);

    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return stat;

    key.m_className = stmt->GetValueText (0);
    key.m_ecSchemaId = stmt->GetValueInt64 (1);
    key.m_ecClassId = ecClassId;
    key.m_resolvedECClass = nullptr;
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECSchemaId (DbECSchemaEntryR key, ECSchemaId ecSchemaId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT S.ECSchemaId, S.Name, S.VersionMajor, S.VersionMinor, (SELECT COUNT(*) FROM ec_Class C WHERE S.ECSchemaId = C.ECSchemaID) FROM ec_Schema S WHERE S.ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecSchemaId);

    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return stat;

    key.m_ecSchemaId = stmt->GetValueInt64 (0);
    key.m_schemaName = stmt->GetValueText (1);
    key.m_versionMajor = (uint32_t) stmt->GetValueInt (2);
    key.m_versionMinor = (uint32_t) stmt->GetValueInt (3);
    key.m_nClassesInSchema = (uint32_t) stmt->GetValueInt (4);
    key.m_nClassesLoaded = 0;
    key.m_resolvedECSchema = nullptr;
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetPrimaryECSchemas (ECSchemaKeyListR schemaKeys, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT S.ECSchemaId, S.Name, S.VersionMajor, S.VersionMinor, COUNT(*) FROM ec_Schema S INNER JOIN ec_Class C ON S.ECSchemaId = C.ECSchemaID WHERE S.SchemaType=? GROUP BY S.ECSchemaId, S.Name, S.VersionMajor, S.VersionMinor");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt (1, PERSISTEDSCHEMATYPE_Primary);

    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        {
        DbECSchemaEntry key;
        key.m_ecSchemaId = stmt->GetValueInt64 (0);
        key.m_schemaName = stmt->GetValueText (1);
        key.m_versionMajor = (uint32_t) stmt->GetValueInt (2);
        key.m_versionMinor = (uint32_t) stmt->GetValueInt (3);
        key.m_nClassesInSchema = (uint32_t) stmt->GetValueInt (4);
        key.m_nClassesLoaded = 0;
        key.m_resolvedECSchema = nullptr;
        schemaKeys.push_back (key);
        }

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECSchemaId (Utf8StringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, ECSchemaId ecSchemaId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT Name, VersionMajor, VersionMinor FROM ec_Schema WHERE ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;
    stmt->BindInt64 (1, ecSchemaId);

    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return stat;

    schemaName = stmt->GetValueText (0);
    versionMajor = (uint32_t) stmt->GetValueInt (1);
    versionMinor = (uint32_t) stmt->GetValueInt (2);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetECSchemaKeys (ECSchemaKeys& keys, BeSQLite::Db& db)
    {
    keys.clear ();
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECSchemaId, Name, VersionMajor, VersionMinor, DisplayLabel FROM ec_Schema ORDER BY Name");
    if (BE_SQLITE_OK != stat)
        return stat;

    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        {
        keys.push_back (
            ECSchemaKey (
            stmt->GetValueInt64 (0),
            stmt->GetValueText (1),
            (uint32_t) stmt->GetValueInt (2),
            (uint32_t) stmt->GetValueInt (3),
            stmt->IsColumnNull (4) ? (Utf8CP)nullptr : stmt->GetValueText (4)));
        }
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteECClass (ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_Class WHERE ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecClassId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteCustomAttribute (ECContainerId containerId, ECContainerType containerType, ECClassId customAttributeClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_CustomAttribute WHERE (ContainerId = ? AND ContainerType = ? AND ECClassId = ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, containerId);
    stmt->BindInt (2, static_cast<int>(containerType));
    stmt->BindInt64 (3, customAttributeClassId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteECSchema (ECSchemaId ecSchemaId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_Schema WHERE ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecSchemaId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECSchemaWithId (BeSQLite::Db& db, ECSchemaId ecSchemaId)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECSchemaId FROM ec_Schema WHERE ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, ecSchemaId);
    return stmt->Step () == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistence::GetECSchemaId (BeSQLite::Db& db, Utf8CP schemaName)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT ECSchemaId FROM ec_Schema WHERE Name = ?");
    if (BE_SQLITE_OK != stat)
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    stat = stmt->Step();
    if (BE_SQLITE_ROW != stat)
        return 0LL;

    return stmt->GetValueInt64(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistence::GetECSchemaId (BeSQLite::Db& db, ECSchemaCR ecSchema)
    {
    Utf8String schemaName(ecSchema.GetName());
    return GetECSchemaId (db, schemaName.c_str());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetDerivedECClasses (ECClassIdListR classIds, ECClassId baseClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECClassId FROM ec_BaseClass WHERE BaseECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, baseClassId);
    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        classIds.push_back (stmt->GetValueInt64 (0));
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::IsCustomAttributeDefined (BeSQLite::Db& db, ECClassId classId, ECContainerId containerId, ECContainerType containerType)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT COUNT(*) FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, containerId);
    stmt->BindInt (2, containerType);
    stmt->BindInt64 (3, classId);

    if (stmt->Step () == BE_SQLITE_ROW)
        return stmt->GetValueInt64 (0) > 0;

    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteECProperty (ECPropertyId propertyId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_Property WHERE ECPropertyId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, propertyId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetBaseECClasses (ECClassIdListR baseClassIds, ECClassId ecClassId, BeSQLite::Db& db)  // WIP_FNV: take a name, instead... and modify the where clause
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT BaseECClassId FROM ec_BaseClass WHERE ECClassId = ? ORDER BY ECIndex");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecClassId);
    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        baseClassIds.push_back (stmt->GetValueInt64 (0));
    
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetECClassKeys (ECClassKeys& keys, ECSchemaId schemaId, BeSQLite::Db& db) // WIP_FNV: take a name, instead... and modify the where clause
    {
    keys.clear ();
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECClassId, Name, DisplayLabel FROM ec_Class WHERE ECSchemaId = ? ORDER BY Name");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, schemaId);
    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        {
        keys.push_back (
            ECClassKey (
            stmt->GetValueInt64 (0),
            stmt->GetValueText (1),
            (stmt->IsColumnNull (2) ? (Utf8CP)nullptr : stmt->GetValueText (2))));
        }
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle        04/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
int ECDbSchemaPersistence::ToInt (ECN::ECRelatedInstanceDirection direction)
    {
    switch (direction)
        {
        case ECRelatedInstanceDirection::Forward:
            return 1;
        case ECRelatedInstanceDirection::Backward:
            return 2;
        default:
            BeAssert (false && "ECRelatedInstanceDirection has a new value. ECDbSchemaPersistence::ToInt needs to adopt to it.");
            return -1;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle        04/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECRelatedInstanceDirection ECDbSchemaPersistence::ToECRelatedInstanceDirection (int relatedInstanceDirection)
    {
    BeAssert ((relatedInstanceDirection == 1 || relatedInstanceDirection == 2) &&"Integer cannot be converted to ECRelatedInstanceDirection.");

    if (relatedInstanceDirection == 2)
        return ECRelatedInstanceDirection::Backward;

    return ECRelatedInstanceDirection::Forward;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetClassesMappedToTable (std::vector<ECClassId>& classIds, ECDbSqlTable const& table, BeSQLite::Db& db)
    {
    BeSQLite::Statement stmt;
    auto stat = stmt.Prepare(db,         
        "SELECT DISTINCT ec_ClassMap.ECClassId  FROM ec_ClassMap"
        "   INNER JOIN ec_PropertyMap ON ec_PropertyMap.[ClassMapId] = ec_ClassMap.[Id]"
        "   INNER JOIN ec_Column ON ec_Column.[Id] = ec_PropertyMap.[ColumnId]"
        "   INNER JOIN ec_Table ON ec_Table.[Id] = ec_Column.[TableId]"
        "   INNER JOIN ec_Class ON ec_Class.[ECClassId] = ec_ClassMap.ECClassId"
        "   WHERE  ec_Table.[Id] = ? AND ec_Class.IsRelationship = 0"
        );

    if (BE_SQLITE_OK != stat)
        return stat;

    stmt.BindInt64 (1, table.GetId());

    while ((stat = stmt.Step ()) == BE_SQLITE_ROW)
        {
        auto ecClassId = stmt.GetValueInt64 (0);
        classIds.push_back (ecClassId);
        }
    
    return stat;
    }    

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::IsECSchemaMapped (bool* schemaNotFound, ECN::ECSchemaCR ecSchema, BeSQLite::Db& db)
    {
    if (schemaNotFound)
        *schemaNotFound = true;

    if (!ecSchema.HasId ())
        {
        auto ecSchemaId = GetECSchemaId (db,
            Utf8String (ecSchema.GetName ().c_str ()).c_str ());
        if (ecSchemaId == 0)
            return false;

        const_cast<ECSchemaR>(ecSchema).SetId (ecSchemaId);
        }

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT COUNT(*) FROM ec_ClassMap INNER JOIN  ec_Class ON ecClass.ECClassId = ec_ClassMap.ECClassId WHERE ec_Class.ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, ecSchema.GetId ());
    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return false;

    if (schemaNotFound)
        *schemaNotFound = false;

    return stmt->GetValueInt (0) > 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
//static
ECClassId ECDbSchemaPersistence::GetECClassIdBySchemaName (BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECClassId FROM ec_Class JOIN ec_Schema WHERE ec_Class.ECSchemaId = ec_Schema.ECSchemaId AND ec_Schema.Name = ? AND ec_Class.Name = ?");
    if (BE_SQLITE_OK != stat)
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    stmt->BindText (2, className, Statement::MakeCopy::No);
    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return 0LL;

    return stmt->GetValueInt64 (0);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                          muhammad.zaighum          10/2014
//---------------------------------------------------------------------------------------
//static
ECClassId ECDbSchemaPersistence::GetECClassIdBySchemaNameSpacePrefix(BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT ECClassId FROM ec_Class JOIN ec_Schema WHERE ec_Class.ECSchemaId = ec_Schema.ECSchemaId AND ec_Schema.NamespacePrefix = ? AND ec_Class.Name = ?");
    if (BE_SQLITE_OK != stat)
        return 0LL;

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
    stmt->BindText(2, className, Statement::MakeCopy::No);
    stat = stmt->Step();
    if (BE_SQLITE_ROW != stat)
        return 0LL;

    return stmt->GetValueInt64(0);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId ECDbSchemaPersistence::GetECPropertyId (BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT ECPropertyId FROM ec_Property INNER JOIN ec_Class ON ec_Property.ECClassId = ec_Class.ECClassId INNER JOIN ec_Schema WHERE ec_Class.ECSchemaId = ec_Schema.ECSchemaId AND ec_Schema.Name = ? AND ec_Class.Name = ? AND ec_Property.Name = ?");
    if (BE_SQLITE_OK != stat)
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    stmt->BindText (2, className, Statement::MakeCopy::No);
    stmt->BindText (3, propertyName, Statement::MakeCopy::No);

    stat = stmt->Step();
    if (BE_SQLITE_ROW != stat)
        return 0LL;

    return stmt->GetValueInt64(0);
    }


//***************************** DbBuffer ************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::~DbBuffer ()
    {
    Reset ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Dettach ()
    {
    m_ownsBuffer = false;
    Reset ();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Reset ()
    {
    if (m_ownsBuffer && m_data != nullptr)
        free (m_data);

    m_data = nullptr;
    m_length = 0;
    m_ownsBuffer = false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::DbBuffer ()
:m_data (nullptr), m_length (0), m_ownsBuffer (false)
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::DbBuffer (size_t length)
    {
    Allocate (length);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbBuffer::Allocate (size_t length)
    {
    Reset ();
    m_data = (Byte*) malloc (length);
    if (m_data != nullptr)
        {
        m_length = length;
        m_ownsBuffer = true;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool  DbBuffer::SetData (Byte* data, size_t length, bool createCopy)
    {
    Reset ();
    if (createCopy == true)
        {
        if (Allocate (length) == true)
            {
            memcpy (m_data, data, length);
            return true;
            }
        else
            return false;
        }
    m_data = data;
    m_length = length;
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Resize (size_t length)
    {
    if (m_data != nullptr)
        {
        size_t minLength = length > m_length ? m_length : length;
        Byte* data = (Byte*) malloc (length);
        memcpy (data, m_data, minLength);
        Reset ();
        m_data = data;
        m_ownsBuffer = true;
        m_length = length;
        }
    else
        Allocate (length);
    }

//******************** DbCustomAttributeInfo *******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbCustomAttributeInfo::SerializeCaInstance 
(
IECInstanceR caInstance
)
    {
    Utf8String caXml;
    InstanceWriteStatus serializerStat = caInstance.WriteToXmlString (caXml, 
                    //don't write XML description header as we only store an XML fragment
                    false, 
                    //store instance id for the rare cases where the client specified one.
                    true);

    if (serializerStat != INSTANCE_WRITE_STATUS_Success)
        {
        LOG.errorv (L"Serializing custom attribute instance to XML failed with error code: %d", serializerStat);
        BeAssert (false && L"Serializing custom attribute instance to XML failed.");
        return ERROR;
        }

    m_caInstanceXml = caXml;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbCustomAttributeInfo::DeserializeCaInstance 
(
IECInstancePtr& caInstance,
ECSchemaCR schema
) const
    {
    ECInstanceReadContextPtr readContext = ECInstanceReadContext::CreateContext (schema);
    IECInstancePtr deserializedCa = nullptr;
    InstanceReadStatus deserializeStat = IECInstance::ReadFromXmlString (deserializedCa, GetCaInstanceXml (), *readContext);

    if (deserializeStat != INSTANCE_READ_STATUS_Success)
        {
        LOG.errorv (L"Deserializing custom attribute instance from XML failed with error code: %d", deserializeStat);
        BeAssert (false && L"Deserializing custom attribute instance from XML failed.");
        return ERROR;
        }

    caInstance = deserializedCa;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
void DbCustomAttributeInfo::SetCaInstanceXml (Utf8CP caInstanceXml)
    {
    m_caInstanceXml = Utf8String (caInstanceXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP DbCustomAttributeInfo::GetCaInstanceXml () const
    {
    return m_caInstanceXml.c_str ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbPropertyPathId ECDbSchemaPersistence::GetECPropertyPathId (ECPropertyId rootECPropertyId, Utf8CP accessString, BeSQLite::Db& db)
    {
    Statement stmt;
    auto stat = stmt.Prepare (db, "SELECT Id FROM ec_PropertyPath WHERE RootECPropertyId = ? AND AccessString = ?");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to prepare statement");
        return 0;
        }

    stmt.BindInt64 (1, rootECPropertyId);
    stmt.BindText (2, accessString, Statement::MakeCopy::No);
    if (stmt.Step () == BE_SQLITE_ROW)
        return stmt.GetValueInt64 (0);

    return 0;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
