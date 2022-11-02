/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "TestRuleSetLocater.h"

USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> TestRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    if (nullptr == rulesetId)
        return m_rulesets;

    bvector<PresentationRuleSetPtr> rulesets;
    for (PresentationRuleSetPtr ruleset : m_rulesets)
        {
        if (ruleset->GetRuleSetId().Equals(rulesetId))
            rulesets.push_back(ruleset);
        }
    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> TestRuleSetLocater::_GetRuleSetIds() const
    {
    bvector<Utf8String> ids;
    for (PresentationRuleSetPtr ruleset : m_rulesets)
        ids.push_back(ruleset->GetRuleSetId());
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRuleSetLocater::_InvalidateCache(Utf8CP rulesetId)
    {
    if (nullptr == rulesetId)
        {
        Clear();
        return;
        }

    for (int i = (int)m_rulesets.size() - 1; i >= 0; --i)
        {
        PresentationRuleSetPtr ruleset = m_rulesets[i];
        if (ruleset->GetRuleSetId().Equals(rulesetId))
            {
            OnRulesetDisposed(*ruleset);
            m_rulesets.erase(m_rulesets.begin() + i);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRuleSetLocater::AddRuleSet(PresentationRuleSetR ruleset)
    {
    m_rulesets.push_back(&ruleset);
    OnRulesetCreated(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRuleSetLocater::Clear()
    {
    for (PresentationRuleSetPtr const& ruleset : m_rulesets)
        OnRulesetDisposed(*ruleset);
    m_rulesets.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TestRuleSetLocater::~TestRuleSetLocater()
    {
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DelayLoadingRuleSetLocater::DelayLoadingRuleSetLocater(PresentationRuleSetP ruleset)
    {
    if (nullptr != ruleset)
        m_rulesets.Insert(ruleset, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationRuleSetPtr> DelayLoadingRuleSetLocater::_LocateRuleSets(Utf8CP rulesetId) const
    {
    bvector<PresentationRuleSetPtr> rulesets;
    for (auto& entry : m_rulesets)
        {
        if (!rulesetId || entry.first->GetRuleSetId().Equals(rulesetId))
            {
            if (!entry.second)
                {
                OnRulesetCreated(*entry.first);
                entry.second = true;
                }
            rulesets.push_back(entry.first);
            }
        }
    return rulesets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> DelayLoadingRuleSetLocater::_GetRuleSetIds() const
    {
    bvector<Utf8String> ids;
    bvector<PresentationRuleSetPtr> rulesets = LocateRuleSets();
    for (PresentationRuleSetPtr ruleset : rulesets)
        ids.push_back(ruleset->GetRuleSetId());
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DelayLoadingRuleSetLocater::AddRuleSet(PresentationRuleSetR ruleset)
    {
    m_rulesets.Insert(&ruleset, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DelayLoadingRuleSetLocater::Reset()
    {
    for (auto& entry : m_rulesets)
        {
        if (entry.second)
            {
            OnRulesetDisposed(*entry.first);
            entry.second = false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DelayLoadingRuleSetLocater::Clear()
    {
    Reset();
    m_rulesets.clear();
    }
