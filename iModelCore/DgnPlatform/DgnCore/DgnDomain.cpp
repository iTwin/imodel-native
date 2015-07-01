/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomains::RegisterDomain(DgnDomain& domain)
    {
    T_HOST.RegisteredDomains().push_back(&domain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomainCP DgnDomains::FindDomain(Utf8CP name) const
    {
    for (DgnDomainCP domain : m_domains)
        {
        if (0 == BeStringUtilities::Stricmp(domain->GetDomainName(), name))
            return  domain;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler* DgnDomain::FindHandler(Utf8CP className) const
    {
    for (Handler* iter : m_handlers)
        {
        if (iter->GetClassName() == className)
            return iter;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* "load" all of the handlers for this DgnDomain.
* Loading involves ensuring that the handlers in the DGN_TABLE_Handler table (i.e. the "required handlers") are all
* registered. For any that are registered but not in the table, add them.
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomain::LoadHandlers(DgnDbR dgndb) const
    {
    bmap<Utf8String,Handler*> myHandlers;
    for (Handler* iter : m_handlers)
        myHandlers.Insert(iter->GetClassName(), iter);

    Statement stmt(dgndb, "SELECT Name,ClassId FROM " DGN_TABLE_Handler " WHERE Domain=?");
    stmt.BindText(1, GetDomainName(), Statement::MakeCopy::No);

    // make sure we have all of the handlers for this domain registered.
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8String handlerName(stmt.GetValueText(0));
        auto thisHandler = myHandlers.find(handlerName);
        if (thisHandler == myHandlers.end())
            {
            LOG.errorv("Missing Handler [%s]", handlerName.c_str());
            continue;
            }

        dgndb.Domains().AddHandler(stmt.GetValueId<DgnClassId>(1), thisHandler->second);
        myHandlers.erase(thisHandler); // so we know we've already found it.
        }

    // any that are left are new and need to be added to the database
    for (auto iter : myHandlers)
        dgndb.Domains().InsertHandler(*iter.second);

    for (auto* tblHandler : m_tableHandlers)
        dgndb.Txns().AddTxnTable(tblHandler);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* load a Domain found in the Domain table of a DgnDb into that dgndb's DgnDomains
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomains::LoadDomain(DgnDomainR domain)
    {
    domain.LoadHandlers(m_dgndb);   // load all the handlers from this domain
    m_domains.push_back(&domain);   // save the fact that we are using this domain
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomains::SyncWithSchemas()
    {
    m_handlers.clear();
    m_domains.clear();

    auto& hostDomains = T_HOST.RegisteredDomains();
    bmap<Utf8String,DgnDomain*> registeredDomains;

    for (DgnDomain* iter : hostDomains)
        registeredDomains.Insert(iter->GetDomainName(), iter);

    Statement stmt(m_dgndb, "SELECT Name,Version FROM " DGN_TABLE_Domain);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8String domainName(stmt.GetValueText(0));
        auto thisDomain = registeredDomains.find(domainName);
        if (thisDomain == registeredDomains.end())
            {
            LOG.errorv("Error Missing Domain [%s]", stmt.GetValueText(0));
            continue;
            }

        if (thisDomain->second->GetVersion() < stmt.GetValueInt(1))
            {
            BeAssert(false);
            LOG.errorv("Wrong Domain version [%s]", stmt.GetValueText(0));
            continue;
            }

        LoadDomain(*thisDomain->second);
        registeredDomains.erase(thisDomain); // so we know we've already found it.
        }

    // if we're opening a readonly database, we know we can't add any new dependencies on a registered but unknown domain. Stop now.
    if (m_dgndb.IsReadonly())
        return BE_SQLITE_OK;

    // any that are left are new and need to be added to the database
    for (auto iter : registeredDomains)
        {
        if (nullptr == m_dgndb.Schemas().GetECSchema(iter.second->GetDomainName(), false))
            continue; // this domain's schema doesn't exist (yet?) in this db.

        auto rc = InsertDomain(*iter.second); // add to database so it will be required from here on
        BeAssert(rc==BE_SQLITE_DONE);
        UNUSED_VARIABLE(rc);
        LoadDomain(*iter.second);
        }

    return BE_SQLITE_OK;
    }


/*---------------------------------------------------------------------------------**//**
* Before you can register a Handler, all of its superclass Handlers must be registered.
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDomain::VerifySuperclass(Handler& handler) 
    {
    Handler* superclass = handler.GetSuperClass();
    if (&DgnDomain::Handler::GetHandler() == superclass) // Handler is always "registered"
        return SUCCESS;

    if (nullptr == superclass || (nullptr == superclass->GetDomain().FindHandler(superclass->m_ecClassName.c_str())))
        {
        BeAssert(false);
        return ERROR;
        }

    return VerifySuperclass(*superclass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDomain::RegisterHandler(Handler& handler) 
    {
    if (SUCCESS != VerifySuperclass(handler))
        return ERROR;

    handler.SetDomain(*this); 

    // make sure we don't already have a handler for this classname
    for (auto thisHandler=m_handlers.begin(); thisHandler!=m_handlers.end(); )
        {
        if ((*thisHandler)->GetClassName() == handler.GetClassName())
            thisHandler = m_handlers.erase(thisHandler);
        else
            ++thisHandler;
        }

    m_handlers.push_back(&handler);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomains::OnDbOpened()
    {
    auto rc = SyncWithSchemas();
    if (BE_SQLITE_OK != rc)
        return rc;

    for (DgnDomainCP domain : m_domains)
        domain->_OnDgnDbOpened(m_dgndb);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomains::OnDbClose()
    {
    for (DgnDomainCP domain : m_domains)
        domain->_OnDgnDbClose(m_dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDomain::ImportSchema(DgnDbR db, BeFileNameCR schemaFile) const
    {
    if (!schemaFile.DoesPathExist())
        {
        BeAssert(false);
        return BentleyStatus::ERROR;
        }

    WString schemaBaseNameW;
    schemaFile.ParseName(NULL, NULL, &schemaBaseNameW, NULL);
    Utf8String schemaBaseName(schemaBaseNameW);

    if (0 != BeStringUtilities::Strnicmp(schemaBaseName.c_str(), GetDomainName(), strlen(GetDomainName()))) // ECSchema base name and DgnDomain name must match
        {
        BeAssert(false);
        return BentleyStatus::ERROR;
        }

    BeFileName schemaDir = schemaFile.GetDirectoryName();

    ECSchemaReadContextPtr contextPtr = ECSchemaReadContext::CreateContext();
    contextPtr->AddSchemaLocater(db.GetSchemaLocater());
    contextPtr->AddSchemaPath(schemaDir.GetName());

    ECSchemaPtr schemaPtr;
    SchemaReadStatus readSchemaStatus = ECSchema::ReadFromXmlFile(schemaPtr, schemaFile.GetName(), *contextPtr);
    if (SCHEMA_READ_STATUS_Success != readSchemaStatus)
        return BentleyStatus::ERROR;

    if (BentleyStatus::SUCCESS != db.Schemas().ImportECSchemas(contextPtr->GetCache()))
        return BentleyStatus::ERROR;

    if (BE_SQLITE_OK != db.Domains().SyncWithSchemas())
        return BentleyStatus::ERROR;

    _OnSchemaImported(db); // notify subclasses so domain objects (like categories) can be created
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomains::InsertDomain(DgnDomainCR domain)
    {
    Statement stmt(m_dgndb, SqlPrintfString("INSERT INTO " DGN_TABLE_Domain " (Name,Descr,Version) VALUES(?,?,?)"));
    stmt.BindText(1, domain.GetDomainName(), Statement::MakeCopy::No);
    stmt.BindText(2, domain.GetDomainDescription(), Statement::MakeCopy::No);
    stmt.BindInt(3, domain.GetVersion());
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler* DgnDomains::LookupHandler(DgnClassId handlerId)
    {
    auto iter = m_handlers.find(handlerId);
    return iter == m_handlers.end() ? nullptr : iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP DgnDomains::FindBaseOfType(DgnClassId subClassId, DgnClassId baseClassId)
    {
    auto const& schemas = m_dgndb.Schemas();
    ECN::ECClassCP subClass  = schemas.GetECClass(subClassId.GetValue());
    ECN::ECClassCP baseClass = schemas.GetECClass(baseClassId.GetValue());

    if (nullptr==subClass || nullptr==baseClass || subClass == baseClass) // can't be baseclass of yourself
        return nullptr;

    //WIP: This won't work if subClass has two base classes and both of them have different handlers
    // this loop is due to multiple inheritance. Look for the right baseclass
    for (auto* thisClass : subClass->GetBaseClasses())
        {
        if (thisClass->Is(baseClass))
            return thisClass;
        }

    return nullptr;    // sub is not derived from base
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler* DgnDomains::FindHandler(DgnClassId handlerId, DgnClassId baseClassId)
    {
    // do we have a registered handler for this class?
    DgnDomain::Handler* handler = LookupHandler(handlerId);
    if (nullptr != handler)
        return handler;

    // Get superclass of type baseClass
    ECN::ECClassCP superClass = FindBaseOfType(handlerId, baseClassId);
    if (nullptr == superClass)
        {
        //BeAssert(false);
        return nullptr;
        }

    // see if baseclass has a handler, recursively.
    handler = FindHandler(DgnClassId(superClass->GetId()), baseClassId);
    if (nullptr != handler)
        {
        m_handlers.Insert(handlerId, handler); // cache this result for all baseclasses
        return handler;
        }

    // the handlerId supplied must not derive from baseClassId or no registered handlers exist for any baseclasses
    BeAssert(false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId DgnDomains::GetClassId(DgnDomain::Handler& handler)
    {
    return DgnClassId(m_dgndb.Schemas().GetECClassId(handler.GetDomain().GetDomainName(), handler.GetClassName().c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* Insert a handler into the DGN_TABLE_Handler table. This involves finding the local DgnClassId for the handler,
* and then saving the DgnClassId with the domain/name of the class it handles (which is really redundant, but avoids a
* join when trying to find all the classes with handlers for a domain.)
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomains::InsertHandler(DgnDomain::Handler& handler)
    {
    DgnClassId id = GetClassId(handler);
    if (!id.IsValid())
        {
        BeAssert(false);
        // handler is registered against a class that doesn't exist
        return BE_SQLITE_ERROR;
        }

    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE_Handler " (Domain,Name,ClassId) VALUES(?,?,?)");
    stmt.BindText(1, handler.GetDomain().GetDomainName(), Statement::MakeCopy::No);
    stmt.BindText(2, handler.GetClassName(), Statement::MakeCopy::No);
    stmt.BindId(3, id);

    auto status = stmt.Step();
    if (BE_SQLITE_DONE != status)
        return status;

    m_handlers.Insert(id, &handler);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler::ExtensionEntry* DgnDomain::Handler::ExtensionEntry::Find(ExtensionEntry* start, Extension::Token const& id)
    {
    while (start && &start->m_token != &id)
        start = start->m_next;
    return start;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDomain::Handler::AddExtension(dgn_ElementHandler::Element::Extension::Token& id, dgn_ElementHandler::Element::Extension& extension)
    {
    ExtensionEntry* prev = ExtensionEntry::Find(m_extensions, id);
    if (NULL != prev)
        return  ERROR;

    m_extensions = new DgnDomain::Handler::ExtensionEntry(id, extension, m_extensions);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDomain::Handler::DropExtension(dgn_ElementHandler::Element::Extension::Token& id)
    {
    for (ExtensionEntry* prev=0, *entry=m_extensions; NULL != entry; prev=entry, entry=entry->m_next)
        {
        if (&entry->m_token != &id)
            continue;

        if (prev)
            prev->m_next = entry->m_next;
        else
            m_extensions = entry->m_next;

        delete entry;
        return SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_ElementHandler::Element::Extension* DgnDomain::Handler::FindExtension(Extension::Token& id)
    {
    ExtensionEntry* found = ExtensionEntry::Find(m_extensions, id);
    if (NULL != found)
        return  &found->m_extension;

    return  m_superClass ? m_superClass->FindExtension(id) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler* DgnDomain::Handler::z_CreateInstance()
    {
    DgnDomain::Handler* instance= new DgnDomain::Handler();
    instance->SetSuperClass((DgnDomain::Handler*) nullptr);
    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler*& DgnDomain::Handler::z_PeekInstance()
    {
    static DgnDomain::Handler* s_instance = 0;
    return s_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler& DgnDomain::Handler::z_GetHandlerInstance()
    {
    DgnDomain::Handler*& instance = z_PeekInstance();

    if (0 == instance)
        instance = z_CreateInstance();

    return *instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerP dgn_ElementHandler::Element::FindHandler(DgnDb const& db, DgnClassId handlerId)
    {
    // quick check for a handler already known
    DgnDomain::Handler* handler = db.Domains().LookupHandler(handlerId);
    if (nullptr != handler)
        return handler->_ToElementHandler();

    // not there, check via base classes
    handler = db.Domains().FindHandler(handlerId, db.Domains().GetClassId(dgn_ElementHandler::Element::GetHandler()));
    return handler ? handler->_ToElementHandler() : (BeAssert(false), nullptr);
    }
