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
// @bsimethod                                   Carole.MacDonald                   02 / 14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceDeleter::ECInstanceDeleter
(
ECDbR ecdb, 
ECClassCR ecClass
) : m_ecdb (ecdb), m_ecClass (ecClass)
    {
    Initialize (nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceDeleter::ECInstanceDeleter
(
ECDbR ecdb,
ECClassCR ecClass,
ECSqlEventHandler& eventHandler
) : m_ecdb (ecdb), m_ecClass (ecClass)
    {
    Initialize (&eventHandler);
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
void ECInstanceDeleter::Initialize (ECSqlEventHandler* listener)
    {
    //register internal event handler
    auto stat = m_statement.RegisterEventHandler (m_internalEventHandler);
    if (stat != ECSqlStatus::Success)
        {
        m_isValid = false;
        return;
        }

    //register user-provided event handler
    if (listener != nullptr)
        {
        stat = m_statement.RegisterEventHandler (*listener);
        if (stat != ECSqlStatus::Success)
            {
            m_isValid = false;
            return;
            }
        }

    Utf8String ecsql ("DELETE FROM ONLY ");
    ecsql.append (ECSqlBuilder::ToECSqlSnippet (m_ecClass));
    ecsql.append (" WHERE ").append (ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY).append (" = ?");

    stat = m_statement.Prepare (m_ecdb, ecsql.c_str ());
    m_isValid = (stat == ECSqlStatus::Success);
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
        LOG.errorv (L"ECInstanceDeleter for ECClass '%ls' is invalid as the ECClass is not mapped or cannot be used for deleting.", m_ecClass.GetFullName ());
        return ERROR;
        }

    if (!m_statement.IsPrepared())
        return ERROR;

    m_statement.Reset();
    m_statement.ClearBindings();
    m_statement.BindId (1, ecInstanceId);

    m_internalEventHandler.Reset ();
    const ECSqlStepStatus stepStatus = m_statement.Step ();
    return (stepStatus == ECSqlStepStatus::Done && m_internalEventHandler.AreInstancesAffected ()) ? SUCCESS : ERROR;
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
        WString displayLabel;
        ecInstance.GetDisplayLabel (displayLabel);
        LOG.errorv (L"Failed to delete ECInstance '%ls'. Invalid ECInstance passed to ECInstanceDeleter. ECClass mismatch: Expected ECClass: '%ls'. ECInstance's ECClass: '%ls'.",
            displayLabel.c_str (), m_ecClass.GetFullName (), ecInstance.GetClass ().GetFullName ());
        return ERROR;
        }

    ECInstanceId ecInstanceId;
    ECInstanceIdHelper::FromString (ecInstanceId, ecInstance.GetInstanceId().c_str());
    return Delete (ecInstanceId);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
