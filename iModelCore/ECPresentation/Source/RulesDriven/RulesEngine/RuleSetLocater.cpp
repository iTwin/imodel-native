/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/RuleSetLocater.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <Bentley/BeDirectoryIterator.h>
#include "ECSchemaHelper.h"

#define PRESENTATION_RULESET_EXTENSION  L".PresentationRuleSet.xml"
#define PRESENTATION_RULESET_WILDCARD   L"*" PRESENTATION_RULESET_EXTENSION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::RegisterLocater(RuleSetLocater& locater)
    {
    locater.SetRulesetCallbacksHandler(this);
    m_locaters.push_back(&locater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::UnregisterLocater(RuleSetLocater const& locater)
    {
    auto iter = m_locaters.begin();
    for (; iter != m_locaters.end(); iter++)
        {
        RuleSetLocater* candidate = (*iter).get();
        if (candidate == &locater)
            break;
        }
    if (iter == m_locaters.end())
        {
        BeAssert(false);
        return;
        }
    (*iter)->SetRulesetCallbacksHandler(nullptr);
    m_locaters.erase(iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::SetRulesetCallbacksHandler(IRulesetCallbacksHandler* handler) {m_rulesetCallbacksHandler = handler;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_OnRulesetDispose(PresentationRuleSetCR ruleset)
    {
    if (nullptr != m_rulesetCallbacksHandler)
        m_rulesetCallbacksHandler->_OnRulesetDispose(ruleset);
    m_rulesetsCache.erase(ruleset.GetRuleSetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::_OnRulesetCreated(PresentationRuleSetCR ruleset)
    {
    if (nullptr != m_rulesetCallbacksHandler)
        m_rulesetCallbacksHandler->_OnRulesetCreated(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocaterManager::InvalidateCache(Utf8CP rulesetId)
    {
    for (RuleSetLocaterPtr const& locater : m_locaters)
        locater->InvalidateCache(rulesetId);

    if (nullptr != rulesetId)
        m_rulesetsCache.erase(rulesetId);
    else
        m_rulesetsCache.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> RuleSetLocaterManager::LocateSupportedRulesets(ECDbCR connection) const
    {
    RelatedPathsCache relatedPathsCache;
    ECSchemaHelper helper(connection, relatedPathsCache, nullptr);

    bvector<RuleSetLocaterPtr> sortedLocaters = m_locaters;
    std::sort(sortedLocaters.begin(), sortedLocaters.end(), [](RuleSetLocaterPtr a, RuleSetLocaterPtr b)
        {
        if (a->GetPriority() < b->GetPriority())
            return -1;
        if (a->GetPriority() > b->GetPriority())
            return 1;
        return 0;
        });

    bmap<Utf8String, PresentationRuleSetPtr> locatedRulesets;
    for (RuleSetLocaterPtr& locater : sortedLocaters)
        {
        bvector<Utf8String> ids = locater->GetRuleSetIds();
        for (Utf8StringCR id : ids)
            {
            bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets(id.c_str());
            for (PresentationRuleSetPtr& ruleset : rulesets)
                {
                if (locatedRulesets.end() == locatedRulesets.find(ruleset->GetFullRuleSetId())
                    && !ruleset->GetIsSupplemental()
                    && helper.AreSchemasSupported(ruleset->GetSupportedSchemas()))
                    {
                    locatedRulesets[ruleset->GetFullRuleSetId()] = ruleset;
                    }
                }
            }
        }

    bvector<PresentationRuleSetPtr> results;
    for (auto& ruleset : locatedRulesets)
        {
        results.push_back(ruleset.second);
        m_rulesetsCache[ruleset.second->GetRuleSetId()].push_back(ruleset.second);
        }

    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> RuleSetLocaterManager::LocateRuleSets(Utf8CP rulesetId) const
    {
    auto cacheIter = m_rulesetsCache.find(rulesetId);
    if (m_rulesetsCache.end() != cacheIter)
        return cacheIter->second;

    bvector<RuleSetLocaterPtr> sortedLocaters = m_locaters;
    std::sort(sortedLocaters.begin(), sortedLocaters.end(), [](RuleSetLocaterPtr a, RuleSetLocaterPtr b)
        {
        if (a->GetPriority() < b->GetPriority())
            return -1;
        if (a->GetPriority() > b->GetPriority())
            return 1;
        return 0;
        });

    bmap<Utf8String, PresentationRuleSetPtr> locatedRulesets;
    for (RuleSetLocaterPtr& locater : sortedLocaters)
        {
        bvector<PresentationRuleSetPtr> rulesets = locater->LocateRuleSets(rulesetId);
        for (PresentationRuleSetPtr& ruleset : rulesets)
            {
            if (locatedRulesets.end() == locatedRulesets.find(ruleset->GetFullRuleSetId()))
                locatedRulesets[ruleset->GetFullRuleSetId()] = ruleset;
            }
        }

    bvector<PresentationRuleSetPtr> results;
    for (auto& ruleset : locatedRulesets)
        {
        results.push_back(ruleset.second);
        m_rulesetsCache[ruleset.second->GetRuleSetId()].push_back(ruleset.second);
        }

    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> RuleSetLocaterManager::GetRuleSetIds() const
    {
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
bvector<ECN::PresentationRuleSetPtr> RuleSetLocater::LocateRuleSets(Utf8CP rulesetId) const {return _LocateRuleSets(rulesetId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> RuleSetLocater::GetRuleSetIds() const {return _GetRuleSetIds();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int RuleSetLocater::GetPriority() const {return _GetPriority();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::OnRulesetDisposed(PresentationRuleSetCR ruleset) const
    {
    if (nullptr != m_rulesetCallbacksHandler)
        m_rulesetCallbacksHandler->_OnRulesetDispose(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::OnRulesetCreated(PresentationRuleSetCR ruleset) const
    {
    if (nullptr != m_rulesetCallbacksHandler)
        m_rulesetCallbacksHandler->_OnRulesetCreated(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RuleSetLocater::SetRulesetCallbacksHandler(IRulesetCallbacksHandler* handler) {m_rulesetCallbacksHandler = handler;}

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
        BeDirectoryIterator::WalkDirsAndMatch(matches, directory, PRESENTATION_RULESET_WILDCARD, true);
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
    for (BeFileName& file : files)
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
            ruleset = PresentationRuleSet::ReadFromXmlFile(file.c_str());
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

    PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromXmlFile(m_path.c_str());
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
    bvector<PresentationRuleSetPtr> rulesets = DirectoryRuleSetLocater::_LocateRuleSets(nullptr);
    rulesets.erase(std::remove_if(rulesets.begin(), rulesets.end(), [](PresentationRuleSetPtr r){return !r->GetIsSupplemental();}), rulesets.end());
    for (PresentationRuleSetPtr ruleset : rulesets)
        ruleset->SetRuleSetId(rulesetId);
    return rulesets;
    }
