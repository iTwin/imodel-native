/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassParams::Add(Utf8StringCR name, StatementType type, uint16_t sortPriority)
    {
    if (name.empty())
        {
        BeAssert(false);
        return;
        }

    BeAssert(m_entries.end() == std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return name.Equals(arg.m_name); }));

    // We want to be able to quickly look up the index for a name for SELECT query results...so group them together at the front of the list.
    if (StatementType::Select != (type & StatementType::Select))
        sortPriority = 1024; // Pick a sortPriority that will sort non-SELECT to end

    Entry entry(name, type, sortPriority);
    m_entries.push_back(entry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
int ECSqlClassParams::GetSelectIndex(Utf8CP name) const
    {
    if (Utf8String::IsNullOrEmpty(name))
        {
        BeAssert(false);
        return -1;
        }

    // NB: All parameters valid for SELECT statements are grouped at the beginning of the list.
    auto found = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return 0 == ::strcmp(arg.m_name.c_str(), name);; });

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
template<typename T> static uint16_t buildParamString(Utf8StringR str, ECClassCR ecclass, ECSqlClassParams::Entries const& entries, ECSqlClassParams::StatementType type, T func)
    {
    uint16_t count = 0;
    for (auto const& entry : entries)
        {
        if (type != (entry.m_type & type))
            continue;

        // the handler may know about an ECProperty from a newer version of the schema than is imported into the iModel, so skip any properties that are not part of the ECClass
        if (nullptr == ecclass.GetPropertyP(entry.m_name.c_str()) && (0 != strcmp(entry.m_name.c_str(), "ECInstanceId")))
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
void ECSqlClassParams::AppendClassName(Utf8StringR classname, DgnDbCR db, ECClassCR ecclass)
    {
    classname.append(1, '[');
    classname.append(ecclass.GetSchema().GetName());
    classname.append("].[");
    classname.append(ecclass.GetName());
    classname.append(1, ']');
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t ECSqlClassParams::BuildInsertECSql(Utf8StringR ecsql, DgnDbCR dgndb, ECClassCR ecclass) const
    {
    ecsql.clear();
    ecsql.append("INSERT INTO ");
    AppendClassName(ecsql, dgndb, ecclass);
    ecsql.append(1, '(');

    auto const& entries = GetEntries();
    Utf8String insertValues;
    uint16_t numInsertParams = buildParamString(ecsql, ecclass, entries, ECSqlClassParams::StatementType::Insert,
                                                [&] (Utf8StringCR name, uint16_t count)
        {
        ecsql.append(1, '[').append(name).append(1, ']');
        if (0 < count)
            insertValues.append(1, ',');

        insertValues.append(":[").append(name).append(1, ']');
        });

    if (0 == numInsertParams)
        {
        ecsql.clear();
        return 0;
        }

    ecsql.append(")VALUES(").append(insertValues).append(1, ')');
    return numInsertParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t ECSqlClassParams::BuildSelectECSql(Utf8StringR ecsql, DgnDbCR dgndb, ECClassCR ecclass) const
    {
    ecsql.clear();
    ecsql.append("SELECT ");
    auto const& entries = GetEntries();
    uint16_t numSelectParams = buildParamString(ecsql, ecclass, entries, ECSqlClassParams::StatementType::Select,
                                                [&] (Utf8StringCR name, uint16_t count)
        {
        ecsql.append(1, '[').append(name).append(1, ']');
        });

    if (0 == numSelectParams)
        {
        ecsql.clear();
        return 0;
        }

    ecsql.append(" FROM ONLY ");
    AppendClassName(ecsql, dgndb, ecclass);
    ecsql.append(" WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter");
    return numSelectParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t ECSqlClassParams::BuildUpdateECSql(Utf8StringR ecsql, DgnDbCR dgndb, ECClassCR ecclass) const
    {
    ecsql.clear();
    ecsql.append("UPDATE ONLY ");
    AppendClassName(ecsql, dgndb, ecclass);
    ecsql.append(" SET ");

    auto const& entries = GetEntries();
    uint16_t numUpdateParams = buildParamString(ecsql, ecclass, entries, ECSqlClassParams::StatementType::Update,
                                                     [&] (Utf8StringCR name, uint16_t count)
        {
        ecsql.append(1, '[').append(name).append("]=:[").append(name).append(1, ']');
        });

    if (0 == numUpdateParams)
        {
        ecsql.clear();
        return 0;
        }

    ecsql.append(" WHERE ECInstanceId=?");
    return numUpdateParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSqlClassParams::BuildClassInfo(ECSqlClassInfo& info, DgnDbCR dgndb, DgnClassId classId) const
    {
    Utf8String select, insert, update;
    ECClassCP ecclass = dgndb.Schemas().GetClass(classId);
    BeAssert(nullptr != ecclass);
    if (nullptr == ecclass)
        return false;

    BuildSelectECSql(select, dgndb, *ecclass);
    BuildInsertECSql(insert, dgndb, *ecclass);
    uint16_t numUpdateParams = BuildUpdateECSql(update, dgndb, *ecclass);

    info.m_updateParameterIndex = numUpdateParams + 1;
    info.m_select = select;
    info.m_insert = insert;
    info.m_update = update;

    auto ehandler = dgn_ElementHandler::Element::FindHandler(dgndb, classId);
    if (nullptr != ehandler)
        {
        // Elements:
        auto& layout = ecclass->GetDefaultStandaloneEnabler()->GetClassLayout();

        // Tell handler to register its custom-handled property accessors
        ehandler->_RegisterPropertyAccessors(info, layout);

        // Make sure that each custom-handled property has accessors. (That's how we detect which properties are custom-handled.)
        AutoHandledPropertiesCollection props(*ecclass, const_cast<DgnDbR>(dgndb), ECSqlClassParams::StatementType::All, true);
        for (auto i = props.begin(); i != props.end(); ++i)
            {
            uint32_t propIdx;
            auto status = layout.GetPropertyIndex(propIdx, (*i)->GetName().c_str());
            BeAssert(ECObjectsStatus::Success == status);

            if (nullptr == info.GetPropertyAccessors(propIdx))
                {
                LOG.infov("%s.%s - missing accessors for custom-handled property", ecclass->GetECSqlName().c_str(), (*i)->GetName().c_str());

                static std::once_flag s_nullAccessorsFlag;
                static ECSqlClassInfo::T_ElementPropGet s_nullGetter;
                static ECSqlClassInfo::T_ElementPropSet s_nullSetter;
                std::call_once(s_nullAccessorsFlag, []() {
                    s_nullGetter = [](ECN::ECValueR, DgnElement const&) {BeAssert("Missing accessor"); return DgnDbStatus::MissingHandler;};
                    s_nullSetter = [](DgnElement&, ECN::ECValueCR) {BeAssert("Missing accessor"); return DgnDbStatus::MissingHandler;};
                    });
                info.RegisterPropertyAccessors(layout, (*i)->GetName().c_str(), s_nullGetter, s_nullSetter);
                }
            }

        // Record info about auto-handled properties
        AutoHandledPropertiesCollection autoprops(*ecclass, const_cast<DgnDbR>(dgndb), ECSqlClassParams::StatementType::All, false);
        for (auto i = autoprops.begin(); i != autoprops.end(); ++i)
            {
            if (ECSqlClassParams::StatementType::All != i.GetStatementType())
                {
                uint32_t propIdx;
                auto status = layout.GetPropertyIndex(propIdx, (*i)->GetName().c_str());
                BeAssert(ECObjectsStatus::Success == status);
                info.m_autoPropertyStatementType[propIdx] = (int32_t)i.GetStatementType();
                }
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassParams::Initialize(IECSqlClassParamsProvider& provider)
    {
    if (m_initialized)
        return;
    
    provider._GetClassParams(*this); // populate param entries
    std::sort(m_entries.begin(), m_entries.end(), [&](Entry const& a, Entry const& b) {return a.m_sortPriority < b.m_sortPriority;}); // sort SELECT entries to front in ascending "version" order
    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr ECSqlClassInfo::GetInsertStmt(DgnDbCR dgndb) const
    {
    return m_insert.empty() ? nullptr : dgndb.GetNonSelectPreparedECSqlStatement(m_insert.c_str(), dgndb.GetECCrudWriteToken());
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
    CachedECSqlStatementPtr stmt = m_update.empty() ? nullptr : dgndb.GetNonSelectPreparedECSqlStatement(m_update.c_str(), dgndb.GetECCrudWriteToken());
    if (stmt.IsValid())
        stmt->BindId(m_updateParameterIndex, id);

    return stmt;
    }

