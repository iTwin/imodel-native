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
// @bsiclass                                                    Krischan.Eberle  02/2015
//+===============+===============+===============+===============+===============+======
struct SqlClauseBuilder
    {
private:
    Utf8String m_clause;
    Utf8String m_delimiter;

public:
    explicit SqlClauseBuilder (Utf8CP delimiter) : m_delimiter (delimiter) {}

    //! Adds the specified item to the clause.
    //! If there are already other items in the clause, the 
    //! delimiter specified to the constructor is injected, too.
    //! Escaping tokens has to be done by the caller.
    void AddItem (Utf8CP item)
        {
        if (!m_clause.empty ())
            m_clause.append(" ").append (m_delimiter).append (" ");
        
        m_clause.append (item);
        }

    bool IsEmpty () const { return m_clause.empty (); }

    Utf8CP ToString () const { return m_clause.c_str (); }
    };


//***************************************************************************************
// ECDbSchemaPersistence
//***************************************************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECSchema(BeSQLite::Db& db, DbECSchemaInfo const& info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO ec_Schema (Id, Name, DisplayLabel, Description, NamespacePrefix, VersionMajor, VersionMinor) VALUES(?, ?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECSchemaInfo::COL_Id) stmt->BindInt64(1, info.m_ecSchemaId);
    if (info.ColsInsert & DbECSchemaInfo::COL_Name) stmt->BindText(2, info.m_name, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_DisplayLabel) stmt->BindText(3, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_Description) stmt->BindText(4, info.m_description, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_NamespacePrefix) stmt->BindText(5, info.m_namespacePrefix, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_VersionMajor) stmt->BindInt(6, info.m_versionMajor);
    if (info.ColsInsert & DbECSchemaInfo::COL_VersionMinor) stmt->BindInt(7, info.m_versionMinor);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECSchema(BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECSchemaInfo const& spec)
    {
    SqlClauseBuilder selectClause(",");

    //prepare select list
    if (spec.ColsSelect & DbECSchemaInfo::COL_Id) selectClause.AddItem("Id");
    if (spec.ColsSelect & DbECSchemaInfo::COL_Name) selectClause.AddItem("Name");
    if (spec.ColsSelect & DbECSchemaInfo::COL_DisplayLabel) selectClause.AddItem("DisplayLabel");
    if (spec.ColsSelect & DbECSchemaInfo::COL_Description) selectClause.AddItem("Description");
    if (spec.ColsSelect & DbECSchemaInfo::COL_NamespacePrefix) selectClause.AddItem("NamespacePrefix");
    if (spec.ColsSelect & DbECSchemaInfo::COL_VersionMajor) selectClause.AddItem("VersionMajor");
    if (spec.ColsSelect & DbECSchemaInfo::COL_VersionMinor) selectClause.AddItem("VersionMinor");
    BeAssert(!selectClause.IsEmpty());

    Utf8String sql("SELECT ");
    sql.append(selectClause.ToString()).append(" FROM ec_Schema");

    //prepare where
    SqlClauseBuilder whereClause("AND");
    if (spec.ColsWhere & DbECSchemaInfo::COL_Id) whereClause.AddItem("Id=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_Name) whereClause.AddItem("Name=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix) whereClause.AddItem("NamespacePrefix=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMajor) whereClause.AddItem("VersionMajor=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMinor) whereClause.AddItem("VersionMinor=?");

    if (!whereClause.IsEmpty())
        sql.append(" WHERE ").append(whereClause.ToString());

    auto stat = db.GetCachedStatement(stmt, sql.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (spec.ColsWhere & DbECSchemaInfo::COL_Id) stmt->BindInt64(nCol++, spec.m_ecSchemaId);
    if (spec.ColsWhere & DbECSchemaInfo::COL_Name) stmt->BindText(nCol++, spec.m_name, Statement::MakeCopy::No);
    if (spec.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix) stmt->BindText(nCol++, spec.m_namespacePrefix, Statement::MakeCopy::No);
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMajor) stmt->BindInt(nCol++, spec.m_versionMajor);
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMinor) stmt->BindInt(nCol++, spec.m_versionMinor);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step(DbECSchemaInfo& info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECSchemaInfo::COL_Id  )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_Id;              nCol++; } else info.m_ecSchemaId   = stmt.GetValueInt64(nCol++);
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
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECClass(BeSQLite::Db& db, DbECClassInfo const& info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO ec_Class (Id, SchemaId, Name, DisplayLabel, Description, IsDomainClass, IsStruct, IsCustomAttribute, RelationStrength, RelationStrengthDirection, IsRelationship) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECClassInfo::COL_Id) stmt->BindInt64(1, info.m_ecClassId);
    if (info.ColsInsert & DbECClassInfo::COL_SchemaId) stmt->BindInt64(2, info.m_ecSchemaId);
    if (info.ColsInsert & DbECClassInfo::COL_Name) stmt->BindText(3, info.m_name, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECClassInfo::COL_DisplayLabel) stmt->BindText(4, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECClassInfo::COL_Description) stmt->BindText(5, info.m_description, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECClassInfo::COL_IsDomainClass) stmt->BindInt(6, info.m_isDomainClass ? 1 : 0);
    if (info.ColsInsert & DbECClassInfo::COL_IsStruct) stmt->BindInt(7, info.m_isStruct ? 1 : 0);
    if (info.ColsInsert & DbECClassInfo::COL_IsCustomAttribute) stmt->BindInt(8, info.m_isCustomAttribute ? 1 : 0);
    if (info.ColsInsert & DbECClassInfo::COL_RelationStrength) stmt->BindInt(9, info.m_relationStrength);
    if (info.ColsInsert & DbECClassInfo::COL_RelationStrengthDirection) stmt->BindInt(10, ToInt(info.m_relationStrengthDirection));
    if (info.ColsInsert & DbECClassInfo::COL_IsRelationship) stmt->BindInt(11, info.m_isRelationship ? 1 : 0);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECClass(BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECClassInfo const& info)
    {
    //prepare select list
    SqlClauseBuilder selectClause(",");
    if (info.ColsSelect & DbECClassInfo::COL_Id) selectClause.AddItem("Id");
    if (info.ColsSelect & DbECClassInfo::COL_SchemaId) selectClause.AddItem("SchemaId");
    if (info.ColsSelect & DbECClassInfo::COL_Name) selectClause.AddItem("Name");
    if (info.ColsSelect & DbECClassInfo::COL_DisplayLabel) selectClause.AddItem("DisplayLabel");
    if (info.ColsSelect & DbECClassInfo::COL_Description) selectClause.AddItem("Description");
    if (info.ColsSelect & DbECClassInfo::COL_IsDomainClass) selectClause.AddItem("IsDomainClass");
    if (info.ColsSelect & DbECClassInfo::COL_IsStruct) selectClause.AddItem("IsStruct");
    if (info.ColsSelect & DbECClassInfo::COL_IsCustomAttribute) selectClause.AddItem("IsCustomAttribute");
    if (info.ColsSelect & DbECClassInfo::COL_RelationStrength) selectClause.AddItem("RelationStrength");
    if (info.ColsSelect & DbECClassInfo::COL_RelationStrengthDirection) selectClause.AddItem("RelationStrengthDirection");
    if (info.ColsSelect & DbECClassInfo::COL_IsRelationship) selectClause.AddItem("IsRelationship");

    BeAssert(!selectClause.IsEmpty());

    Utf8String sql("SELECT ");
    sql.append(selectClause.ToString()).append(" FROM ec_Class");

    //prepare where
    SqlClauseBuilder whereClause("AND");
    if (info.ColsWhere & DbECClassInfo::COL_Id) whereClause.AddItem("Id=?");
    if (info.ColsWhere & DbECClassInfo::COL_SchemaId) whereClause.AddItem("SchemaId=?");
    if (info.ColsWhere & DbECClassInfo::COL_Name) whereClause.AddItem("Name=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass) whereClause.AddItem("IsDomainClass=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct) whereClause.AddItem("IsStruct=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute) whereClause.AddItem("IsCustomAttribute=?");

    if (!whereClause.IsEmpty())
        sql.append(" WHERE ").append(whereClause.ToString());

    auto stat = db.GetCachedStatement(stmt, sql.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECClassInfo::COL_Id) stmt->BindInt64(nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECClassInfo::COL_SchemaId) stmt->BindInt64(nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECClassInfo::COL_Name) stmt->BindText(nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_Description) stmt->BindText(nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass) stmt->BindInt(nCol++, info.m_isDomainClass ? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct) stmt->BindInt(nCol++, info.m_isStruct ? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute) stmt->BindInt(nCol++, info.m_isCustomAttribute ? 1 : 0);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECClassInfo& info,  BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECClassInfo::COL_Id                )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_Id;                        nCol++; } else info.m_ecClassId                 = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_SchemaId               )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_SchemaId;                nCol++; } else info.m_ecSchemaId                = stmt.GetValueInt64(nCol++);
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
DbResult ECDbSchemaPersistence::InsertBaseClass(BeSQLite::Db& db, DbBaseClassInfo const& info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO ec_BaseClass (ClassId, BaseClassId, Ordinal) VALUES (?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbBaseClassInfo::COL_ClassId) stmt->BindInt64(1, info.m_ecClassId);
    if (info.ColsInsert & DbBaseClassInfo::COL_BaseClassId) stmt->BindInt64(2, info.m_baseECClassId);
    if (info.ColsInsert & DbBaseClassInfo::COL_Ordinal) stmt->BindInt(3, info.m_ecIndex);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindBaseClass(BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbBaseClassInfo const& info)
    {
    //prepare select list
    SqlClauseBuilder selectClause(",");
    if (info.ColsSelect & DbBaseClassInfo::COL_ClassId) selectClause.AddItem("ClassId");
    if (info.ColsSelect & DbBaseClassInfo::COL_BaseClassId) selectClause.AddItem("BaseClassId");
    if (info.ColsSelect & DbBaseClassInfo::COL_Ordinal) selectClause.AddItem("Ordinal");
    BeAssert(!selectClause.IsEmpty());

    Utf8String sql("SELECT ");
    sql.append(selectClause.ToString()).append(" FROM ec_BaseClass");

    //prepare where
    SqlClauseBuilder whereClause("AND");
    if (info.ColsWhere & DbBaseClassInfo::COL_ClassId) whereClause.AddItem("ClassId=?");
    if (info.ColsWhere & DbBaseClassInfo::COL_BaseClassId) whereClause.AddItem("BaseClassId=?");
    if (info.ColsWhere & DbBaseClassInfo::COL_Ordinal) whereClause.AddItem("Ordinal=?");

    if (!whereClause.IsEmpty())
        sql.append(" WHERE ").append(whereClause.ToString());

    sql.append(" ORDER BY Ordinal");

    auto stat = db.GetCachedStatement(stmt, sql.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbBaseClassInfo::COL_ClassId) stmt->BindInt64(nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbBaseClassInfo::COL_BaseClassId) stmt->BindInt64(nCol++, info.m_baseECClassId);
    if (info.ColsWhere & DbBaseClassInfo::COL_Ordinal) stmt->BindInt(nCol++, info.m_ecIndex);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbBaseClassInfo& info , BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbBaseClassInfo::COL_ClassId) info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbBaseClassInfo::COL_BaseClassId) info.m_baseECClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbBaseClassInfo::COL_Ordinal) info.m_ecIndex = stmt.GetValueInt (nCol++);
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECSchemaReference(BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECSchemaReferenceInfo const& info)
    {
    Utf8String sql("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause(",");
    if (info.ColsSelect & DbECSchemaReferenceInfo::COL_SchemaId) selectClause.AddItem("SchemaId");
    if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ReferencedSchemaId) selectClause.AddItem("ReferencedSchemaId");

    BeAssert(!selectClause.IsEmpty());

    sql.append(selectClause.ToString()).append(" FROM ec_SchemaReference");

    //prepare where
    SqlClauseBuilder whereClause("AND");
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_SchemaId) whereClause.AddItem("SchemaId=?");
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ReferencedSchemaId) whereClause.AddItem("ReferencedSchemaId=?");

    if (!whereClause.IsEmpty())
        sql.append(" WHERE ").append(whereClause.ToString());

    auto stat = db.GetCachedStatement(stmt, sql.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_SchemaId) stmt->BindInt64(nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ReferencedSchemaId) stmt->BindInt64(nCol++, info.m_referencedECSchemaId);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECSchemaReferenceInfo& info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step ()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECSchemaReferenceInfo::COL_SchemaId) info.m_ecSchemaId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ReferencedSchemaId) info.m_referencedECSchemaId = stmt.GetValueInt64 (nCol++);
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECProperty(BeSQLite::Db& db, DbECPropertyInfo const& info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO ec_Property (Id, ClassId, Name, DisplayLabel, Description, IsArray, PrimitiveType, StructType, Ordinal, IsReadonly, MinOccurs, MaxOccurs) VALUES(?,?,?,?,?,?,?,?,?,?,?,?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECPropertyInfo::COL_Id) stmt->BindInt64(1, info.m_ecPropertyId);
    if (info.ColsInsert & DbECPropertyInfo::COL_ClassId) stmt->BindInt64(2, info.m_ecClassId);
    if (info.ColsInsert & DbECPropertyInfo::COL_Name) stmt->BindText(3, info.m_name, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_DisplayLabel) stmt->BindText(4, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_Description) stmt->BindText(5, info.m_description, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_IsArray) stmt->BindInt(6, info.m_isArray ? 1 : 0);
    if (info.ColsInsert & DbECPropertyInfo::COL_PrimitiveType) stmt->BindInt(7, info.m_primitiveType);
    if (info.ColsInsert & DbECPropertyInfo::COL_StructType) stmt->BindInt64(8, info.m_structType);
    if (info.ColsInsert & DbECPropertyInfo::COL_Ordinal) stmt->BindInt(9, info.m_ordinal);
    if (info.ColsInsert & DbECPropertyInfo::COL_IsReadonly) stmt->BindInt(10, info.m_isReadOnly ? 1 : 0);

    if (info.m_isArray)
        {
        if (info.ColsInsert & DbECPropertyInfo::COL_MinOccurs) stmt->BindInt(11, (int32_t) info.m_minOccurs);
        if (info.ColsInsert & DbECPropertyInfo::COL_MaxOccurs) stmt->BindInt(12, (int32_t) info.m_maxOccurs);
        }

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECProperty(BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECPropertyInfo const& info)
    {
    Utf8String sql("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause(",");
    if (info.ColsSelect & DbECPropertyInfo::COL_Id) selectClause.AddItem("Id");
    if (info.ColsSelect & DbECPropertyInfo::COL_ClassId) selectClause.AddItem("ClassId");
    if (info.ColsSelect & DbECPropertyInfo::COL_Name) selectClause.AddItem("Name");
    if (info.ColsSelect & DbECPropertyInfo::COL_DisplayLabel) selectClause.AddItem("DisplayLabel");
    if (info.ColsSelect & DbECPropertyInfo::COL_Description) selectClause.AddItem("Description");
    if (info.ColsSelect & DbECPropertyInfo::COL_IsArray) selectClause.AddItem("IsArray");
    if (info.ColsSelect & DbECPropertyInfo::COL_PrimitiveType) selectClause.AddItem("PrimitiveType");
    if (info.ColsSelect & DbECPropertyInfo::COL_StructType) selectClause.AddItem("StructType");
    if (info.ColsSelect & DbECPropertyInfo::COL_Ordinal) selectClause.AddItem("Ordinal");
    if (info.ColsSelect & DbECPropertyInfo::COL_IsReadonly) selectClause.AddItem("IsReadonly");
    if (info.ColsSelect & DbECPropertyInfo::COL_MinOccurs) selectClause.AddItem("MinOccurs");
    if (info.ColsSelect & DbECPropertyInfo::COL_MaxOccurs) selectClause.AddItem("MaxOccurs");

    BeAssert(!selectClause.IsEmpty());
    sql.append(selectClause.ToString()).append(" FROM ec_Property");

    //prepare where
    SqlClauseBuilder whereClause("AND");
    if (info.ColsWhere & DbECPropertyInfo::COL_ClassId) whereClause.AddItem("ClassId=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_Id) whereClause.AddItem("Id=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) whereClause.AddItem("Name=?");

    if (!whereClause.IsEmpty())
        sql.append(" WHERE ").append(whereClause.ToString());

    sql.append(" ORDER BY Ordinal");

    auto stat = db.GetCachedStatement(stmt, sql.c_str());
    if (stat != BE_SQLITE_OK)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECPropertyInfo::COL_ClassId) stmt->BindInt64(nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECPropertyInfo::COL_Id) stmt->BindInt64(nCol++, info.m_ecPropertyId);
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) stmt->BindText(nCol++, info.m_name, Statement::MakeCopy::No);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step(DbECPropertyInfo& info , BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;

        if (info.ColsSelect & DbECPropertyInfo::COL_Id)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_Id;              nCol++; } else info.m_ecPropertyId = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_ClassId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_ClassId;       nCol++; } else info.m_ecClassId = stmt.GetValueInt64 (nCol++);            
        if (info.ColsSelect & DbECPropertyInfo::COL_Name)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_Name;            nCol++; } else info.m_name = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_DisplayLabel)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_DisplayLabel;    nCol++; } else info.m_displayLabel = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_Description)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_Description;     nCol++; } else info.m_description = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_IsArray)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_IsArray;         nCol++; } else info.m_isArray = stmt.GetValueInt  (nCol++) == 1;
        if (info.ColsSelect & DbECPropertyInfo::COL_PrimitiveType)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_PrimitiveType; nCol++; } else info.m_primitiveType = (ECN::PrimitiveType)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_StructType)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_StructType;    nCol++; } else info.m_structType = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_Ordinal)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_Ordinal;         nCol++; } else info.m_ordinal = stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_IsReadonly)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_IsReadonly;      nCol++; } else info.m_isReadOnly = stmt.GetValueInt (nCol++) == 1;
        if (info.ColsSelect & DbECPropertyInfo::COL_MinOccurs)
            if (stmt.IsColumnNull (nCol)) { info.ColsNull |= DbECPropertyInfo::COL_MinOccurs;      nCol++; } else info.m_minOccurs = (uint32_t)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_MaxOccurs)
            if (stmt.IsColumnNull (nCol)) { info.ColsNull |= DbECPropertyInfo::COL_MaxOccurs;      nCol++; } else info.m_maxOccurs = (uint32_t)stmt.GetValueInt (nCol++);

        }
    return r;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECRelationshipConstraint(BeSQLite::Db& db, DbECRelationshipConstraintInfo const& info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraint (RelationshipClassId, RelationshipEnd, CardinalityLowerLimit, CardinalityUpperLimit, RoleLabel, IsPolymorphic) VALUES (?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_RelationshipClassId) stmt->BindInt64(1, info.m_relationshipClassId);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_RelationshipEnd) stmt->BindInt(2, info.m_ecRelationshipEnd);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) stmt->BindInt(3, info.m_cardinalityLowerLimit);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) stmt->BindInt(4, info.m_cardinalityUpperLimit);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_RoleLabel) stmt->BindText(5, info.m_roleLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_IsPolymorphic) stmt->BindInt(6, info.m_isPolymorphic ? 1 : 0);

    return stmt->Step();
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECRelationshipConstraint(BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECRelationshipConstraintInfo const& info)
    {
    Utf8String sql("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause(",");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RelationshipClassId) selectClause.AddItem("RelationshipClassId");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RelationshipEnd) selectClause.AddItem("RelationshipEnd");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) selectClause.AddItem("CardinalityLowerLimit");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) selectClause.AddItem("CardinalityUpperLimit");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RoleLabel) selectClause.AddItem("RoleLabel");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_IsPolymorphic) selectClause.AddItem("IsPolymorphic");

    BeAssert(!selectClause.IsEmpty());
    sql.append(selectClause.ToString()).append(" FROM ec_RelationshipConstraint");


    //prepare where
    SqlClauseBuilder whereClause("AND");
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_RelationshipClassId) whereClause.AddItem("RelationshipClassId=?");
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_RelationshipEnd) whereClause.AddItem("RelationshipEnd=?");

    if (!whereClause.IsEmpty())
        sql.append(" WHERE ").append(whereClause.ToString());

    auto stat = db.GetCachedStatement(stmt, sql.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_RelationshipClassId) stmt->BindInt64(nCol++, info.m_relationshipClassId);
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_RelationshipEnd) stmt->BindInt(nCol++, info.m_ecRelationshipEnd);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step(DbECRelationshipConstraintInfo& info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RelationshipClassId) info.m_relationshipClassId = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RelationshipEnd) info.m_ecRelationshipEnd = (ECN::ECRelationshipEnd)stmt.GetValueInt(nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) info.m_cardinalityLowerLimit = stmt.GetValueInt(nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) info.m_cardinalityUpperLimit = stmt.GetValueInt(nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RoleLabel)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECRelationshipConstraintInfo::COL_RoleLabel; nCol++; }
            else info.m_roleLabel = stmt.GetValueText(nCol++);
            if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_IsPolymorphic) info.m_isPolymorphic = stmt.GetValueInt(nCol++) == 1;
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECRelationshipConstraintClass(BeSQLite::Db& db, DbECRelationshipConstraintClassInfo const& info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraintClass (RelationshipClassId, RelationshipEnd, ClassId) VALUES (?,?,?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECRelationshipConstraintClassInfo::COL_RelationshipClassId) stmt->BindInt64(1, info.m_relationshipClassId);
    if (info.ColsInsert & DbECRelationshipConstraintClassInfo::COL_RelationshipEnd) stmt->BindInt(2, info.m_ecRelationshipEnd);
    if (info.ColsInsert & DbECRelationshipConstraintClassInfo::COL_ConstraintClassId) stmt->BindInt64(3, info.m_constraintClassId);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECRelationshipConstraintClass(BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECRelationshipConstraintClassInfo const& info)
    {
    Utf8String sql("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause(",");
    if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_RelationshipClassId) selectClause.AddItem("RelationshipClassId");
    if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_RelationshipEnd) selectClause.AddItem("RelationshipEnd");
    if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ConstraintClassId) selectClause.AddItem("ClassId");

    BeAssert(!selectClause.IsEmpty());
    sql.append(selectClause.ToString()).append(" FROM ec_RelationshipConstraintClass");

    //prepare where
    SqlClauseBuilder whereClause("AND");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationshipClassId) whereClause.AddItem("RelationshipClassId=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationshipEnd) whereClause.AddItem("RelationshipEnd=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ConstraintClassId) whereClause.AddItem("ClassId=?");

    if (!whereClause.IsEmpty())
        sql.append(" WHERE ").append(whereClause.ToString());

    auto stat = db.GetCachedStatement(stmt, sql.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationshipClassId) stmt->BindInt64(nCol++, info.m_relationshipClassId);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationshipEnd) stmt->BindInt(nCol++, info.m_ecRelationshipEnd);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ConstraintClassId) stmt->BindInt64(nCol++, info.m_constraintClassId);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step(DbECRelationshipConstraintClassInfo& info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_RelationshipClassId) info.m_relationshipClassId = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_RelationshipEnd) info.m_ecRelationshipEnd = (ECRelationshipEnd) stmt.GetValueInt(nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ConstraintClassId) info.m_constraintClassId = stmt.GetValueInt64(nCol++);
        }

    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertCustomAttribute(BeSQLite::Db& db, DbCustomAttributeInfo const& info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_CustomAttribute (ContainerId, ContainerType, ClassId, Ordinal, Instance) VALUES(?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbCustomAttributeInfo::COL_ContainerId) stmt->BindInt64 (1, info.m_containerId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_ContainerType) stmt->BindInt (2, (int) info.m_containerType);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_ClassId) stmt->BindInt64 (3, info.m_ecClassId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_Ordinal) stmt->BindInt (4, info.m_index);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_Instance)
        {
        DbResult stat = stmt->BindText (5, info.GetCaInstanceXml (), Statement::MakeCopy::No);
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindCustomAttribute(BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbCustomAttributeInfo const& info)
    {
    Utf8String sql("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause(",");

    //prepare select list
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerId) selectClause.AddItem("ContainerId");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerType) selectClause.AddItem("ContainerType");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ClassId) selectClause.AddItem("ClassId");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_Ordinal) selectClause.AddItem("Ordinal");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_Instance) selectClause.AddItem("Instance");

    BeAssert(!selectClause.IsEmpty());
    sql.append(selectClause.ToString()).append(" FROM ec_CustomAttribute");

    //prepare where
    SqlClauseBuilder whereClause("AND");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId)
        whereClause.AddItem("ContainerId=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType)
        whereClause.AddItem("ContainerType=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ClassId)
        whereClause.AddItem("ClassId=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_Ordinal)
        whereClause.AddItem("Ordinal=?");

    if (!whereClause.IsEmpty())
        sql.append(" WHERE ").append(whereClause.ToString());

    sql.append(" ORDER BY Ordinal");

    auto stat = db.GetCachedStatement(stmt, sql.c_str());
    if (stat != BE_SQLITE_OK)
        return stat;

    int nCol = 1;
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId) stmt->BindInt64(nCol++, info.m_containerId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType) stmt->BindInt(nCol++, (int) info.m_containerType);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ClassId) stmt->BindInt64(nCol++, info.m_ecClassId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_Ordinal) stmt->BindInt(nCol++, info.m_index);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbCustomAttributeInfo& info, BeSQLite::Statement& stmt)
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
        if (info.ColsSelect & DbCustomAttributeInfo::COL_ClassId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_ClassId; nCol++; } else info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_Ordinal) 
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_Ordinal; nCol++; } else info.m_index = stmt.GetValueInt(nCol++);
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
DbResult ECDbSchemaPersistence::InsertECRelationshipConstraintClassKeyProperty(BeSQLite::Db& db, DbECRelationshipConstraintClassKeyPropertyInfo const& info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraintClassKeyProperty (RelationshipClassId, ConstraintClassId, RelationshipEnd,KeyPropertyName) VALUES (?,?,?,?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64(1, info.m_relationECClassId);
    stmt->BindInt64(2, info.m_constraintClassId);
    stmt->BindInt(3, info.m_ecRelationshipEnd);
    stmt->BindText(4, info.m_keyPropertyName.c_str(), Statement::MakeCopy::No);
    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECSchemaReference(BeSQLite::Db& db, DbECSchemaReferenceInfo const& info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_SchemaReference (SchemaId, ReferencedSchemaId) VALUES(?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, info.m_ecSchemaId);
    stmt->BindInt64 (2, info.m_referencedECSchemaId);
    return stmt->Step ();
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECClass(BeSQLite::Db& db, ECClassCR ecClass)
    {
    if (ecClass.HasId()) //This is unsafe but since we donot delete ecclass any class that hasId does exist in db
        return true;

    const ECClassId classId = GetECClassIdBySchemaName(db,
                                            Utf8String(ecClass.GetSchema().GetName().c_str()).c_str(),
                                            Utf8String(ecClass.GetName().c_str()).c_str());

    return classId != 0ULL;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECSchemaReference (BeSQLite::Db& db, ECSchemaId ecPrimarySchemaId, ECSchemaId ecReferencedSchemaId)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT SchemaId FROM ec_SchemaReference WHERE SchemaId = ? AND ReferencedSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, ecPrimarySchemaId);
    stmt->BindInt64 (2, ecReferencedSchemaId);
    return stmt->Step () == BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECClassId(Utf8StringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, Utf8StringR className, ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT s.Name, s.VersionMajor, s.VersionMinor, c.Name FROM ec_Class c INNER JOIN ec_Schema s ON s.Id = c.SchemaId WHERE c.Id = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64(1, ecClassId);

    stat = stmt->Step();
    if (BE_SQLITE_ROW != stat)
        return stat;

    schemaName = stmt->GetValueText(0);
    versionMajor = (uint32_t) stmt->GetValueInt(1);
    versionMinor = (uint32_t) stmt->GetValueInt(2);
    className = stmt->GetValueText(3);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECClassId(DbECClassEntry& key, ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT Name, SchemaId  FROM ec_Class WHERE Id = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64(1, ecClassId);

    stat = stmt->Step();
    if (BE_SQLITE_ROW != stat)
        return stat;

    key.m_className = stmt->GetValueText(0);
    key.m_ecSchemaId = stmt->GetValueInt64(1);
    key.m_ecClassId = ecClassId;
    key.m_resolvedECClass = nullptr;
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECSchemaId (DbECSchemaEntry& key, ECSchemaId ecSchemaId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT S.Id, S.Name, S.VersionMajor, S.VersionMinor, (SELECT COUNT(*) FROM ec_Class C WHERE S.Id = C.SchemaID) FROM ec_Schema S WHERE S.Id = ?");
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
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECSchemaId (Utf8StringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, ECSchemaId ecSchemaId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT Name, VersionMajor, VersionMinor FROM ec_Schema WHERE Id = ?");
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
DbResult ECDbSchemaPersistence::GetECSchemaKeys(ECSchemaKeys& keys, BeSQLite::Db& db)
    {
    keys.clear();
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT Id, Name, VersionMajor, VersionMinor, DisplayLabel FROM ec_Schema ORDER BY Name");
    if (BE_SQLITE_OK != stat)
        return stat;

    while ((stat = stmt->Step()) == BE_SQLITE_ROW)
        {
        keys.push_back(
            ECSchemaKey(
            stmt->GetValueInt64(0),
            stmt->GetValueText(1),
            (uint32_t) stmt->GetValueInt(2),
            (uint32_t) stmt->GetValueInt(3),
            stmt->IsColumnNull(4) ? (Utf8CP)nullptr : stmt->GetValueText(4)));
        }
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteECClass(ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "DELETE FROM ec_Class WHERE Id = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64(1, ecClassId);
    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteCustomAttribute(ECContainerId containerId, ECContainerType containerType, ECClassId customAttributeClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "DELETE FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64(1, containerId);
    stmt->BindInt(2, static_cast<int>(containerType));
    stmt->BindInt64(3, customAttributeClassId);
    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteECSchema (ECSchemaId ecSchemaId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_Schema WHERE Id = ?");
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
    auto stat = db.GetCachedStatement (stmt, "SELECT NULL FROM ec_Schema WHERE Id = ?");
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
    auto stat = db.GetCachedStatement(stmt, "SELECT Id FROM ec_Schema WHERE Name = ?");
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
    auto stat = db.GetCachedStatement (stmt, "SELECT ClassId FROM ec_BaseClass WHERE BaseClassId = ?");
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
    auto stat = db.GetCachedStatement (stmt, "SELECT COUNT(*) FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, containerId);
    stmt->BindInt (2, (int) containerType);
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
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_Property WHERE Id = ?");
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
    auto stat = db.GetCachedStatement (stmt, "SELECT BaseClassId FROM ec_BaseClass WHERE ClassId = ? ORDER BY Ordinal");
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
    auto stat = db.GetCachedStatement (stmt, "SELECT Id, Name, DisplayLabel FROM ec_Class WHERE SchemaId = ? ORDER BY Name");
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
DbResult ECDbSchemaPersistence::GetClassesMappedToTable(std::vector<ECClassId>& classIds, ECDbSqlTable const& table, bool skipRelationships, BeSQLite::Db& db)
    {
    BeSQLite::Statement stmt;
    auto stat = skipRelationships ?
        stmt.Prepare(db,
        "SELECT DISTINCT ec_ClassMap.ClassId  FROM ec_ClassMap"
        "   INNER JOIN ec_PropertyMap ON ec_PropertyMap.ClassMapId = ec_ClassMap.Id"
        "   INNER JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId"
        "   INNER JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
        "   INNER JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId"
        "   WHERE ec_Table.Id = ? AND ec_Class.IsRelationship = 0"
        ) :
        stmt.Prepare(db,
        "SELECT DISTINCT ec_ClassMap.ClassId  FROM ec_ClassMap"
        "   INNER JOIN ec_PropertyMap ON ec_PropertyMap.ClassMapId = ec_ClassMap.Id"
        "   INNER JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId"
        "   INNER JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
        "   INNER JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId"
        "   WHERE ec_Table.Id = ?"
        );

    if (BE_SQLITE_OK != stat)
        return stat;

    stmt.BindInt64(1, table.GetId());
    while ((stat = stmt.Step()) == BE_SQLITE_ROW)
        {
        auto ecClassId = stmt.GetValueInt64(0);
        classIds.push_back(ecClassId);
        }

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::IsECSchemaMapped(bool* schemaNotFound, ECN::ECSchemaCR ecSchema, BeSQLite::Db& db)
    {
    if (schemaNotFound)
        *schemaNotFound = true;

    if (!ecSchema.HasId())
        {
        auto ecSchemaId = GetECSchemaId(db,
                                        Utf8String(ecSchema.GetName().c_str()).c_str());
        if (ecSchemaId == 0)
            return false;

        const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);
        }

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT COUNT(*) FROM ec_ClassMap INNER JOIN ec_Class ON ecClass.Id = ec_ClassMap.ClassId WHERE ec_Class.SchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64(1, ecSchema.GetId());
    stat = stmt->Step();
    if (BE_SQLITE_ROW != stat)
        return false;

    if (schemaNotFound)
        *schemaNotFound = false;

    return stmt->GetValueInt(0) > 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
//static
ECClassId ECDbSchemaPersistence::GetECClassIdBySchemaName (BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = ? AND c.Name = ?");
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
    auto stat = db.GetCachedStatement(stmt, "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.NamespacePrefix = ? AND c.Name = ?");
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
    auto stat = db.GetCachedStatement(stmt, "SELECT p.Id FROM ec_Property p INNER JOIN ec_Class c ON p.ClassId = c.Id INNER JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = ? AND c.Name = ? AND p.Name = ?");
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
    auto stat = stmt.Prepare (db, "SELECT Id FROM ec_PropertyPath WHERE RootPropertyId = ? AND AccessString = ?");
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
