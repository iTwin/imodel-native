/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonReader.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=====================================================================================
// !Automatically resets a statement and clears its bindings once the scope is destructed
// @bsiclass                                               Krischan.Eberle     09/2017
//+===============+===============+===============+===============+===============+======
struct StatementResetScope final
    {
    private:
        ECSqlStatement& m_stmt;

        //not copyable
        StatementResetScope(StatementResetScope const&) = delete;
        StatementResetScope& operator=(StatementResetScope const&) = delete;

    public:
        explicit StatementResetScope(ECSqlStatement& stmt) : m_stmt(stmt) {}
        ~StatementResetScope()
            {
            if (m_stmt.IsPrepared())
                {
                m_stmt.Reset();
                m_stmt.ClearBindings();
                }
            }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                    Krischan.Eberle                09/2017
//+---------------+---------------+---------------+---------------+---------------+------
JsonReader::JsonReader(ECDbCR ecdb, ECClassCR ecClass, JsonECSqlSelectAdapter::FormatOptions const& formatOptions) : m_ecdb(ecdb), m_formatOptions(formatOptions) { m_isValid = Initialize(ecClass) == SUCCESS; }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Ramanujam.Raman                 09/2013
//+---------------+---------------+---------------+---------------+---------------+------
JsonReader::JsonReader(ECDbCR ecdb, ECClassId ecClassId, JsonECSqlSelectAdapter::FormatOptions const& formatOptions) : m_ecdb(ecdb), m_formatOptions(formatOptions) { m_isValid = Initialize(ecClassId) == SUCCESS; }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle               09/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Initialize(ECClassId ecClassId)
    {
    ECClassCP ecClass = m_ecdb.Schemas().GetClass(ecClassId);
    if (ecClass == nullptr)
        return ERROR;
    
    return Initialize(*ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle               09/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Initialize(ECClassCR ecClass)
    {
    Utf8String ecsql("SELECT ECInstanceId,ECClassId");

    if (ecClass.IsRelationshipClass())
        ecsql.append(",SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId");

    for (ECPropertyCP prop : ecClass.GetProperties())
        {
        ecsql.append(",[").append(prop->GetName()).append("]");
        }

    ecsql.append(" FROM ONLY ").append(ecClass.GetECSqlName()).append(" WHERE ECInstanceId=?");

    if (ECSqlStatus::Success != m_statement.Prepare(m_ecdb, ecsql.c_str()))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 09 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Read(Json::Value& jsonValue, ECInstanceId ecInstanceId) const
    {
    if (!IsValid())
        return ERROR;

    BeAssert(m_statement.IsPrepared());

    if (ECSqlStatus::Success != m_statement.BindId(1, ecInstanceId))
        return ERROR;

    StatementResetScope stmtScope(m_statement);

    if (BE_SQLITE_ROW != m_statement.Step())
        return ERROR;

    JsonECSqlSelectAdapter jsonAdapter(m_statement, m_formatOptions);
    return jsonAdapter.GetRow(jsonValue);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
