/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceDeleter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   02 / 14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceDeleter::ECInstanceDeleter(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken)
    : m_ecdb(ecdb), m_ecClass(ecClass), m_isValid(false) 
    { 
    Initialize(writeToken); 
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   02 / 14
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceDeleter::Initialize(ECCrudWriteToken const* writeToken)
    {
    Utf8String ecsql("DELETE FROM ONLY ");
    ecsql.append(m_ecClass.GetECSqlName()).append(" WHERE ECInstanceId=?");

    ECSqlStatus stat = m_statement.Prepare(m_ecdb, ecsql.c_str(), writeToken);
    m_isValid = (stat.IsSuccess());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECInstanceDeleter::Delete(ECInstanceId ecInstanceId) const
    {
    if (!IsValid())
        {
        LOG.errorv("ECInstanceDeleter for ECClass '%s' is invalid as the ECClass is not mapped or cannot be used for deleting.", m_ecClass.GetFullName());
        return BE_SQLITE_ERROR;
        }

    if (!m_statement.IsPrepared())
        return BE_SQLITE_ERROR;

    m_statement.BindId(1, ecInstanceId);

    const DbResult stepStatus = m_statement.Step();

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return BE_SQLITE_DONE == stepStatus ? BE_SQLITE_OK : stepStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECInstanceDeleter::Delete(IECInstanceCR ecInstance) const
    {
    if (!ECInstanceAdapterHelper::Equals(ecInstance.GetClass(), m_ecClass))
        {
        Utf8String displayLabel;
        ecInstance.GetDisplayLabel(displayLabel);
        LOG.errorv("Failed to delete ECInstance '%s'. Invalid ECInstance passed to ECInstanceDeleter. ECClass mismatch: Expected ECClass: '%s'. ECInstance's ECClass: '%s'.",
                   displayLabel.c_str(), m_ecClass.GetFullName(), ecInstance.GetClass().GetFullName());
        return BE_SQLITE_ERROR;
        }

    ECInstanceId ecInstanceId;
    ECInstanceId::FromString(ecInstanceId, ecInstance.GetInstanceId().c_str());
    return Delete(ecInstanceId);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
