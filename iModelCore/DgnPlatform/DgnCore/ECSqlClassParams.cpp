/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ECSqlClassParams.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassParams::Add(Utf8StringCR name, StatementType type, bool ah)
    {
    if (name.empty())
        {
        BeAssert(false);
        return;
        }

    BeAssert(m_entries.end() == std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return name.Equals(arg.m_name); }));
    Entry entry(name, type, ah);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
int ECSqlClassParams::GetSelectIndex(Utf8StringCR name) const
    {
    if (name.empty())
        {
        BeAssert(false);
        return -1;
        }

    // NB: All parameters valid for SELECT statements are grouped at the beginning of the list.
    auto found = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return arg.m_name.Equals(name); });

    if (m_entries.end() == found)
        {
        BeAssert(false);
        return -1;
        }

    if (StatementType::Select != (found->m_type & StatementType::Select))
        {
        BeAssert(false);
        return -1;
        }

    return static_cast<int>(found - m_entries.begin());
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
bool ECSqlClassParams::AppendClassName(Utf8StringR classname, DgnDbCR db, DgnClassId classId)
    {
    ECClassCP ecclass = db.Schemas().GetECClass(classId);
    BeAssert(nullptr != ecclass);
    if (nullptr == ecclass)
        return false;

    classname.append(1, '[');
    classname.append(ecclass->GetSchema().GetName());
    classname.append("].[");
    classname.append(ecclass->GetName());
    classname.append(1, ']');

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSqlClassParams::BuildInsertECSql(Utf8StringR ecsql, DgnDbCR dgndb, DgnClassId classId) const
    {
    ecsql.clear();
    if (m_insertTemplate.empty())
        return true;

    ecsql.append("INSERT INTO ");
    if (!AppendClassName(ecsql, dgndb, classId))
        {
        ecsql.clear();
        return false;
        }

    ecsql.append(m_insertTemplate);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSqlClassParams::BuildSelectECSql(Utf8StringR ecsql, DgnDbCR dgndb, DgnClassId classId) const
    {
    ecsql.clear();
    if (m_selectTemplate.empty())
        return true;

    ecsql.append(m_selectTemplate);
    ecsql.append(" FROM ONLY ");
    if (!AppendClassName(ecsql, dgndb, classId))
        {
        ecsql.clear();
        return false;
        }

    ecsql.append(" WHERE ECInstanceId=?");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSqlClassParams::BuildUpdateECSql(Utf8StringR ecsql, DgnDbCR dgndb, DgnClassId classId) const
    {
    ecsql.clear();
    if (m_updateTemplate.empty())
        return true;

    ecsql.append("UPDATE ONLY ");
    if (!AppendClassName(ecsql, dgndb, classId))
        {
        ecsql.clear();
        return false;
        }

    ecsql.append(m_updateTemplate);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSqlClassParams::BuildClassInfo(ECSqlClassInfo& info, DgnDbCR dgndb, DgnClassId classId) const
    {
    Utf8String select, insert, update;
    if (!BuildSelectECSql(select, dgndb, classId) || !BuildInsertECSql(insert, dgndb, classId) || !BuildUpdateECSql(update, dgndb, classId))
        return false;

    info.m_updateParameterIndex = m_numUpdateParams + 1;
    info.m_select = select;
    info.m_insert = insert;
    info.m_update = update;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassParams::Initialize(IECSqlClassParamsProvider& provider)
    {
    if (m_initialized)
        return;
    
    provider._GetClassParams(*this);
    auto const& entries = GetEntries();

    // Build SELECT statement
    m_selectTemplate = "SELECT ";
    uint16_t numSelectParams = buildParamString(m_selectTemplate, entries, ECSqlClassParams::StatementType::Select,
                                                [&] (Utf8StringCR name, uint16_t count) { m_selectTemplate.append(1, '[').append(name).append(1, ']'); });

    if (0 == numSelectParams)
        m_selectTemplate.clear();

    // Build INSERT statement
    m_insertTemplate.append(1, '(');
    Utf8String insertValues;
    uint16_t numInsertParams = buildParamString(m_insertTemplate, entries, ECSqlClassParams::StatementType::Insert,
                                                [&] (Utf8StringCR name, uint16_t count)
        {
        m_insertTemplate.append(1, '[').append(name).append(1, ']');
        if (0 < count)
            insertValues.append(1, ',');

        insertValues.append(":[").append(name).append(1, ']');
        });

    if (0 < numInsertParams)
        m_insertTemplate.append(")VALUES(").append(insertValues).append(1, ')');
    else
        m_insertTemplate.clear();

    // Build UPDATE statement
    m_updateTemplate.append(" SET ");
    m_numUpdateParams = buildParamString(m_updateTemplate, entries, ECSqlClassParams::StatementType::Update,
                                                     [&] (Utf8StringCR name, uint16_t count)
        {
        m_updateTemplate.append(1, '[').append(name).append("]=:[").append(name).append(1, ']');
        });

    if (0 < m_numUpdateParams)
        m_updateTemplate.append("WHERE ECInstanceId=?");
    else
        m_updateTemplate.clear();
    
    // We no longer need any param names except those used in SELECT.
    RemoveAllButSelect();

    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr ECSqlClassInfo::GetInsertStmt(DgnDbCR dgndb) const
    {
    return m_insert.empty() ? nullptr : dgndb.GetPreparedECSqlStatement(m_insert.c_str());
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
        stmt->BindId(m_updateParameterIndex, id);

    return stmt;
    }

