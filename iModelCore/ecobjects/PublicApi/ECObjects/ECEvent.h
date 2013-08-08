/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECEvent.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECEvent
    {
    private:
        void*   m_eventHub;

    public:
        ECEvent ()
            :m_eventHub (NULL)
            {}

        ECEvent (void* eventHub)
            :m_eventHub(eventHub)
            {}

        void*   GetEventHub () const {return m_eventHub;}
        void    SetEventHub (void* eventHub) {m_eventHub = eventHub;}
    };

END_BENTLEY_ECOBJECT_NAMESPACE
