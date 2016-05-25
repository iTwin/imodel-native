/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SqlUtilities.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbPolicyManager.h"
#include <set>
#include "ECSql/ECSqlPrepareContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=====================================SqlViewBuilder===================================

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlViewBuilder::IsValid() const
    {
    if (m_name.empty())
        {
        BeAssert(false && "Must specify a view name");
        return false;
        }

    if (m_selectStatementList.empty())
        {
        BeAssert(false && "View must have atleast one select statement");
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8String SqlViewBuilder::ToString(SqlOption option, bool escape, bool useUnionAll) const
    {
    if (!IsValid())
        {
        BeAssert(false && "view specification is not valid");
        }

    NativeSqlBuilder sql;
    if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
        {
        sql.Append("DROP VIEW ").AppendIf(option == SqlOption::DropIfExists, "IF EXISTS ").Append(GetName(), escape).Append(";");
        }
    else
        {
        sql.Append("CREATE VIEW ").AppendIf(option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").Append(GetName(), escape).AppendLine(" AS");

        if (!m_sqlComment.empty())
            sql.Append("--").AppendLine(m_sqlComment.c_str());

        for (auto& select : m_selectStatementList)
            {
            if (&select != &m_selectStatementList.front())
                sql.Append("UNION ").AppendIf(useUnionAll, "ALL").AppendEol();

            sql.AppendLine(select.ToString());
            }
        sql.Append(";");
        }

    return sql.ToString();
    }

//=====================================SqlTriggerBuilder::TriggerList====================

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::TriggerList::TriggerList()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder& SqlTriggerBuilder::TriggerList::Create(SqlTriggerBuilder::Type type, SqlTriggerBuilder::Condition condition, bool temprary)
    {
    m_list.push_back(SqlTriggerBuilder(type, condition, temprary));
    return m_list.back();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::TriggerList::List const& SqlTriggerBuilder::TriggerList::GetTriggers() const { return m_list; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void SqlTriggerBuilder::TriggerList::Delete(SqlTriggerBuilder const& trigger)
    {
    for (auto itor = m_list.begin(); itor != m_list.end(); ++itor)
        {
        if (&(*itor) == &trigger)
            {
            m_list.erase(itor);
            break;
            }
        }
    }

//=====================================SqlTriggerBuilder=================================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::SqlTriggerBuilder(Type type, Condition condition, bool temprary)
    :m_type(type), m_condition(condition), m_temprory(temprary)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::SqlTriggerBuilder(SqlTriggerBuilder && rhs)
    : m_type(std::move(rhs.m_type)), m_condition(std::move(rhs.m_condition)), m_temprory(std::move(rhs.m_temprory)), m_name(std::move(rhs.m_name)),
    m_when(std::move(rhs.m_when)), m_body(std::move(rhs.m_body)), m_on(rhs.m_on), m_ofColumns(std::move(rhs.m_ofColumns))
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::SqlTriggerBuilder(SqlTriggerBuilder const& rhs)
    : m_type(rhs.m_type), m_condition(rhs.m_condition), m_temprory(rhs.m_temprory), m_name(rhs.m_name),
    m_when(rhs.m_when), m_body(rhs.m_body), m_on(rhs.m_on), m_ofColumns(rhs.m_ofColumns)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder& SqlTriggerBuilder::operator = (SqlTriggerBuilder&& rhs)
    {
    if (this != &rhs)
        {
        m_name = rhs.m_name;
        m_type = rhs.m_type;
        m_body = rhs.m_body;
        m_condition = rhs.m_condition;
        m_on = rhs.m_on;
        m_temprory = rhs.m_temprory;
        m_when = rhs.m_when;
        rhs.m_ofColumns = rhs.m_ofColumns;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder& SqlTriggerBuilder::operator = (SqlTriggerBuilder const& rhs)
    {
    if (this != &rhs)
        {
        m_name = std::move(rhs.m_name);
        m_type = std::move(rhs.m_type);
        m_body = std::move(rhs.m_body);
        m_condition = std::move(rhs.m_condition);
        m_on = std::move(rhs.m_on);
        m_temprory = std::move(rhs.m_temprory);
        m_when = std::move(rhs.m_when);
        m_ofColumns = std::move(rhs.m_ofColumns);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetNameBuilder() { return m_name; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetWhenBuilder() { return m_when; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetBodyBuilder() { return m_body; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetOnBuilder() { return m_on; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetName() const { return m_name.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetWhen() const { return m_when.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetBody() const { return m_body.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetOn() const { return m_on.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlTriggerBuilder::IsTemporary() const { return m_temprory; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlTriggerBuilder::IsValid() const
    {
    if (m_name.IsEmpty())
        {
        BeAssert(false && "Must specify a trigger name");
        return false;
        }

    if (m_on.IsEmpty())
        {
        BeAssert(false && "Must specify a trigger ON Table/View");
        return false;
        }

    if (m_body.IsEmpty())
        {
        BeAssert(false && "Must specify a trigger body");
        return false;
        }

    if (m_type == Type::UpdateOf && m_ofColumns.empty())
        {
        BeAssert(false && "For UPDATE OF trigger must specify atleast one column");
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8String SqlTriggerBuilder::ToString(SqlOption option, bool escape) const
    {
    NativeSqlBuilder sql;
    if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
        {
        BeAssert(!m_name.IsEmpty() && "At least trigger name must be specified when trying to delete it");
        sql.Append("DROP TRIGGER ").AppendIf(option == SqlOption::DropIfExists, "IF EXISTS ").Append(GetName(), escape).Append(";");
        }
    else
        {
        if (!IsValid())
            {
            BeAssert(false && "Trigger specification is not valid");
            return Utf8String();
            }

        sql.Append("CREATE TRIGGER ").AppendIf(IsTemporary(), "TEMP ").AppendIf(option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").Append(GetName(), escape).AppendEol();
        switch (m_condition)
            {
                case Condition::After:
                    sql.Append("AFTER "); break;
                case Condition::Before:
                    sql.Append("BEFORE "); break;
                case Condition::InsteadOf:
                    sql.Append("INSTEAD OF "); break;
            }

        switch (m_type)
            {
                case Type::Delete:
                    sql.Append("DELETE "); break;
                case Type::Insert:
                    sql.Append("INSERT "); break;
                case Type::Update:
                    sql.Append("UPDATE "); break;
                case Type::UpdateOf:
                    sql.Append("UPDATE OF ");
                    for (auto& column : m_ofColumns)
                        {
                        if (&column != &m_ofColumns.front())
                            sql.Append(", ");

                        sql.Append(column.c_str(), escape);
                        }
                    break;
            }

        sql.AppendEol();
        sql.Append("ON ").Append(GetOn(), escape).AppendEol();
        if (!m_when.IsEmpty())
            {
            sql.Append("  WHEN ").Append(GetWhen()).AppendEol();
            }

        sql.Append("BEGIN").AppendEol();
        sql.Append(GetBody());
        sql.Append("END;").AppendEol();;
        }

    return sql.ToString();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE