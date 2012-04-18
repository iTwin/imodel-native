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
struct  IJournalProvider
    {
    virtual void    JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData) = 0;
    virtual ~IJournalProvider() {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  UIPresentationManager: public NonCopyableClass
    {
    private:
        typedef bset<IUICommandProviderCP>          T_CmdProviderSet;
        typedef bset<IECViewDefinitionProviderP>    T_DisplayProviderSet;
        typedef bset<IJournalProviderP>             T_JournalProviderSet;

        T_CmdProviderSet        m_cmdProviders;
        T_DisplayProviderSet    m_displayProviders;
        T_JournalProviderSet    m_journalProviders;
        
        UIPresentationManager ();
        
    public:
    
                     void                           JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData);

    ECOBJECTS_EXPORT void                           AddProvider (IJournalProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IJournalProviderR provider);

    ECOBJECTS_EXPORT void                           AddProvider (IUICommandProviderCR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IUICommandProviderCR provider);
    
    ECOBJECTS_EXPORT void                           AddProvider (IECViewDefinitionProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IECViewDefinitionProviderR provider);

    ECOBJECTS_EXPORT IECViewDefinitionPtr           GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCP instanceData) const;

    ECOBJECTS_EXPORT UICommandPtr                   GetCommand (IAUIDataContextCR instance) const;

    ECOBJECTS_EXPORT static UIPresentationManagerR  GetManager();
    };

END_BENTLEY_EC_NAMESPACE
