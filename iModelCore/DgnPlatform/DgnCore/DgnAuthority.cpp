/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnAuthority.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId DgnAuthorities::Insert (Authority& authority, DgnDbStatus* outResult)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(result, outResult);
    DgnAuthorityId newId;

    auto status = m_dgndb.GetServerIssuedId (newId, DGN_TABLE(DGN_CLASSNAME_Authority), "Id");
    if (BE_SQLITE_OK != status)
        {
        result = DgnDbStatus::ForeignKeyConstraint;
        return DgnAuthorityId();
        }

    Statement stmt (m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Authority) " (Id,Name,Uri) VALUES(?,?,?)");
    stmt.BindId (1, newId);
    stmt.BindText (2, authority.GetName(), Statement::MakeCopy::No);
    stmt.BindText (3, authority.GetUri(), Statement::MakeCopy::No);

    status = stmt.Step();
    if (BE_SQLITE_DONE != status)
        {
        result = DgnDbStatus::DuplicateName;
        return DgnAuthorityId();
        }

    result = DgnDbStatus::Success;
    authority.m_id = newId;
    return newId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnAuthorities::Update (Authority const& authority) const
    {
    if (!authority.IsValid())
        return DgnDbStatus::InvalidId;

    Statement stmt (m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Authority) " SET Uri=? WHERE Id=?");
    stmt.BindText (1, authority.GetUri(), Statement::MakeCopy::No);
    stmt.BindId (2, authority.GetId());

    DbResult status = stmt.Step();
    BeDataAssert (BE_SQLITE_DONE == status);
    return (BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorities::Authority DgnAuthorities::Query (DgnAuthorityId id) const
    {
    if (!id.IsValid())
        return Authority();

    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;
    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement (stmt, "SELECT Name,Uri FROM " DGN_TABLE(DGN_CLASSNAME_Authority) " WHERE Id=?");
    stmt->BindId (1, id);

    Authority authority;
    if (stmt->Step() == BE_SQLITE_ROW)
        {
        authority.m_id = id;
        authority.m_name.AssignOrClear (stmt->GetValueText (0));
        authority.m_uri.AssignOrClear (stmt->GetValueText (1));
        }

    return authority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId DgnAuthorities::QueryAuthorityId (Utf8StringCR name) const
    {
    Statement stmt (m_dgndb, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Authority) " WHERE Name=?");
    stmt.BindText (1, name, Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt.Step() ? stmt.GetValueId<DgnAuthorityId>(0) : DgnAuthorityId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorities::Iterator::const_iterator DgnAuthorities::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString ("SELECT Id,Name,Uri FROM " DGN_TABLE(DGN_CLASSNAME_Authority));
        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        m_params.Bind (*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnAuthorities::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString ("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Authority));
    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());
    return (BE_SQLITE_ROW == sql.Step()) ? static_cast<size_t>(sql.GetValueInt (0)) : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId DgnAuthorities::Iterator::Entry::GetId() const   { Verify(); return m_sql->GetValueId<DgnAuthorityId>(0); }
Utf8CP DgnAuthorities::Iterator::Entry::GetName() const         { Verify(); return m_sql->GetValueText (1); }
Utf8CP DgnAuthorities::Iterator::Entry::GetUri() const          { Verify(); return m_sql->GetValueText (2); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId DgnImportContext::RemapAuthorityId(DgnAuthorityId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnAuthorityId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    DgnAuthorities::Authority sourceAuthority = m_sourceDb.Authorities().Query(source);
    if (!sourceAuthority.IsValid())
        {
        BeDataAssert(false && "Missing source authority");
        return source;
        }

    dest = m_destDb.Authorities().QueryAuthorityId(sourceAuthority.GetName());
    if (!dest.IsValid())
        {
        DgnAuthorities::Authority destAuthority (sourceAuthority.GetName().c_str(), sourceAuthority.GetUri().c_str());
        dest = m_destDb.Authorities().Insert(destAuthority);
        if (!dest.IsValid())
            {
            BeDataAssert(false && "Invalid source authority");
            return source;
            }
        }

    return m_remap.Add(source, dest);
    }
