/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auievent.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_EC_NAMESPACE

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

        void*   GeteventHub () const {return m_eventHub;}
        void    SeteventHub (void* eventHub) {m_eventHub = eventHub;}
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSelectionEvent: public ECEvent
    {
    private:
        ECInstanceIterableDataContextP  m_instanceIterable;
        void*                           m_selectionSource;
    public:
    ECSelectionEvent (void* eventhub, ECInstanceIterableDataContextR instanceIterable)
        :ECEvent(eventhub), m_instanceIterable(&instanceIterable), m_selectionSource(NULL)
        {}
    
    ECInstanceIterableDataContextCP GetInstanceIterable () const {return m_instanceIterable;}

    void    SetSelectionSource (void* src) {m_selectionSource = src;}
    void*   GetSelectionSource () const {return m_selectionSource;}

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSelectionListener
    {
    friend ECPresentationManager;
    protected:
    //! Return NULL to listen to all events
    virtual void const* _GeteventHub () const = 0;

    virtual void _OnSelection (ECSelectionEventCR selectionEvent) = 0;


    public:
        ECOBJECTS_EXPORT void const *  GeteventHub () const;

        ECOBJECTS_EXPORT void   OnSelection (ECSelectionEventCR selectionEvent);
    };


END_BENTLEY_EC_NAMESPACE