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
struct  UIPresentationManager: public NonCopyableClass
    {
    private:
        typedef bset<IUICommandProviderCP>          T_CmdProviderSet;
        typedef bset<IECViewDefinitionProviderP>    T_ViewProviderSet;
        typedef bset<IJournalProviderP>             T_JournalProviderSet;
        typedef bset<IAUIContentServiceProviderP>   T_ContentProviderSet;

        T_CmdProviderSet        m_cmdProviders;
        T_ViewProviderSet       m_viewProviders;
        T_JournalProviderSet    m_journalProviders;
        T_ContentProviderSet    m_contentProviders;
        UIPresentationManager ();
        
        IECViewDefinitionPtr AggregateViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR instanceData) const;

    public:
    //! Get the presentation manager.
    ECOBJECTS_EXPORT static UIPresentationManagerR  GetManager();

    //! Add or remove the journal provider
    ECOBJECTS_EXPORT void                           AddProvider (IJournalProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IJournalProviderR provider);

    //! Add or remove the command provider
    ECOBJECTS_EXPORT void                           AddProvider (IUICommandProviderCR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IUICommandProviderCR provider);
    
    //! Add or remove the view definition provider
    ECOBJECTS_EXPORT void                           AddProvider (IECViewDefinitionProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IECViewDefinitionProviderR provider);

    //! Add or remove the view definition provider
    ECOBJECTS_EXPORT void                           AddProvider (IAUIContentServiceProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IAUIContentServiceProviderR provider);

    //!Get the view definition associated with a particular data context.
    //!@param[in] itemInfo      A hint to provide the context in which the view definition will be used. eg. MenuItem
    //!@param[in] dataContext   The data context for which the view definition is requested.
    ECOBJECTS_EXPORT IECViewDefinitionPtr           GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR instanceData) const;

    //!Get the view definition associated with a particular data context.
    //!@param[in] itemInfo      A hint to provide the context in which the view definition will be used. eg. MenuItem
    //!@param[in] dataContext   The data context for which the view definition is requested.
    ECOBJECTS_EXPORT IECContentDefinitionPtr    GetContentDefinition (IECViewDefinitionCR viewDef) const;

    //! Get the list of commands that is associated with this data context. Its a union of all commands provided by 
    //! different command providers.
    ECOBJECTS_EXPORT bvector<UICommandPtr>          GetCommands (IAUIDataContextCR instance) const;

    //! Execute command automatically calls these. So explicit call is usually un necessary.
    void                           JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData);
    };

END_BENTLEY_EC_NAMESPACE
