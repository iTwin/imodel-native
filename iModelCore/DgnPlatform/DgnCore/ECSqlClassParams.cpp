/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ECSqlClassParams.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassParams::Add(Utf8CP name, StatementType type)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(name));
    if (!Utf8String::IsNullOrEmpty(name))
        {
        BeAssert(m_entries.end() == std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return 0 == ::strcmp(name, arg.m_name); }));
        Entry entry(name, type);
        if (StatementType::Select == (type & StatementType::Select) && 0 < m_entries.size())
            {
            // We want to be able to quickly look up the index for a name for SELECT query results...so group them together at the front of the list.
            m_entries.insert(m_entries.begin(), entry);
            }
        else
            {
            m_entries.push_back(entry);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
int ECSqlClassParams::GetSelectIndex(Utf8CP name) const
    {
    // NB: All parameters valid for SELECT statements are grouped at the beginning of the list.
    BeAssert(!Utf8String::IsNullOrEmpty(name));
    if (!Utf8String::IsNullOrEmpty(name))
        {
        auto found = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return arg.m_name == name; });
        if (m_entries.end() == found)
            {
            // Ideally callers always pass the same static string we originally stored...fallback to string comparison...
            found = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return 0 == ::strcmp(arg.m_name, name); });
            BeAssert(m_entries.end() == found && "Prefer to pass the same string with static storage duration to GetSelectIndex() as was previously passed to Add()");
            }

        BeAssert(m_entries.end() != found);
        if (m_entries.end() != found)
            {
            BeAssert(StatementType::Select == (found->m_type & StatementType::Select));
            if (StatementType::Select == (found->m_type & StatementType::Select))
                return static_cast<int>(found - m_entries.begin());
            }
        }

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassParams::RemoveAllButSelect()
    {
    // Once we've constructed the handler info, we need only retain those property names which are used in SELECT statements.
    auto removeAt = std::remove_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return StatementType::Select != (arg.m_type & StatementType::Select); });
    m_entries.erase(removeAt, m_entries.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static uint16_t buildParamString(Utf8StringR str, ECSqlClassParams::Entries const& entries, ECSqlClassParams::StatementType type, T func)
    {
    uint16_t count = 0;
    for (auto const& entry : entries)
        {
        if (type != (entry.m_type & type))
            continue;

        if (0 < count)
            str.append(1, ',');

        func(entry.m_name, count);
        ++count;
        }

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECSqlClassInfo::GetInsertECSql(DgnDbCR dgndb, DgnClassId classId) const
    {
    ECClassCP ecClass = dgndb.Schemas().GetECClass(classId.GetValue());
    BeAssert(nullptr != ecClass);
    if (nullptr == ecClass)
        return "";

    Utf8String ecsql("INSERT INTO [");
    ecsql.append(ecClass->GetSchema().GetName());
    ecsql.append("].[");
    ecsql.append(ecClass->GetName());
    ecsql.append(1, ']');

    ecsql.append(m_insert);

    return ecsql;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassInfo::Initialize(Utf8StringCR fullClassName, ECSqlClassParamsCR params)
    {
    if (m_initialized)
        return;
    
    m_params = params;
    auto const& entries = m_params.GetEntries();

    // Build SELECT statement
    m_select = "SELECT ";
    uint16_t numSelectParams = buildParamString(m_select, entries, ECSqlClassParams::StatementType::Select,
                                                [&] (Utf8CP name, uint16_t count) { m_select.append(1, '[').append(name).append(1, ']'); });

    if (0 < numSelectParams)
        {
        m_select.append(" FROM ONLY ").append(fullClassName);
        m_select.append(" WHERE ECInstanceId=?");
        }
    else
        {
        m_select.clear();
        }

    // Build bulk INSERT statement sans INSERT INTO [schema].[class] - schema+class may vary for subclasses without their own handlers.
    // (That variance only matters for INSERT statements)
    m_insert.append(1, '(');
    Utf8String insertValues;
    uint16_t numInsertParams = buildParamString(m_insert, entries, ECSqlClassParams::StatementType::Insert,
                                                [&] (Utf8CP name, uint16_t count)
        {
        m_insert.append(1, '[').append(name).append(1, ']');
        if (0 < count)
            insertValues.append(1, ',');

        insertValues.append(":[").append(name).append(1, ']');
        });

    if (0 < numInsertParams)
        m_insert.append(")VALUES(").append(insertValues).append(1, ')');
    else
        m_insert.clear();

    // Build UPDATE statement
    m_update.append("UPDATE ONLY ").append(fullClassName).append(" SET ");
    m_numUpdateParams = buildParamString(m_update, entries, ECSqlClassParams::StatementType::Update,
                                                     [&] (Utf8CP name, uint16_t count)
        {
        m_update.append(1, '[').append(name).append("]=:[").append(name).append(1, ']');
        });
    if (0 < m_numUpdateParams)
        {
        m_update.append("WHERE ECInstanceId=?");
        }
    else
        m_update.clear();

    // We no longer need any param names except those used in INSERT.
    m_params.RemoveAllButSelect();

    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr ECSqlClassInfo::GetInsertStmt(DgnDbCR dgndb, DgnClassId classId) const
    {
    return m_insert.empty() ? nullptr : dgndb.GetPreparedECSqlStatement(GetInsertECSql(dgndb, classId).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr ECSqlClassInfo::GetSelectStmt(DgnDbCR dgndb, ECInstanceId id) const
    {
    CachedECSqlStatementPtr stmt = m_select.empty() ? nullptr : dgndb.GetPreparedECSqlStatement(m_select.c_str());
    if (stmt.IsValid())
        stmt->BindId(1, id);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr ECSqlClassInfo::GetUpdateStmt(DgnDbCR dgndb, ECInstanceId id) const
    {
    CachedECSqlStatementPtr stmt = m_update.empty() ? nullptr : dgndb.GetPreparedECSqlStatement(m_update.c_str());
    if (stmt.IsValid())
        stmt->BindId(m_numUpdateParams + 1, id);
    return stmt;
    }
