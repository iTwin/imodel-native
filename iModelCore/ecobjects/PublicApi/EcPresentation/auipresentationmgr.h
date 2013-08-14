/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auipresentationmgr.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/

#pragma once

#include <Geom/GeomApi.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  ECPresentationManager: public NonCopyableClass
    {
    private:
        typedef bvector<RefCountedPtr<ECPresentationCommandProvider> >      T_CmdProviderSet;
        typedef bvector<RefCountedPtr<IECPresentationViewProvider> >        T_ViewProviderSet;
        typedef bvector<RefCountedPtr<IJournalProvider> >                   T_JournalProviderSet;
        typedef bvector<RefCountedPtr<IAUIContentServiceProvider> >         T_ContentProviderSet;
        typedef bvector<RefCountedPtr<ECPresentationImageProvider> >        T_ImageProviderSet;
        typedef bvector<RefCountedPtr<ECPresentationLocalizationProvider> > T_LocalizationProviderSet;
        typedef bvector<RefCountedPtr<ECSelectionListener> >                T_SelectionListeners;
        T_CmdProviderSet          m_cmdProviders;
        T_ViewProviderSet         m_viewProviders;
        T_JournalProviderSet      m_journalProviders;
        T_ContentProviderSet      m_contentProviders;
        T_ImageProviderSet        m_imageProviders;
        T_LocalizationProviderSet m_localizationProviders;
        T_SelectionListeners      m_selecitonListeners;

        ECPresentationManager ();
        
        IECPresentationViewDefinitionPtr AggregateViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR instanceData) const;

        template <typename ProviderType, typename ContainerType> 
        static bool CheckAndAddProviderFromList (ProviderType & provider, ContainerType& );

        template <typename ProviderType, typename ContainerType> 
        static void RemoveProviderFromList (ProviderType & provider, ContainerType& );

    public:
    static const int GENERAL_PURPOSE_QUERY = 0;
    
    //! Get the presentation manager.
    ECOBJECTS_EXPORT static ECPresentationManagerR  GetManager();

    //! Add or remove the journal provider
    ECOBJECTS_EXPORT void                           AddProvider (IJournalProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IJournalProviderR provider);

    //! Add or remove the command provider
    ECOBJECTS_EXPORT void                           AddProvider (ECPresentationCommandProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (ECPresentationCommandProviderR provider);

    //! Obtain command provider by id
    ECOBJECTS_EXPORT ECPresentationCommandProviderCP GetCommandProviderById (size_t providerId);

    //! Add or remove the view definition provider
    ECOBJECTS_EXPORT void                           AddProvider (IECPresentationViewProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IECPresentationViewProviderR provider);

    //! Add or remove the view definition provider
    ECOBJECTS_EXPORT void                           AddProvider (IAUIContentServiceProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IAUIContentServiceProviderR provider);

    //! Add or remove the image provider
    ECOBJECTS_EXPORT void                           AddProvider (ECPresentationImageProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (ECPresentationImageProviderR provider);

    //! Add or remove the localization provider
    ECOBJECTS_EXPORT void                           AddProvider (ECPresentationLocalizationProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (ECPresentationLocalizationProviderR provider);

    //!Get the view definition associated with a particular data context.
    //!@param[in] itemInfo      A hint to provide the context in which the view definition will be used. eg. MenuItem
    //!@param[in] dataContext   The data context for which the view definition is requested.
    ECOBJECTS_EXPORT IECPresentationViewDefinitionPtr   GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR instanceData) const;

    //!Get the view definition associated with a particular data context.
    //!@param[in] itemInfo      A hint to provide the context in which the view definition will be used. eg. MenuItem
    //!@param[in] dataContext   The data context for which the view definition is requested.
    ECOBJECTS_EXPORT IECContentDefinitionPtr            GetContentDefinition (IECPresentationViewDefinitionCR viewDef) const;

    //! Get the list of commands that is associated with this data context for a given purpose. Its a union of all 
    //! commands provided by different command providers.
    ECOBJECTS_EXPORT void                               GetCommands (bvector<IUICommandPtr>& commands, IAUIDataContextCR instance, int purpose);

    //! Fetch an image of the specified size and name from the image providers.
    ECOBJECTS_EXPORT IECNativeImagePtr                  GetImage (ECImageKeyCR imageKey, DPoint2dCR size);

    //! Fetch an overlay image of the specified size and name from the image providers.
    ECOBJECTS_EXPORT IECNativeImagePtr                  GetOverlayImage (IAUIDataContextCR imageKey, DPoint2dCR size);

    //! Get string from localization providers.
    //!@param[in] rscFileName   Dll name which contains the resource.
    //!@param[in] tableId       Table ID form which resource shoud be taken.
    //!@param[in] rscId         Resource ID.
    ECOBJECTS_EXPORT WCharCP                            GetString (WCharCP rscFileName, UInt tableId, UInt rscId);

    //! Get current selection or sub-selection.
    //!@param[in] eventHub      Event hub.
    //!@param[in] subSelection  If true then returns sub-selection.
    ECOBJECTS_EXPORT IAUIDataContextCP                  GetSelection (void const* eventHub, bool subSelection);

    //!This returns the ECQueryProcessFlags that is to be used when creating a datacontext that represents a selection set.
    ECOBJECTS_EXPORT int                                GetSelectionQueryScope ();
    //! Execute command automatically calls these. So explicit call is usually un necessary.
    void                                                JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData);


    //!Hooks for individual EC events
    ECOBJECTS_EXPORT    void RegisterSelectionHook (ECSelectionListener& listener);
    ECOBJECTS_EXPORT    void UnRegisterSelectionHook (ECSelectionListener& listener);

    ECOBJECTS_EXPORT    void TriggerSelectionEvent (ECSelectionEventCR selectionEvent);
    ECOBJECTS_EXPORT    void TriggerSubSelectionEvent (ECSelectionEventCR selectionEvent);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
