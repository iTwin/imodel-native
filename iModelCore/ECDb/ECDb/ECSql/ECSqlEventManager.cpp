/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlEventManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
ECSqlEventManager::ECSqlEventManager() : m_defaultHandler(nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
ECSqlEventManager::~ECSqlEventManager ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
void ECSqlEventManager::RegisterEventHandler (ECSqlEventHandler& eventHandler)
    {
    auto it = std::find(m_eventHandlers.begin(), m_eventHandlers.end(), &eventHandler);
    if (it == m_eventHandlers.end())
        m_eventHandlers.push_back (&eventHandler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlEventManager::UnregisterEventHandler (ECSqlEventHandler& eventHandler)
    {
    auto it = std::find (m_eventHandlers.begin (), m_eventHandlers.end (), &eventHandler);
    if (it != m_eventHandlers.end ())
        {
        if (m_defaultHandler != nullptr && m_defaultHandler.get() == *it)
            m_defaultHandler = nullptr;

        m_eventHandlers.erase (it);
        return ECSqlStatus::Success;
        }

    return ECSqlStatus::UserError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        06/14
//---------------------------------------------------------------------------------------
void ECSqlEventManager::UnregisterAllEventHandlers ()
    {
    m_eventHandlers.clear ();
    m_defaultHandler = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        02/15
//---------------------------------------------------------------------------------------
void ECSqlEventManager::ToggleDefaultEventHandler(bool enable)
    {
    if (enable)
        {
        if (m_defaultHandler == nullptr)
            {
            m_defaultHandler = std::unique_ptr<DefaultECSqlEventHandler>(new DefaultECSqlEventHandler());
            RegisterEventHandler(*m_defaultHandler);
            }
        }
    else
        {
        if (m_defaultHandler != nullptr)
            {
            UnregisterEventHandler(*m_defaultHandler);
            BeAssert(m_defaultHandler == nullptr && "If UnregisterEventHandler is called with the default handler, the method is expected to null out m_defaultHandler.");
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        02/15
//---------------------------------------------------------------------------------------
DefaultECSqlEventHandler const* ECSqlEventManager::GetDefaultEventHandler() const
    {
    return m_defaultHandler.get();
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
void ECSqlEventManager::Reset ()
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


//*************************************************************************************
// DefaultECSqlEventHandler
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2015
//---------------------------------------------------------------------------------------
DefaultECSqlEventHandler::DefaultECSqlEventHandler() : ECSqlEventHandler(), m_args (nullptr) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2015
//---------------------------------------------------------------------------------------
void DefaultECSqlEventHandler::_OnEvent(EventType eventType, ECSqlEventArgs const& args)
    {
    m_eventType = eventType;
    m_args = &args;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2015
//---------------------------------------------------------------------------------------
int DefaultECSqlEventHandler::GetInstancesAffectedCount() const
    {
    if (m_args == nullptr)
        return -1;

    return (int) m_args->GetInstanceKeys().size();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE