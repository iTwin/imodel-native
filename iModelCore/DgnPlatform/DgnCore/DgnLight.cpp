/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnLight.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLightId DgnLights::Insert(Light& light, DgnDbStatus* outResult)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(result, outResult);
    DgnLightId newId;

    auto status = m_dgndb.GetServerIssuedId(newId, DGN_TABLE(DGN_CLASSNAME_Light), "Id");
    if (status != BE_SQLITE_OK)
        {
        result = DgnDbStatus::ForeignKeyConstraint;
        return DgnLightId();
        }

    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Light) " (Id,Value,Name,Descr) VALUES(?,?,?,?)");
    stmt.BindId(1, newId);
    stmt.BindText(2, light.GetValue(), Statement::MakeCopy::No);
    stmt.BindText(3, light.GetName(), Statement::MakeCopy::No);
    stmt.BindText(4, light.GetDescr(), Statement::MakeCopy::No);

    status = stmt.Step();
    if (BE_SQLITE_DONE != status)
        {
        result = DgnDbStatus::DuplicateName;
        return DgnLightId();
        }

    result = DgnDbStatus::Success;
    light.m_id = newId;
    return newId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnLights::Update(Light const& light) const
    {
    if (!light.IsValid())
        return DgnDbStatus::InvalidId;

    Statement stmt(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Light) " SET Value=?,Descr=? WHERE Id=?");
    stmt.BindText(1, light.GetValue(), Statement::MakeCopy::No);
    stmt.BindText(2, light.GetDescr(), Statement::MakeCopy::No);
    stmt.BindId(3, light.GetId());

    DbResult status = stmt.Step();
    BeDataAssert(BE_SQLITE_DONE == status);
    return (BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLights::Light DgnLights::Query(DgnLightId id) const
    {
    if (!id.IsValid())
        return Light();

    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;
    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement(stmt, "SELECT Value,Name,Descr FROM " DGN_TABLE(DGN_CLASSNAME_Light) " WHERE Id=?");
    stmt->BindId(1, id);

    Light light;
    if (BE_SQLITE_ROW == stmt->Step())
        {
        light.m_id = id;
        light.m_value.AssignOrClear(stmt->GetValueText(0));
        light.m_name.AssignOrClear(stmt->GetValueText(1));
        light.m_descr.AssignOrClear(stmt->GetValueText(2));
        }

    return light;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLightId DgnLights::QueryLightId(Utf8StringCR name) const
    {
    Statement stmt(m_dgndb, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Light) " WHERE Name=?");
    stmt.BindText(1, name, Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt.Step() ? stmt.GetValueId<DgnLightId>(0) : DgnLightId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLights::Iterator::const_iterator DgnLights::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Name,Descr,Value FROM " DGN_TABLE(DGN_CLASSNAME_Light));
        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnLights::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Light));
    Statement sql;
    sql.Prepare(*m_db, sqlString.c_str());
    return (BE_SQLITE_ROW == sql.Step()) ? static_cast<size_t>(sql.GetValueInt(0)) : 0;
    }

DgnLightId DgnLights::Iterator::Entry::GetId() const {Verify(); return m_sql->GetValueId<DgnLightId>(0);}
Utf8CP DgnLights::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP DgnLights::Iterator::Entry::GetDescr() const {Verify(); return m_sql->GetValueText(2);}
Utf8CP DgnLights::Iterator::Entry::GetValue() const {Verify(); return m_sql->GetValueText(3);}

