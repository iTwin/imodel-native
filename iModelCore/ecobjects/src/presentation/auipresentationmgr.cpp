/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auipresentationmgr.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ProviderType, typename ContainerType>
bool            ECPresentationManager::CheckAndAddProviderFromList (ProviderType & provider, ContainerType& providerList)
    {
    if (providerList.empty())
        {
        providerList.push_back(&provider);
        return false;
        }
    
    typename ContainerType::iterator iter = std::lower_bound(providerList.begin(), providerList.end(), &provider);
    if (iter != providerList.end() && *iter == &provider)
        return false;
    providerList.insert (iter, &provider);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ProviderType, typename ContainerType>
void            ECPresentationManager::RemoveProviderFromList (ProviderType & provider, ContainerType& providerList)
    {
    if (providerList.empty())
        return;

    typename ContainerType::iterator iter = std::lower_bound(providerList.begin(), providerList.end(), &provider);
    if (iter == providerList.end() || *iter != &provider)
        return;

    providerList.erase(iter);
    }

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
    CheckAndAddProviderFromList (provider, m_journalProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (IJournalProviderR provider)
    {
    RemoveProviderFromList (provider, m_journalProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (IECPresentationViewProviderR provider)
    {
    CheckAndAddProviderFromList (provider, m_viewProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (IECPresentationViewProviderR provider)
    {
    RemoveProviderFromList (provider, m_viewProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (IAUIContentServiceProviderR provider)
    {
    CheckAndAddProviderFromList (provider, m_contentProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (IAUIContentServiceProviderR provider)
    {
    RemoveProviderFromList (provider, m_contentProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (ECPresentationCommandProviderR provider)
    {
    CheckAndAddProviderFromList (provider, m_cmdProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (ECPresentationCommandProviderR provider)
    {
    RemoveProviderFromList (provider, m_cmdProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas              11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationCommandProviderCP ECPresentationManager::GetCommandProviderById (size_t providerId)
    {
    for (T_CmdProviderSet::iterator iter = m_cmdProviders.begin(); iter != m_cmdProviders.end(); ++iter)
        {
        if ((*iter)->GetProviderId() == providerId)
            return *iter;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (ECPresentationImageProviderR provider)
    {
    CheckAndAddProviderFromList (provider, m_imageProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (ECPresentationImageProviderR provider)
    {
    RemoveProviderFromList (provider, m_imageProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::AddProvider (ECPresentationLocalizationProviderR provider)
    {
    CheckAndAddProviderFromList (provider, m_localizationProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RemoveProvider (ECPresentationLocalizationProviderR provider)
    {
    RemoveProviderFromList (provider, m_localizationProviders);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECPresentationManager::GetCommands (bvector<IUICommandPtr>& commands, IAUIDataContextCR instance, int purpose)
    {
    for (T_CmdProviderSet::const_iterator iter = m_cmdProviders.begin(); iter != m_cmdProviders.end(); ++iter)
        (*iter)->GetCommand(commands, instance, purpose);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LoggingJournalProvider : public IJournalProvider
    {
    virtual WCharCP _GetProviderName(void) const override {return L"LoggingJournalProvider";}
    virtual ProviderType _GetProviderType(void) const override {return JournalService;}

    virtual void    _JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData) override
        {
        
        }
    static LoggingJournalProvider& GetProvider ()
        {
        LoggingJournalProvider* provider = new LoggingJournalProvider();
        return *provider;
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
    for (T_ImageProviderSet::const_iterator iter = m_imageProviders.begin(); iter != m_imageProviders.end(); ++iter)
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
WString         ECPresentationManager::GetString (WCharCP rscFileName, unsigned int tableId, unsigned int rscId)
    {
    for (T_LocalizationProviderSet::const_iterator iter = m_localizationProviders.begin(); iter != m_localizationProviders.end(); ++iter)
        {
        WString localizedString = (*iter)->GetString (rscFileName, tableId, rscId);
        if (!WString::IsNullOrEmpty (localizedString.c_str()))
            return localizedString;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIDataContextCP ECPresentationManager::GetSelection (void const* eventHub, bool subSelection)
    {
    for (T_SelectionListeners::const_iterator iter = m_selectionListeners.begin(); iter != m_selectionListeners.end(); ++iter)
        {
        if (NULL == eventHub || (*iter)->GetEventHub() == eventHub)
            {
            IAUIDataContextCP dataContext = (*iter)->_GetSelection (subSelection);
            if (NULL == dataContext)
                continue;

#if defined (REMOVED_FOR_BOOST)
            ECInstanceIterableCP iterable = dataContext->GetInstanceIterable();
            if (NULL == iterable || iterable->begin() == iterable->end())
                {
                delete dataContext;
                continue;
                }
#endif

            return dataContext;
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool sortECSelectionListenersByPriority (ECSelectionListenerP x, ECSelectionListenerP y)
    {
    return (x->GetPriority () > y->GetPriority ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::RegisterSelectionHook (ECSelectionListener& listener)
    {
    if (m_selectionListeners.end() != std::find(m_selectionListeners.begin(), m_selectionListeners.end(), &listener))
        return; // do not allow duplicates

    m_selectionListeners.push_back(&listener);
    std::sort (m_selectionListeners.begin(), m_selectionListeners.end(), sortECSelectionListenersByPriority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::UnRegisterSelectionHook (ECSelectionListener& listener)
    {
    T_SelectionListeners::iterator iter = std::find(m_selectionListeners.begin(), m_selectionListeners.end(), &listener);
    if (m_selectionListeners.end() == iter)
        return;

    m_selectionListeners.erase(iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::TriggerSelectionEvent (ECSelectionEventCR selectionEvent)
    {
    void const* eventHub = selectionEvent.GetEventHub();
    for (T_SelectionListeners::const_iterator iter = m_selectionListeners.begin(); iter != m_selectionListeners.end(); ++iter)
        {
        if (NULL == eventHub || (*iter)->GetEventHub() == eventHub)
            (*iter)->_OnSelection(selectionEvent);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECPresentationManager::TriggerSubSelectionEvent (ECSelectionEventCR selectionEvent)
    {
    void const* eventHub = selectionEvent.GetEventHub();
    for (T_SelectionListeners::const_iterator iter = m_selectionListeners.begin(); iter != m_selectionListeners.end(); ++iter)
        {
        if (NULL == eventHub || (*iter)->GetEventHub() == eventHub)
            (*iter)->_OnSubSelection(selectionEvent);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECNativeImagePtr ECPresentationManager::GetOverlayImage (IAUIDataContextCR context, DPoint2dCR size)
    {
    for (T_ImageProviderSet::const_iterator iter = m_imageProviders.begin(); iter != m_imageProviders.end(); ++iter)
        {
        IECNativeImagePtr nativeImage = (*iter)->GetOverlayImage(context, size);
        if (nativeImage.IsNull())
            continue;

        return nativeImage;
        }
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int             ECPresentationManager::GetSelectionQueryScope()
    {
    int mask = 0;
    for (T_SelectionListeners::const_iterator iter = m_selectionListeners.begin(); iter != m_selectionListeners.end(); ++iter)
        mask |= (*iter)->_GetSelectionQueryScope();
    
    return mask;
    }
