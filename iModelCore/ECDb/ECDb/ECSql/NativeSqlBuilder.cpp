/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NativeSqlBuilder.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "NativeSqlBuilder.h"
#include "ExpHelper.h"

using namespace std;
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder::NativeSqlBuilder ()
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder::NativeSqlBuilder (Utf8CP initialString)
    : m_nativeSql (initialString)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder::NativeSqlBuilder (NativeSqlBuilder const& rhs)
    : m_nativeSql (rhs.m_nativeSql), m_parameterIndexMappings (rhs.m_parameterIndexMappings)
    {
    }

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
NativeSqlBuilder::NativeSqlBuilder (NativeSqlBuilder&& rhs)
    : m_nativeSql (move (rhs.m_nativeSql)), m_parameterIndexMappings (move (rhs.m_parameterIndexMappings))
    {
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::operator= (NativeSqlBuilder&& rhs)
    {
    if (this != &rhs)
        {
        m_nativeSql = move (rhs.m_nativeSql);
        m_parameterIndexMappings = move (rhs.m_parameterIndexMappings);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append (Utf8CP str, bool appendTrailingSpace) 
    { 
    m_nativeSql.append (str);
    if (appendTrailingSpace)
        AppendSpace ();

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append (NativeSqlBuilder const& builder, bool appendTrailingSpace)
    {
    if (!builder.m_parameterIndexMappings.empty ())
        m_parameterIndexMappings.insert (m_parameterIndexMappings.end (), builder.m_parameterIndexMappings.begin (), builder.m_parameterIndexMappings.end ());

    return Append (builder.ToString (), appendTrailingSpace);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append (List const& builderList, Utf8CP separator)
    {
    auto isFirstBuilder = true;
    for (auto const& builder : builderList)
        {
        if (!isFirstBuilder)
            {
            if (separator == nullptr)
                AppendComma ();
            else
                Append (separator);
            }

        Append (builder);
        isFirstBuilder = false;
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append (List const& lhsBuilderList, Utf8CP operatorStr, List const& rhsBuilderList, Utf8CP separator /*= nullptr*/)
    {
    BeAssert (lhsBuilderList.size () == rhsBuilderList.size ());
    BeAssert (!Utf8String::IsNullOrEmpty (operatorStr));

    const size_t builderCount = lhsBuilderList.size ();
    bool isFirstBuilder = true;
    for (size_t i = 0; i < builderCount; i++)
        {
        if (!isFirstBuilder)
            {
            if (Utf8String::IsNullOrEmpty (separator))
                AppendComma ();
            else
                Append (separator);
            }

        Append (lhsBuilderList[i]).Append (operatorStr).Append (rhsBuilderList[i]);

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
NativeSqlBuilder& NativeSqlBuilder::Append (BinarySqlOperator op, bool appendTrailingSpace)
    {
    //No difference of operator in ECSQL and SQLite SQL
    return Append (ExpHelper::ToString (op), appendTrailingSpace);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append (BooleanSqlOperator op, bool appendTrailingSpace)
    {
    //No difference of operator in ECSQL and SQLite SQL
    return Append (ExpHelper::ToString (op), appendTrailingSpace);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append (SqlSetQuantifier setQuantifier, bool appendTrailingSpace)
    {
    //No difference of operator in ECSQL and SQLite SQL
    auto selectionTypeStr = ExpHelper::ToString (setQuantifier);
    if (!Utf8String::IsNullOrEmpty (selectionTypeStr))
        Append (selectionTypeStr, appendTrailingSpace);

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append (UnarySqlOperator op, bool appendTrailingSpace)
    {
    //No difference of operator in ECSQL and SQLite SQL
    return Append (ExpHelper::ToString (op), appendTrailingSpace);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendParameter (Utf8CP ecsqlParameterName, int ecsqlParameterComponentIndex)
    {
    if (!Utf8String::IsNullOrEmpty (ecsqlParameterName))
        {
        Append (":").Append (ecsqlParameterName, false);

        Utf8String nativeSqlParameterName;
        nativeSqlParameterName.Sprintf ("_%d", ecsqlParameterComponentIndex);
        Append (nativeSqlParameterName.c_str ());
        }
    else
        Append ("?");

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendParameter (Utf8CP ecsqlParameterName, int ecsqlParameterIndex, int ecsqlParameterComponentIndex)
    {
    AppendParameter (ecsqlParameterName, ecsqlParameterComponentIndex);

    m_parameterIndexMappings.push_back (ECSqlParameterIndex (ecsqlParameterIndex, ecsqlParameterComponentIndex));

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::Append (ECClassId ecClassId)
    {
    Utf8String classIdStr;
    classIdStr.Sprintf ("%lld", ecClassId);
    return Append (classIdStr.c_str ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendEscaped (Utf8CP identifier)
    {
    return Append ("[").Append (identifier).Append ("]");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendQuoted (Utf8CP stringLiteral)
    {
    return Append ("'").Append (stringLiteral).Append ("'");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendSpace ()
    {
    return Append (" ");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendComma (bool appendTrailingSpace)
    {
    return Append (",", appendTrailingSpace);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendDot()
    {
    return Append (".");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendParenLeft()
    {
    return Append ("(");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
NativeSqlBuilder& NativeSqlBuilder::AppendParenRight()
    {
    return Append (")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool NativeSqlBuilder::IsEmpty () const
    {
    return m_nativeSql.empty ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP NativeSqlBuilder::ToString () const
    {
    return m_nativeSql.c_str ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
NativeSqlBuilder::List NativeSqlBuilder::FlattenJaggedList (ListOfLists const& listOfLists, std::vector<size_t> const& indexSkipList)
    {
    List flattenedList;
    auto skipIt = indexSkipList.begin ();
    auto skipEndIt = indexSkipList.end ();
    for (size_t i = 0; i < listOfLists.size (); i++)
        {
        if (skipIt != skipEndIt && i == *skipIt)
            {
            skipIt++;
            continue;
            }

        auto const& innerList = listOfLists[i];
        flattenedList.insert (flattenedList.end (), innerList.begin (), innerList.end ());
        }

    return move (flattenedList);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
