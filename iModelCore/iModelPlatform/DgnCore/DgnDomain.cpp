/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDomains::RegisterDomain(DgnDomain& domain, DgnDomain::Required isRequired, DgnDomain::Readonly isReadonly, BeFileNameCP schemaRootDir)
    {
    auto& domains = T_HOST.RegisteredDomains();
    for (DgnDomainCP it : domains)
        {
        if (it->m_domainName.EqualsI(domain.m_domainName))
            return SUCCESS;
        }

    domain.SetRequired(isRequired);
    domain.SetReadonly(isReadonly);
    if (schemaRootDir)
        domain.SetSchemaRootDir(*schemaRootDir);

    if (!domain.ValidateSchemaPathname())
        return ERROR;

    domains.push_back(&domain);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDomain::GetSchemaPathname() const
    {
    BeFileName schemaPathname = m_schemaRootDir.IsEmpty() ? T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory() : m_schemaRootDir;
    schemaPathname.AppendToPath(_GetSchemaRelativePath());
    return schemaPathname;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDomain::ValidateSchemaPathname() const
    {
    BeFileName schemaPathname = GetSchemaPathname();
    if (!schemaPathname.DoesPathExist())
        {
        LOG.errorv(L"Schema '%s' does not exist", schemaPathname.GetName());
        // BeAssert(false && "Schema does not exist");
        return false;
        }

    BeFileName schemaBaseNameW;
    schemaPathname.ParseName(NULL, NULL, &schemaBaseNameW, NULL);
    Utf8String schemaBaseName(schemaBaseNameW);

    if (0 != BeStringUtilities::Strnicmp(schemaBaseName.c_str(), GetDomainName(), strlen(GetDomainName())))
        {
        LOG.errorv("Schema name '%s' does not match Domain name '%s'", schemaBaseName.c_str(), GetDomainName());
        BeAssert(false && "Schema name and DgnDomain name must match");
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomain::LoadHandlers(DgnDbR dgndb) const
    {
    // save all registered handlers for this Domain into the DgnDomains list *for this DgnDb* by looking up their classId
    for (Handler* handler : m_handlers)
        dgndb.Domains().SaveHandlerByClassId(*handler);

    for (auto* tblHandler : m_tableHandlers)
        dgndb.Txns().AddTxnTable(tblHandler);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDomain::IsSchemaImported(DgnDbCR dgndb) const
    {
    return dgndb.Schemas().ContainsSchema(m_domainName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr DgnDomain::ReadSchema(ECSchemaReadContextR schemaContext) const
    {
    BeFileName pathname = GetSchemaPathname();
    if (!pathname.DoesPathExist())
        {
        BeAssert(false && "Schema for domain doesn't exist");
        return nullptr;
        }

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, pathname.GetName(), schemaContext);

    if (SchemaReadStatus::Success != status)
        {
        LOG.errorv("Error reading schema %ls", GetSchemaPathname().GetName());
        BeAssert(false && "Error reading schema");
        }
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus DgnDomain::ValidateSchema(ECSchemaCR schema, DgnDbCR dgndb) const
    {
    if (0 != BeStringUtilities::StricmpAscii(schema.GetName().c_str(), GetDomainName()))
        {
        LOG.errorv("Schema name %s must match domain name %s", schema.GetName().c_str(), GetDomainName());
        BeAssert(false && "Schema name must match domain name");
        return SchemaStatus::SchemaDomainNamesMismatched;
        }

    return DgnDomains::DoValidateSchema(schema, IsReadonly() || dgndb.IsReadonly(), dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* load a Domain found in the Domain table of a DgnDb into that dgndb's DgnDomains
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomains::LoadDomain(DgnDomainR domain)
    {
    domain.LoadHandlers(m_dgndb);   // load all the handlers from this domain
    m_domains.push_back(&domain);   // save the fact that we are using this domain
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
            LOG.infov("Missing Domain [%s]", stmt.GetValueText(0));
            continue;
            }

        if (thisDomain->second->GetVersion() < stmt.GetValueInt(1))
            {
            LOG.errorv("Wrong Domain version [%s]", stmt.GetValueText(0));
            BeAssert(false);
            continue;
            }

        LoadDomain(*thisDomain->second);
        registeredDomains.erase(thisDomain); // so we know we've already found it.
        }

    // any that are left are new and need to be added to the database
    for (auto iter : registeredDomains)
        {
        if (nullptr == m_dgndb.Schemas().GetSchema(iter.second->GetDomainName(), false))
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
* Before you can register a Handler, its superclass Handler must be registered.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnDomain::VerifySuperclass(Handler& handler)
    {
    Handler* superclass = handler.GetSuperClass();
    if (&DgnDomain::Handler::GetHandler() == superclass) // Handler is always "registered"
        return DgnDbStatus::Success;

    if (nullptr == superclass || (nullptr == superclass->GetDomain().FindHandler(superclass->m_ecClassName.c_str())))
        {
        BeAssert(false && "Could not locate handler superclass");
        return DgnDbStatus::MissingHandler;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
            BeAssert(false && "Cannot re-register a handler which was not previously registered");
            return DgnDbStatus::NotFound;
            }

        m_handlers.erase(thisHandler);
        }
    else if (thisHandler!=m_handlers.end()) // register only works if we DON'T already have this handler.
        {
        BeAssert(false && "Handler already registered");
        return DgnDbStatus::AlreadyLoaded;
        }

    handler.SetDomain(*this);
    m_handlers.push_back(&handler);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomains::OnDbOpened()
    {
    if (m_dgndb.AreTxnsEnabled())
        {
        TxnManagerR txnManager = m_dgndb.Txns();
        txnManager.EnableTracking(true);
        txnManager.InitializeTableHandlers(); // Necessary to this after the domains are loaded so that the tables are already setup. The method calls SaveChanges().
        }

    for (DgnDomainCP domain : m_domains)
        domain->_OnDgnDbOpened(m_dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomains::OnDbClose()
    {
    for (DgnDomainCP domain : m_domains)
        domain->_OnDgnDbClose(m_dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus DgnDomain::ImportSchema(DgnDbR dgndb)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);
    schemaContext->SetFinalSchemaLocater(dgndb.GetSchemaLocater());

    ECSchemaPtr schema = ReadSchema(*schemaContext);
    if (!schema.IsValid())
        return SchemaStatus::SchemaReadFailed;

    SchemaStatus status = ValidateSchema(*schema, dgndb);
    if (status != SchemaStatus::SchemaNotFound)
        {
        BeAssert((status != SchemaStatus::SchemaUpgradeRequired) && "Upgrading of individual domains should be done only when the DgnDb is opened. See SchemaUpgradeOptions in DgnDb::OpenParams");
        return status;
        }

    bvector<ECSchemaPtr> schemasToImport;
    schemasToImport.push_back(schema);

    bvector<DgnDomainP> domainsToImport;
    domainsToImport.push_back(this);

    return dgndb.Domains().DoImportSchemas(schemasToImport, domainsToImport);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaStatus DgnDomains::ImportSchemas()
    {
    bvector<ECSchemaPtr> schemasToImport;
    bvector<DgnDomainP> domainsToImport;

    SchemaStatus status = DoValidateSchemas(&schemasToImport, &domainsToImport);
    if (status != SchemaStatus::SchemaUpgradeRequired && status != SchemaStatus::SchemaUpgradeRecommended)
        return status;

    return DoImportSchemas(schemasToImport, domainsToImport);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaStatus DgnDomains::DoImportSchemas(bvector<ECSchemaPtr> const& schemasToImport, bvector<DgnDomainP> const& domainsToImport)
    {
    BeAssert(!schemasToImport.empty());
    bvector<ECSchemaCP> importSchemas;
    for (auto& schema : schemasToImport)
        importSchemas.push_back(schema.get());

    SchemaStatus status = DoImportSchemas(importSchemas, SchemaManager::SchemaImportOptions::None);
    if (SchemaStatus::Success != status)
        return status;

    SyncWithSchemas();

    for (DgnDomainP domain : domainsToImport)
        domain->_OnSchemaImported(m_dgndb);

    return SchemaStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus DgnDomains::UpgradeSchemas(bvector<ECSchemaPtr> const& schemasToImport, bvector<DgnDomainP> const& domainsToImport)
    {
    BeAssert(m_schemaUpgradeOptions.AreDomainUpgradesAllowed() && !schemasToImport.empty());

    bvector<ECSchemaCP> importSchemas;
    for (auto& schema : schemasToImport)
        importSchemas.push_back(schema.get());

    if (m_dgndb.AreTxnsRequired())
        m_dgndb.Txns().EnableTracking(true); // Ensure all schema changes are captured in the txn table for creating revisions

    SchemaManager::SchemaImportOptions importOptions = SchemaManager::SchemaImportOptions::None;
    SchemaStatus status = DoImportSchemas(importSchemas, importOptions);
    if (SchemaStatus::Success != status)
        return status;

    SyncWithSchemas();

    if (m_dgndb.AreTxnsRequired())
        {
        m_dgndb.Txns().InitializeTableHandlers();
        // Necessary to this after the domains are loaded so that the tables are already setup. The method calls SaveChanges().
        }

    for (DgnDomainCP domain : m_domains)
        {
        // Call domain handlers of dependent domains before the newly imported domains
        if (std::find(domainsToImport.begin(), domainsToImport.end(), domain) != domainsToImport.end())
            continue;
        domain->_OnDgnDbOpened(m_dgndb);
        }

    for (DgnDomainP domain : domainsToImport)
        {
        domain->_OnSchemaImported(m_dgndb);
        domain->_OnDgnDbOpened(m_dgndb);
        }
    return SchemaStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus DgnDomains::InitializeSchemas(SchemaUpgradeOptions const& schemaUpgradeOptions, bvector<ECSchemaPtr>* schemasToImport, bvector<DgnDomainP>* domainsToImport)
    {
    m_schemaUpgradeOptions = schemaUpgradeOptions;
    SchemaUpgradeOptions::DomainUpgradeOptions domainUpgradeOptions = m_schemaUpgradeOptions.GetDomainUpgradeOptions();

    SchemaStatus status = SchemaStatus::Success;
    if (domainUpgradeOptions != SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck)
        {
        status = DoValidateSchemas(schemasToImport, domainsToImport);
        if (status == SchemaStatus::SchemaTooNew || status == SchemaStatus::SchemaTooOld || status == SchemaStatus::SchemaUpgradeRequired)
            return status;
        if (status == SchemaStatus::SchemaUpgradeRecommended &&
            (domainUpgradeOptions == SchemaUpgradeOptions::DomainUpgradeOptions::CheckRecommendedUpgrades ||
            domainUpgradeOptions == SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade))
            return status;
        }

    SyncWithSchemas();
    OnDbOpened();

    return SchemaStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContextPtr DgnDomains::PrepareSchemaReadContext() const
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext(false/*=acceptLegacyImperfectLatestCompatibleMatch*/, true/*=includeFilesWithNoVerExt*/);

    BeFileName schemaPath = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    schemaPath.AppendToPath(L"ECSchemas");

    BeFileName standardSchemasPath(schemaPath);
    standardSchemasPath.AppendToPath(L"Standard");
    context->AddSchemaPath(standardSchemasPath);

    // Legacy file handling: When importing/upgrading domain schemas into a file that does not support EC 3.2 yet (corresponds to ECDb profile 4.0.0.1 or older)
    // the ECDb schema assets must not be included, because they are all EC3.2 schemas and the schema locate
    // would prefer those over the ones in the DgnDb file. For the legacy file handling case we cannot import EC3.2 schemas though.
    // We can safely ignore those for legacy files, as the ECDbSchemaPolicies schema is already included in any legacy iModel (because BisCore references it).
    // So we never have to look that schema up from disk for legacy files.
    if (GetDgnDb().GetECDbProfileVersion() >= BeVersion(4, 0, 0, 2))
        {
        //Not all ECDb schemas are always included in the ECDb file, e.g. ECDbSchemaPolicies. Therefore make those locatable
        //from the assets dir
        BeFileName ecdbSchemasPath(schemaPath);
        ecdbSchemasPath.AppendToPath(L"ECDb");
        context->AddSchemaPath(ecdbSchemasPath);
        }

    context->SetFinalSchemaLocater(GetDgnDb().GetSchemaLocater()); // Schemas must first be located in disk (i.e., domain schemas) before finding them in the Db.
    return context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus DgnDomains::DoValidateSchemas(bvector<ECSchemaPtr>* schemasToImport, bvector<DgnDomainP>* domainsToImport)
    {
    SchemaStatus status = SchemaStatus::Success;
    DgnDbR dgndb = GetDgnDb();
    ECSchemaReadContextPtr schemaContext = PrepareSchemaReadContext();
    ECSchemaCacheR cache = schemaContext->GetCache();
    bset<ECSchemaP> validatedSchemas;

    /* Validate the schema for all the domains */
    auto& hostDomains = T_HOST.RegisteredDomains();
    for (DgnDomainP domain : hostDomains)
        {
        bvector<ECSchemaP> existingSchemas;
        cache.GetSchemas(existingSchemas);

        ECSchemaPtr schema = domain->ReadSchema(*schemaContext);
        if (!schema.IsValid())
            return SchemaStatus::SchemaReadFailed;

        SchemaStatus locStatus = domain->ValidateSchema(*schema, dgndb);
        validatedSchemas.insert(schema.get());

        if (locStatus == SchemaStatus::Success)
            continue;

        if (locStatus == SchemaStatus::SchemaNotFound)
            {
            if (!domain->IsRequired())
                {
                // Restore schema context to the state before the read so that we don't
                // later import unwanted references of optional domain schemas.
                bvector<ECSchemaP> currentSchemas;
                cache.GetSchemas(currentSchemas);
                for (ECSchemaP currentSchema : currentSchemas)
                    {
                    if (std::find(existingSchemas.begin(), existingSchemas.end(), currentSchema) == existingSchemas.end())
                        cache.DropSchema(currentSchema->GetSchemaKey());
                    }
                continue;
                }

            status = SchemaStatus::SchemaUpgradeRequired; // It could get worse!
            if (domainsToImport)
                domainsToImport->push_back(domain);
            if (schemasToImport)
                schemasToImport->push_back(schema);

            // an iModelBridge may open db twice, one to check if schemas need upgrades and another to actually upgrade them. Separate logs to help diagnosis in PROD:
            if (m_schemaUpgradeOptions.GetDomainUpgradeOptions() == SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade && m_allowSchemaImport)
                LOG.infov("Domain schema %s will be imported", domain->GetDomainName());
            else
                LOG.infov("Schema for a required domain %s is not found, but may be imported later in the process by the same calling app", domain->GetDomainName());
            continue;
            }

        if (locStatus == SchemaStatus::SchemaTooNew || locStatus == SchemaStatus::SchemaTooOld)
            return locStatus;

        BeAssert(locStatus == SchemaStatus::SchemaUpgradeRequired || locStatus == SchemaStatus::SchemaUpgradeRecommended);
        if (status != SchemaStatus::SchemaUpgradeRequired)
            status = locStatus; // SchemaUpgradeRequired trumps SchemaUpgradeRecommended
        if (schemasToImport)
            schemasToImport->push_back(schema);
        }

    BeAssert(status == SchemaStatus::Success || status == SchemaStatus::SchemaUpgradeRequired || status == SchemaStatus::SchemaUpgradeRecommended);

    /* Validate all (remaining) reference schemas */
    bvector<ECSchemaP> allSchemas;
    cache.GetSchemas(allSchemas);
    for (ECSchemaP schema : allSchemas)
        {
        if (validatedSchemas.end() != std::find(validatedSchemas.begin(), validatedSchemas.end(), schema))
            continue;

        SchemaStatus locStatus = DoValidateSchema(*schema, true /*=isReadonly*/, dgndb);

        validatedSchemas.insert(schema);

        if (locStatus == SchemaStatus::Success)
            continue;

        if (locStatus == SchemaStatus::SchemaTooNew || locStatus == SchemaStatus::SchemaTooOld)
            return locStatus;

        BeAssert(locStatus == SchemaStatus::SchemaNotFound || locStatus == SchemaStatus::SchemaUpgradeRequired || locStatus == SchemaStatus::SchemaUpgradeRecommended);
        // A referenced schema that is not found does not make a recommended domain schema upgrade required
        if (status != SchemaStatus::SchemaUpgradeRequired && (locStatus == SchemaStatus::SchemaUpgradeRecommended || locStatus == SchemaStatus::SchemaNotFound))
            status = SchemaStatus::SchemaUpgradeRecommended;
        else
            status = SchemaStatus::SchemaUpgradeRequired;
        
        if (schemasToImport)
            schemasToImport->push_back(schema);
        }

    return status;
    }

bool canBeUpgradedWithoutVersionChange(ECSchemaCR ecSchema)
    {
    ECN::SchemaKey schemaKeyECDbMap("ECDbMap", 2, 0, 0);
    ECN::SchemaKey schemaKeyECDbSchemaPolicies("ECDbSchemaPolicies", 1, 0, 0);
    auto const& references = ecSchema.GetReferencedSchemas();
    return references.end() == references.Find(schemaKeyECDbMap, SchemaMatchType::Latest) &&
        references.end() == references.Find(schemaKeyECDbSchemaPolicies, SchemaMatchType::Latest) &&
        !ecSchema.IsDefined("BisCore", "SchemaHasBehavior");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaStatus DgnDomains::DoValidateSchema(ECSchemaCR appSchema, bool isSchemaReadonly, DgnDbCR db)
    {
    SchemaKeyCR appSchemaKey = appSchema.GetSchemaKey();
    ECSchemaCP bimSchema = db.Schemas().GetSchema(appSchemaKey.GetName().c_str(), false);
    if (!bimSchema)
        {
        LOG.tracev("Application schema %s was not found in the iModel.", appSchemaKey.GetFullSchemaName().c_str());
        return SchemaStatus::SchemaNotFound;
        }
    SchemaKeyCR bimSchemaKey = bimSchema->GetSchemaKey();

    if (appSchema.IsDynamicSchema() && 0 == bimSchemaKey.CompareByVersion(appSchemaKey) && canBeUpgradedWithoutVersionChange(appSchema))
        {
        LOG.debugv("Schema found in the BIM %s has the same version as that in the application %s.  The schema is dynamic so its contents must be checked for updates.", bimSchemaKey.GetFullSchemaName().c_str(), appSchemaKey.GetFullSchemaName().c_str());
        return SchemaStatus::SchemaIsDynamic;
        }

    if (appSchemaKey.GetVersionRead() == bimSchemaKey.GetVersionRead() && appSchemaKey.GetVersionWrite() == bimSchemaKey.GetVersionWrite() && appSchemaKey.GetVersionMinor() <= bimSchemaKey.GetVersionMinor())
        return SchemaStatus::Success; // Most common case

    if (appSchemaKey.GetVersionRead() < bimSchemaKey.GetVersionRead())
        {
        LOG.errorv("Schema found in the BIM %s is too new compared to that in the application %s", bimSchemaKey.GetFullSchemaName().c_str(), appSchemaKey.GetFullSchemaName().c_str());
        return SchemaStatus::SchemaTooNew;
        }

    if (appSchemaKey.GetVersionRead() > bimSchemaKey.GetVersionRead())
        {
        LOG.errorv("Schema found in the BIM %s is too old compared to that in the application %s", bimSchemaKey.GetFullSchemaName().c_str(), appSchemaKey.GetFullSchemaName().c_str());
        return SchemaStatus::SchemaTooOld;
        }

    if (appSchemaKey.GetVersionWrite() > bimSchemaKey.GetVersionWrite())
        {
        if (!isSchemaReadonly)
            {
            LOG.debugv("Schema found in the BIM %s needs to (and can) be upgraded to that in the application %s", bimSchemaKey.GetFullSchemaName().c_str(), appSchemaKey.GetFullSchemaName().c_str());
            return SchemaStatus::SchemaUpgradeRequired;
            }

        LOG.debugv("Schema found in the BIM %s needs to (and can) be upgraded to that in the application %s. However, this is not an issue since the application or domain is readonly.", bimSchemaKey.GetFullSchemaName().c_str(), appSchemaKey.GetFullSchemaName().c_str());
        return SchemaStatus::SchemaUpgradeRecommended;
        }

    if (appSchemaKey.GetVersionMinor() > bimSchemaKey.GetVersionMinor())
        {
        LOG.debugv("Schema found in the BIM %s is recommended to be upgraded to that in the application %s", bimSchemaKey.GetFullSchemaName().c_str(), appSchemaKey.GetFullSchemaName().c_str());
        return SchemaStatus::SchemaUpgradeRecommended;
        }

    BeAssert(appSchemaKey.GetVersionWrite() < bimSchemaKey.GetVersionWrite());

    if (!isSchemaReadonly)
        {
        LOG.errorv("Schema found in the BIM %s is too new compared to that in the application %s", bimSchemaKey.GetFullSchemaName().c_str(), appSchemaKey.GetFullSchemaName().c_str());
        return SchemaStatus::SchemaTooNew;
        }

    LOG.debugv("Schema found in the BIM %s is newer and found to be write incompatible to that in the application %s. However, this is not an issue since the application or domain is readonly.", bimSchemaKey.GetFullSchemaName().c_str(), appSchemaKey.GetFullSchemaName().c_str());
    return SchemaStatus::SchemaUpgradeRecommended;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::DropSchemaResult DgnDomains::DoDropSchema(Utf8StringCR name, bool logIssue) {
    DgnDbR dgndb = GetDgnDb();
    if (dgndb.IsReadonly()) {
        LOG.error("Cannot drop schema from a Readonly Db");
        return BeSQLite::EC::DropSchemaResult(DropSchemaResult::ErrorDbIsReadonly);
    }

    if (!m_allowSchemaImport) {
        LOG.error("Drop schema is prohibited");
        return BeSQLite::EC::DropSchemaResult(DropSchemaResult::Error);
    }

    if (dgndb.IsBriefcase()) {
        if (dgndb.Txns().HasLocalChanges()) {
            LOG.error("Cannot drop schema when there are local changes. Commit any outstanding changes, then create and finish/abandon a revision to flush the TxnTable");
            return BeSQLite::EC::DropSchemaResult(DropSchemaResult::ErrorDbHasLocalChanges);
        }
    }

    if (LOG.isSeverityEnabled(SEVERITY::LOG_DEBUG)) {
        LOG.debugv("Schema to be dropped: %s", name.c_str());
    }

    dgndb.Txns().SetHasEcSchemaChanges(true);
    return dgndb.Schemas().DropSchema(name, dgndb.GetSchemaImportToken(), logIssue);
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus DgnDomains::DoImportSchemas(bvector<ECSchemaCP> const &importSchemas, SchemaManager::SchemaImportOptions importOptions) {
    if (importSchemas.empty())
        return SchemaStatus::Success;

    // always disallow major schema upgrades for domain schema imports. Major schema upgrades are only allowed across software generations
    importOptions |= SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade;

    DgnDbR dgndb = GetDgnDb();
    if (dgndb.IsReadonly()) {
        BeAssert(false && "Cannot import schemas into a Readonly Db");
        return SchemaStatus::DbIsReadonly;
    }

    if (!m_allowSchemaImport) {
        BeAssert(false && "ImportSchemas is prohibited");
        return SchemaStatus::SchemaImportFailed;
    }

    if (dgndb.IsBriefcase()) {
        if (dgndb.Txns().HasLocalChanges()) {
            // The dgnv8converter generates changes to the be_EmbedFile table just prior to importing a generated schema. Don't reject the schema just for that.
            if ((importOptions & SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues) != SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues) {
                BeAssert(false && "Cannot upgrade schemas when there are local changes. Commit any outstanding changes, then create and finish/abandon a revision to flush the TxnTable");
                return SchemaStatus::DbHasLocalChanges;
            }
        }
    }

    if (LOG.isSeverityEnabled(SEVERITY::LOG_DEBUG)) {
        LOG.debug("Schemas to be imported:");
        for (ECSchemaCP schema : importSchemas)
            LOG.debugv("\t%s", schema->GetFullSchemaName().c_str());
    }

    dgndb.Txns().SetHasEcSchemaChanges(true);

    if (BentleyStatus::SUCCESS != dgndb.Schemas().ImportSchemas(importSchemas, importOptions, dgndb.GetSchemaImportToken())) {
        if ((importOptions & SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues) == SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues) {
            LOG.errorv("Failed to import legacy V8 schemas");
        } else {
            DbResult result = dgndb.AbandonChanges();
            UNUSED_VARIABLE(result);
            BeAssert(result == BE_SQLITE_OK);
        }

        return SchemaStatus::SchemaImportFailed;
    }

    return SchemaStatus::Success;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDomains::InsertDomain(DgnDomainCR domain) {
    Statement stmt(m_dgndb, SqlPrintfString("INSERT INTO " DGN_TABLE_Domain " (Name,Description,Version) VALUES(?,?,?)"));
    stmt.BindText(1, domain.GetDomainName(), Statement::MakeCopy::No);
    stmt.BindText(2, domain.GetDomainDescription(), Statement::MakeCopy::No);
    stmt.BindInt(3, domain.GetVersion());
    return stmt.Step();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler* DgnDomains::LookupHandler(DgnClassId handlerId)
    {
    auto iter = m_handlers.find(handlerId);
    return iter == m_handlers.end() ? nullptr : iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP DgnDomains::FindBaseOfType(DgnClassId subClassId, DgnClassId baseClassId)
    {
    auto const& schemas = m_dgndb.Schemas();
    ECN::ECClassCP subClass  = schemas.GetClass(subClassId);
    ECN::ECClassCP baseClass = schemas.GetClass(baseClassId);

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
* @bsimethod
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
        return nullptr;

    // see if baseclass has a handler, recursively.
    handler = FindHandler(DgnClassId(superClass->GetId()), baseClassId);
    if (nullptr == handler) {
        // the handlerId supplied must not derive a class with a registered handler. Something is wrong.
        BeAssert(false && "Handler not found");
        return nullptr;
    }

    m_handlers.Insert(handlerId, handler); // cache this result for baseclass
    return handler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId DgnDomains::GetClassId(DgnDomain::Handler& handler)
    {
    return m_dgndb.Schemas().GetClassId(handler.GetDomain().GetDomainName(), handler.GetClassName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* record the ClassId --> Handler mapping for the given handler
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDomains::SaveHandlerByClassId(DgnDomain::Handler& handler) {
    DgnClassId id = GetClassId(handler);
    if (id.IsValid())
        AddHandler(id, &handler);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler* DgnDomain::Handler::GetRootClass()
    {
    // "Handler" class itself has no domain...
    return nullptr != m_superClass && nullptr != m_superClass->m_domain ? m_superClass->GetRootClass() : this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    ECN::ECClassCP myEcClass    = schemas.GetClass(classId);
    ECN::ECClassCP superEcClass = schemas.GetClass(superClassId);

    if (!myEcClass->Is(superEcClass))
        {
        LOG.errorv("ERROR: HANDLER hiearchy does not match ECSCHMA hiearchy:\n"
               " Handler [%s] says it handles ECClass '%s', \n"
               " but that class does not derive from its superclass handler's ECClass '%s'\n",
                typeid(*this).name(), GetClassName().c_str(), handlerSuperClass->GetClassName().c_str());

        BeAssert(false && "Handler::_VerifySchema() failed. Check log for details");
        }
    else
        {
        Handler* rootClass = GetRootClass();
        ECN::ECClassCP rootEcClass = schemas.GetClass(domains.GetClassId(*rootClass));
        BeAssert(nullptr != rootEcClass);
        if (nullptr != rootEcClass && rootEcClass != myEcClass && !myEcClass->IsSingularlyDerivedFrom(*rootEcClass))
            {
            LOG.errorv("ERROR: HANDLER [%s] handles ECClass '%s' which derives more than once from root ECClass '%s'.\n",
            typeid(*this).name(), GetClassName().c_str(), rootClass->GetClassName().c_str());

            BeAssert(false && "Handler::_VerifySchema() failed. Check log for details");
            }
        }
#endif

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler::ExtensionEntry* DgnDomain::Handler::ExtensionEntry::Find(ExtensionEntry* start, Extension::Token const& id)
    {
    while (start && &start->m_token != &id)
        start = start->m_next;
    return start;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_ElementHandler::Element::Extension* DgnDomain::Handler::FindExtension(Extension::Token& id)
    {
    ExtensionEntry* found = ExtensionEntry::Find(m_extensions, id);
    if (NULL != found)
        return  &found->m_extension;

    return  m_superClass ? m_superClass->FindExtension(id) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler* DgnDomain::Handler::z_CreateInstance()
    {
    DgnDomain::Handler* instance= new DgnDomain::Handler();
    instance->SetSuperClass((DgnDomain::Handler*) nullptr);
    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler*& DgnDomain::Handler::z_PeekInstance()
    {
    static DgnDomain::Handler* s_instance = 0;
    return s_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler& DgnDomain::Handler::z_GetHandlerInstance()
    {
    DgnDomain::Handler*& instance = z_PeekInstance();

    if (0 == instance)
        instance = z_CreateInstance();

    return *instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerP dgn_ElementHandler::Element::FindHandler(DgnDb const& db, DgnClassId handlerId)
    {
    // quick check for a handler already known
    DgnDomain::Handler* handler = db.Domains().LookupHandler(handlerId);
    if (nullptr != handler)
        return handler->_ToElementHandler();

    // not there, check via base classes
    handler = db.Domains().FindHandler(handlerId, db.Domains().GetClassId(dgn_ElementHandler::Element::GetHandler()));
    return handler ? handler->_ToElementHandler() : (BeAssert(false && "Element handler not found"), nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> struct HandlerTraits {};

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
template<> struct HandlerTraits<dgn_ElementHandler::Element>
{
    typedef DgnElement T_Instantiation;

    static DgnDomain::Handler* GetBaseHandler() {return &dgn_ElementHandler::Element::GetHandler();}
    static DgnElementPtr CreateInstance(dgn_ElementHandler::Element& handler, DgnDbR db)
        {
        DgnElement::CreateParams params(db, DgnModelId(), DgnClassId());
        params.SetIsLoadingElement(true);
        return handler.Create(params);
        }
    static Utf8CP GetECClassName(DgnElementCR el) {return el.GetHandlerECClassName();}
    static Utf8CP GetSuperECClassName(DgnElementCR el) {return el.GetSuperHandlerECClassName();}
    static Utf8CP GetCppClassName() {return "DgnElement";}
    static Utf8CP GetMacroName() {return "DGNELEMENT_DECLARE_MEMBERS";}
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
template<> struct HandlerTraits<dgn_AspectHandler::Aspect>
{
    typedef DgnElement::Aspect T_Instantiation;

    static DgnDomain::Handler* GetBaseHandler() {return &dgn_AspectHandler::Aspect::GetHandler();}
    static RefCountedPtr<DgnElement::Aspect> CreateInstance(dgn_AspectHandler::Aspect& handler, DgnDbR db) {return handler._CreateInstance();}
    static Utf8CP GetECClassName(DgnElement::Aspect const& aspect) {return aspect.GetECClassName();}
    static Utf8CP GetSuperECClassName(DgnElement::Aspect const& aspect) {return aspect.GetSuperECClassName();}
    static Utf8CP GetCppClassName() {return "DgnElement::Aspect";}
    static Utf8CP GetMacroName() {return "DGNASPECT_DECLARE_MEMBERS";}
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
template<> struct HandlerTraits<dgn_ModelHandler::Model>
{
    typedef DgnModel T_Instantiation;

    static DgnDomain::Handler* GetBaseHandler() {return &dgn_ModelHandler::Model::GetHandler();}
    static DgnModelPtr CreateInstance(dgn_ModelHandler::Model& handler, DgnDbR db) { return handler.Create(DgnModel::CreateParams(db, DgnClassId(), DgnElementId())); }
    static Utf8CP GetECClassName(DgnModelCR model) {return model._GetHandlerECClassName();}
    static Utf8CP GetSuperECClassName(DgnModelCR model) {return model._GetSuperHandlerECClassName();}
    static Utf8CP GetCppClassName() {return "DgnModel";}
    static Utf8CP GetMacroName() {return "DGNMODEL_DECLARE_MEMBERS";}
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_Handler> struct HandlerVerifier
{
private:
    DgnDomains& m_domains;
    T_Handler&  m_handler;

    typedef HandlerTraits<T_Handler> T_Traits;
    typedef typename T_Traits::T_Instantiation T_Instantiation;

public:
    HandlerVerifier(DgnDomains& domains, T_Handler& handler) : m_domains(domains), m_handler(handler) {}

    DgnDbStatus Verify()
        {
#if !defined (NDEBUG)
        if (&DgnDomain::Handler::GetHandler() == &m_handler) // Handler is always ok
            return DgnDbStatus::Success;

        // Verify Handler inheritance
        DgnClassId classId = m_domains.GetClassId(m_handler);
        BeAssert(classId.IsValid());

        DgnDomain::Handler* handlerSuperClass = m_handler.GetSuperClass();
        if (&DgnDomain::Handler::GetHandler() == handlerSuperClass)
            return DgnDbStatus::Success;

        DgnClassId superClassId = m_domains.GetClassId(*handlerSuperClass);

        auto const& schemas = m_domains.GetDgnDb().Schemas();
        ECN::ECClassCP myECClass = schemas.GetClass(classId);
        ECN::ECClassCP superECClass = schemas.GetClass(superClassId);

        if (!myECClass->Is(superECClass))
            {
            LOG.errorv("ERROR: HANDLER hierarchy does not match ECSCHMA hiearchy:\n"
                   " Handler [%s] says it handles ECClass '%s', \n"
                   " but that class does not derive from its superclass handler's ECClass '%s'\n",
                    typeid(m_handler).name(), m_handler.GetClassName().c_str(), handlerSuperClass->GetClassName().c_str());

            BeAssert(false && "Incorrect handler hierarchy - see log for details");
            }
        else
            {
            DgnDomain::Handler* rootClass = m_handler.GetRootClass();
            ECN::ECClassCP rootEcClass = schemas.GetClass(m_domains.GetClassId(*rootClass));
            BeAssert(nullptr != rootEcClass);
            if (nullptr != rootEcClass && rootEcClass != myECClass && !myECClass->IsSingularlyDerivedFrom(*rootEcClass))
                {
                LOG.errorv("ERROR: HANDLER [%s] handles ECClass '%s' which derives more than once from root ECClass '%s'.\n",
                typeid(m_handler).name(), m_handler.GetClassName().c_str(), rootClass->GetClassName().c_str());

                BeAssert(false && "Handler derives more than once from root ECClass - see log for details");
                }
            }

        if (T_Traits::GetBaseHandler() == &m_handler)
            return DgnDbStatus::Success;

        // Verify inheritance of the type instantiated by this handler
        auto instance = T_Traits::CreateInstance(m_handler, m_domains.GetDgnDb());
        if (!instance.IsValid())
            return DgnDbStatus::Success; // abstract class

        if (0 != strcmp(T_Traits::GetECClassName(*instance), m_handler.GetClassName().c_str()))
            {
            Utf8PrintfString msg("HANDLER SETUP ERROR: Handler [%s] says it handles ECClass '%s', \n"
                "    but its instantiated class [%s] says its ECClass is '%s'\n"
                "    (make sure you have a %s macro in your %s class).\n",
                typeid(m_handler).name(), m_handler.GetClassName().c_str(),
                typeid(*instance).name(), T_Traits::GetECClassName(*instance),
                T_Traits::GetMacroName(), T_Traits::GetCppClassName());

            LOG.errorv("%s", msg.c_str());
            BeAssert(false && "Inconsistent handler class hierarchy - see log for details");
            }

        if (0 != strcmp(T_Traits::GetSuperECClassName(*instance), handlerSuperClass->GetClassName().c_str()))
            {
            Utf8PrintfString msg("HANDLER SUPERCLASS ERROR: Handler [%s] says its superclass ECClass is '%s', \n"
                   "    but its %s class [%s] says its ECClass superclass is '%s'\n",
                    typeid(m_handler).name(), handlerSuperClass->GetClassName().c_str(),
                    T_Traits::GetCppClassName(), typeid(*instance).name(), T_Traits::GetSuperECClassName(*instance));

            LOG.errorv("%s", msg.c_str());
            BeAssert(false && "Inconsistent handler class hierarchy - see log for details");
            }

        DgnClassId instanceClassId(schemas.GetClassId(m_handler.GetDomain().GetDomainName(), T_Traits::GetECClassName(*instance)));
        DgnClassId instanceSuperClassId(schemas.GetClassId(handlerSuperClass->GetDomain().GetDomainName(), T_Traits::GetSuperECClassName(*instance)));

        ECN::ECClassCP instanceECClass = schemas.GetClass(instanceClassId);
        ECN::ECClassCP instanceSuperECClass = schemas.GetClass(instanceSuperClassId);
        if (!instanceECClass->Is(instanceSuperECClass))
            {
            LOG.errorv("C++ INHERITANCE ERROR: %s class [%s] handlers ECClass '%s', but it does not derive from ECClass '%s'\n",
                T_Traits::GetCppClassName(), typeid(*instance).name(), T_Traits::GetECClassName(*instance), T_Traits::GetSuperECClassName(*instance));
            BeAssert(false && "Inconsistent handler class hierarchy - see log for details");
            }
#endif
        return DgnDbStatus::Success;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus dgn_ElementHandler::Element::_VerifySchema(DgnDomains& domains)
    {
    HandlerVerifier<dgn_ElementHandler::Element> verifier(domains, *this);
    return verifier.Verify();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus dgn_AspectHandler::Aspect::_VerifySchema(DgnDomains& domains)
    {
    HandlerVerifier<dgn_AspectHandler::Aspect> verifier(domains, *this);
    return verifier.Verify();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus dgn_ModelHandler::Model::_VerifySchema(DgnDomains& domains)
    {
    HandlerVerifier<dgn_ModelHandler::Model> verifier(domains, *this);
    return verifier.Verify();
    }
