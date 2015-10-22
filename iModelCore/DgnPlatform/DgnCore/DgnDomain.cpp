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
    auto& domains = T_HOST.RegisteredDomains();
    for (DgnDomainCP it : domains)
        {
        if (it->m_domainName.EqualsI(domain.m_domainName))
            return;
        }

    domains.push_back(&domain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomainCP DgnDomains::FindDomain(Utf8CP name) const
    {
    for (DgnDomainCP domain : m_domains)
        {
        if (domain->m_domainName.EqualsI(name))
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
        if (iter->GetClassName().Equals(className))
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
void DgnDomains::SyncWithSchemas()
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

    // any that are left are new and need to be added to the database
    for (auto iter : registeredDomains)
        {
        if (nullptr == m_dgndb.Schemas().GetECSchema(iter.second->GetDomainName(), false))
            continue; // this domain's schema doesn't exist (yet?) in this db.

        if (!m_dgndb.IsReadonly())
            {
            auto rc = InsertDomain(*iter.second); // add to database so it will be required from here on
            BeAssert(rc==BE_SQLITE_DONE);
            UNUSED_VARIABLE(rc);
            }
        LoadDomain(*iter.second);
        }

#if !defined (NDEBUG)
    for (auto it : m_handlers)
        it.second->_VerifySchema(*this);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* Before you can register a Handler, of its superclass Handler must be registered.
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnDomain::VerifySuperclass(Handler& handler) 
    {
    Handler* superclass = handler.GetSuperClass();
    if (&DgnDomain::Handler::GetHandler() == superclass) // Handler is always "registered"
        return DgnDbStatus::Success;

    if (nullptr == superclass || (nullptr == superclass->GetDomain().FindHandler(superclass->m_ecClassName.c_str())))
        {
        BeAssert(false);
        return DgnDbStatus::MissingHandler;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnDomain::RegisterHandler(Handler& handler, bool reregister) 
    {
    auto stat = VerifySuperclass(handler);
    if (DgnDbStatus::Success != stat)
        return stat;

    auto thisHandler=m_handlers.begin();
    for (; thisHandler!=m_handlers.end(); ++thisHandler)
        {
        if ((*thisHandler)->GetClassName() == handler.GetClassName())
            break;
        }

    if (reregister)
        {
        if (thisHandler==m_handlers.end()) // reregister only works if we DO already have this handler
            {
            BeAssert(false);
            return DgnDbStatus::NotFound;
            }

        m_handlers.erase(thisHandler);
        }
    else if (thisHandler!=m_handlers.end()) // register only works if we DON'T already have this handler.
        {
        BeAssert(false);
        return DgnDbStatus::AlreadyLoaded;
        }

    handler.SetDomain(*this); 
    m_handlers.push_back(&handler);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomains::OnDbOpened()
    {
    SyncWithSchemas();

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
DgnDbStatus DgnDomain::ImportSchema(DgnDbR db, BeFileNameCR schemaFile) const
    {
    if (!schemaFile.DoesPathExist())
        {
        BeAssert(false);
        return DgnDbStatus::FileNotFound;
        }

    BeFileName schemaBaseNameW;
    schemaFile.ParseName(NULL, NULL, &schemaBaseNameW, NULL);
    Utf8String schemaBaseName(schemaBaseNameW);

    if (0 != BeStringUtilities::Strnicmp(schemaBaseName.c_str(), GetDomainName(), strlen(GetDomainName()))) // ECSchema base name and DgnDomain name must match
        {
        BeAssert(false);
        return DgnDbStatus::WrongDomain;
        }

    BeFileName schemaDir = schemaFile.GetDirectoryName();

    ECSchemaReadContextPtr contextPtr = ECSchemaReadContext::CreateContext();
    contextPtr->AddSchemaLocater(db.GetSchemaLocater());
    contextPtr->AddSchemaPath(schemaDir.GetName());

    ECSchemaPtr schemaPtr;
    SchemaReadStatus readSchemaStatus = ECSchema::ReadFromXmlFile(schemaPtr, schemaFile.GetName(), *contextPtr);
    if (SCHEMA_READ_STATUS_Success != readSchemaStatus)
        return DgnDbStatus::ReadError;

    if (BentleyStatus::SUCCESS != db.Schemas().ImportECSchemas(contextPtr->GetCache()))
        return DgnDbStatus::BadSchema;

    db.Domains().SyncWithSchemas();
    _OnSchemaImported(db); // notify subclasses so domain objects (like categories) can be created
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnDomain::ImportSchema(DgnDbR db, ECSchemaCacheR schemaCache) const
    {
    if (BentleyStatus::SUCCESS != db.Schemas().ImportECSchemas(schemaCache))
        return DgnDbStatus::BadSchema;

    db.Domains().SyncWithSchemas();
    _OnSchemaImported(db); // notify subclasses so domain objects (like categories) can be created
    return DgnDbStatus::Success;
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
        // ###TODO Test if class HasHandler
        // ###TODO Whether missing or not, set the handler's ECClassName to match that of the subclass
        //  umm why? And we can't modify the base handler if class is NOT missing but simply doesn't have its own handler.
        //  GetClassName() returns the name of the class the *handler* handles.
        handler = handler->_CreateMissingHandler(0 /* ###TODO look up restrictions */);
        if (nullptr != handler)
            {
            m_handlers.Insert(handlerId, handler); // cache this result for all baseclasses
            return handler;
            }
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
        // BeAssert(false);
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
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler* DgnDomain::Handler::GetRootClass()
    {
    // "Handler" class itself has no domain...
    return nullptr != m_superClass && nullptr != m_superClass->m_domain ? m_superClass->GetRootClass() : this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnDomain::Handler::_VerifySchema(DgnDomains& domains)
    {
#if !defined (NDEBUG)
    if (&DgnDomain::Handler::GetHandler() == this) // Handler is always ok
        return DgnDbStatus::Success;
    
    DgnClassId classId = domains.GetClassId(*this);
    BeAssert(classId.IsValid());

    Handler* handlerSuperClass = GetSuperClass();
    if (&DgnDomain::Handler::GetHandler() == handlerSuperClass)
        return DgnDbStatus::Success;
    
    DgnClassId superClassId = domains.GetClassId(*handlerSuperClass);

    auto const& schemas = domains.GetDgnDb().Schemas();
    ECN::ECClassCP myEcClass    = schemas.GetECClass(classId.GetValue());
    ECN::ECClassCP superEcClass = schemas.GetECClass(superClassId.GetValue());

    if (!myEcClass->Is(superEcClass))
        {
        printf("ERROR: HANDLER hiearchy does not match ECSCHMA hiearchy:\n"
               " Handler [%s] says it handles ECClass '%s', \n"
               " but that class does not derive from its superclass handler's ECClass '%s'\n", 
                typeid(*this).name(), GetClassName().c_str(), handlerSuperClass->GetClassName().c_str());

        BeAssert(false);
        }
    else
        {
        Handler* rootClass = GetRootClass();
        ECN::ECClassCP rootEcClass = schemas.GetECClass(domains.GetClassId(*rootClass).GetValue());
        BeAssert(nullptr != rootEcClass);
        if (nullptr != rootEcClass && rootEcClass != myEcClass && !myEcClass->IsSingularlyDerivedFrom(*rootEcClass))
            {
            printf("ERROR: HANDLER [%s] handles ECClass '%s' which derives more than once from root ECClass '%s'.\n",
            typeid(*this).name(), GetClassName().c_str(), rootClass->GetClassName().c_str());

            BeAssert(false);
            }
        }
#endif
    
    return DgnDbStatus::Success;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus dgn_ElementHandler::Element::_VerifySchema(DgnDomains& domains)
    {
#if !defined (NDEBUG)
    T_Super::_VerifySchema(domains);

    if (&dgn_ElementHandler::Element::GetHandler() == this) // Element base class is always ok
        return DgnDbStatus::Success;
    
    auto const& schemas = domains.GetDgnDb().Schemas();
    dgn_ElementHandler::Element* handlerSuperClass = (dgn_ElementHandler::Element*) GetSuperClass();

    DgnElement::CreateParams params(domains.GetDgnDb(), DgnModelId(), DgnClassId());
    DgnElementPtr thisEl = _CreateInstance(params);
    if (0 != strcmp(thisEl->_GetECClassName(), GetClassName().c_str()))
        {
        printf("HANDLER SETUP ERROR: Handler [%s] says it handles ECClass '%s', \n"
               "    but its DgnElement class [%s] says its ECClass is '%s'\n"
               "    (make sure you have a DGNELEMENT_DECLARE_MEMBERS macro in your DgnElement class).\n",
                typeid(*this).name(), GetClassName().c_str(), 
                typeid(*thisEl).name(), thisEl->_GetECClassName());

        BeAssert(false);
        }

    if (0 != strcmp(thisEl->_GetSuperECClassName(), handlerSuperClass->GetClassName().c_str()))
        {
        printf("HANDLER SUPERCLASS ERROR: Handler [%s] says its superclass ECClass is '%s', \n"
               "    but its DgnElement class [%s] says its ECClass superclass is '%s'\n", 
                typeid(*this).name(), handlerSuperClass->GetClassName().c_str(), 
                typeid(*thisEl).name(), thisEl->_GetSuperECClassName());

        BeAssert(false);
        }

    DgnClassId classId(schemas.GetECClassId(GetDomain().GetDomainName(), thisEl->_GetECClassName()));
    DgnClassId superClassId(schemas.GetECClassId(handlerSuperClass->GetDomain().GetDomainName(), thisEl->_GetSuperECClassName()));

    ECN::ECClassCP myEcClass    = schemas.GetECClass(classId.GetValue());
    ECN::ECClassCP superEcClass = schemas.GetECClass(superClassId.GetValue());

    if (!myEcClass->Is(superEcClass))
        {
        printf("ELEMENT INHERITANCE ERROR: Element class [%s] handles ECClass '%s', but it does not derive from ECClass '%s'\n", typeid(*thisEl).name(), 
                thisEl->_GetECClassName(), thisEl->_GetSuperECClassName());
        BeAssert(false);
        }
#endif
    
    return DgnDbStatus::Success;
    }

