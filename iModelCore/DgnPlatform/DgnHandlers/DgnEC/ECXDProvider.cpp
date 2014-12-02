/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/ECXDProvider.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct BindECXDPropertyExpressions;

using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::Key&   ECXDProvider::GetHostKey ()
    {
    static DgnHost::Key key;
    return key;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDProvider::InitProvider (DgnECManagerR mgr)
    {
    ECXDProvider* provider = static_cast<ECXDProvider*> (T_HOST.GetHostObject (GetHostKey()));
    if (NULL != provider)
        return;
    
    provider = ECXDProvider::CreateProvider ();
    T_HOST.SetHostObject (GetHostKey(), provider);

    if (SUCCESS != mgr.RegisterProvider (*provider))
        BeAssert(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ECXDProvider::BackDoor_RegisterProvider()
    {
    ECXDProvider::InitProvider (DgnECManager::GetManager());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDProviderR            ECXDProvider::GetProvider()
    {
    ECXDProvider* provider = static_cast<ECXDProvider*> (T_HOST.GetHostObject (GetHostKey()));
    if (NULL != provider)
        return *provider;

    DgnECManager::GetManager().GetLogger().error (L"ECXDProvider is not being exposed in DgnPlatform");
    throw "ProgrammerError: ECXDProvider is not being exposed in DgnPlatform";
    
    // Unreachable code
    // return *provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void*                   ECXDProvider::_OnInitializeForFile (DgnProjectR dgnFile)
    {
    ECXDPerFileCacheP perFileCache = new ECXDPerFileCache (dgnFile);
    InitializePerFileCache (*perFileCache);

    // add the perfile cache as a schema locater so we can locate referenced schemas
    AddSchemaLocater ((ECN::IECSchemaLocater*)perFileCache);

    return perFileCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDProvider::_OnDisconnectFromFile (DgnProjectR dgnFile, void* providerPerFileCache)
    {
    ECXDPerFileCacheP perFileCache = (ECXDPerFileCacheP)providerPerFileCache;
    DEBUG_EXPECT (NULL != perFileCache);
    if (NULL != perFileCache)
        {
        RemoveSchemaLocater ((ECN::IECSchemaLocater*)perFileCache);
        delete perFileCache;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportStatus      ECXDProvider::_ImportSchema (ECSchemaR schema, DgnProjectR dgnFile, bool isExternal)
    {
    WCharCP schemaName  = schema.GetName().c_str();
    UInt32 major        = schema.GetVersionMajor();
    UInt32 minor        = schema.GetVersionMinor();

    if (DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        DgnECManager::GetManager().GetLogger().debugv (L"ECXDProvider::ImportSchema %ls.%02d.%02d into file %ls", schemaName, major, minor, dgnFile.GetFileName().c_str());
    
    WString schemaXml;
    SchemaWriteStatus status = schema.WriteToXmlString (schemaXml);
    if (SCHEMA_WRITE_STATUS_Success != status)
        return SCHEMAIMPORT_FailedToSerializeAsXml;

    ECXDPerFileCache& perFileCache = GetPerFileCache (dgnFile);

    ECXDPerSchemaCacheCP perSchemaCache = dynamic_cast<ECXDPerSchemaCacheP>(perFileCache.FindPerSchemaCacheByName (schemaName));
    if (NULL != perSchemaCache)
        return SCHEMAIMPORT_SchemaAlreadyStoredInFile;
    
    SchemaElementHandler& handler = SchemaElementHandler::GetInstance();

    ElementRefP elemRef = handler.CreateSchemaElement (schemaName, major, minor, GetProviderName(), schemaXml.c_str(), dgnFile);
    if (NULL == elemRef)
        return SCHEMAIMPORT_FailedToWriteElement;

    SchemaInfo schemaInfo (schema.GetSchemaKey(), dgnFile, GetProviderName(), 0, true);
    perFileCache.CreatePerSchemaCache (&schema, schemaInfo, elemRef);

    return SCHEMAIMPORT_Success;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECXDProvider::_ElementRequiresStoredSchema (ElementHandle& eh) const 
    {
    for (ElementHandle::XAttributeIter iterator = eh.GetXAttributeIter(); iterator.IsValid(); iterator.ToNext())
        {
        if (ECXDInstanceXAttributeHandler::GetHandler().GetId() == iterator.GetHandlerId())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDProvider::_OnSchemaElementUndoRedo (ElementRefP elemRef, bool isRemove)
    {
    DgnModelR   dgnModel = *elemRef->GetDgnModelP();
    DgnProjectR    dgnFile  = *dgnModel.GetDgnProject();

    ECXDPerFileCache& perFileCache = GetPerFileCache (dgnFile);

    if ( ! isRemove)
        {
        // A schema element has been reinstated, we need to recreate the PerSchemaCache
        SchemaInfo schemaInfo(SchemaKey(), dgnFile);
        SchemaElementHandler::GetInstance().SchemaInfoFromElement (ElementHandle (elemRef), schemaInfo);

        perFileCache.CreatePerSchemaCache (NULL, schemaInfo, elemRef);
        return;
        }

    // A schema element has been reversed, we need to clean up references to it in the PerFileCache
    ECXDPerSchemaCacheP perSchemaCache = perFileCache.FindPerSchemaCacheByElementRef (elemRef);

    if (NULL == perSchemaCache)
        return;

    ECSchemaP schema = perSchemaCache->GetSchemaPtr();

    if (NULL != schema)
        {
        DgnECPerFileCache& dgnPerFileCache = DgnECManager::GetManager().GetPerFileCache (dgnFile);

        dgnPerFileCache.ClearCachesForSchema (*schema);
        }

    perFileCache.DropPerSchemaCache (*perSchemaCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECXDProvider::_LocateSchemaInDgnFile (SchemaInfoR schemaInfo, SchemaMatchType matchType)
    {
    ECSchemaPtr schema;
    LocateSchemaInDgnFile (schema, schemaInfo, matchType);
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus           ECXDProvider::_LocateSchemaXmlInDgnFile (WStringR schemaXml, SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType)
    {
    SchemaLocateStatus status = LocateSchemaXmlInDgnFile (schemaXml, schemaInfo, matchType);
    if (status != SUCCESS)
        schemaXml.clear();
    return (BentleyStatus)status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void    FindAllReferenceSchema (bvector<ECN::ECSchemaCP>& refschemasToLoad, ECSchemaCR rootSchema, ECXDPerFileCache& perFileCache)
    {
    ECSchemaReferenceListCR referencedSchemas = rootSchema.GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        bvector<ECN::ECSchemaCP>::iterator it = std::find (refschemasToLoad.begin(), refschemasToLoad.end(), iter->second.get());
        if (it != refschemasToLoad.end())
            continue;

        // see if the reference schema is already in the perFileCache, if not add it to list to load
        if (NULL == perFileCache.FindPerSchemaCacheByName (iter->first.m_schemaName.c_str()))
            {
            refschemasToLoad.push_back (iter->second.get());
            FindAllReferenceSchema (refschemasToLoad, *iter->second, perFileCache);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDProvider::_GetSchemaInfos (bvector<SchemaInfo>& infos, DgnProjectR dgnFile, ECSchemaPersistence persistence)
    {
    // note that since a persistence of ECSCHEMAPERSISTENCE_External requires no processing as ECXD does not support external schemas like ECXAttributes. 
    if (ECSCHEMAPERSISTENCE_External == persistence)
        return;

    // get all cached schemas associated with dgnfile
    //if (ECSCHEMAPERSISTENCE_All == persistence)
    //    {
    //    GetSchemaInfosFromProviderPerFileCache (infos, dgnFile);
    //    return;
    //    }

    // get only the stored schemas
    if (ECSCHEMAPERSISTENCE_Stored & persistence)
        {
        //bvector<SchemaInfo> dgnCachedInfos;
        GetSchemaInfosFromProviderPerFileCache (infos, dgnFile);
        FOR_EACH(SchemaInfo info, infos)
            {
            BeAssert (info.IsStoredSchema() && "ECXDProvider only supports stored ECSchemas");
            //BeAssert (std::find (infos.begin(), infos.end(), info) != infos.end());
            //infos.push_back (info);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDProvider::InitializePerFileCache (ECXDPerFileCache& perFileCache)
    {
    DgnProjectR dgnFile = perFileCache.GetDgnFile();
    SchemaElementHandler::GetInstance().ScanForSchemaElementsInDgnFile (LoadPerFileCacheCallbackObj (perFileCache, GetProviderName(), true), dgnFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           ECXDProvider::EnsureClassLayoutIsStored (ECClassCR ecClass, ClassLayoutCR classLayout, DgnProjectR dgnFile)
    {
    SchemaLayoutElementHandler& schemaLayoutHandler = SchemaLayoutElementHandler::GetInstance();

    // Special case: The classLayouts used to store class layout information itself
    //               do not need to be cached or persisted.
    SchemaClassIndexPair index;
    if (schemaLayoutHandler.IsInternalECClass (index, ecClass))
        return SUCCESS;

    // Ensure that the schema is in the cache
    ECXDPerFileCacheR perFileCache = GetPerFileCache (dgnFile);

    // WIP_FUSION: What to do with this?   ECXDPerClassCache& perClassCache = perFileCache.ObtainPerClassCache (ecClass); assume it is there, see if it is persisted
    ECSchemaCR        ecSchema = ecClass.GetSchema();

    ECXDPerSchemaCache * perSchemaCache = dynamic_cast<ECXDPerSchemaCacheP>(perFileCache.FindPerSchemaCacheByName (ecSchema.GetName().c_str()));

    if (NULL == perSchemaCache)
        { DEBUG_EXPECT (false); return ERROR; }

    ECXDPerClassCacheP   perClassCache = perSchemaCache->FindPerClassCacheByIndex (classLayout.GetClassIndex());

    if (NULL == perClassCache)
        { DEBUG_EXPECT (false); return ERROR; }

    ClassLayoutCP       cachedClassLayout = perClassCache->m_classLayout;
    
    if (NULL == cachedClassLayout)
        { DEBUG_EXPECT (false); return ERROR; }

    // Ensure that ClassLayout is the same one from the cache
    if (&classLayout != cachedClassLayout)
        { DEBUG_EXPECT (false); return ERROR; }

    // If the cache says the classLayout is persistent, nothing to do
    if (perClassCache->IsClassLayoutPersisted())
        return SUCCESS;

    if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        {
        DgnECManager::GetManager().GetLogger().debugv (L"Writing ClassLayout %ls:%ls (schemaIndex=%d, classIndex=%d) to file %ls.", 
            ecSchema.GetName().c_str(), classLayout.GetECClassName().c_str(), classLayout.GetSchemaIndex(), classLayout.GetClassIndex(), dgnFile.GetFileName().c_str());
        }
    if (SUCCESS != schemaLayoutHandler.WriteClassLayout (classLayout, ecSchema.GetName().c_str(), dgnFile))
        return ERROR;

    perClassCache->SetIsClassLayoutPersisted(true);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDProvider::OnUndoRedoClassLayout (SchemaIndex schemaIndex, ClassIndex classIndex, DgnProjectR dgnFile, bool isUndo) const
    {
    ECXDPerFileCacheR    perFileCache   = ECXDProvider::GetProvider().GetPerFileCache(dgnFile);
    ECXDPerSchemaCacheP  perSchemaCache = perFileCache.FindPerSchemaCacheByIndex (schemaIndex);

    if ( ! EXPECTED_CONDITION (NULL != perSchemaCache))
        return;

    if (isUndo)
        {
        ECXDPerClassCacheP   perClassCache  = perSchemaCache->FindPerClassCacheByIndex (classIndex);

        if ( ! EXPECTED_CONDITION (NULL != perClassCache))
            return;

        perClassCache->SetIsClassLayoutPersisted (false);
        }
    else
        {
        ECXDPerClassCacheP   perClassCache  = perSchemaCache->PeekPerClassCacheByIndex (classIndex);

        if (NULL == perClassCache)
            {
            SchemaLayoutP   schemaLayout = perSchemaCache->PeekSchemaLayout();

            // If the schema layout is not loaded we don't need to force it.
            // The reinstated classLayout will be found when the schema layout is lazy-loaded.
            if (NULL == schemaLayout)
                return;

            // If the schema layout *is* loaded, then the classLayouts are not going
            // to be lazy loaded.  The loading of the schemaLayout always loads all
            // the class layouts.  So we need to reload the individual class layout here.
            ClassLayoutP    classLayout = SchemaLayoutElementHandler::GetInstance().ReadClassLayout (schemaIndex, classIndex, dgnFile);

            if ( ! EXPECTED_CONDITION (NULL != classLayout))
                return;

            schemaLayout->AddClassLayout (*classLayout, classIndex);

            // It adds itself to its parent ECXDPerSchemaCache
            perClassCache = new ECXDPerClassCache (*perSchemaCache, *classLayout);
            }
        
        perClassCache->SetIsClassLayoutPersisted (true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDProvider::OnUndoRedoSchemaLayout (WCharCP schemaName, SchemaIndex schemaIndex, ElementRefP elemRef, bool isUndo) const
    {
    DgnModelP           dgnModel        = elemRef->GetDgnModelP();
    DgnProjectP            dgnFile         = dgnModel->GetDgnProject();
    ECXDPerFileCacheR    perFileCache    = ECXDProvider::GetProvider().GetPerFileCache(*dgnFile);

    if (isUndo)
        {
        ECXDPerSchemaCacheP  perSchemaCache  = perFileCache.FindPerSchemaCacheByIndex (schemaIndex);

        if ( ! EXPECTED_CONDITION (NULL != perSchemaCache))
            return;

        perSchemaCache->ClearSchemaLayout (schemaIndex);
        }
    else
        {
        ECXDPerSchemaCacheP  perSchemaCache  = dynamic_cast<ECXDPerSchemaCacheP> (perFileCache.FindPerSchemaCacheByName (schemaName));

        if ( ! EXPECTED_CONDITION (NULL != perSchemaCache))
            return;

        perSchemaCache->SetSchemaIndex (schemaIndex, elemRef);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void                            ECXDProvider::_OnUnregistered ()
    {
    ECXDProvider* provider = static_cast<ECXDProvider*> (T_HOST.GetHostObject (GetHostKey()));
    if (NULL == provider)
        return;
    
    T_HOST.SetHostObject (GetHostKey(), NULL);
    delete provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ProviderPerFileCacheR ECXDProvider::_GetPerFileCache (DgnProjectR dgnFile) const
    {
    DgnECPerFileCache&  managerPerFileCache =  DgnECManager::GetManager().GetPerFileCache (dgnFile);
    return *(ProviderPerFileCacheP)managerPerFileCache.GetProviderPerFileCache(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerFileCacheR        ECXDProvider::GetPerFileCache (DgnProjectR dgnFile) const
    {
    DgnECManagerR       manager           = DgnECManager::GetManager();
    DgnECPerFileCache&  managerPerFileCache = manager.GetPerFileCache (dgnFile);

    return *(ECXDPerFileCacheP)managerPerFileCache.GetProviderPerFileCache(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceEnablerP   ECXDProvider::_ObtainInstanceEnablerByName (WCharCP schemaName, WCharCP className, DgnProjectR dgnFile, void* providerPerFileCache)
    {
    SchemaClassIndexPair        internalIndex;
    SchemaLayoutElementHandler& handler = SchemaLayoutElementHandler::GetInstance();

    if (handler.IsInternalECClass(internalIndex, schemaName, className))
        return handler.GetInstanceEnabler(internalIndex);

    ECXDPerFileCacheR   perFileCache  = *(ECXDPerFileCache*)providerPerFileCache;
    DEBUG_EXPECT(&perFileCache.GetDgnFile() == &dgnFile);

    return ObtainInstanceEnablerByName (schemaName, className, perFileCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerP     ECXDProvider::ObtainInstanceEnablerByName (WCharCP schemaName, WCharCP className, ECXDPerFileCacheR perFileCache) const
    {
    SchemaClassIndexPair        internalIndex;
    SchemaLayoutElementHandler& handler = SchemaLayoutElementHandler::GetInstance();

    if (handler.IsInternalECClass(internalIndex, schemaName, className))
        return handler.GetInstanceEnabler(internalIndex);

    ECXDPerClassCacheP perClassCache = perFileCache.ObtainPerClassCacheByName (schemaName, className);
    if (NULL == perClassCache)
       {
#ifdef NOT_YET
        FOR_EACH (PerSchemaCachePtrMap::value_type pair , m_perSchemaCachesByName)
            {
            ECPerSchemaCacheP perSchemaCache = pair.second.get();
            ECN::ECSchemaP schemaP = perSchemaCache->GetSchemaPtr();
            if (schemaP)
                {
                FOR_EACH (ECSchemaP refSchema , schemaP->GetReferencedSchemas())
                    {
                    if (refSchema->GetName().Equals (schemaName))
                        {
                        SchemaInfo schemaInfo (refSchema->GetSchemaKey(), perFileCache.GetDgnFile(), m_provider->GetProviderName(), L"ReferenceSchema");
                        
                        ECXDPerSchemaCacheP perSchemaCache = new ECXDPerSchemaCache (*this, schemaInfo, NULL);
                        perSchemaCache->SetSchemaPAndAddRefInSharedSchemaCache (refSchema);

                        AddPerSchemaCache (*perSchemaCache);

                        return perSchemaCache;
                        }
                    }
                }

            }
#endif

        return NULL;
        }

    return perClassCache->ObtainInstanceEnabler ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerP     ECXDProvider::ObtainInstanceEnabler (ECN::ECClassCR ecClass, ECXDPerFileCacheR perFileCache) const
    {
    SchemaClassIndexPair        internalIndex;
    SchemaLayoutElementHandler& handler = SchemaLayoutElementHandler::GetInstance();

    if (handler.IsInternalECClass(internalIndex, ecClass))
        return handler.GetInstanceEnabler(internalIndex);

    ECXDPerClassCacheP perClassCache = perFileCache.ObtainPerClassCache (ecClass);
    if (NULL == perClassCache)
        return NULL;
        
    return perClassCache->ObtainInstanceEnabler (); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECRelationshipEnablerP       ECXDProvider::_ObtainDgnECRelationshipEnabler (WCharCP schemaName, WCharCP className, DgnProjectR dgnFile)
    {
    ECXDPerFileCacheR perFileCache = GetPerFileCache (dgnFile);

    ECXDPerClassCacheP perClassCache = perFileCache.ObtainPerClassCacheByName (schemaName, className);
    if (NULL == perClassCache)
        return NULL;

    return perClassCache->ObtainRelationshipEnabler ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationshipEnablerP ECXDProvider::ObtainRelationshipEnabler (ECN::ECRelationshipClassCR ecRelationshipClass, ECXDPerFileCacheR perFileCache) const
    {
    ECXDPerClassCacheP perClassCache = perFileCache.ObtainPerClassCache (ecRelationshipClass);
    if (NULL == perClassCache)
        return NULL;
        
    return perClassCache->ObtainRelationshipEnabler (); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationshipEnablerP ECXDProvider::ObtainRelationshipEnabler (ECN::ECRelationshipClassCR ecRelationshipClass, DgnProjectR file) const
    {
    return ObtainRelationshipEnabler (ecRelationshipClass, GetPerFileCache(file));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerP     ECXDProvider::ObtainInstanceEnabler (SchemaClassIndexPair indexPair, ECXDPerFileCacheR perFileCache) const
    {
    ECXDInstanceEnablerP enabler = SchemaLayoutElementHandler::GetInstance().GetInstanceEnabler(indexPair);

    if (NULL != enabler)
        return enabler;

    SchemaIndex schemaIndex = indexPair.first;
    ECXDPerSchemaCacheP perSchemaCache = perFileCache.FindPerSchemaCacheByIndex (schemaIndex);
    if (NULL == perSchemaCache)
        return NULL;

    ClassIndex classIndex = indexPair.second;
    ECXDPerClassCacheP  perClassCache = perSchemaCache->FindPerClassCacheByIndex (classIndex);
    if (NULL == perClassCache)
        return NULL;

    return perClassCache->ObtainInstanceEnabler ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationshipEnablerP ECXDProvider::ObtainRelationshipEnabler (SchemaClassIndexPair indexPair, ECXDPerFileCacheR perFileCache) const
    {
    SchemaIndex schemaIndex = indexPair.first;
    ECXDPerSchemaCacheP perSchemaCache = perFileCache.FindPerSchemaCacheByIndex (schemaIndex);
    if (NULL == perSchemaCache)
        return NULL;

    ClassIndex classIndex = indexPair.second;
    ECXDPerClassCacheP  perClassCache = perSchemaCache->FindPerClassCacheByIndex (classIndex);
    if (NULL == perClassCache)
        return NULL;

    return perClassCache->ObtainRelationshipEnabler ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/     
ECXDInstanceEnabler::ECXDInstanceEnabler(ECClassCR ecClass, ClassLayoutCR classLayout, ECXDPerFileCacheR perFileCache) :
    ClassLayoutHolder (classLayout), 
    DgnElementECInstanceEnabler (ecClass, PROVIDERID_ECXData, &perFileCache),
    m_sharedWipInstance (InitializeSharedWipInstance(&perFileCache))
    {
    m_perSchemaCache = perFileCache.FindPerSchemaCacheByName (ecClass.GetSchema().GetName().c_str());
    };

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/     
ECXDInstanceEnabler::ECXDInstanceEnabler(ECClassCR ecClass, ClassLayoutCR classLayout, ECN::IStandaloneEnablerLocaterP standaloneInstanceEnablerLocater) :
    ClassLayoutHolder (classLayout), 
    DgnElementECInstanceEnabler (ecClass, PROVIDERID_ECXData, standaloneInstanceEnablerLocater),
    m_sharedWipInstance (InitializeSharedWipInstance(standaloneInstanceEnablerLocater))
    {
    m_perSchemaCache = NULL;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/     
ECXDInstanceEnabler::~ECXDInstanceEnabler()
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::StandaloneECEnablerR    ECXDInstanceEnabler::_GetStandaloneECInstanceEnabler () const 
    {
    return *m_sharedWipEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::StandaloneECEnablerR    ECXDInstanceEnabler::GetStandaloneInstanceEnabler () const 
    {
    return *m_sharedWipEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const*          ECXDInstanceEnabler::_GetName() const
    {
    return L"Bentley::ECN::ECXDInstanceEnabler";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXDInstanceEnabler::_GetPropertyIndex(UInt32& propIdx, const wchar_t * accessStr) const { return GetClassLayout().GetPropertyIndex (propIdx, accessStr); }
ECObjectsStatus ECXDInstanceEnabler::_GetAccessString(wchar_t const *& accessStr, UInt32 propIdx)  const { return GetClassLayout().GetAccessStringByIndex (accessStr, propIdx); }
UInt32          ECXDInstanceEnabler::_GetPropertyCount()                                           const { return GetClassLayout().GetPropertyCount (); }
UInt32          ECXDInstanceEnabler::_GetFirstPropertyIndex (UInt32 parentIndex)                   const { return GetClassLayout().GetFirstChildPropertyIndex (parentIndex); }
UInt32          ECXDInstanceEnabler::_GetNextPropertyIndex (UInt32 parentIndex, UInt32 inputIndex) const { return GetClassLayout().GetNextChildPropertyIndex (parentIndex, inputIndex);  }
ECObjectsStatus ECXDInstanceEnabler::_GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const { return GetClassLayout().GetPropertyIndices (indices, parentIndex);  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECXDInstanceEnabler::_HasChildProperties (UInt32 parentIndex) const
    {
    PropertyLayoutCP propertyLayout;
    return ECOBJECTS_STATUS_Success == GetClassLayout().GetPropertyLayoutByIndex (propertyLayout, parentIndex) && propertyLayout->GetTypeDescriptor().IsStruct();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECInstancePtr ECXDInstanceEnabler::InitializeSharedWipInstance (IStandaloneEnablerLocaterP standaloneInstanceEnablerLocater)
    {
    if (!m_sharedWipEnabler.IsValid())
        {
        m_sharedWipEnabler = StandaloneECEnabler::CreateEnabler(GetClass(), GetClassLayout(), standaloneInstanceEnablerLocater, false);
        }

    return m_sharedWipEnabler->CreateInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECInstanceP   ECXDInstanceEnabler::_GetSharedWipInstance() const
    {
    return m_sharedWipInstance.get();
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECInstancePtr ECXDInstanceEnabler::_GetPrivateWipInstance() const
    {
    // we can still use the shared wip enabler because technically it is the same enabler to create a private wip
    return m_sharedWipEnabler->CreateInstance();
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerPtr   ECXDInstanceEnabler::CreateEnabler(ECClassCR ecClass, ClassLayoutCR classLayout, ECXDPerFileCacheR perFileCache)
    {
    return new ECXDInstanceEnabler (ecClass, classLayout, perFileCache);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerPtr   ECXDInstanceEnabler::CreateInternalEnabler(ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterP standaloneInstanceEnablerLocater)
    {
    return new ECXDInstanceEnabler (ecClass, classLayout, standaloneInstanceEnablerLocater);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECXDInstanceEnabler::_IsEnablerValidForDgnFile (DgnProjectR dgnFile) const
    {
    if (NULL == m_perSchemaCache)
        return true;

    return (&m_perSchemaCache->GetPerFileCache().GetDgnFile () == &dgnFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/  
DgnECInstanceStatus               ECXDInstanceEnabler::CreateECXDataOnElement (DgnElementECInstancePtr* dgnecInstance, StandaloneECInstanceR wipInstance, DgnModelR modelRef, ElementRefP elementRef, bool createAsStructValue) const
    {
    PRECONDITION (NULL != elementRef, DGNECINSTANCESTATUS_InvalidElementRef)

    if (!IsEnablerValidForDgnFile (*elementRef->GetDgnModelP()->GetDgnProject()))
        return DGNECINSTANCESTATUS_EnablerNotValidForFile;

    if (&wipInstance.GetClassLayout() != &GetClassLayout())
        {
        if (!wipInstance.GetClassLayout().IsCompatible(GetClassLayout())) // see if compatible but different ClassLayouts, e.g. from different files.
            return DGNECINSTANCESTATUS_IncompatibleWipInstance;
        }

    if (SUCCESS != ECXDProvider::GetProvider().EnsureClassLayoutIsStored (GetClass(), GetClassLayout(), *elementRef->GetDgnModelP()->GetDgnProject()))
        return DGNECINSTANCESTATUS_Error;

    byte const * data = wipInstance.GetData();
    UInt32       size = wipInstance.GetBytesUsed();
       
    UInt32 id = XAttributeHandle::INVALID_XATTR_ID;
    StatusInt status;
    if (createAsStructValue)
        status = ECXDStructValueXAttributeHandler::GetHandler().CreateXAttribute (id, elementRef, data, size);
    else
        status = ECXDInstanceXAttributeHandler::GetHandler().CreateXAttribute (id, elementRef, data, size);
        
    if (SUCCESS != status)
        return DGNECINSTANCESTATUS_UnableToAddInstance;
        
    if (XAttributeHandle::INVALID_XATTR_ID == id)
        return DGNECINSTANCESTATUS_UnableToAddInstance;

    if (dgnecInstance)
        {
        if (createAsStructValue)
            *dgnecInstance = CreateStructValue (modelRef, elementRef, id).get();
        else
            *dgnecInstance = CreateInstance (modelRef, elementRef, id).get();
        }

    return DGNECINSTANCESTATUS_Success;    
    }        
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/  
DgnECInstanceStatus              ECXDInstanceEnabler::_CreateInstanceOnElement (DgnElementECInstancePtr* dgnecInstance, StandaloneECInstanceR wipInstance, DgnModelR modelRef, ElementRefP elementRef) const
    {
    return CreateECXDataOnElement (dgnecInstance, wipInstance, modelRef, elementRef, false);
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/  
bool                    ECXDInstanceEnabler::_SupportsCreateInstanceOnElement () const
    {
    return true;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/   
DgnECInstanceStatus ECXDInstanceEnabler::_CreateStructValueOnElement(DgnElementECInstancePtr* dgnecInstance, StandaloneECInstanceR wipInstance, DgnModelR modelRef, ElementRefP elementRef) const
    {
    return CreateECXDataOnElement (dgnecInstance, wipInstance, modelRef, elementRef, true);
    }      
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/  
bool                    ECXDInstanceEnabler::_SupportsCreateStructValueOnElement() const
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstancePtr          ECXDInstanceEnabler::CreateInstance (DgnModelR modelRef, ElementRefP elementRef, UInt32 xAttrId) const
    {
    return new ECXDInstance (*this, modelRef, elementRef, xAttrId);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstancePtr          ECXDInstanceEnabler::CreateStructValue (DgnModelR modelRef, ElementRefP elementRef, UInt32 xAttrId) const
    {    
    return new ECXDStructValue (*this, modelRef, elementRef, xAttrId);    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/10
+---------------+---------------+---------------+---------------+---------------+------*/    
IDgnECInstanceFinderPtr         ECXDProvider::_CreateFinder (DgnProjectR dgnFile, ECQueryCR query, void * providerPerFileCache) const  
    {
    ECXDPerFileCacheR    perFileCache = *((ECXDPerFileCacheP)providerPerFileCache);
    if (perFileCache.m_perSchemaCachesByIndex.size() == 0) 
        return NULL; // We have no data in the file, don't bother looking

    if (query.IncludesExtrinsic())
        return ECXDInstanceFinder::CreateFinder(perFileCache, query.GetWhereCriterionPtr(), query.GetPropertyValuePreFilter().get()).get();
    else if (!query.IsFilteredByClassList())
        return NULL;

    ECXDInstanceFilteredFinderPtr ecxInstanceFinder;
    
    FOR_EACH(ECClassCP ecClass , *query.LookupSearchClasses (dgnFile))
        {
        ECXDInstanceEnablerP instanceEnabler = NULL;
        ECRelationshipClassCP ecRelationshipClass = ecClass->GetRelationshipClassCP();
        if (NULL != ecRelationshipClass)
            instanceEnabler = ObtainRelationshipEnabler (*ecRelationshipClass, perFileCache);
        else
            instanceEnabler = ObtainInstanceEnabler (*ecClass, perFileCache);

        if (NULL == instanceEnabler)
            continue;

        if (ecxInstanceFinder.IsNull())
            ecxInstanceFinder  = ECXDInstanceFilteredFinder::CreateFinder (query.GetPropertyValuePreFilter().get()); // just-in-time construction in case we don't need one at all

        ecxInstanceFinder->AddEnablerAndQuery (*instanceEnabler, &query);
        }

    return ecxInstanceFinder.get();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaClassIndexPair    ECXDProvider::GetTypeIndiciesFromECXData (const void * ecxData) const
    {
    InstanceHeader const& header = MemoryInstanceSupport::PeekInstanceHeader (ecxData);

    SchemaClassIndexPair idPair (header.m_schemaIndex, header.m_classIndex);
    return idPair;
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
DgnElementECInstancePtr        ECXDProvider::LoadInstance (DgnModelR modelRef, ElementRefP elementRef, XAttributeHandle xa, bool loadStructValue) const
    {        
    SchemaClassIndexPair    idPair       = GetTypeIndiciesFromECXData (xa.PeekData());
    ECXDPerFileCacheR        perFileCache = GetPerFileCache (*modelRef.GetDgnProject());
    ECXDInstanceEnablerCP    enabler      = ObtainInstanceEnabler (idPair, perFileCache);
    
    if (enabler == NULL)
        return NULL;

    if (loadStructValue)
        return enabler->CreateStructValue (modelRef, elementRef, xa.GetId()).get();
    else
        return enabler->CreateInstance (modelRef, elementRef, xa.GetId()).get();    
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/10
+---------------+---------------+---------------+---------------+---------------+------*/    
ECXDRelationshipPtr      ECXDProvider::LoadRelationshipInstance (DgnModelR modelRef, ElementRefP elementRef, XAttributeHandle xa) const
    {        
    SchemaClassIndexPair     idPair       = GetTypeIndiciesFromECXData (xa.PeekData());
    ECXDPerFileCacheR         perFileCache = GetPerFileCache (*modelRef.GetDgnProject());
    ECXDRelationshipEnablerCP enabler      = ObtainRelationshipEnabler (idPair, perFileCache);
    
    if (enabler == NULL)
        return NULL;

    return enabler->CreateECXDRelationshipInstance (modelRef, elementRef, xa.GetId());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/   
DgnElementECInstancePtr        ECXDProvider::_LoadInstance (DgnModelR modelRef, ElementRefP elementRef, UInt32 localId, bool loadProperties) const
    {
    // WIP: determine if ECXD needs to support option not to loadProperties

    XAttributeHandle xa (elementRef, ECXDInstanceXAttributeHandler::GetHandler().GetId(), localId);
    if (!xa.IsValid())
        return NULL;
        
    return LoadInstance (modelRef, elementRef, xa, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/  
DgnECInstancePtr               ECXDProvider::_LocateInstance (IDgnECInstanceLocatorCR locator, bool loadProperties) const
    {
    DgnModelP   modelRef   = locator.GetDgnModel();
    if (NULL != modelRef)
        {
        ElementRefP elementRef = locator.GetElementRef();
        XAttributeHandle xa (elementRef, ECXDInstanceXAttributeHandler::GetHandler().GetId(), locator.GetLocalId());
        if (xa.IsValid())
            return LoadInstance (*modelRef, elementRef, xa, loadProperties);
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
DgnElementECInstancePtr        ECXDProvider::_LoadStructValue (DgnModelR modelRef, ElementRefP elementRef, UInt32 localId) const
    {
    XAttributeHandle xa (elementRef, ECXDStructValueXAttributeHandler::GetHandler().GetId(), localId);
    if (!xa.IsValid())
        return NULL;
        
    return LoadInstance (modelRef, elementRef, xa, true);    
    }

/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      03/2010
+===============+===============+===============+===============+===============+======*/
struct          DetectECXDPropertyExpression : WhereCriterion::IExpressionBinder
{
    bool        m_any;

    DetectECXDPropertyExpression () : m_any(false) {;}

    virtual void _BindExpression (RefCountedPtr<WhereExpression>& x) override 
        {
        if (!m_any) 
            m_any = (NULL != dynamic_cast<WhereCriterion::PropertyExpression*>(x.get()));
        }
}; // DetectECXDPropertyExpression

/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      03/2010
+===============+===============+===============+===============+===============+======*/
struct          BindECXDPropertyExpressions : WhereCriterion::IExpressionBinder
{
  private:
    struct      ECXDPropertyExpression : WhereExpression
        {
        friend struct  BindECXDPropertyExpressions;

        PropertyLayout                                 m_propertyLayout;
        WhereCriterion::PropertyExpression::ArrayIndex m_array;

        ECXDPropertyExpression (PropertyLayout const& pl, WhereCriterion::PropertyExpression::ArrayIndex const& ai) 
            : m_propertyLayout (pl), m_array(ai) 
            {;}

        virtual StatusInt _Evaluate (ECN::ECValue& v, DgnECInstanceCR ecinst) const override
            {
            ECXDInstance const* ecxinst = dynamic_cast<ECXDInstance const*> (&ecinst);
            return ecxinst->GetValueFromPropertyLayout (v, m_propertyLayout, m_array.m_useIndex, m_array.m_index);
            }

        virtual WhereExpressionPtr _DeepCopy () const override {return new ECXDPropertyExpression(*this);}
        virtual ECTypeDescriptor            _GetType ()  const override {return m_propertyLayout.GetTypeDescriptor();}

        virtual bool _TestIn (DgnECInstanceCR, ArrayInfo const&, ECValue const& v) const override;

        }; // ECXDPropertyExpression

  private:
    ECXDInstanceEnabler const& m_enabler;

    virtual void _BindExpression (WhereExpressionPtr& x) override 
        {
        WhereCriterion::PropertyExpression* pvx = dynamic_cast<WhereCriterion::PropertyExpression*>(x.get());
        if (NULL == pvx)
            return;

        PropertyLayoutCP propertyLayout = NULL;
        if (m_enabler.GetClassLayout().GetPropertyLayout (propertyLayout, pvx->GetAccessString()) != SUCCESS)
            return;

        x = new ECXDPropertyExpression (*propertyLayout, pvx->GetArrayIndex()); 
        }

  public:
    BindECXDPropertyExpressions (ECXDInstanceEnabler const& e) : m_enabler(e) {;}

}; // BindECXDPropertyExpressions

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool BindECXDPropertyExpressions::ECXDPropertyExpression::_TestIn (DgnECInstanceCR ecinstance, ArrayInfo const& arrayInfo, ECValue const& lhsval) const
    {
    DEBUG_EXPECT (arrayInfo.IsPrimitiveArray());

    //  To speed this up for fixed-type, homogenous arrays, promote the lhs to the array element type up front.
    ECValue lhsvalPromoted = WhereCriterion::PromoteValueToPrimitiveType (lhsval, arrayInfo.GetElementPrimitiveType());
    if (lhsvalPromoted.IsNull() || lhsvalPromoted.IsUninitialized())
        lhsvalPromoted = lhsval;            // maybe this is a heterogenous array. We'll have to check types and promote on each entry.

    ECXDInstance const* ecxinst = dynamic_cast<ECXDInstance const*> (&ecinstance);

    // DEFERRED_FUSION: Need a more efficient way to test for a value in an array
    for (UInt32 i=0, count=arrayInfo.GetCount(); i<count; ++i)
        {
        ECValue arrayitem;
        if (ecxinst->GetValueFromPropertyLayout (arrayitem, m_propertyLayout, true, i) == SUCCESS)
            {
            if (WhereCriterion::ComparePrimitiveValues (lhsvalPromoted, WhereCriterion::EQ, arrayitem))
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ECXDInstanceEnabler::BindWhereCriterion (WhereCriterionPtr& wh) const
    {
    DetectECXDPropertyExpression detect;
    wh->BindExpressions (detect);
    if (!detect.m_any)
        return;

    WhereCriterionPtr cc = wh->DeepCopy ();

    BindECXDPropertyExpressions binder (*this);
    cc->BindExpressions (binder);

    wh = cc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ECXDInstanceFilteredFinder::AddEnablerAndQuery (ECXDInstanceEnablerCR enabler, ECQueryCP query) 
    {
    ClassLayoutCR   classLayout  = enabler.GetClassLayout();

    SchemaClassIndexPair idPair(classLayout.GetSchemaIndex(), classLayout.GetClassIndex());

    ClassToEnablerAndWhereCriterionMap::const_iterator mapIter = m_enablerMap.find (idPair);
    if (mapIter != m_enablerMap.end())
        {
        // Something is wrong if we try to identify a different enabler with the same class.
        DEBUG_EXPECT ((*mapIter).second.m_enabler == &enabler);
        }

    WhereCriterionPtr wh = query? query->GetWhereCriterionPtr(): NULL;
    if (wh != NULL)
        enabler.BindWhereCriterion (wh);

    m_enablerMap[idPair] = EnablerAndWhereCriterion (&enabler, wh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/10
+---------------+---------------+---------------+---------------+---------------+------*/    
ECXDInstanceFinderBase::EnablerAndWhereCriterion ECXDInstanceFinder::_GetEnablerAndWhereCriterionForIndices (SchemaClassIndexPair idPair) const 
    {
    ECXDInstanceEnablerP enabler = ECXDProvider::GetProvider().ObtainInstanceEnabler (idPair, m_perFileCache);
    DEBUG_EXPECT (NULL != enabler);

    return EnablerAndWhereCriterion ((ECXDInstanceEnablerP)enabler, m_where);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/10
+---------------+---------------+---------------+---------------+---------------+------*/    
ECXDInstanceFinderBase::EnablerAndWhereCriterion ECXDInstanceFilteredFinder::_GetEnablerAndWhereCriterionForIndices (SchemaClassIndexPair idPair) const 
    {
    ClassToEnablerAndWhereCriterionMap::const_iterator mapIter = m_enablerMap.find (idPair);

    if (mapIter == m_enablerMap.end())
        return EnablerAndWhereCriterion (NULL, NULL);  // we are not interested in this ECInstance

    DEBUG_EXPECT (NULL != (*mapIter).second.m_enabler);

    return (*mapIter).second;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt ECXDInstanceFinderBase::_FindInstances (DgnElementECInstanceVector& results, ElementHandleCR eh, DgnModelP pepDependentModel, WCharCP hostPepString) const 
    {
    size_t count=0;
    for (ElementHandle::XAttributeIter xa(eh); xa.IsValid(); xa.ToNext())
        {
        if (ECXDInstanceXAttributeHandler::GetHandler().GetId()     != xa.GetHandlerId() &&
            ECXDRelationshipXAttributeHandler::GetHandler().GetId() != xa.GetHandlerId())
            continue;

        ++count;
    
        SchemaClassIndexPair idPair = ECXDProvider::GetProvider().GetTypeIndiciesFromECXData (xa.PeekData());
        EnablerAndWhereCriterion ew = _GetEnablerAndWhereCriterionForIndices (idPair);
        
        if (ew.m_enabler == NULL)
            continue;  // This ECInstance is not what we are looking for
                
        DgnElementECInstancePtr ecxInstance = ew.m_enabler->CreateInstance (*pepDependentModel, eh.GetElementRef(), xa.GetId());
        if (hostPepString)
            ecxInstance->SetHostPepString (hostPepString);

        if (!InstanceSupportsValueFilter(*ecxInstance))
            continue;

        if (ew.m_where != NULL && !ew.m_where->Accept (*ecxInstance))
            continue;

        results.push_back (ecxInstance);
        }
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isRelationshipAlwaysStoredOnEnd (QueryRelatedClassSpecifier const&) {return false;} // WIP_FUSION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32   getSourceUpperLimit (ECRelationshipClassCR relClass, ECRelatedInstanceDirection dir)
    {
    if (dir == ECN::ECRelatedInstanceDirection::Forward)
        return relClass.GetSource().GetCardinality().GetUpperLimit();
    return relClass.GetTarget().GetCardinality().GetUpperLimit();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDProvider::FindRelatedInstancesOnElement
(
IDgnECRelationshipInstanceVector*   relationships, 
DgnElementECInstanceVector*                endPoints, 
DgnElementECInstanceCR                     sourceInstance,
QueryRelatedClassSpecifier const&   classSpecifier,
ElementRefP                         hostElement, 
ECN::ECRelationshipEnd               sourceEnd, 
ECN::ECRelationshipEnd               targetEnd
)   const
    {
    DgnModelR hostModel = *hostElement->GetDgnModelP();

    XAttributeHandlerId xAttrId = ECXDRelationshipXAttributeHandler::GetHandler().GetId();

    FOR_EACH (XAttributeCollection::Entry const& xa, XAttributeCollection(hostElement, xAttrId))
        {
        // Is it a relationship instance?
        ECXDRelationshipPtr foundRelationship = LoadRelationshipInstance (hostModel, hostElement, xa);
        if (foundRelationship == NULL)
            continue;

        //  Is it an instance of the relationship class that we're looking for?
        if (!classSpecifier.Accept (foundRelationship->GetRelationshipClass()))
            continue;

        //  sourceInstance must be at the desired end of the relationship
        //      We use a temporary ECXDRelationship instance on the stack to access the stored DgnECPointer data.
        //      We do this in order to avoid looking up the XAttribute, which we already have.
        //      Calling  foundRelationship->GetRelatedInstance would give the same result, but it would have to
        //      acquire the XAttribute first.
        XAttributeHandle xaHandle (xa);
        ECXDRelationship tempRel (foundRelationship->GetRelationshipEnabler(), hostModel, hostElement, xaHandle);
        DgnElementECInstancePtr relationshipSourceInstance = tempRel.GetRelatedInstance (sourceEnd);

        if (relationshipSourceInstance == NULL)
            continue;

        if (!relationshipSourceInstance->IsFromSameXAttribute (sourceInstance))
            continue;

        DgnElementECInstancePtr endPoint = tempRel.GetRelatedInstance (targetEnd);

        if (endPoint == NULL || !classSpecifier.Accept (endPoint->GetClass()))
            continue;
    
        // This relationship instance and related instance meet the criteria. 
        if (relationships != NULL)
            relationships->push_back (foundRelationship.get());

        if (endPoints != NULL)
            endPoints->push_back (endPoint);

        //  If there can only be one instance on the sourceEnd of this relationship, then we have found it. Stop looking.
        if (1 == getSourceUpperLimit (foundRelationship->GetRelationshipClass(), classSpecifier.GetDirection()))
            break;
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ECXDProvider::_FindRelatedInstances 
(
IDgnECRelationshipInstanceVector*   relationships, 
DgnElementECInstanceVector*                endPoints, 
DgnElementECInstanceCR                     sourceInstance, 
QueryRelatedClassSpecifier const&   classSpecifier,
bool                                useRecursion   // not yet supported
)
    {
    ECN::ECRelationshipEnd sourceEnd = ECN::ECRelationshipEnd_Source;
    ECN::ECRelationshipEnd targetEnd = ECN::ECRelationshipEnd_Target;
    if (classSpecifier.GetDirection() == ECN::ECRelatedInstanceDirection::Backward)
        std::swap (sourceEnd, targetEnd);        

    if (isRelationshipAlwaysStoredOnEnd (classSpecifier))
        FindRelatedInstancesOnElement (relationships, endPoints, sourceInstance, classSpecifier, sourceInstance.GetElementRef(), sourceEnd, targetEnd);
    else
        {
#ifdef DGNPROJECT_MODELS_CHANGES_WIP
        T_StdElementRefVector processedElementRefs;
        for (DependentElm* dep = sourceInstance.GetElementRef()->GetFirstDependent (); dep != NULL; dep = dep->GetNext())
            {
            ElementRefP elementRefP = dep->GetElementRef();

            if (processedElementRefs.end() == std::find (processedElementRefs.begin(), processedElementRefs.end(), elementRefP))
                {
                processedElementRefs.push_back (elementRefP); // make sure the same element is not processed more than once
                FindRelatedInstancesOnElement (relationships, endPoints, sourceInstance, classSpecifier, dep->GetElementRef(), sourceEnd, targetEnd);
                }
            }
#endif
        }
    }

#if defined (EXPERIMENTAL_TEXT_FILTER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnabler::PropertyProcessingResult ECXDInstanceEnabler::_ProcessPrimitiveProperties (bset<ECClassCP>& failedClasses, IECInstanceCR instance, ECN::PrimitiveType primitiveType, IPropertyProcessor const& proc, PropertyProcessingOptions opts) const
    {
    if (failedClasses.find (&instance.GetClass()) != failedClasses.end())
        return PROPERTY_PROCESSING_RESULT_NoCandidates;

    ClassLayoutCR classLayout = GetClassLayout();

    ECXDInstance const* ecxinstance = dynamic_cast<const ECXDInstance*> (&instance);

    bool noCandidateInAnyStruct = true;
    bool anyCandidates = false;
    bool allTypes = (opts & PROPERTY_PROCESSING_OPTIONS_AllTypes) != 0;

    for (UInt32 propertyIndex = 0;  propertyIndex < classLayout.GetPropertyCount();  ++propertyIndex)
        {
        PropertyLayoutCP propertyLayout = NULL;
        classLayout.GetPropertyLayoutByIndex (propertyLayout, propertyIndex);
        ECTypeDescriptor ect = propertyLayout->GetTypeDescriptor  ();
    
        if (ect.IsPrimitive())
            {
            if (allTypes || ect.GetPrimitiveType() == primitiveType)
                {
                anyCandidates = true;
                
                ECValue v;
                ecxinstance->GetValueFromPropertyLayout (v, *propertyLayout, false, 0);
                
                if (v.IsNull())
                    continue;

                if (proc._ProcessPrimitiveProperty (instance, propertyLayout->GetAccessString(), v))
/*<==*/             return PROPERTY_PROCESSING_RESULT_Hit;
                }
            }
        else if (ect.IsStruct ())
            {
            ECValue v;
            ecxinstance->GetValueFromPropertyLayout (v, *propertyLayout, false, 0);

            if (v.IsNull())
                continue;

            if (ProcessStructProperty (failedClasses, noCandidateInAnyStruct, v, primitiveType, proc, opts))
/*<==*/         return PROPERTY_PROCESSING_RESULT_Hit;
            }
        else
            {
            BeAssert (ect.IsArray());

            ECValue v;
            ecxinstance->GetValueFromPropertyLayout (v, *propertyLayout, false, 0);

            if (v.IsNull())
                continue;

            ArrayInfo ai = v.GetArrayInfo ();
            bool isStructArray = ai.IsStructArray();

            if (!isStructArray && !allTypes && ai.GetElementPrimitiveType() != primitiveType)
                continue;

            for (UInt32 idx = 0, count = ai.GetCount(); idx < count; ++idx)
                {
                ECValue vitem;
                ecxinstance->GetValueFromPropertyLayout (vitem, *propertyLayout, true, idx);
                
                bool foundOne;

                if (isStructArray)
                    foundOne = ProcessStructProperty (failedClasses, noCandidateInAnyStruct, vitem, primitiveType, proc, opts);
                else
                    {
                    anyCandidates = true;
                    foundOne = proc._ProcessPrimitiveProperty (instance, propertyLayout->GetAccessString(), vitem);
                    }

                if (foundOne)
/*<==*/             return PROPERTY_PROCESSING_RESULT_Hit;
                }
            }
        }

    if (!anyCandidates && noCandidateInAnyStruct)
        {
        failedClasses.insert (&instance.GetClass());
        return PROPERTY_PROCESSING_RESULT_NoCandidates;
        }

    return PROPERTY_PROCESSING_RESULT_Miss;
    }

#endif // defined (EXPERIMENTAL_TEXT_FILTER)

END_BENTLEY_DGNPLATFORM_NAMESPACE
