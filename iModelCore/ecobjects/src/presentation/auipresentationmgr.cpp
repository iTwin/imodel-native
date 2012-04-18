/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auipresentationmgr.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UIPresentationManagerR   UIPresentationManager::GetManager()
    {
    static UIPresentationManagerP mgr = new UIPresentationManager();
    return *mgr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            UIPresentationManager::AddProvider (IJournalProviderR provider)
    {
    m_journalProviders.insert(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            UIPresentationManager::RemoveProvider (IJournalProviderR provider)
    {
    m_journalProviders.erase(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            UIPresentationManager::AddProvider (IECViewDefinitionProviderR provider)
    {
    m_displayProviders.insert(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            UIPresentationManager::RemoveProvider (IECViewDefinitionProviderR provider)
    {
    m_displayProviders.erase(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            UIPresentationManager::AddProvider (IUICommandProviderCR provider)
    {
    m_cmdProviders.insert(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            UIPresentationManager::RemoveProvider (IUICommandProviderCR provider)
    {
    m_cmdProviders.erase(&provider);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIItemPtr      UIPresentationManager::GetUIItem (IAUIItemInfoCR itemType, IAUIDataContextCP instanceData) const
    {
    for (T_DisplayProviderSet::const_iterator iter = m_displayProviders.begin(); iter != m_displayProviders.end(); ++iter)
        {
        IAUIItemPtr item  = (*iter)->GetUIItem (itemType, instanceData);
        if (item.IsValid())
            return item;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UICommandPtr    UIPresentationManager::GetCommand (IAUIDataContextCR instance) const
    {
    for (T_CmdProviderSet::const_iterator iter = m_cmdProviders.begin(); iter != m_cmdProviders.end(); ++iter)
        {
        UICommandPtr command = (*iter)->GetCommand(instance);
        if (command.IsValid())
            return command;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ProviderType>
class ProviderSingletonPattern : public NonCopyableClass
    {
    public:
    static ProviderType& GetProvider ()
        {
        ProviderType* provider = new ProviderType();
        return *provider;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct BaseCommandProvider : public IUICommandProvider, public ProviderSingletonPattern<BaseCommandProvider>
    {
    virtual UICommandPtr _GetCommand (IAUIDataContextCR instance) const override
        {
        return NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LoggingJournalProvider : public IJournalProvider, public ProviderSingletonPattern<LoggingJournalProvider>
    {
    virtual void    JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData) override
        {
        
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UIPresentationManager::UIPresentationManager ()
    {
    //AddProvider (BaseDisplayProvider::GetProvider());
    AddProvider (BaseCommandProvider::GetProvider());
    AddProvider (LoggingJournalProvider::GetProvider());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            UIPresentationManager::JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData)
    {
    for (T_JournalProviderSet::iterator iter = m_journalProviders.begin(); iter != m_journalProviders.end(); ++iter)
        (*iter)->JournalCmd(cmd, instanceData);
    }
