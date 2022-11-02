/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <Bentley/BeDirectoryIterator.h>
#include "Shared/ECSchemaHelper.h"
#include "Shared/Queries/QueryExecutor.h"

#define PRESENTATION_RULESET_EXTENSION_Xml   L".PresentationRuleSet.xml"
#define PRESENTATION_RULESET_EXTENSION_Json  L".PresentationRuleSet.json"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RuleSetLocaterManager::RuleSetLocaterManager(IConnectionManagerCR connections)
    : m_connections(connections), m_isLocating(false)
    {
    m_connections.AddListener(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RuleSetLocaterManager::~RuleSetLocaterManager()
    {
    m_connections.DropListener(*this);

    BeMutexHolder lock(m_mutex);
    for (RuleSetLocaterPtr const& locater : m_locaters)
        {
        locater->SetMutex(nullptr);
        locater->RemoveRulesetCallbacksHandler(*this);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_RegisterLocater(RuleSetLocater& locater)
    {
    locater.SetMutex(&m_mutex);
    locater.AddRulesetCallbacksHandler(*this);

    BeMutexHolder lock(m_mutex);
    m_locaters.push_back(&locater);
    m_rulesetsCache.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_UnregisterLocater(RuleSetLocater& locater)
    {
    locater.SetMutex(nullptr);
    locater.RemoveRulesetCallbacksHandler(*this);

    BeMutexHolder lock(m_mutex);
    auto iter = std::find(m_locaters.begin(), m_locaters.end(), RuleSetLocaterCPtr(&locater));
    if (iter == m_locaters.end())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to find a locater that was requested to be unregistered");

    m_locaters.erase(iter);
    m_rulesetsCache.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_OnRulesetDispose(RuleSetLocaterCR locater, PresentationRuleSetR ruleset)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_OnRulesetCreated(RuleSetLocaterCR locater, PresentationRuleSetR ruleset)
    {
    BeMutexHolder lock(m_mutex);

    if (!m_isLocating)
        RemoveCachedRulesets(ruleset.GetRuleSetId().c_str());

    IRulesetCallbacksHandler* handler = GetRulesetCallbacksHandler();
    lock.unlock();

    if (nullptr != handler)
        handler->_OnRulesetCreated(locater, ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_InvalidateCache(Utf8CP rulesetId)
    {
    BeMutexHolder lock(m_mutex);

    for (RuleSetLocaterPtr const& locater : m_locaters)
        locater->InvalidateCache(rulesetId);

    RemoveCachedRulesets(rulesetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> RuleSetLocaterManager::_LocateRuleSets(IConnectionCR connection, Utf8CP rulesetId) const
    {
    BeMutexHolder lock(m_mutex);
    ECSchemaHelper helper(connection, nullptr, nullptr);

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
                rulesets.push_back(cacheValue.m_ruleset);
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeMutex& RuleSetLocater::GetMutex() const
    {
    if (nullptr != m_mutex)
        return *m_mutex;
    return m_defaultMutex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> RuleSetLocater::LocateRuleSets(Utf8CP rulesetId) const
    {
    BeMutexHolder lock(GetMutex());
    return _LocateRuleSets(rulesetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> RuleSetLocater::GetRuleSetIds() const
    {
    BeMutexHolder lock(GetMutex());
    return _GetRuleSetIds();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int RuleSetLocater::GetPriority() const { return _GetPriority(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::InvalidateCache(Utf8CP rulesetId)
    {
    BeMutexHolder lock(GetMutex());
    _InvalidateCache(rulesetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::OnRulesetDisposed(PresentationRuleSetR ruleset) const
    {
    BeMutexHolder lock(GetMutex());

    auto iter = std::find(m_createdRulesets.begin(), m_createdRulesets.end(), &ruleset);
    if (m_createdRulesets.end() != iter)
        m_createdRulesets.erase(iter);

    bset<IRulesetCallbacksHandler*> handlers = m_rulesetCallbacksHandlers;
    lock.unlock();

    for (IRulesetCallbacksHandler* handler : handlers)
        handler->_OnRulesetDispose(*this, ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::OnRulesetCreated(PresentationRuleSetR ruleset) const
    {
    BeMutexHolder lock(GetMutex());
    m_createdRulesets.push_back(&ruleset);

    bset<IRulesetCallbacksHandler*> handlers = m_rulesetCallbacksHandlers;
    lock.unlock();

    for (IRulesetCallbacksHandler* handler : handlers)
        handler->_OnRulesetCreated(*this, ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::AddRulesetCallbacksHandler(IRulesetCallbacksHandler& handler) const
    {
    BeMutexHolder lock(GetMutex());
    m_rulesetCallbacksHandlers.insert(&handler);

    bvector<PresentationRuleSetPtr> createdRulesets = m_createdRulesets;
    lock.unlock();

    for (RefCountedPtr<PresentationRuleSet> const& ruleset : createdRulesets)
        handler._OnRulesetCreated(*this, *ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::RemoveRulesetCallbacksHandler(IRulesetCallbacksHandler& handler) const
    {
    BeMutexHolder lock(GetMutex());
    m_rulesetCallbacksHandlers.erase(&handler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DirectoryRuleSetLocater::_GetRuleSetDirectoryList() const
    {
    return m_directoryList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeFileName> DirectoryRuleSetLocater::_GetRuleSetFileNames() const
    {
    bvector<BeFileName> files;
    bvector<BeFileName> directories = _GetRuleSetDirectories();
    for (BeFileName& directory : directories)
        {
        if (!directory.DoesPathExist())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_DEBUG, LOG_WARNING, Utf8PrintfString("Requested rulesets directory does not exist: '%s'", directory.GetNameUtf8().c_str()));
            continue;
            }

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> DirectoryRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    bvector<PresentationRuleSetPtr> rulesets;
    bvector<BeFileName> files = _GetRuleSetFileNames();
    for (BeFileNameCR file : files)
        {
        if (!file.DoesPathExist())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_DEBUG, LOG_WARNING, Utf8PrintfString("Requested ruleset file does not exist: '%s'", file.GetNameUtf8().c_str()));
            continue;
            }

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
            else
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Detected a file with non-ruleset extension: '%s'", file.GetNameUtf8().c_str()));
                continue;
                }

            if (ruleset.IsValid())
                {
                m_cache[file] = ruleset;
                OnRulesetCreated(*ruleset);
                DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Loaded ruleset '%s' from file: '%s'", ruleset->GetRuleSetId().c_str(), file.GetNameUtf8().c_str()));
                }
            else
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Failed to load ruleset from file: '%s'", file.GetNameUtf8().c_str()));
                }
            }

        if (ruleset.IsValid() && (nullptr == rulesetId || ruleset->GetRuleSetId().Equals(rulesetId)))
            rulesets.push_back(ruleset);
        }
    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalRuleSetLocater::SupplementalRuleSetLocater(RuleSetLocater const& locater)
    : m_locater(&locater)
    {
    m_locater->AddRulesetCallbacksHandler(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalRuleSetLocater::~SupplementalRuleSetLocater()
    {
    m_locater->RemoveRulesetCallbacksHandler(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> SupplementalRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    bvector<PresentationRuleSetPtr> rulesets = m_locater->LocateRuleSets(nullptr);
    auto iter = std::remove_if(rulesets.begin(), rulesets.end(), [](PresentationRuleSetPtr r){return !r->GetIsSupplemental();});
    if (rulesets.end() != iter)
        rulesets.erase(iter, rulesets.end());
    for (PresentationRuleSetPtr ruleset : rulesets)
        ruleset->SetRuleSetId(rulesetId);
    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NonSupplementalRuleSetLocater::NonSupplementalRuleSetLocater(RuleSetLocater const& locater)
    : m_locater(&locater)
    {
    m_locater->AddRulesetCallbacksHandler(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NonSupplementalRuleSetLocater::~NonSupplementalRuleSetLocater()
    {
    m_locater->RemoveRulesetCallbacksHandler(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> NonSupplementalRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    bvector<PresentationRuleSetPtr> rulesets = m_locater->LocateRuleSets(nullptr);
    auto iter = std::remove_if(rulesets.begin(), rulesets.end(), [](PresentationRuleSetPtr r){return r->GetIsSupplemental();});
    if (rulesets.end() != iter)
        rulesets.erase(iter, rulesets.end());
    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> SimpleRuleSetLocater::_LocateRuleSets(Utf8CP ruleSetId) const
    {
    bvector<PresentationRuleSetPtr> ruleSets;
    if (ruleSetId == nullptr)
        {
        for (auto entry : m_cached)
            {
            DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, entry.second->GetRuleSetId().Equals(entry.first), Utf8PrintfString("Expected cached ruleset id '%s' to equal cache key '%s'",
                entry.second->GetRuleSetId().c_str(), entry.first.c_str()));
            ruleSets.push_back(entry.second);
            }
        return ruleSets;
        }

    auto iter = m_cached.find(ruleSetId);
    if (iter != m_cached.end())
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, iter->second->GetRuleSetId().Equals(iter->first), Utf8PrintfString("Expected cached ruleset id '%s' to equal cache key '%s'",
            iter->second->GetRuleSetId().c_str(), iter->first.c_str()));
        ruleSets.push_back(iter->second);
        }

    return ruleSets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> SimpleRuleSetLocater::_GetRuleSetIds() const
    {
    bvector<Utf8String> ruleSetIds;
    for (auto entry : m_cached)
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, entry.second->GetRuleSetId().Equals(entry.first), Utf8PrintfString("Expected cached ruleset id '%s' to equal cache key '%s'",
            entry.second->GetRuleSetId().c_str(), entry.first.c_str()));
        ruleSetIds.push_back(entry.first);
        }
    return ruleSetIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleRuleSetLocater::AddRuleSet(PresentationRuleSetR ruleSet)
    {
    BeMutexHolder lock(GetMutex());
    PresentationRuleSetPtr disposedRuleset;
    auto iter = m_cached.find(ruleSet.GetRuleSetId());
    if (iter != m_cached.end())
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, iter->second->GetRuleSetId().Equals(iter->first), Utf8PrintfString("Expected cached ruleset id '%s' to equal cache key '%s'",
            iter->second->GetRuleSetId().c_str(), iter->first.c_str()));

        if (iter->second->GetHash().Equals(ruleSet.GetHash()))
            return;

        disposedRuleset = iter->second;
        m_cached.erase(iter);
        }

    m_cached.Insert(ruleSet.GetRuleSetId(), &ruleSet);
    lock.unlock();

    if (disposedRuleset.IsValid())
        OnRulesetDisposed(*disposedRuleset);
    OnRulesetCreated(ruleSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleRuleSetLocater::RemoveRuleSet(Utf8StringCR ruleSetId)
    {
    BeMutexHolder lock(GetMutex());
    auto iter = m_cached.find(ruleSetId);
    if (iter != m_cached.end())
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, iter->second->GetRuleSetId().Equals(iter->first), Utf8PrintfString("Expected cached ruleset id '%s' to equal cache key '%s'",
            iter->second->GetRuleSetId().c_str(), iter->first.c_str()));
        PresentationRuleSetPtr ruleset = iter->second;
        m_cached.erase(iter);
        lock.unlock();
        OnRulesetDisposed(*ruleset);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleRuleSetLocater::Clear()
    {
    BeMutexHolder lock(GetMutex());
    bmap<Utf8String, PresentationRuleSetPtr> rulesets = m_cached;
    m_cached.clear();
    lock.unlock();

    for (auto entry : rulesets)
        OnRulesetDisposed(*entry.second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
EmbeddedRuleSetLocater::EmbeddedRuleSetLocater(IConnectionCR connection)
    : m_connection(connection)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void EmbeddedRuleSetLocater::LoadRuleSets() const
    {
    if (!m_connection.GetECDb().Schemas().ContainsSchema(PRESENTATION_RULESET_SCHEMA_NAME))
        return;

    bvector<RulesetJsonRecord> rulesets = QueryRulesets();
    for (RulesetJsonRecord const& entry : rulesets)
        {
        BeInt64Id elementId = entry.first.first;
        DateTime lastMod = entry.first.second;
        Json::Value json = Json::Value::From(entry.second);

        // note: we used to put the ruleset under a `jsonProperties` attribute and now we don't, but we still need to support both formats:
        // - old format: JsonProperties = { jsonProperties: RULESET_JSON }
        // - new format: JsonProperties = RULESET_JSON
        JsonValueCR rulesetJson = json["jsonProperties"].isNull() ? json : json["jsonProperties"];

        if (rulesetJson.isNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Ruleset element %" PRIu64 " doesn't store a valid ruleset definition", elementId.GetValue()));

        PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromJsonValue(rulesetJson);
        if (ruleset.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Ruleset element %" PRIu64 " doesn't store a valid ruleset definition", elementId.GetValue()));

        m_cache.push_back(RulesetRecord(RulesetInfo(elementId, lastMod), ruleset));
        OnRulesetCreated(*ruleset);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> EmbeddedRuleSetLocater::GetCachedRuleSets() const
    {
    bvector<PresentationRuleSetPtr> results = bvector<PresentationRuleSetPtr>();
    for (RulesetRecord record : m_cache)
        results.push_back(record.second);

    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<EmbeddedRuleSetLocater::RulesetJsonRecord> EmbeddedRuleSetLocater::QueryRulesets() const
    {
    if (!m_connection.GetECDb().Schemas().ContainsSchema(PRESENTATION_RULESET_SCHEMA_NAME))
        return bvector<RulesetJsonRecord>();

    Utf8CP sql = "SELECT ECInstanceId, JsonProperties, LastMod FROM " PRESENTATION_RULESET_FULL_CLASS_NAME;

    ECSqlStatement stmt = ECSqlStatement();
    stmt.Prepare(m_connection.GetECDb(), sql);
    if (!stmt.IsPrepared())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to prepare ruleset elements selection query");

    bvector<RulesetJsonRecord> rulesets = bvector<RulesetJsonRecord>();
    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(stmt))
        {
        BeInt64Id instanceId = stmt.GetValueId<BeInt64Id>(0);
        Utf8String jsonProps = stmt.GetValueText(1);
        DateTime lastModified = stmt.GetValueDateTime(2);
        rulesets.push_back(RulesetJsonRecord(RulesetInfo(instanceId, lastModified), jsonProps));
        }

    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
EmbeddedRuleSetLocater::RulesetJsonRecord EmbeddedRuleSetLocater::QueryRulesetById(BeInt64Id id) const
    {
    if (!m_connection.GetECDb().Schemas().ContainsSchema(PRESENTATION_RULESET_SCHEMA_NAME))
        return RulesetJsonRecord();

    Utf8CP sql = "SELECT jsonProperties, LastMod FROM " PRESENTATION_RULESET_FULL_CLASS_NAME " WHERE ECInstanceId = ?";

    ECSqlStatement stmt = ECSqlStatement();
    stmt.Prepare(m_connection.GetECDb(), sql);
    if (!stmt.IsPrepared())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to prepare ruleset elements selection query");

    stmt.BindId(1, id);

    if (BE_SQLITE_ROW == QueryExecutorHelper::Step(stmt))
        {
        Utf8String jsonProps = stmt.GetValueText(0);
        DateTime lastModified = stmt.GetValueDateTime(1);
        return RulesetJsonRecord(RulesetInfo(id, lastModified), jsonProps);
        }

    return RulesetJsonRecord();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> EmbeddedRuleSetLocater::_GetRuleSetIds() const
    {
    bvector<Utf8String> ids;
    for (PresentationRuleSetPtr ruleset : LocateRuleSets())
        ids.push_back(ruleset->GetRuleSetId().c_str());

    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void EmbeddedRuleSetLocater::_InvalidateCache(Utf8CP rulesetId)
    {
    for (PresentationRuleSetPtr ruleset : GetCachedRuleSets())
        OnRulesetDisposed(*ruleset);

    m_cache.clear();
    LoadRuleSets();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Ruleset element %" PRIu64 " doesn't store a valid ruleset definition", rulesetId.GetValue()));

        OnRulesetDisposed(*recordR.second);
        recordR.second = ruleset;
        OnRulesetCreated(*recordR.second);
        recordR.first.second = lastModified;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t EmbeddedRuleSetLocater::QueryRuleSetCount()
    {
    if (!m_connection.GetECDb().Schemas().ContainsSchema(PRESENTATION_RULESET_SCHEMA_NAME))
        return 0;

    Utf8String sql("SELECT count(*) FROM " PRESENTATION_RULESET_FULL_CLASS_NAME);

    ECSqlStatement stmt = ECSqlStatement();
    stmt.Prepare(m_connection.GetECDb(), sql.c_str());
    if (!stmt.IsPrepared())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to prepare ruleset elements count selection query");

    if (BE_SQLITE_ROW != QueryExecutorHelper::Step(stmt))
        return 0;

    return stmt.GetValueInt(0);
    }
