/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auievent.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <ECObjects/ECEvent.h>
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*=================================================================================**//**
* @bsistruct                                    Abeesh.Basheer                  06/2012
+===============+===============+===============+===============+===============+======*/
struct ECSelectionEvent: public ECEvent
    {
    private:
        IAUIDataContextP                m_dataContext;
        void*                           m_selectionSource;
    public:
    ECSelectionEvent (void* eventhub, IAUIDataContextR dataContext)
        :ECEvent(eventhub), m_dataContext(&dataContext), m_selectionSource(NULL)
        {}
    
    IAUIDataContextP GetContext () const {return m_dataContext;}

    void    SetSelectionSource (void* src) {m_selectionSource = src;}
    void*   GetSelectionSource () const {return m_selectionSource;}

    };

/*=================================================================================**//**
* @bsistruct                                    Abeesh.Basheer                  06/2012
+===============+===============+===============+===============+===============+======*/
struct ECSelectionListener : public IECPresentationProvider
    {
    friend struct ECPresentationManager;
    protected:
    //! Return NULL to listen to all events
    virtual void const*       _GetEventHub () const = 0;

    virtual int               _GetPriority () const { return 1000; }

    virtual void              _OnSelection (ECSelectionEventCR selectionEvent) = 0;

    virtual void              _OnSubSelection (ECSelectionEventCR selectionEvent) {}

    virtual IAUIDataContextCP _GetSelection (bool subSelection) { return NULL; }

    virtual ProviderType      _GetProviderType(void) const override { return SelectionService; }

    virtual int               _GetSelectionQueryScope (void) const {return 0;}

    public:
        ECOBJECTS_EXPORT void const *      GetEventHub () const;

        ECOBJECTS_EXPORT int               GetPriority () const;

        ECOBJECTS_EXPORT void              OnSelection (ECSelectionEventCR selectionEvent);

        ECOBJECTS_EXPORT void              OnSubSelection (ECSelectionEventCR selectionEvent);

        ECOBJECTS_EXPORT IAUIDataContextCP GetSelection (bool subSelection);
    };


END_BENTLEY_ECOBJECT_NAMESPACE