/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlEventManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlEventManager.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//*************************************************************************************
// ECSqlEventManager
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
ECSqlEventManager::ECSqlEventManager ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
ECSqlEventManager::~ECSqlEventManager ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlEventManager::RegisterEventHandler (ECSqlEventHandler& eventHandler)
    {
    auto it = std::find (m_eventHandlers.begin (), m_eventHandlers.end (), &eventHandler);
    if (it != m_eventHandlers.end ())
        return ECSqlStatus::UserError;

    m_eventHandlers.push_back (&eventHandler);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlEventManager::UnregisterEventHandler (ECSqlEventHandler& eventHandler)
    {
    auto it = std::find (m_eventHandlers.begin (), m_eventHandlers.end (), &eventHandler);
    if (it != m_eventHandlers.end ())
        {
        m_eventHandlers.erase (it);
        return ECSqlStatus::Success;
        }

    return ECSqlStatus::UserError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlEventManager::UnregisterAllEventHandlers ()
    {
    m_eventHandlers.clear ();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
void ECSqlEventManager::FireEvent (ECSqlEventHandler::EventType eventType) const
    {
    for (auto handler : m_eventHandlers)
        {
        handler->OnEvent (eventType, m_eventArgs);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        07/14
//---------------------------------------------------------------------------------------
void ECSqlEventManager::ResetEventArgs ()
    {
    m_eventArgs.GetInstanceKeysR ().clear ();
    }


//*************************************************************************************
// ECSqlEventArgs
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan   06/2014
//---------------------------------------------------------------------------------------
ECSqlEventArgs::ECSqlEventArgs ()
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan   06/2014
//---------------------------------------------------------------------------------------
ECSqlEventArgs::~ECSqlEventArgs ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan   06/2014
//---------------------------------------------------------------------------------------
ECSqlEventArgs::ECInstanceKeyList const&  ECSqlEventArgs::GetInstanceKeys () const
    {
    return m_instanceKeyList;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan   06/2014
//---------------------------------------------------------------------------------------
ECSqlEventArgs::ECInstanceKeyList& ECSqlEventArgs::GetInstanceKeysR ()
    {
    return m_instanceKeyList;
    }


//*************************************************************************************
// ECSqlEventHandler
//*************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan   06/2014
//---------------------------------------------------------------------------------------
ECSqlEventHandler::ECSqlEventHandler ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan   06/2014
//---------------------------------------------------------------------------------------
ECSqlEventHandler::~ECSqlEventHandler ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan   06/2014
//---------------------------------------------------------------------------------------
void ECSqlEventHandler::OnEvent (EventType eventType, ECSqlEventArgs const& args)
    {
    _OnEvent (eventType, args);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE