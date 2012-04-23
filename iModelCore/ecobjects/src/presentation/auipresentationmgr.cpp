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
    m_viewProviders.insert(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            UIPresentationManager::RemoveProvider (IECViewDefinitionProviderR provider)
    {
    m_viewProviders.erase(&provider);
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
bvector<UICommandPtr>    UIPresentationManager::GetCommands (IAUIDataContextCR instance) const
    {
    bvector<UICommandPtr> commands;
    for (T_CmdProviderSet::const_iterator iter = m_cmdProviders.begin(); iter != m_cmdProviders.end(); ++iter)
        {
        bvector<UICommandPtr> commandList = (*iter)->GetCommand(instance);
        std::copy (commandList.begin(), commandList.end(), std::back_inserter(commands));
        }

    return commands;
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
struct LoggingJournalProvider : public IJournalProvider, public ProviderSingletonPattern<LoggingJournalProvider>
    {
    virtual void    _JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData) override
        {
        
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UIPresentationManager::UIPresentationManager ()
    {
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECViewDefinitionPtr    UIPresentationManager::GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR instanceData) const
    {
    for (T_ViewProviderSet::const_iterator iter = m_viewProviders.begin(); iter != m_viewProviders.end(); ++iter)
        {
        IECViewDefinitionPtr viewDef = (*iter)->GetViewDefinition(itemInfo, instanceData);
        if (viewDef.IsValid())
            return viewDef;
        }

    return NULL;
    }
