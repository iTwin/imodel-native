/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistence.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
BentleyStatus ECDbSchemaPersistence::InsertBaseClass(ECDbCR db, DbBaseClassInfo const& info)
    {
    CachedStatementPtr stmt = nullptr;
    if(BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO ec_BaseClass (ClassId, BaseClassId, Ordinal) VALUES (?, ?, ?)"))
        return ERROR;

    if (info.ColsInsert & DbBaseClassInfo::COL_ClassId) stmt->BindInt64(1, info.m_ecClassId);
    if (info.ColsInsert & DbBaseClassInfo::COL_BaseClassId) stmt->BindInt64(2, info.m_baseECClassId);
    if (info.ColsInsert & DbBaseClassInfo::COL_Ordinal) stmt->BindInt(3, info.m_ecIndex);

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistence::FindBaseClass(BeSQLite::CachedStatementPtr& stmt, ECDbCR db, DbBaseClassInfo const& info)
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

    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, sql.c_str()))
        return ERROR;

    int nCol = 1;
    if (info.ColsWhere & DbBaseClassInfo::COL_ClassId) stmt->BindInt64(nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbBaseClassInfo::COL_BaseClassId) stmt->BindInt64(nCol++, info.m_baseECClassId);
    if (info.ColsWhere & DbBaseClassInfo::COL_Ordinal) stmt->BindInt(nCol++, info.m_ecIndex);

    return SUCCESS;
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
BentleyStatus ECDbSchemaPersistence::InsertECRelationshipConstraint(ECDbCR db, DbECRelationshipConstraintInfo const& info)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraint (RelationshipClassId, RelationshipEnd, MultiplicityLowerLimit, MultiplicityUpperLimit, RoleLabel, IsPolymorphic) VALUES (?, ?, ?, ?, ?, ?)"))
        return ERROR;

    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_RelationshipClassId) stmt->BindInt64(1, info.m_relationshipClassId);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_RelationshipEnd) stmt->BindInt(2, info.m_ecRelationshipEnd);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_MultiplicityLowerLimit) stmt->BindInt(3, info.m_multiplicityLowerLimit);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_MultiplicityUpperLimit) stmt->BindInt(4, info.m_multiplicityUpperLimit);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_RoleLabel) stmt->BindText(5, info.m_roleLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_IsPolymorphic) stmt->BindInt(6, info.m_isPolymorphic ? 1 : 0);

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistence::FindECRelationshipConstraint(BeSQLite::CachedStatementPtr& stmt, ECDbCR db, DbECRelationshipConstraintInfo const& info)
    {
    Utf8String sql("SELECT ");

    //prepare select list
    SqlClauseBuilder selectClause(",");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RelationshipClassId) selectClause.AddItem("RelationshipClassId");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RelationshipEnd) selectClause.AddItem("RelationshipEnd");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_MultiplicityLowerLimit) selectClause.AddItem("MultiplicityLowerLimit");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_MultiplicityUpperLimit) selectClause.AddItem("MultiplicityUpperLimit");
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

    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, sql.c_str()))
        return ERROR;

    int nCol = 1;
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_RelationshipClassId) stmt->BindInt64(nCol++, info.m_relationshipClassId);
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_RelationshipEnd) stmt->BindInt(nCol++, info.m_ecRelationshipEnd);
    return SUCCESS;
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
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_MultiplicityLowerLimit) info.m_multiplicityLowerLimit = stmt.GetValueInt(nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_MultiplicityUpperLimit) info.m_multiplicityUpperLimit = stmt.GetValueInt(nCol++);
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
BentleyStatus ECDbSchemaPersistence::InsertECRelationshipConstraintClass(ECDbCR ecdb, ECN::ECClassId relClassId, ECN::ECRelationshipConstraintClassCR constraintClass, ECN::ECRelationshipEnd end)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraintClass (RelationshipClassId, RelationshipEnd, ClassId, KeyProperties) VALUES (?,?,?,?)"))
        return ERROR;

    BeAssert(relClassId != ECClass::UNSET_ECCLASSID);
    if (BE_SQLITE_OK != stmt->BindInt64(1, relClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, (int) end))
        return ERROR;

    BeAssert(constraintClass.GetClass().GetId() != ECClass::UNSET_ECCLASSID);
    if (BE_SQLITE_OK != stmt->BindInt64(3, constraintClass.GetClass().GetId()))
        return ERROR;
    
    bvector<Utf8String> const& keyPropNames = constraintClass.GetKeys();
    if (!keyPropNames.empty())
        {
        rapidjson::Document keyPropJson;
        auto& allocator = keyPropJson.GetAllocator();
        keyPropJson.SetArray();
        keyPropJson.Reserve((rapidjson::SizeType) keyPropNames.size(), allocator);

        for (Utf8StringCR keyPropertyName : keyPropNames)
            {
            keyPropJson.PushBack(keyPropertyName.c_str(), allocator);
            }

        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        keyPropJson.Accept(writer);
        if (BE_SQLITE_OK != stmt->BindText(4, buf.GetString(), Statement::MakeCopy::Yes))
            return ERROR;
        }

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistence::InsertCustomAttribute(ECDbCR db, DbCustomAttributeInfo const& info)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO ec_CustomAttribute (ContainerId, ContainerType, ClassId, Ordinal, Instance) VALUES(?, ?, ?, ?, ?)"))
        return ERROR;

    if (info.ColsInsert & DbCustomAttributeInfo::COL_ContainerId) stmt->BindInt64 (1, info.m_containerId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_ContainerType) stmt->BindInt (2, (int) info.m_containerType);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_ClassId) stmt->BindInt64 (3, info.m_ecClassId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_Ordinal) stmt->BindInt (4, info.m_index);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_Instance)
        {
        if (BE_SQLITE_OK != stmt->BindText(5, info.GetCaInstanceXml(), Statement::MakeCopy::No))
            return ERROR;
        }

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistence::FindCustomAttribute(BeSQLite::CachedStatementPtr& stmt, ECDbCR db, DbCustomAttributeInfo const& info)
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

    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, sql.c_str()))
        return ERROR;

    int nCol = 1;
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId) stmt->BindInt64(nCol++, info.m_containerId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType) stmt->BindInt(nCol++, (int) info.m_containerType);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ClassId) stmt->BindInt64(nCol++, info.m_ecClassId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_Ordinal) stmt->BindInt(nCol++, info.m_index);

    return SUCCESS;
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
BentleyStatus ECDbSchemaPersistence::InsertECSchemaReference(ECDbCR db, DbECSchemaReferenceInfo const& info)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO ec_SchemaReference (SchemaId, ReferencedSchemaId) VALUES(?, ?)"))
        return ERROR;

    stmt->BindInt64 (1, info.m_ecSchemaId);
    stmt->BindInt64 (2, info.m_referencedECSchemaId);

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECClass(ECDbCR db, ECClassCR ecClass)
    {
    if (ecClass.HasId()) //This is unsafe but since we donot delete ecclass any class that hasId does exist in db
        return true;

    const ECClassId classId = GetECClassId(db, Utf8String(ecClass.GetSchema().GetName().c_str()).c_str(),
                                            Utf8String(ecClass.GetName().c_str()).c_str(), ResolveSchema::BySchemaName);

    return classId > ECClass::UNSET_ECCLASSID;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     11/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECDbSchemaPersistence::GetReferencedSchemas(bvector<ECSchemaId>& referencedSchemaIds, ECDbCR ecdb, ECSchemaId schemaId)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "SELECT ReferencedSchemaId FROM ec_SchemaReference WHERE SchemaId=?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, schemaId))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        BeAssert(!stmt->IsColumnNull(0));
        referencedSchemaIds.push_back((ECSchemaId) stmt->GetValueInt64(0));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECSchemaReference(ECDbCR db, ECSchemaId ecPrimarySchemaId, ECSchemaId ecReferencedSchemaId)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT NULL FROM ec_SchemaReference WHERE SchemaId = ? AND ReferencedSchemaId = ?"))
        return false;

    stmt->BindInt64 (1, ecPrimarySchemaId);
    stmt->BindInt64 (2, ecReferencedSchemaId);
    return stmt->Step () == BE_SQLITE_ROW;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
/*BentleyStatus ECDbSchemaPersistence::ResolveECClassId(DbECClassEntry& key, ECClassId ecClassId, ECDbCR db)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Name, SchemaId FROM ec_Class WHERE Id=?"))
        return ERROR;

    stmt->BindInt64(1, ecClassId);

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    key.m_className = stmt->GetValueText(0);
    key.m_ecSchemaId = stmt->GetValueInt64(1);
    key.m_ecClassId = ecClassId;
    key.m_cachedECClass = nullptr;

    return SUCCESS;
    }*/

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistence::ResolveECSchemaId(DbECSchemaEntry& key, ECSchemaId ecSchemaId, ECDbCR db)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT S.Id, S.Name, S.VersionMajor, S.VersionMinor, "
                                              "(SELECT COUNT(*) FROM ec_Class C WHERE S.Id = C.SchemaID) + (SELECT COUNT(*) FROM ec_Enumeration e WHERE S.Id = e.SchemaID) "
                                              "FROM ec_Schema S WHERE S.Id = ?"))
        return ERROR;

    stmt->BindInt64 (1, ecSchemaId);

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    key.m_ecSchemaId = stmt->GetValueInt64(0);
    key.m_schemaName = stmt->GetValueText (1);
    key.m_versionMajor = (uint32_t) stmt->GetValueInt (2);
    key.m_versionMinor = (uint32_t) stmt->GetValueInt (3);
    key.m_nTypesInSchema = (uint32_t) stmt->GetValueInt (4);
    key.m_nTypesLoaded = 0;
    key.m_cachedECSchema = nullptr;
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistence::GetECSchemaKeys(ECSchemaKeys& keys, ECDbCR db)
    {
    keys.clear();
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Id, Name, VersionMajor, VersionMinor, DisplayLabel FROM ec_Schema ORDER BY Name"))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        keys.push_back(
            ECSchemaKey(
            stmt->GetValueInt64(0),
            stmt->GetValueText(1),
            (uint32_t) stmt->GetValueInt(2),
            (uint32_t) stmt->GetValueInt(3),
            stmt->IsColumnNull(4) ? (Utf8CP)nullptr : stmt->GetValueText(4)));
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECSchemaWithId(ECDbCR db, ECSchemaId ecSchemaId)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT NULL FROM ec_Schema WHERE Id = ?"))
        return false;

    stmt->BindInt64 (1, ecSchemaId);
    return stmt->Step () == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistence::GetECSchemaId(ECDbCR db, Utf8CP schemaName)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Id FROM ec_Schema WHERE Name = ?"))
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    
    if (BE_SQLITE_ROW != stmt->Step())
        return 0LL;

    return stmt->GetValueInt64(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistence::GetECSchemaId(ECDbCR db, ECSchemaCR ecSchema)
    {
    Utf8String schemaName(ecSchema.GetName());
    return GetECSchemaId (db, schemaName.c_str());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistence::GetDerivedECClasses(ECClassIdList& classIds, ECClassId baseClassId, ECDbCR db)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT ClassId FROM ec_BaseClass WHERE BaseClassId = ?"))
        return ERROR;

    stmt->BindInt64 (1, baseClassId);
    while (stmt->Step () == BE_SQLITE_ROW)
        classIds.push_back (stmt->GetValueInt64 (0));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::IsCustomAttributeDefined(ECDbCR db, ECClassId classId, ECContainerId containerId, ECContainerType containerType)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement (stmt, "SELECT COUNT(*) FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?"))
        return false;

    stmt->BindInt64 (1, containerId);
    stmt->BindInt (2, (int) containerType);
    stmt->BindInt64 (3, classId);

    if (stmt->Step () == BE_SQLITE_ROW)
        return stmt->GetValueInt64 (0) > 0;

    return false;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistence::GetBaseECClasses(ECClassIdList& baseClassIds, ECClassId ecClassId, ECDbCR db)  // WIP_FNV: take a name, instead... and modify the where clause
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT BaseClassId FROM ec_BaseClass WHERE ClassId = ? ORDER BY Ordinal"))
        return ERROR;

    stmt->BindInt64 (1, ecClassId);
    while (stmt->Step () == BE_SQLITE_ROW)
        baseClassIds.push_back (stmt->GetValueInt64 (0));
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaPersistence::GetECClassKeys(ECClassKeys& keys, ECSchemaId schemaId, ECDbCR db) // WIP_FNV: take a name, instead... and modify the where clause
    {
    keys.clear ();
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Id, Name, DisplayLabel FROM ec_Class WHERE SchemaId = ? ORDER BY Name"))
        return ERROR;

    stmt->BindInt64 (1, schemaId);
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        keys.push_back (
            ECClassKey (
            stmt->GetValueInt64 (0),
            stmt->GetValueText (1),
            (stmt->IsColumnNull (2) ? (Utf8CP)nullptr : stmt->GetValueText (2))));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::IsECSchemaMapped(bool* schemaNotFound, ECN::ECSchemaCR ecSchema, ECDbCR db)
    {
    if (schemaNotFound)
        *schemaNotFound = true;

    if (!ecSchema.HasId())
        {
        auto ecSchemaId = GetECSchemaId(db,Utf8String(ecSchema.GetName().c_str()).c_str());
        if (ecSchemaId == 0)
            return false;

        const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);
        }

    CachedStatementPtr stmt = nullptr;
    if(BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT COUNT(*) FROM ec_ClassMap INNER JOIN ec_Class ON ecClass.Id = ec_ClassMap.ClassId WHERE ec_Class.SchemaId = ?"))
        return false;

    stmt->BindInt64(1, ecSchema.GetId());
    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    if (schemaNotFound)
        *schemaNotFound = false;

    return stmt->GetValueInt(0) > 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
//static
ECClassId ECDbSchemaPersistence::GetECClassId(ECDbCR db, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema)
    {
    Utf8CP sql = nullptr;
    switch (resolveSchema)
        {
            case ResolveSchema::BySchemaName:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = ? AND c.Name = ?";
                break;

            case ResolveSchema::BySchemaNamespacePrefix:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.NamespacePrefix = ? AND c.Name = ?";
                break;

            default:
                sql = "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND (s.Name = ?1 OR s.NamespacePrefix = ?1) AND c.Name = ?2";
                break;
        }

    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, sql))
        return ECClass::UNSET_ECCLASSID;

    stmt->BindText(1, schemaNameOrPrefix, Statement::MakeCopy::No);
    stmt->BindText (2, className, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return ECClass::UNSET_ECCLASSID;

    return stmt->GetValueInt64 (0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId ECDbSchemaPersistence::GetECPropertyId(ECDbCR db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT p.Id FROM ec_Property p INNER JOIN ec_Class c ON p.ClassId = c.Id INNER JOIN ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = ? AND c.Name = ? AND p.Name = ?"))
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    stmt->BindText (2, className, Statement::MakeCopy::No);
    stmt->BindText (3, propertyName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return 0LL;

    return stmt->GetValueInt64(0);
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbPropertyPathId ECDbSchemaPersistence::GetECPropertyPathId(ECPropertyId rootECPropertyId, Utf8CP accessString, ECDbCR db)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(db, "SELECT Id FROM ec_PropertyPath WHERE RootPropertyId = ? AND AccessString = ?"))
        {
        BeAssert(false && "Failed to prepare statement");
        return 0;
        }

    stmt.BindInt64(1, rootECPropertyId);
    stmt.BindText(2, accessString, Statement::MakeCopy::No);
    if (stmt.Step() == BE_SQLITE_ROW)
        return stmt.GetValueInt64(0);

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaPersistence::GetSchemaNamespacePrefixes(bvector<Utf8String>& prefixes, ECDbCR ecdb)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "SELECT NamespacePrefix FROM ec_Schema"))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        prefixes.push_back(stmt->GetValueText(0));
        }

    return SUCCESS;
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

    if (serializerStat != InstanceWriteStatus::Success)
        {
        LOG.errorv ("Serializing custom attribute instance to XML failed with error code: %d", serializerStat);
        BeAssert (false && "Serializing custom attribute instance to XML failed.");
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

    if (deserializeStat != InstanceReadStatus::Success)
        {
        LOG.errorv ("Deserializing custom attribute instance from XML failed with error code: %d", deserializeStat);
        BeAssert (false && "Deserializing custom attribute instance from XML failed.");
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



END_BENTLEY_SQLITE_EC_NAMESPACE
