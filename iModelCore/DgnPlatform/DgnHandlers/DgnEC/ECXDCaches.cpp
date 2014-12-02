/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/ECXDCaches.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerFileCache::ECXDPerFileCache (DgnProjectR dgnFile) : ProviderPerFileCache(dgnFile, &ECXDProvider::GetProvider())
    {
    if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_TRACE))
        DgnECManager::GetManager().GetLogger().tracev (L"Constructing ECXDPerFileCache   of %ls.", GetDgnFile().GetFileName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerFileCache::~ECXDPerFileCache () 
    {
    if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_TRACE))
        DgnECManager::GetManager().GetLogger().tracev (L"Destructing ECXDPerFileCache of %ls.", GetDgnFile().GetFileName().c_str());    
    };

/*---------------------------------------------------------------------------------**//**
ECSchemaPtr ECXDPerFileCache::LocateSchema (const wchar_t* schemaName, UInt32& versionMajor, UInt32& versionMinor, SchemaMatchType matchType, void * schemaContext) const
* @bsimethod                                                    CaseyMullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECPerSchemaCacheP       ECXDPerFileCache::_FindPerSchemaCacheByName (WCharCP schemaName)
    {
    PerSchemaCachePtrMap::iterator it = m_perSchemaCachesByName.find (schemaName);
    if (it == m_perSchemaCachesByName.end())
        return NULL;

    return it->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerSchemaCacheP      ECXDPerFileCache::FindPerSchemaCacheByElementRef (ElementRefP schemaElementRef)
    {
    PerSchemaCachePtrMap::iterator it;
    for (it = m_perSchemaCachesByName.begin(); it != m_perSchemaCachesByName.end(); ++it)
        {
        if (it->second->GetSchemaElementRefP() == schemaElementRef)
            return it->second.get();
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDPerFileCache::AddToPerSchemaCachesByIndex (ECXDPerSchemaCache& perSchemaCache)
    {
    BeAssert (perSchemaCache.IsSchemaIndexInitialized() && "ECXDPerFileCache::AddToPerSchemaCachesByIndex requires that SchemaIndex be initialized.");

    SchemaIndex schemaIndex = perSchemaCache.GetSchemaIndex();
    if (schemaIndex >= m_perSchemaCachesByIndex.size())
        m_perSchemaCachesByIndex.resize (schemaIndex + 1, NULL);

    BeAssert (NULL == m_perSchemaCachesByIndex[schemaIndex] && "Attempted to overwrite a perSchemaCache in m_perSchemaCachesByIndex of ECXDProvider");
    //WString schemaName = perSchemaCache.GetSchemaInfo().GetSchemaName();
    m_perSchemaCachesByIndex[schemaIndex] = m_perSchemaCachesByName[perSchemaCache.GetSchemaInfo().GetSchemaName()].get();

    if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        DgnECManager::GetManager().GetLogger().debugv (L"ECSchema %ls just claimed SchemaIndex %d in %ls", perSchemaCache.GetSchemaInfo().GetSchemaName(), schemaIndex, perSchemaCache.m_perFileCache.GetDgnFile().GetFileName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDPerFileCache::AddPerSchemaCache (ECXDPerSchemaCacheR perSchemaCache)
    {
    //WString schemaName = perSchemaCache.GetSchemaInfo().GetSchemaName();
    m_perSchemaCachesByName[perSchemaCache.GetSchemaInfo().GetSchemaName()] = &perSchemaCache;
    
    if (perSchemaCache.IsSchemaIndexInitialized())
        AddToPerSchemaCachesByIndex (perSchemaCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDPerFileCache::DropFromPerSchemaCachesByIndex (SchemaIndex schemaIndex)
    {
    PRECONDITION (m_perSchemaCachesByIndex.size() > schemaIndex, );
    PRECONDITION (NULL != m_perSchemaCachesByIndex[schemaIndex], );

    m_perSchemaCachesByIndex[schemaIndex] = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDPerFileCache::DropPerSchemaCache (ECXDPerSchemaCacheR dropThis)
    {
    if (dropThis.IsSchemaIndexInitialized())
        DropFromPerSchemaCachesByIndex (dropThis.GetSchemaIndex());

    WCharCP schemaNameP = dropThis.GetSchemaInfo().GetSchemaName();
    BeAssert(NULL != FindPerSchemaCacheByName(schemaNameP));
    m_perSchemaCachesByName.erase(schemaNameP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDPerFileCache::_CreatePerSchemaCache (ECN::ECSchemaCP schema, SchemaInfoCR info, ElementRefP schemaElementRef)
    {
    ECXDPerSchemaCacheP perSchemaCache = new ECXDPerSchemaCache (*this, schema, info, schemaElementRef);

    AddPerSchemaCache (*perSchemaCache);

    if (schemaElementRef)
        {
        SchemaLayoutElementHandler& schemaLayoutHandler = SchemaLayoutElementHandler::GetInstance();

        SchemaIndex     schemaIndex;
        // DEFERRED_FUSION: may need fully-qualified name (with version)
        ElementHandle   elemHandle (schemaLayoutHandler.FindSchemaLayoutElement (schemaIndex, info.GetSchemaName(), GetDgnFile()));

        if (elemHandle.IsValid())
            perSchemaCache->SetSchemaIndex (schemaIndex, elemHandle.GetElementRef());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           ECXDPerFileCache::FindAvailableSchemaIndex(SchemaIndex& schemaIndex)
    {
    // We intentionally skip index 0 and leave it NULL, because that slot is reserved for the ECXDSystemSchema
    if (m_perSchemaCachesByIndex.size() == 0)
        {
        schemaIndex = 1; 
        return SUCCESS;
        }
    
    ECXDPerSchemaCacheP nullValue = NULL; 
    PerSchemaCacheVector::iterator iter = std::find (m_perSchemaCachesByIndex.begin() + 1, m_perSchemaCachesByIndex.end(), nullValue);

    size_t firstNullIndex = iter - m_perSchemaCachesByIndex.begin();

    if (USHRT_MAX > firstNullIndex)
        { 
        schemaIndex = (UInt16) firstNullIndex;
        return SUCCESS;
        }

    // The max size for schemaIndex is 0xffff, but if we reach that limit,
    // most likely something else has gone wrong.
    BeAssert(false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDPerFileCache::_GetAllSchemaInfos (bvector<SchemaInfo>& schemaInfos)
    {
    FOR_EACH (PerSchemaCachePtrMap::value_type pair, m_perSchemaCachesByName)
        {
        ECPerSchemaCacheP perSchemaCache = pair.second.get();

        BeAssert (schemaInfos.end() == std::find (schemaInfos.begin(), schemaInfos.end(), perSchemaCache->GetSchemaInfo()));
        schemaInfos.push_back (perSchemaCache->GetSchemaInfo());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerClassCacheP       ECXDPerFileCache::ObtainPerClassCache (ECClassCR ecClass)
    {
    PerClassCacheMap::iterator it = m_perClassCachesByClass.find (&ecClass);
    if (it != m_perClassCachesByClass.end())
        return it->second.get();

    return ObtainPerClassCacheByName (ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerClassCacheP       ECXDPerFileCache::ObtainPerClassCacheByName (WCharCP schemaName, WCharCP className)
    {
    ECXDPerSchemaCacheP perSchemaCache = dynamic_cast<ECXDPerSchemaCacheP>(FindPerSchemaCacheByName (schemaName));
    if (NULL == perSchemaCache)
        return NULL;

    return perSchemaCache->ObtainPerClassCache (className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerSchemaCacheP      ECXDPerFileCache::FindPerSchemaCacheByIndex (SchemaIndex schemaIndex)
    {
    if (m_perSchemaCachesByIndex.size() <= schemaIndex)
        m_perSchemaCachesByIndex.resize(schemaIndex + 1, NULL);
    
    ECXDPerSchemaCacheP perSchemaCache = m_perSchemaCachesByIndex[schemaIndex];
    if (NULL == perSchemaCache)
        { 
        BeAssert(false && "Programmer error! You should not be able to call this without having first triggered the loading of all ECSchema info for the file. So you are passing a bad index or the file is corrupt."); 
        return NULL;
        }

    return perSchemaCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr           ECXDPerFileCache::_LocateStandaloneEnabler (SchemaKeyCR schemaKey, const wchar_t* className)
    {
    ECXDPerClassCacheP  perClassCache = ObtainPerClassCacheByName (schemaKey.m_schemaName.c_str(), className);
    if (NULL == perClassCache)
        return NULL;

    return &perClassCache->ObtainInstanceEnabler()->GetStandaloneInstanceEnabler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerSchemaCache::ECXDPerSchemaCache (ECXDPerFileCacheR perFileCache, ECSchemaCP schema, SchemaInfoCR info, ElementRefP schemaElementRef) : 
    m_perFileCache (perFileCache), ECPerSchemaCache(perFileCache, schema, info, schemaElementRef)
    {
    if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        DgnECManager::GetManager().GetLogger().debugv (L"Constructing ECXDPerSchemaCache in %ls of %ls.", m_perFileCache.GetDgnFile().GetFileName().c_str(), GetSchemaInfo().GetSchemaName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerSchemaCache::~ECXDPerSchemaCache ()
    {
    if (DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        DgnECManager::GetManager().GetLogger().debugv (L" Destructing ECXDPerSchemaCache of %ls in %ls.", 
            GetSchemaInfo().GetSchemaName(),
            m_perFileCache.GetDgnFile().GetFileName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerFileCacheR ECXDPerSchemaCache::GetPerFileCache ()
    {
    return m_perFileCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECN::IECSchemaLocaterP>* ECXDPerSchemaCache::_GetSchemaLocaters()
    {
    return ECXDProvider::GetProvider().GetSchemaLocaters();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDPerSchemaCache::AddToPerClassCachesByIndex (ECN::ClassIndex classIndex, ECXDPerClassCacheR perClassCache)
    {
    if (m_perClassCaches.size() <= classIndex)
        m_perClassCaches.resize(classIndex + 1, NULL);

    m_perClassCaches[classIndex] = &perClassCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerClassCacheP       ECXDPerSchemaCache::PeekPerClassCacheByIndex (ClassIndex classIndex)
    {
    if (m_perClassCaches.size() <= classIndex)
        return NULL;
    
    return m_perClassCaches[classIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerClassCacheP       ECXDPerSchemaCache::FindPerClassCacheByIndex (ClassIndex classIndex)
    {
    if (NULL == PeekSchemaLayout())
        LoadSchemaLayoutFromDgn();

    BeAssert (m_perClassCaches.size() > classIndex);
    return PeekPerClassCacheByIndex (classIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerClassCacheP       ECXDPerSchemaCache::ObtainPerClassCache (WCharCP className)
    {
    PerClassCachePtrByName::iterator it = m_perClassCachesByName.find (className);
    if (m_perClassCachesByName.end() != it)
        return it->second.get();

    // Perhaps its just needs to be lazy-loaded...
    ECSchemaPtr schema = NULL;
    if (SCHEMALOCATE_Success != ObtainSchema (schema))
        return NULL;

    ECClassP ecClass = schema->GetClassP(className);
    if (NULL == ecClass)
        {
        DgnECManager::GetManager().GetLogger().warningv (L"ECXDPerSchemaCache::ObtainPerClassCache failed to find ECClass %ls in ECSchema %ls", className, schema->GetName().c_str());
        return NULL;
        }

    return new ECXDPerClassCache (*this, *ecClass); // It adds itself to its parent ECXDPerSchemaCache
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ECXDPerSchemaCache::AddToPerClassCachesByName (ECXDPerClassCacheR perClassCache)
    {
    BeAssert (m_perClassCachesByName[perClassCache.m_className.c_str()].IsNull());

    m_perClassCachesByName[perClassCache.m_className.c_str()] = &perClassCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDPerSchemaCache::SetSchemaIndex (SchemaIndex schemaIndex, ElementRefP elemRef)
    {
    BeAssert (NULL == PeekSchemaLayout());    
    BeAssert ( ! IsSchemaIndexInitialized());
    BeAssert (NULL == m_schemaLayoutEntry.m_schemaLayoutElement);

    m_schemaLayoutEntry.m_schemaIndex            = schemaIndex;
    m_schemaLayoutEntry.m_schemaIndexInitialized = true;
    m_schemaLayoutEntry.m_schemaLayoutElement    = elemRef;

    m_perFileCache.AddToPerSchemaCachesByIndex (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDPerSchemaCache::SetSchemaLayout (SchemaLayoutR schemaLayout)
    {
    BeAssert (NULL == PeekSchemaLayout());
    BeAssert ( ! IsSchemaIndexInitialized() || schemaLayout.GetSchemaIndex() == m_schemaLayoutEntry.m_schemaIndex);

    m_schemaLayoutEntry.m_schemaLayout = &schemaLayout;

    if ( ! m_schemaLayoutEntry.m_schemaIndexInitialized)
        {
        m_schemaLayoutEntry.m_schemaIndex  = schemaLayout.GetSchemaIndex();
        m_schemaLayoutEntry.m_schemaIndexInitialized = true;

        m_perFileCache.AddToPerSchemaCachesByIndex (*this);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDPerSchemaCache::ClearSchemaLayout (SchemaIndex schemaIndex)
    {
    m_perFileCache.DropFromPerSchemaCachesByIndex (schemaIndex);

    m_schemaLayoutEntry.m_schemaIndex            = 0;
    m_schemaLayoutEntry.m_schemaIndexInitialized = false;
    m_schemaLayoutEntry.m_schemaLayoutElement    = NULL;

    DELETE_AND_CLEAR (m_schemaLayoutEntry.m_schemaLayout);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           ECXDPerSchemaCache::LoadSchemaLayoutFromDgn ()
    {
    BeAssert (NULL == PeekSchemaLayout() && "In LoadSchemaLayoutFromDgn: Do not attempt to load a SchemaLayout twice");

    ElementHandle schemaLayoutElement (m_schemaLayoutEntry.m_schemaLayoutElement);

    if ( ! schemaLayoutElement.IsValid())
        return ERROR;

    if ( ! EXPECTED_CONDITION (schemaLayoutElement.GetDgnModelP() == m_perFileCache.GetDgnFile().GetDictionaryModel()))
        return ERROR;

    SchemaLayoutElementHandler& schemaLayoutHandler = SchemaLayoutElementHandler::GetInstance();

    SchemaLayoutP schemaLayout = schemaLayoutHandler.LoadSchemaLayout (schemaLayoutElement);

    if (NULL == schemaLayout)
        return ERROR;

    SetSchemaLayout (*schemaLayout);

    // Add the newly loaded classLayouts into the perSchemaCache
    for (ClassIndex classIndex = 0; classIndex <= schemaLayout->GetMaxIndex(); classIndex++)
        {
        ClassLayoutCP classLayout = schemaLayout->GetClassLayout(classIndex);
        if (NULL == classLayout)
            continue;

        BeAssert (schemaLayout->GetSchemaIndex() == classLayout->GetSchemaIndex());

        ECXDPerClassCacheP perClassCache = NULL;

        PerClassCachePtrByName::iterator it = m_perClassCachesByName.find (classLayout->GetECClassName().c_str());
        if (m_perClassCachesByName.end() != it)
            perClassCache = it->second.get();

        if (NULL == perClassCache)
            {
            perClassCache = new ECXDPerClassCache (*this, *classLayout); // It adds itself to its parent ECXDPerSchemaCache
            }
        else
            {
            perClassCache->m_classLayout = classLayout;
            AddToPerClassCachesByIndex (classIndex, *perClassCache);
            }

        perClassCache->SetIsClassLayoutPersisted(true);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerClassCache::ECXDPerClassCache(ECXDPerSchemaCacheR perSchemaCache, ECN::ECClassCR ecClass) : 
    m_perSchemaCache(perSchemaCache), 
    m_className(ecClass.GetName().c_str()),
    m_ecClass(&ecClass), 
    m_classLayout(NULL), m_classLayoutIsPersisted (false)
    {
#if defined (BEIJING_WIP_ECCLASS_WITH_EMPTY_NAME)
#endif
    WString cname = ecClass.GetName().empty()? ecClass.GetDisplayLabel(): ecClass.GetName();
    if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        {
        DgnECManager::GetManager().GetLogger().debugv (L"Constructing ECXDPerClassCache of %ls:%ls from ECClass.", 
            ecClass.GetSchema().GetName().c_str(), cname.c_str());
        }
    perSchemaCache.AddToPerClassCachesByName (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerClassCache::ECXDPerClassCache(ECXDPerSchemaCacheR perSchemaCache, ECN::ClassLayoutCR classLayout) : 
    m_perSchemaCache(perSchemaCache), 
    m_className(classLayout.GetECClassName().c_str()),
    m_ecClass(NULL), 
    m_classLayout(&classLayout), m_classLayoutIsPersisted (false)
    {
    BeAssert (NULL != perSchemaCache.PeekSchemaLayout() && "This constructor is to be called during population of caches from a persisted SchemaLayout");
    
    if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        {
        DgnECManager::GetManager().GetLogger().debugv (L"Constructing ECXDPerClassCache of %ls:%ls from ClassLayout. ClassIndex=%d",
            perSchemaCache.GetSchemaInfo().GetSchemaName(), classLayout.GetECClassName().c_str(), classLayout.GetClassIndex());
        }

    perSchemaCache.AddToPerClassCachesByIndex (classLayout.GetClassIndex(), *this);
    perSchemaCache.AddToPerClassCachesByName  (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDPerClassCache::~ECXDPerClassCache() 
    {
    if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        {
        WString className;
        if (NULL != m_ecClass)
            className = m_ecClass->GetName();
        else if (NULL != m_classLayout)
            className = m_classLayout->GetECClassName();
        else
            className = L"<unknown>";
        
        DgnECManager::GetManager().GetLogger().debugv (L" Destructing ECXDPerClassCache of %ls:%ls.",
            m_perSchemaCache.GetSchemaInfo().GetSchemaName(), className.c_str());
        }

    if (NULL != m_classLayout)
        delete m_classLayout;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerP     ECXDPerClassCache::ObtainInstanceEnabler ()
    {
    if (m_instanceEnabler.IsValid())
        return m_instanceEnabler.get();
        
    if (NULL == m_classLayout)
        m_classLayout = ObtainClassLayout ();

    BeAssert (NULL != m_classLayout);

    if (NULL == m_ecClass)
        {
        ECSchemaPtr schema = NULL;
        if (SCHEMALOCATE_Success != m_perSchemaCache.ObtainSchema (schema) || schema.IsNull())
            return NULL;

        m_ecClass = schema->GetClassP(m_className.c_str());
        if (NULL == m_ecClass)
            {
            DgnECManager::GetManager().GetLogger().errorv (L"Loaded ECSchema %ls but could not find expected ECClass %ls!", schema->GetName().c_str(), m_className.c_str());
            BeAssert (false);
            return NULL;
            }
        }

    m_instanceEnabler = ECXDInstanceEnabler::CreateEnabler (*m_ecClass, *m_classLayout, m_perSchemaCache.GetPerFileCache ());

    return m_instanceEnabler.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationshipEnablerP ECXDPerClassCache::ObtainRelationshipEnabler ()
    {
    if (m_relationshipEnabler.IsValid())
        return m_relationshipEnabler.get();
        
    if (NULL == m_classLayout)
        m_classLayout = ObtainClassLayout ();

    BeAssert (NULL != m_classLayout);

    if (NULL == m_ecClass)
        {
        ECSchemaPtr schema = NULL;
        if (SCHEMALOCATE_Success != m_perSchemaCache.ObtainSchema (schema) || schema.IsNull())
            return NULL;

        m_ecClass = schema->GetClassP(m_className.c_str());
        if (NULL == m_ecClass)
            {
            DgnECManager::GetManager().GetLogger().errorv (L"Loaded ECSchema %ls but could not find expected ECClass %ls!", schema->GetName().c_str(), m_className.c_str());
            BeAssert (false && "Loaded ECSchema but could not find expected ECClass");
            return NULL;
            }
        }

    ECRelationshipClassCP relationshipClass = m_ecClass->GetRelationshipClassCP ();
    if (NULL == relationshipClass)
        return NULL;

    m_relationshipEnabler = ECXDRelationshipEnabler::CreateRelationshipEnabler (*relationshipClass, *m_classLayout, m_perSchemaCache.GetPerFileCache ());
    //m_instanceEnabler = m_relationshipEnabler; // It is also an instance enabler for purposes of searching for the relationships themselves. But it might create confusion when doing CRUD

    return m_relationshipEnabler.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCP           ECXDPerClassCache::CreateClassLayout ()
    {
    BeAssert (NULL != m_ecClass && "Not sure if this will hold true. -Casey");
    BeAssert (NULL == m_classLayout && "In CreateClassLayout: Do not attempt to create a ClassLayout twice");
    if (NULL == m_perSchemaCache.PeekSchemaLayout())
        {
        BeAssert (FALSE == m_perSchemaCache.m_schemaLayoutEntry.m_schemaIndexInitialized);
        BeAssert (NULL  == m_perSchemaCache.m_schemaLayoutEntry.m_schemaLayoutElement);

        ECXDPerFileCacheR perFileCache = m_perSchemaCache.m_perFileCache;
        
        SchemaIndex schemaIndex;
        if (SUCCESS != perFileCache.FindAvailableSchemaIndex (schemaIndex))
            { BeAssert(false); return NULL; }

        SchemaLayoutP schemaLayout = SchemaLayout::Create(schemaIndex);
        m_perSchemaCache.SetSchemaLayout(*schemaLayout);
        }

    SchemaLayoutR schemaLayout = *m_perSchemaCache.PeekSchemaLayout();
    ClassIndex  classIndex = 0;
    if (SUCCESS != schemaLayout.FindAvailableClassIndex (classIndex))
        { BeAssert (false); return NULL; }

    m_classLayout = ClassLayout::BuildFromClass (*m_ecClass, classIndex, schemaLayout.GetSchemaIndex()); // use schemaLayout index since in ECXA case no schema layout is required
    schemaLayout.AddClassLayout (*m_classLayout, classIndex);
    m_perSchemaCache.AddToPerClassCachesByIndex (classIndex, *this);
    if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_DEBUG))
        DgnECManager::GetManager().GetLogger().debugv (L"ECClass %ls:%ls just claimed ClassIndex %d in %ls.", 
            m_perSchemaCache.GetSchemaInfo().GetSchemaName(), m_ecClass->GetName().c_str(), classIndex, m_perSchemaCache.m_perFileCache.GetDgnFile().GetFileName().c_str());

    return m_classLayout; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCP           ECXDPerClassCache::ObtainClassLayout ()
    {
    if (NULL != m_classLayout)
        return m_classLayout;

    if (NULL == m_perSchemaCache.PeekSchemaLayout())
        {
        m_perSchemaCache.LoadSchemaLayoutFromDgn ();
        
        if (NULL != m_classLayout)
            return m_classLayout;
        }

    return CreateClassLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECXDPerClassCache::IsClassLayoutPersisted ()
    {
    return m_classLayoutIsPersisted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDPerClassCache::SetIsClassLayoutPersisted (bool val)
    {
    BeAssert (NULL != m_classLayout);
    BeAssert (val != m_classLayoutIsPersisted);

    m_classLayoutIsPersisted = val;
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

