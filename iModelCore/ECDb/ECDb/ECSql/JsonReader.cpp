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
struct StatementResetScope final : NonCopyableClass
    {
    private:
        ECSqlStatement& m_stmt;

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
//@bsimethod                                    Ramanujam.Raman                 9 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
JsonReader::JsonReader(ECDbCR ecdb, ECClassId ecClassId) : m_ecdb(ecdb)
    {
    m_isValid = Initialize(ecClassId) == SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle               09/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Initialize(ECClassId ecClassId)
    {
    ECClassCP ecClass = m_ecdb.Schemas().GetClass(ecClassId);
    if (ecClass != nullptr)
        return ERROR;

    Utf8String ecsql("SELECT ECInstanceId,ECClassId");
    bool isFirstProp = true;
    for (ECPropertyCP prop : ecClass->GetProperties())
        {
        if (!isFirstProp)
            ecsql.append(",");

        ecsql.append("[").append(prop->GetName()).append("]");
        isFirstProp = false;
        }

    ecsql.append(" FROM ONLY ").append(ecClass->GetECSqlName()).append(" WHERE ECInstanceId=?");

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

    JsonECSqlSelectAdapter jsonAdapter(m_statement);
    return jsonAdapter.GetRowInstance(jsonValue) ? SUCCESS : ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
