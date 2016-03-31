/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NativeSqlBuilder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "NativeSqlBuilder.h"
#include "ExpHelper.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::operator= (NativeSqlBuilder const& rhs)
    {
    if (this != &rhs)
        {
        m_nativeSql = rhs.m_nativeSql;
        m_parameterIndexMappings = rhs.m_parameterIndexMappings;
        }

    return *this;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::operator= (NativeSqlBuilder&& rhs)
    {
    if (this != &rhs)
        {
        m_nativeSql = std::move(rhs.m_nativeSql);
        m_parameterIndexMappings = std::move(rhs.m_parameterIndexMappings);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::CopyParameters(NativeSqlBuilder const& sqlBuilder)
    {
    if (this != &sqlBuilder)
        m_parameterIndexMappings = sqlBuilder.m_parameterIndexMappings;

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(NativeSqlBuilder const& builder)
    {
    if (!builder.m_parameterIndexMappings.empty())
        m_parameterIndexMappings.insert(m_parameterIndexMappings.end(), builder.m_parameterIndexMappings.begin(), builder.m_parameterIndexMappings.end());

    return Append(builder.ToString());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(List const& builderList, Utf8CP separator)
    {
    auto isFirstBuilder = true;
    for (auto const& builder : builderList)
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
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(List const& lhsBuilderList, Utf8CP operatorStr, List const& rhsBuilderList, Utf8CP separator /*= nullptr*/)
    {
    BeAssert(lhsBuilderList.size() == rhsBuilderList.size());
    BeAssert(!Utf8String::IsNullOrEmpty(operatorStr));

    const size_t builderCount = lhsBuilderList.size();
    bool isFirstBuilder = true;
    for (size_t i = 0; i < builderCount; i++)
        {
        if (!isFirstBuilder)
            {
            if (Utf8String::IsNullOrEmpty(separator))
                AppendComma();
            else
                Append(separator);
            }

        Append(lhsBuilderList[i]).Append(operatorStr).Append(rhsBuilderList[i]);

        isFirstBuilder = false;
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(Utf8CP classIdentifier, Utf8CP identifier)
    {
    if (!Utf8String::IsNullOrEmpty(classIdentifier))
        AppendEscaped(classIdentifier).AppendDot();

    return AppendEscaped(identifier);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(BinarySqlOperator op)
    {
    //No difference of operator in ECSQL and SQLite SQL
    return Append(ExpHelper::ToSql(op));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(BooleanSqlOperator op)
    {
    //No difference of operator in ECSQL and SQLite SQL
    return Append(ExpHelper::ToSql(op));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(SqlSetQuantifier setQuantifier)
    {
    //No difference of operator in ECSQL and SQLite SQL
    if (setQuantifier == SqlSetQuantifier::NotSpecified)
        return *this;

    Utf8CP selectionTypeStr = ExpHelper::ToSql(setQuantifier);
    Append(selectionTypeStr);
    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append(UnarySqlOperator op)
    {
    //No difference of operator in ECSQL and SQLite SQL
    return Append(ExpHelper::ToSql(op));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendFormatted(Utf8CP format, ...)
    {
    va_list ap;
    va_start(ap, format);
    Append(Utf8PrintfString(format, ap).c_str());
    va_end(ap);
    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
void NativeSqlBuilder::Push(bool clear /*= true*/)
    {
    m_stack.push_back(m_nativeSql);

    if (clear)
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
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendParameter(Utf8CP ecsqlParameterName, int ecsqlParameterComponentIndex)
    {
    if (!Utf8String::IsNullOrEmpty(ecsqlParameterName))
        {
        Append(":").Append(ecsqlParameterName, false);

        Utf8String nativeSqlParameterName;
        nativeSqlParameterName.Sprintf("_%d", ecsqlParameterComponentIndex);
        Append(nativeSqlParameterName.c_str());
        }
    else
        Append("?");

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendParameter(Utf8CP ecsqlParameterName, int ecsqlParameterIndex, int ecsqlParameterComponentIndex)
    {
    AppendParameter(ecsqlParameterName, ecsqlParameterComponentIndex);

    m_parameterIndexMappings.push_back(ECSqlParameterIndex(ecsqlParameterIndex, ecsqlParameterComponentIndex));

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
void NativeSqlBuilder::Reset()
    {
    m_nativeSql.clear();
    m_stack.clear();
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

        auto const& innerList = listOfLists[i];
        flattenedList.insert(flattenedList.end(), innerList.begin(), innerList.end());
        }

    return flattenedList;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
