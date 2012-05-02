/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auipresentationmgr.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  ECPresentationManager: public NonCopyableClass
    {
    private:
        typedef bset<ECPresentationCommandProviderCP>   T_CmdProviderSet;
        typedef bset<IECPresentationViewProviderP>        T_ViewProviderSet;
        typedef bset<IJournalProviderP>                 T_JournalProviderSet;
        typedef bset<IAUIContentServiceProviderP>       T_ContentProviderSet;
        typedef bset<ECPresentationImageProviderP>      T_ImageProviderSet;

        T_CmdProviderSet        m_cmdProviders;
        T_ViewProviderSet       m_viewProviders;
        T_JournalProviderSet    m_journalProviders;
        T_ContentProviderSet    m_contentProviders;
        T_ImageProviderSet      m_imageProviders;

        ECPresentationManager ();
        
        IECPresentationViewDefinitionPtr AggregateViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR instanceData) const;

    public:
    //! Get the presentation manager.
    ECOBJECTS_EXPORT static ECPresentationManagerR  GetManager();

    //! Add or remove the journal provider
    ECOBJECTS_EXPORT void                           AddProvider (IJournalProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IJournalProviderR provider);

    //! Add or remove the command provider
    ECOBJECTS_EXPORT void                           AddProvider (ECPresentationCommandProviderCR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (ECPresentationCommandProviderCR provider);
    
    //! Add or remove the view definition provider
    ECOBJECTS_EXPORT void                           AddProvider (IECPresentationViewProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IECPresentationViewProviderR provider);

    //! Add or remove the view definition provider
    ECOBJECTS_EXPORT void                           AddProvider (IAUIContentServiceProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IAUIContentServiceProviderR provider);

    //! Add or remove the image provider
    ECOBJECTS_EXPORT void                           AddProvider (ECPresentationImageProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (ECPresentationImageProviderR provider);

    //!Get the view definition associated with a particular data context.
    //!@param[in] itemInfo      A hint to provide the context in which the view definition will be used. eg. MenuItem
    //!@param[in] dataContext   The data context for which the view definition is requested.
    ECOBJECTS_EXPORT IECPresentationViewDefinitionPtr   GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR instanceData) const;

    //!Get the view definition associated with a particular data context.
    //!@param[in] itemInfo      A hint to provide the context in which the view definition will be used. eg. MenuItem
    //!@param[in] dataContext   The data context for which the view definition is requested.
    ECOBJECTS_EXPORT IECContentDefinitionPtr            GetContentDefinition (IECPresentationViewDefinitionCR viewDef) const;

    //! Get the list of commands that is associated with this data context. Its a union of all commands provided by 
    //! different command providers.
    ECOBJECTS_EXPORT bvector<UICommandPtr>              GetCommands (IAUIDataContextCR instance) const;

    //! Fetch an image of the specified size and name from the image providers.
    ECOBJECTS_EXPORT IECNativeImagePtr GetImage (ECImageKeyCR imageKey, DPoint2dCR size);

    //! Execute command automatically calls these. So explicit call is usually un necessary.
    void                           JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData);
    };

END_BENTLEY_EC_NAMESPACE
