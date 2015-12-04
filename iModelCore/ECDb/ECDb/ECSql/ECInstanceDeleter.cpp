/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceDeleter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE



//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceDeleter::ECInstanceDeleter
(
ECDbCR ecdb,
ECClassCR ecClass
) : m_ecdb (ecdb), m_ecClass (ecClass)
    {
    Initialize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceDeleter::~ECInstanceDeleter ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
bool ECInstanceDeleter::IsValid () const
    {
    return m_isValid;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   02 / 14
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceDeleter::Initialize()
    {
    Utf8String ecsql ("DELETE FROM ONLY ");
    ecsql.append (m_ecClass.GetECSqlName());
    ecsql.append (" WHERE ECInstanceId=?");

    ECSqlStatus stat = m_statement.Prepare (m_ecdb, ecsql.c_str ());
    m_isValid = (stat.IsSuccess());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceDeleter::Delete
(
ECInstanceId const& ecInstanceId
) const
    {
    if (!IsValid ())
        {
        LOG.errorv ("ECInstanceDeleter for ECClass '%s' is invalid as the ECClass is not mapped or cannot be used for deleting.", m_ecClass.GetFullName ());
        return ERROR;
        }

    if (!m_statement.IsPrepared())
        return ERROR;

    m_statement.BindId (1, ecInstanceId);

    const DbResult stepStatus = m_statement.Step ();

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return (BE_SQLITE_DONE == stepStatus) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceDeleter::Delete
(
IECInstanceCR ecInstance
) const
    {
    if (ecInstance.GetClass () != m_ecClass)
        {
        Utf8String displayLabel;
        ecInstance.GetDisplayLabel (displayLabel);
        LOG.errorv ("Failed to delete ECInstance '%s'. Invalid ECInstance passed to ECInstanceDeleter. ECClass mismatch: Expected ECClass: '%s'. ECInstance's ECClass: '%s'.",
            displayLabel.c_str (), m_ecClass.GetFullName (), ecInstance.GetClass ().GetFullName ());
        return ERROR;
        }

    ECInstanceId ecInstanceId;
    ECInstanceIdHelper::FromString (ecInstanceId, ecInstance.GetInstanceId().c_str());
    return Delete (ecInstanceId);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
