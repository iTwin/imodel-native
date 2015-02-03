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
    std::vector<ECSqlEventHandler*> m_eventHandlers;

    ECSqlEventArgs m_eventArgs;

public:
    ECSqlEventManager ();
    ~ECSqlEventManager ();

    bool HasEventHandlers () const { return !m_eventHandlers.empty (); }

    void FireEvent (ECSqlEventHandler::EventType eventType) const;
    void ResetEventArgs ();
    ECSqlEventArgs& GetEventArgsR () { return m_eventArgs; }


    ECSqlStatus RegisterEventHandler (ECSqlEventHandler& eventHandler);
    ECSqlStatus UnregisterEventHandler (ECSqlEventHandler& eventHandler);
    ECSqlStatus UnregisterAllEventHandlers ();
    
    };

END_BENTLEY_SQLITE_EC_NAMESPACE