/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlEventManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include  <ECDb/ECSqlStatement.h>
#include "ECSqlStatusContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECSqlEventManager manage events
//! @bsiclass                                                Affan.Khan      06/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlEventManager : NonCopyableClass
    {
private:
    mutable std::vector<ECSqlEventHandler*> m_eventHandlers;
    mutable std::unique_ptr<DefaultECSqlEventHandler> m_defaultHandler;
    ECSqlEventArgs m_eventArgs;

    DefaultECSqlEventHandler& GetDefaultEventHandlerR() const;

public:
    ECSqlEventManager ();
    ~ECSqlEventManager ();

    bool HasEventHandlers () const { return !m_eventHandlers.empty (); }

    void FireEvent (ECSqlEventHandler::EventType eventType) const;
    ECSqlEventArgs& GetEventArgsR () { return m_eventArgs; }

    //! Resets the event args, but doesn't unregister any handlers
    void Reset();

    void RegisterEventHandler (ECSqlEventHandler& eventHandler);
    ECSqlStatus UnregisterEventHandler (ECSqlEventHandler& eventHandler);
    void UnregisterAllEventHandlers ();

    void ToggleDefaultEventHandler(bool enable);
    DefaultECSqlEventHandler const* GetDefaultEventHandler() const;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE