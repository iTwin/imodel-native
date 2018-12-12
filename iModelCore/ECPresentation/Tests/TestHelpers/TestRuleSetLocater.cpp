/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/TestHelpers/TestRuleSetLocater.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "TestRuleSetLocater.h"

USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
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
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> TestRuleSetLocater::_GetRuleSetIds() const
    {
    bvector<Utf8String> ids;
    for (PresentationRuleSetPtr ruleset : m_rulesets)
        ids.push_back(Utf8String(ruleset->GetRuleSetId().c_str()).c_str());
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
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
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRuleSetLocater::AddRuleSet(PresentationRuleSetR ruleset)
    {
    m_rulesets.push_back(&ruleset);
    OnRulesetCreated(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRuleSetLocater::Clear()
    {
    for (PresentationRuleSetPtr const& ruleset : m_rulesets)
        OnRulesetDisposed(*ruleset);
    m_rulesets.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TestRuleSetLocater::~TestRuleSetLocater()
    {
    Clear();
    }
