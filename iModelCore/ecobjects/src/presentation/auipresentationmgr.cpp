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
ECPresentationManagerR   ECPresentationManager::GetManager()
    {
    static ECPresentationManagerP mgr = new ECPresentationManager();
    return *mgr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (IJournalProviderR provider)
    {
    m_journalProviders.insert(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (IJournalProviderR provider)
    {
    m_journalProviders.erase(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (IECPresentationViewProviderR provider)
    {
    m_viewProviders.insert(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (IECPresentationViewProviderR provider)
    {
    m_viewProviders.erase(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (IAUIContentServiceProviderR provider)
    {
    m_contentProviders.insert(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (IAUIContentServiceProviderR provider)
    {
    m_contentProviders.erase(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (ECPresentationCommandProviderCR provider)
    {
    m_cmdProviders.insert(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (ECPresentationCommandProviderCR provider)
    {
    m_cmdProviders.erase(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (ECPresentationResourceProviderR provider)
    {
    m_resourceProviders.insert(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (ECPresentationResourceProviderR provider)
    {
    m_resourceProviders.erase(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<UICommandPtr>    ECPresentationManager::GetCommands (IAUIDataContextCR instance) const
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
ECPresentationManager::ECPresentationManager ()
    {
    AddProvider (LoggingJournalProvider::GetProvider());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData)
    {
    for (T_JournalProviderSet::iterator iter = m_journalProviders.begin(); iter != m_journalProviders.end(); ++iter)
        (*iter)->JournalCmd(cmd, instanceData);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationViewDefinitionPtr    ECPresentationManager::GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR instanceData) const
    {
    if (itemInfo.IsAggregatable())
        return AggregateViewDefinition (itemInfo, instanceData);

    for (T_ViewProviderSet::const_iterator iter = m_viewProviders.begin(); iter != m_viewProviders.end(); ++iter)
        {
        IECPresentationViewDefinitionPtr viewDef = (*iter)->GetViewDefinition(itemInfo, instanceData);
        if (viewDef.IsValid())
            return viewDef;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECContentDefinitionPtr ECPresentationManager::GetContentDefinition (IECPresentationViewDefinitionCR viewDef) const
    {
    for (T_ContentProviderSet::const_iterator iter = m_contentProviders.begin(); iter != m_contentProviders.end(); ++iter)
        {
        IECContentDefinitionPtr contentDef = (*iter)->GetContent(viewDef);
        if (contentDef.IsValid())
            return contentDef;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationViewDefinitionPtr ECPresentationManager::AggregateViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR instanceData) const
    {
    if (!itemInfo.IsAggregatable())
        {
        BeAssert(false);
        return NULL;
        }

    bvector<IECPresentationViewDefinitionPtr> viewDefs;
    for (T_ViewProviderSet::const_iterator iter = m_viewProviders.begin(); iter != m_viewProviders.end(); ++iter)
        {
        IECPresentationViewDefinitionPtr viewDef = (*iter)->GetViewDefinition(itemInfo, instanceData);
        if (viewDef.IsNull())
            continue;

        viewDefs.push_back(viewDef);
        }

    return IECPresentationViewDefinition::CreateCompositeViewDef(viewDefs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECNativeImagePtr ECPresentationManager::GetImage (ECImageKeyCR imageKey, DPoint2dCR size)
    {
    for (T_ResourceProviderSet::const_iterator iter = m_resourceProviders.begin(); iter != m_resourceProviders.end(); ++iter)
        {
        IECNativeImagePtr nativeImage = (*iter)->GetImage(imageKey, size);
        if (nativeImage.IsNull())
            continue;

        return nativeImage;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         ECPresentationManager::GetString (WCharCP rscFileName, UInt tableId, UInt rscId)
    {
    for (T_ResourceProviderSet::const_iterator iter = m_resourceProviders.begin(); iter != m_resourceProviders.end(); ++iter)
        {
        WCharCP localizedString = (*iter)->GetString (rscFileName, tableId, rscId);
        if (!WString::IsNullOrEmpty (localizedString))
            return localizedString;
        }
    return NULL;
    }