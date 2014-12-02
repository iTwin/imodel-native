/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDomain.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnHandlers/DgnECPersistence.h>

static DgnDomains           s_loadedDomains;
static DgnSystemDomain*     s_systemDomain;
static DgnDraftingDomain*   s_draftingDomain;

DgnSystemDomain&        DgnSystemDomain::GetInstance() {return *s_systemDomain;}
DgnDraftingDomain&      DgnDraftingDomain::GetInstance() {return *s_draftingDomain;}
Utf8CP                  DgnDomain::GetName() const {return m_name.c_str();}
Utf8CP                  DgnDomain::GetDescription() const {return m_description.c_str();}
UInt32                  DgnDomain::GetVersion() const {return m_version;}
XAttributeHandlerId     XAttributeHandler::GetHandlerId() const {return m_handlerId;}
DgnDomainP              XAttributeHandler::GetDgnDomain() const {return m_domain;}
DomainList const& DgnDomains::GetDomainList() const {return m_domains;}
DgnDomain::Handlers const& DgnDomain::GetHandlers() const  {return m_handlers;}
DgnDomains const& DgnDomainLoader::GetLoadedDomains() {return s_loadedDomains;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void Handler::SetDgnDomain(DgnDomainR domain, ElementHandlerId id) 
    {
    BeAssert (0==m_domain);    // make sure it's not already registered
    m_handlerId = id;
    m_domain = &domain;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeHandler::SetDgnDomain(DgnDomainR domain, XAttributeHandlerId id) 
    {
    BeAssert (0==m_domain);    // make sure it's not already registered
    m_handlerId = id;
    m_domain = &domain;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomainCP DgnDomains::FindDomain (Utf8CP name) const
    {
    for (DgnDomainCP domain : m_domains)
        {
        if (0 == BeStringUtilities::Stricmp(domain->GetName(), name))
            return  domain;
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomainCP DgnDomainLoader::LoadDomain (Utf8CP name, UInt32 version)
    {
    return s_loadedDomains.FindDomain(name);

    // NEEDS_WORK- load domains from dlls
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomains::AddLoadedDomain (DgnDomainCR domain)
    {
    if (m_domains.end() != std::find (m_domains.begin(), m_domains.end(), &domain))
        return;

    for (auto it = m_domains.begin(); it != m_domains.end(); ++it)
        {
        DgnDomainCP d = *it;
        if (domain._GetPriority() > d->_GetPriority())
            {
            m_domains.insert (it, &domain);
            return;
            }
        }

    // lower priority than all existing domains
    m_domains.push_back(&domain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomains::DropDomain (DgnDomainCR domain)
    {
    auto it = std::find (m_domains.begin(), m_domains.end(), &domain);
    if (m_domains.end() != it)
        m_domains.erase(it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomainLoader::RegisterLoadedDomain (DgnDomainR domain)
    {
    s_loadedDomains.AddLoadedDomain(domain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomainLoader::UnregisterLoadedDomain (DgnDomainR domain)
    {
    s_loadedDomains.DropDomain(domain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
HandlerR DgnDomainLoader::GetIllegalHandler()
    {
    static HandlerP s_illegal = 0;

    if (NULL == s_illegal)
        s_illegal = s_systemDomain->_GenerateMissingHandler(ElementHandlerId (LEGACY_ELEMENT_TypeHandlerMajorId, 0));

    return  *s_illegal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
HandlerP DgnDomains::FindElementHandler (ElementHandlerId id) const
    {
    for (DgnDomainCP domain : m_domains)
        {
        HandlerP handler = domain->FindElementHandler(id);
        if (handler)
            return  handler;
        }
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerP DgnDomains::FindXAttributeHandler (XAttributeHandlerId id) const
    {
    for (DgnDomainCP domain : m_domains)
        {
        XAttributeHandlerP handler = domain->FindXAttributeHandler(id);
        if (handler)
            return  handler;
        }
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDomain::RegisterHandler (ElementHandlerId id, Handler& handler)
    {
    if (NULL != FindElementHandler (id))
        {
        BeAssert (false);
        return  ERROR;
        }

    handler.SetDgnDomain (*this, id);

    m_handlers.Insert (id, &handler);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
HandlerP DgnDomain::FindElementHandler (ElementHandlerId id) const
    {
    auto it = m_handlers.find (id);
    return  it==m_handlers.end() ? NULL : it->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDomain::RegisterXAttributeHandler (XAttributeHandlerId id, XAttributeHandlerR handler)
    {
    if (NULL != FindXAttributeHandler (id))
        return  ERROR;
    handler.SetDgnDomain (*this, id);

    m_xAttHandlers.Insert (id, &handler);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerP DgnDomain::FindXAttributeHandler (XAttributeHandlerId id) const
    {
    auto it = m_xAttHandlers.find (id);
    return  it==m_xAttHandlers.end() ? NULL : it->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
HandlerR DgnDomains::ResolveHandler (ElementHandlerId handlerId) const
    {
    HandlerP handler = FindElementHandler (handlerId);

    if (NULL == handler)
        handler = s_systemDomain->_GenerateMissingHandler(handlerId);

    return *handler;
    }

void DgnDomain::_OnProjectCreated (DgnProjectR project) const {project.Domains().AddDomainToProject(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSystemDomain::InitDgnCore(DgnDraftingDomain& draftingDomain)
    {
    BeAssert (DgnPlatformLib::InStaticInitialization());

    s_systemDomain = this;
    s_draftingDomain = &draftingDomain;

    DgnDomainLoader::RegisterLoadedDomain (*this);
    DgnDomainLoader::RegisterLoadedDomain (draftingDomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
HandlerR ElementHandle::GetHandler() const
    {
    if (IsValid())  
        {
        HandlerP handler = m_dscr.IsValid() ? m_dscr->GetElementHandler() : (m_elmRef.IsValid() ? m_elmRef->GetHandler() : NULL);
        BeAssert (NULL != handler);
        if (NULL != handler)
            return *handler;
        }

    return DgnDomainLoader::GetIllegalHandler();
    }
