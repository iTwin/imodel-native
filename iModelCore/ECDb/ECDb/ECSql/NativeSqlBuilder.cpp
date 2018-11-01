/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NativeSqlBuilder.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "NativeSqlBuilder.h"
#include "ExpHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::operator=(NativeSqlBuilder const& rhs)
    {
    if (this != &rhs)
        m_nativeSql = rhs.m_nativeSql;

    return *this;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::operator=(NativeSqlBuilder&& rhs)
    {
    if (this != &rhs)
        m_nativeSql = std::move(rhs.m_nativeSql);

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(ECN::ECClassId id)
    {
    Utf8Char classIdStr[ECN::ECClassId::ID_STRINGBUFFER_LENGTH];
    id.ToString(classIdStr);
    return Append(classIdStr);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(List const& builderList, Utf8CP separator)
    {
    bool isFirstBuilder = true;
    for (NativeSqlBuilder const& builder : builderList)
        {
        if (!isFirstBuilder)
            {
            if (separator == nullptr)
                AppendComma();
            else
                Append(separator);
            }

        Append(builder);
        isFirstBuilder = false;
        }

    return *this;
    }
    
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendFullyQualified(Utf8CP qualifier, Utf8CP identifier)
    {
    if (!Utf8String::IsNullOrEmpty(qualifier))
        AppendEscaped(qualifier).AppendDot();

    return AppendEscaped(identifier);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendFullyQualified(Utf8StringCR qualifier, Utf8StringCR identifier)
    {
    if (!qualifier.empty())
        AppendEscaped(qualifier).AppendDot();

    return AppendEscaped(identifier);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendFormatted(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Utf8String formattedMessage;
    formattedMessage.VSprintf(format, args);
    Append(formattedMessage);
    va_end(args);
    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
void NativeSqlBuilder::Push()
    {
    m_stack.push_back(m_nativeSql);
    m_nativeSql.clear();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String NativeSqlBuilder::Pop()
    {
    Utf8String r;
    if (m_stack.empty())
        {
        BeAssert(false && "Nothing to pop");
        return r;
        }

    r = m_nativeSql;
    m_nativeSql = m_stack.back();
    m_stack.pop_back();
    return r;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
NativeSqlBuilder::List NativeSqlBuilder::FlattenJaggedList(ListOfLists const& listOfLists, std::vector<size_t> const& indexSkipList)
    {
    List flattenedList;
    auto skipIt = indexSkipList.begin();
    auto skipEndIt = indexSkipList.end();
    for (size_t i = 0; i < listOfLists.size(); i++)
        {
        if (skipIt != skipEndIt && i == *skipIt)
            {
            skipIt++;
            continue;
            }

        NativeSqlBuilder::List const& innerList = listOfLists[i];
        flattenedList.insert(flattenedList.end(), innerList.begin(), innerList.end());
        }

    return flattenedList;
    }

//*******************************************************************************************
// SqlUpdateBuilder
//*******************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SqlUpdateBuilder::ExecuteSql(ECDb const& ecdb) const
    {
    if (!IsValid())
        return ERROR;

    Utf8String sql;
    sql.append("UPDATE main.[").append(m_table).append("] SET ");
    bool isFirstItem = true;
    for (std::pair<Utf8String, ECValue> const& kvPair : m_updateExpressions)
        {
        if (!isFirstItem)
            sql.append(",");

        sql.append("[").append(kvPair.first).append("]=?");

        isFirstItem = false;
        }

    if (!m_whereExpressions.empty())
        {
        sql.append(" WHERE ");
        bool isFirstItem = true;
        for (std::pair<Utf8String, ECValue> const& kvPair : m_whereExpressions)
            {
            if (!isFirstItem)
                sql.append(" AND ");

            sql.append("[").append(kvPair.first).append("]=?");

            isFirstItem = false;
            }
        }

    Statement stmt;
    if (stmt.Prepare(ecdb, sql.c_str()) != BE_SQLITE_OK)
        return ERROR;

    int paramIndex = 1;
    for (std::pair<Utf8String, ECValue> const& kvPair : m_updateExpressions)
        {
        if (SUCCESS != Bind(stmt, paramIndex, kvPair.first.c_str(), kvPair.second))
            return ERROR;

        paramIndex++;
        }

    for (std::pair<Utf8String, ECValue> const& kvPair : m_whereExpressions)
        {
        if (SUCCESS != Bind(stmt, paramIndex, kvPair.first.c_str(), kvPair.second))
            return ERROR;

        paramIndex++;
        }

    const BentleyStatus stat = stmt.Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
    BeAssert(ecdb.GetModifiedRowCount() > 0);
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SqlUpdateBuilder::Bind(Statement& stmt, int paramIndex, Utf8CP columName, ECValue const& val) const
    {
    if (val.IsNull())
        {
        stmt.BindNull(paramIndex);
        return SUCCESS;
        }

    switch (val.GetPrimitiveType())
        {
            case PRIMITIVETYPE_Integer:
                return stmt.BindInt(paramIndex, val.GetInteger()) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_Long:
                return stmt.BindInt64(paramIndex, val.GetLong()) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_Double:
                return stmt.BindDouble(paramIndex, val.GetDouble()) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_String:
                return stmt.BindText(paramIndex, val.GetUtf8CP(), Statement::MakeCopy::No) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_Boolean:
                return stmt.BindBoolean(paramIndex, val.GetBoolean()) == BE_SQLITE_OK ? SUCCESS : ERROR;
        }

    BeAssert(false && "Unsupported case");
    return ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
