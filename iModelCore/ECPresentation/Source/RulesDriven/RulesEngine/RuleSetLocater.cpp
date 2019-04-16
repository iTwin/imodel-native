/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/RuleSetEmbedder.h>
#include <Bentley/BeDirectoryIterator.h>
#include "ECSchemaHelper.h"

#define PRESENTATION_RULESET_EXTENSION_Xml   L".PresentationRuleSet.xml"
#define PRESENTATION_RULESET_EXTENSION_Json  L".PresentationRuleSet.json"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RuleSetLocaterManager::RuleSetLocaterManager(IConnectionManagerCR connections)
    : m_connections(connections), m_isLocating(false)
    {
    m_connections.AddListener(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RuleSetLocaterManager::~RuleSetLocaterManager()
    {
    m_connections.DropListener(*this);

    BeMutexHolder lock(m_mutex);
    for (RuleSetLocaterPtr const& locater : m_locaters)
        locater->SetRulesetCallbacksHandler(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_RegisterLocater(RuleSetLocater& locater)
    {
    BeMutexHolder lock(m_mutex);
    locater.SetRulesetCallbacksHandler(this);
    m_locaters.push_back(&locater);
    m_rulesetsCache.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_UnregisterLocater(RuleSetLocater const& locater)
    {
    BeMutexHolder lock(m_mutex);
    auto iter = std::find(m_locaters.begin(), m_locaters.end(), RuleSetLocaterCPtr(&locater));
    if (iter == m_locaters.end())
        {
        BeAssert(false);
        return;
        }
    (*iter)->SetRulesetCallbacksHandler(nullptr);
    m_locaters.erase(iter);
    m_rulesetsCache.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_OnRulesetDispose(RuleSetLocaterCR locater, PresentationRuleSetCR ruleset)
    {
    if (nullptr != GetRulesetCallbacksHandler())
        GetRulesetCallbacksHandler()->_OnRulesetDispose(locater, ruleset);

    BeMutexHolder lock(m_mutex);
    bvector<CacheKey> toErase;
    for (auto pair : m_rulesetsCache)
        {
        if (ruleset.GetRuleSetId().Equals(pair.first.m_rulesetId))
            toErase.push_back(pair.first);
        }
    for (CacheKey const& key : toErase)
        m_rulesetsCache.erase(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_OnRulesetCreated(RuleSetLocaterCR locater, PresentationRuleSetR ruleset)
    {
    if (nullptr != GetRulesetCallbacksHandler())
        GetRulesetCallbacksHandler()->_OnRulesetCreated(locater, ruleset);

    if (!m_isLocating)
        RemoveCachedRulesets(ruleset.GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_OnConnectionEvent(ConnectionEvent const& evt)
    {
    BeMutexHolder lock(m_mutex);
    bvector<CacheKey> toErase;
    for (auto pair : m_rulesetsCache)
        {
        if (pair.first.m_connectionId == evt.GetConnection().GetId())
            toErase.push_back(pair.first);
        }
    for (CacheKey const& key : toErase)
        m_rulesetsCache.erase(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_InvalidateCache(Utf8CP rulesetId)
    {
    BeMutexHolder lock(m_mutex);

    for (RuleSetLocaterPtr const& locater : m_locaters)
        locater->InvalidateCache(rulesetId);

    RemoveCachedRulesets(rulesetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::RemoveCachedRulesets(Utf8CP rulesetId)
    {
    bvector<CacheKey> toErase;
    for (auto pair : m_rulesetsCache)
        {
        if (nullptr == rulesetId || pair.first.m_rulesetId.Equals(rulesetId))
            toErase.push_back(pair.first);
        }
    for (CacheKey const& key : toErase)
        m_rulesetsCache.erase(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> RuleSetLocaterManager::_LocateRuleSets(IConnectionCR connection, Utf8CP rulesetId) const
    {
    BeMutexHolder lock(m_mutex);
    ECSchemaHelper helper(connection, nullptr, nullptr, nullptr, nullptr);

    if (nullptr != rulesetId)
        {
        bvector<PresentationRuleSetPtr> rulesets;
        auto cacheIterWithId = m_rulesetsCache.find(CacheKey(connection.GetId(), rulesetId));
        if (m_rulesetsCache.end() != cacheIterWithId)
            {
            for (CacheValue& cacheValue : cacheIterWithId->second)
                rulesets.push_back(cacheValue.m_ruleset);
            }

        auto cacheIterWithoutId = m_rulesetsCache.find(CacheKey(nullptr, rulesetId));
        if (m_rulesetsCache.end() != cacheIterWithoutId)
            {
            for (CacheValue& cacheValue : cacheIterWithoutId->second)
                {
                if (!helper.AreSchemasSupported(cacheValue.m_ruleset->GetSupportedSchemas()))
                    continue;
                rulesets.push_back(cacheValue.m_ruleset);
                }
            }

        if (cacheIterWithId != m_rulesetsCache.end() || cacheIterWithoutId != m_rulesetsCache.end())
            return rulesets;
        }
    
    bvector<RuleSetLocaterPtr> sortedLocaters = m_locaters;
    std::sort(sortedLocaters.begin(), sortedLocaters.end(), [](RuleSetLocaterPtr a, RuleSetLocaterPtr b)
        {
        if (a->GetPriority() < b->GetPriority())
            return -1;
        if (a->GetPriority() > b->GetPriority())
            return 1;
        return 0;
        });

    m_isLocating = true;

    bmap<Utf8String, PresentationRuleSetPtr> locatedRulesets;
    for (RuleSetLocaterPtr& locater : sortedLocaters)
        {
        IConnectionCP designatedConnection = locater->GetDesignatedConnection();
        if (nullptr != designatedConnection && !designatedConnection->GetId().Equals(connection.GetId()))
            continue;

        bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets(rulesetId);
        for (PresentationRuleSetPtr& ruleset : rulesets)
            {
            if (!helper.AreSchemasSupported(ruleset->GetSupportedSchemas()))
                continue;

            if (locatedRulesets.end() == locatedRulesets.find(ruleset->GetFullRuleSetId()))
                {
                locatedRulesets[ruleset->GetFullRuleSetId()] = ruleset;
                m_rulesetsCache[CacheKey(designatedConnection == nullptr ? "" : designatedConnection->GetId(), ruleset->GetRuleSetId())].push_back(CacheValue(ruleset, locater->GetPriority()));
                }
            }
        }

    m_isLocating = false;

    bvector<PresentationRuleSetPtr> results;
    for (auto& pair : locatedRulesets)
        results.push_back(pair.second);

    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> RuleSetLocaterManager::_GetRuleSetIds() const
    {
    BeMutexHolder lock(m_mutex);
    bvector<Utf8String> results;
    for (RuleSetLocaterPtr const& locater : m_locaters)
        {
        bvector<Utf8String> ids = locater->GetRuleSetIds();
        results.insert(results.end(), ids.begin(), ids.end());
        }
    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> RuleSetLocater::LocateRuleSets(Utf8CP rulesetId) const
    {
    BeMutexHolder lock(GetMutex());
    return _LocateRuleSets(rulesetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> RuleSetLocater::GetRuleSetIds() const
    {
    BeMutexHolder lock(GetMutex());
    return _GetRuleSetIds();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int RuleSetLocater::GetPriority() const { return _GetPriority(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::InvalidateCache(Utf8CP rulesetId)
    {
    BeMutexHolder lock(GetMutex());
    _InvalidateCache(rulesetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::OnRulesetDisposed(PresentationRuleSetR ruleset) const
    {
    BeMutexHolder lock(GetMutex());

    auto iter = std::find(m_createdRulesets.begin(), m_createdRulesets.end(), &ruleset);
    if (m_createdRulesets.end() != iter)
        m_createdRulesets.erase(iter);

    if (nullptr != m_rulesetCallbacksHandler)
        m_rulesetCallbacksHandler->_OnRulesetDispose(*this, ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::OnRulesetCreated(PresentationRuleSetR ruleset) const
    {
    BeMutexHolder lock(GetMutex());

    m_createdRulesets.push_back(&ruleset);

    if (nullptr != m_rulesetCallbacksHandler)
        m_rulesetCallbacksHandler->_OnRulesetCreated(*this, ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::SetRulesetCallbacksHandler(IRulesetCallbacksHandler* handler)
    {
    BeMutexHolder lock(GetMutex());

    m_rulesetCallbacksHandler = handler;

    if (nullptr != m_rulesetCallbacksHandler)
        {
        for (RefCountedPtr<PresentationRuleSet> const& ruleset : m_createdRulesets)
            m_rulesetCallbacksHandler->_OnRulesetCreated(*this, *ruleset);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DirectoryRuleSetLocater::_GetRuleSetDirectoryList() const
    {
    return m_directoryList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeFileName> DirectoryRuleSetLocater::_GetRuleSetDirectories() const
    {
    Utf8String directoryList = _GetRuleSetDirectoryList();
    size_t offset = 0;
    Utf8String singleDirectory;
    bvector<BeFileName> directories;
    while (Utf8String::npos != (offset = directoryList.GetNextToken(singleDirectory, ";", offset)))
        directories.push_back(BeFileName(singleDirectory.c_str(), true));
    return directories;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeFileName> DirectoryRuleSetLocater::_GetRuleSetFileNames() const
    {
    bvector<BeFileName> files;
    bvector<BeFileName> directories = _GetRuleSetDirectories();
    for (BeFileName& directory : directories)
        {
        if (!directory.DoesPathExist())
            continue;

        bvector<BeFileName> matches;
        BeDirectoryIterator::WalkDirsAndMatch(matches, directory, L"*" PRESENTATION_RULESET_EXTENSION_Xml, true);
        files.insert(files.end(), matches.begin(), matches.end());

        matches.clear();
        BeDirectoryIterator::WalkDirsAndMatch(matches, directory, L"*" PRESENTATION_RULESET_EXTENSION_Json, true);
        files.insert(files.end(), matches.begin(), matches.end());
        }
    return files;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> DirectoryRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    bvector<PresentationRuleSetPtr> rulesets;
    bvector<BeFileName> files = _GetRuleSetFileNames();
    for (BeFileNameCR file : files)
        {
        if (!file.DoesPathExist())
            continue;

        PresentationRuleSetPtr ruleset;
        auto rulesetIter = m_cache.find(file);
        if (m_cache.end() != rulesetIter)
            {
            ruleset = rulesetIter->second;
            }
        else
            {
            if (file.EndsWithI(PRESENTATION_RULESET_EXTENSION_Xml)) 
                ruleset = PresentationRuleSet::ReadFromXmlFile(file);
            else if (file.EndsWithI(PRESENTATION_RULESET_EXTENSION_Json))
                ruleset = PresentationRuleSet::ReadFromJsonFile(file);

            if (ruleset.IsValid())
                {
                m_cache[file] = ruleset;
                OnRulesetCreated(*ruleset);
                }
            }

        if (ruleset.IsValid() && (nullptr == rulesetId || ruleset->GetRuleSetId().Equals(rulesetId)))
            rulesets.push_back(ruleset);
        }
    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> DirectoryRuleSetLocater::_GetRuleSetIds() const
    {
    bvector<Utf8String> rulesetIds;
    bvector<PresentationRuleSetPtr> rulesets = LocateRuleSets();
    for (PresentationRuleSetPtr& ruleset : rulesets)
        rulesetIds.push_back(Utf8String(ruleset->GetRuleSetId().c_str()));
    return rulesetIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryRuleSetLocater::_InvalidateCache(Utf8CP rulesetId)
    {
    bvector<BeFileName> filenamesToRemove;
    for (auto iter : m_cache)
        {
        PresentationRuleSetPtr const& ruleset = iter.second;
        if (nullptr == rulesetId || ruleset->GetRuleSetId().Equals(rulesetId))
            {
            OnRulesetDisposed(*ruleset);
            filenamesToRemove.push_back(iter.first);
            }
        }
    for (BeFileNameCR filename : filenamesToRemove)
        m_cache.erase(filename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void FileRuleSetLocater::SetPath(BeFileNameCR path)
    {
    BeMutexHolder lock(GetMutex());

    if (path.Equals(m_path))
        return;

    m_path = path;
    m_cached = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> FileRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    bvector<PresentationRuleSetPtr> rulesets;

    time_t lastModifiedTime;
    if (m_cached.IsValid())
        {
        if ((nullptr == rulesetId || m_cached->GetRuleSetId().Equals(rulesetId))
            && (!m_path.DoesPathExist() || BeFileNameStatus::Success == m_path.GetFileTime(nullptr, nullptr, &lastModifiedTime) && lastModifiedTime <= m_cachedLastModifiedTime))
            {
            rulesets.push_back(m_cached);
            return rulesets;
            }
        else
            {
            OnRulesetDisposed(*m_cached);
            m_cached = nullptr;
            }
        }

    if (m_path.IsEmpty() || !m_path.DoesPathExist())
        return rulesets;

    PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromXmlFile(m_path);
    if (ruleset.IsValid() && (nullptr == rulesetId || ruleset->GetRuleSetId().Equals(rulesetId)))
        {
        m_path.GetFileTime(nullptr, nullptr, &m_cachedLastModifiedTime);
        m_cached = ruleset;
        OnRulesetCreated(*ruleset);
        rulesets.push_back(ruleset);
        }

    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> FileRuleSetLocater::_GetRuleSetIds() const
    {
    bvector<Utf8String> rulesetIds;
    bvector<PresentationRuleSetPtr> rulesets = LocateRuleSets();
    for (PresentationRuleSetPtr& ruleset : rulesets)
        rulesetIds.push_back(Utf8String(ruleset->GetRuleSetId().c_str()));
    return rulesetIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void FileRuleSetLocater::_InvalidateCache(Utf8CP rulesetId)
    {
    if (m_cached.IsNull())
        return;

    if (nullptr == rulesetId || m_cached->GetRuleSetId().Equals(rulesetId))
        {
        OnRulesetDisposed(*m_cached);
        m_cached = nullptr;
        m_cachedLastModifiedTime = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> SupplementalRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    bvector<PresentationRuleSetPtr> rulesets = m_locater->LocateRuleSets(nullptr);
    auto iter = std::remove_if(rulesets.begin(), rulesets.end(), [](PresentationRuleSetPtr r){return !r->GetIsSupplemental();});
    if (rulesets.end() != iter)
        rulesets.erase(iter);
    for (PresentationRuleSetPtr ruleset : rulesets)
        ruleset->SetRuleSetId(rulesetId);
    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> NonSupplementalRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    bvector<PresentationRuleSetPtr> rulesets = m_locater->LocateRuleSets(nullptr);
    auto iter = std::remove_if(rulesets.begin(), rulesets.end(), [](PresentationRuleSetPtr r){return r->GetIsSupplemental();});
    if (rulesets.end() != iter)
        rulesets.erase(iter);
    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> SimpleRuleSetLocater::_LocateRuleSets(Utf8CP ruleSetId) const
    {
    bvector<PresentationRuleSetPtr> ruleSets;
    if (ruleSetId == nullptr)
        {
        for (auto entry : m_cached)
            {
            BeAssert(entry.second->GetRuleSetId().Equals(entry.first));
            ruleSets.push_back(entry.second);
            }
        return ruleSets;
        }

    auto iter = m_cached.find(ruleSetId);
    if (iter != m_cached.end())
        {
        BeAssert(iter->second->GetRuleSetId().Equals(iter->first));
        ruleSets.push_back(iter->second);
        }

    return ruleSets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> SimpleRuleSetLocater::_GetRuleSetIds() const
    {
    bvector<Utf8String> ruleSetIds;
    for (auto entry : m_cached)
        {
        BeAssert(entry.second->GetRuleSetId().Equals(entry.first));
        ruleSetIds.push_back(entry.first);
        }
    return ruleSetIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleRuleSetLocater::AddRuleSet(PresentationRuleSetR ruleSet)
    {
    auto iter = m_cached.find(ruleSet.GetRuleSetId());
    if (iter != m_cached.end())
        {
        BeAssert(iter->second->GetRuleSetId().Equals(iter->first));
        OnRulesetDisposed(*iter->second);
        m_cached.erase(iter);
        }

    m_cached.Insert(ruleSet.GetRuleSetId(), &ruleSet);
    OnRulesetCreated(ruleSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleRuleSetLocater::RemoveRuleSet(Utf8StringCR ruleSetId)
    {
    auto iter = m_cached.find(ruleSetId);
    if (iter != m_cached.end())
        {
        BeAssert(iter->second->GetRuleSetId().Equals(iter->first));
        OnRulesetDisposed(*iter->second);
        m_cached.erase(iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleRuleSetLocater::Clear()
    {
    for (auto entry : m_cached)
        {
        OnRulesetDisposed(*entry.second);
        }
    m_cached.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                11/18
+---------------+---------------+---------------+---------------+---------------+------*/
EmbeddedRuleSetLocater::EmbeddedRuleSetLocater(IConnectionCR connection)
    : m_connection(connection)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void EmbeddedRuleSetLocater::LoadRuleSets() const
    {
    if (!m_connection.GetECDb().Schemas().ContainsSchema(PRESENTATION_RULESET_SCHEMA_NAME))
        return;

    bvector<RulesetJsonRecord> rulesets = QueryRulesets();
    for (RulesetJsonRecord& entry : rulesets)
        {
        BeInt64Id elementId = entry.first.first;
        DateTime lastMod = entry.first.second;
        Utf8String rulesetJson = entry.second;
        
        Json::Value asJson = Json::Value::From(rulesetJson);

        PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromJsonString(asJson["jsonProperties"].ToString());
        if (ruleset.IsNull())
            {
            BeAssert(false && "Ruleset Element must contain valid JSon Properties to construct a Presentation Ruleset");
            continue;
            }
        
        m_cache.push_back(RulesetRecord(RulesetInfo(elementId, lastMod), ruleset));
        OnRulesetCreated(*ruleset);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                11/18
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> EmbeddedRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    (const_cast<EmbeddedRuleSetLocater*>(this))->InvalidateCacheIfNeeded();

    if (nullptr == rulesetId)
        return GetCachedRuleSets();

    bvector<PresentationRuleSetPtr> rulesets;
    for (PresentationRuleSetPtr ruleset : GetCachedRuleSets())
        {
        if (ruleset->GetRuleSetId().Equals(rulesetId))
            rulesets.push_back(ruleset);
        }

    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                11/18
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> EmbeddedRuleSetLocater::GetCachedRuleSets() const
    {
    bvector<PresentationRuleSetPtr> results = bvector<PresentationRuleSetPtr>();
    for (RulesetRecord record : m_cache)
        results.push_back(record.second);

    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<EmbeddedRuleSetLocater::RulesetJsonRecord> EmbeddedRuleSetLocater::QueryRulesets() const
    {
    if (!m_connection.GetECDb().Schemas().ContainsSchema(PRESENTATION_RULESET_SCHEMA_NAME))
        return bvector<RulesetJsonRecord>();

    Utf8CP sql = "SELECT ECInstanceId, jsonProperties, LastMod FROM " PRESENTATION_RULESET_FULL_CLASS_NAME;

    ECSqlStatement stmt = ECSqlStatement();
    stmt.Prepare(m_connection.GetECDb(), sql);

    if (!stmt.IsPrepared())
        {
        BeAssert(false);
        return bvector<RulesetJsonRecord>();;
        }

    bvector<RulesetJsonRecord> rulesets = bvector<RulesetJsonRecord>();
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        BeInt64Id instanceId = stmt.GetValueId<BeInt64Id>(0);
        Utf8String jsonProps = stmt.GetValueText(1);
        DateTime lastModified = stmt.GetValueDateTime(2);
        rulesets.push_back(RulesetJsonRecord(RulesetInfo(instanceId, lastModified), jsonProps));
        }

    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
EmbeddedRuleSetLocater::RulesetJsonRecord EmbeddedRuleSetLocater::QueryRulesetById(BeInt64Id id) const
    {
    if (!m_connection.GetECDb().Schemas().ContainsSchema(PRESENTATION_RULESET_SCHEMA_NAME))
        return RulesetJsonRecord();

    Utf8CP sql = "SELECT jsonProperties, LastMod FROM " PRESENTATION_RULESET_FULL_CLASS_NAME " WHERE ECInstanceId = ?";

    ECSqlStatement stmt = ECSqlStatement();
    stmt.Prepare(m_connection.GetECDb(), sql);

    if (!stmt.IsPrepared())
        {
        BeAssert(false);
        return RulesetJsonRecord();
        }

    stmt.BindId(1, id);

    if (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        Utf8String jsonProps = stmt.GetValueText(0);
        DateTime lastModified = stmt.GetValueDateTime(1);
        return RulesetJsonRecord(RulesetInfo(id, lastModified), jsonProps);
        }

    return RulesetJsonRecord();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                11/18
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> EmbeddedRuleSetLocater::_GetRuleSetIds() const
    {
    bvector<Utf8String> ids;
    for (PresentationRuleSetPtr ruleset : LocateRuleSets())
        ids.push_back(ruleset->GetRuleSetId().c_str());    

    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void EmbeddedRuleSetLocater::_InvalidateCache(Utf8CP rulesetId)
    {
    for (PresentationRuleSetPtr ruleset : GetCachedRuleSets())
        OnRulesetDisposed(*ruleset);

    m_cache.clear();
    LoadRuleSets();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void EmbeddedRuleSetLocater::InvalidateCacheIfNeeded()
    {
    if (m_cache.size() != QueryRuleSetCount())
        {
        InvalidateCache();
        return;
        }

    for (RulesetRecord& recordR : m_cache)
        {
        BeInt64Id rulesetId = recordR.first.first;
        RulesetJsonRecord rulesetElement = QueryRulesetById(rulesetId);
        if (Utf8String::IsNullOrEmpty(rulesetElement.second.c_str()))
            {
            // This ruleset no longer exists in db. Cache should be invalidated
            InvalidateCache();
            return;
            }

        DateTime ruleSetLoaded = recordR.first.second;
        DateTime lastModified = rulesetElement.first.second;
        if (DateTime::CompareResult::EarlierThan != DateTime::Compare(ruleSetLoaded, lastModified))
            continue;
        
        // This ruleset has been modified since being loaded. It should be reloaded
        PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromJsonString(rulesetElement.second);
        if (ruleset.IsNull())
            {
            BeAssert(false && "Ruleset Element must contain a valid JSon Properties to construct a Presentation Ruleset");
            continue;
            }

        OnRulesetDisposed(*recordR.second);
        recordR.second = ruleset;
        OnRulesetCreated(*recordR.second);
        recordR.first.second = lastModified;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                11/18
+---------------+---------------+---------------+---------------+---------------+------*/
size_t EmbeddedRuleSetLocater::QueryRuleSetCount()
    {
    if (!m_connection.GetECDb().Schemas().ContainsSchema(PRESENTATION_RULESET_SCHEMA_NAME))
        return 0;

    Utf8String sql("SELECT count(*) FROM " PRESENTATION_RULESET_FULL_CLASS_NAME);

    ECSqlStatement stmt = ECSqlStatement();
    stmt.Prepare(m_connection.GetECDb(), sql.c_str());
    if (!stmt.IsPrepared())
        {
        BeAssert(false);
        return 0;
        }

    BeSQLite::DbResult result = stmt.Step();
    if (BeSQLite::BE_SQLITE_ROW != result)
        return 0;

    return stmt.GetValueInt(0);
    }