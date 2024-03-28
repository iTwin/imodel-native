/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "TestHelpers.h"
#include "../../../Source/PresentationManagerImpl.h"
#include "TestNodesCache.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImplBase : ECPresentationManager::Impl, IRulesetCallbacksHandler, IUserSettingsChangeListener
{
private:
    std::shared_ptr<IConnectionManager> m_connections;
    std::shared_ptr<IRulesetLocaterManager> m_locaters;
    std::shared_ptr<IUserSettingsManager> m_userSettings;
    std::shared_ptr<IJsonLocalState> m_localState;
    std::shared_ptr<IECPropertyFormatter const> m_ecPropertyFormatter;
    std::shared_ptr<IPropertyCategorySupplier const> m_categorySupplier;
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> m_ecInstanceChangeEventSources;
    std::shared_ptr<TestNodesCache> m_nodesCache;
protected:
    IPropertyCategorySupplier const& _GetCategorySupplier() const override { return *m_categorySupplier; }
    IECPropertyFormatter const& _GetECPropertyFormatter() const override { return *m_ecPropertyFormatter; }
    IUserSettingsManager& _GetUserSettingsManager() const override { return *m_userSettings; }
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& _GetECInstanceChangeEventSources() const override { return m_ecInstanceChangeEventSources; }
    IJsonLocalState* _GetLocalState() const override { return m_localState.get(); }
    IRulesetLocaterManager& _GetLocaters() const override { return *m_locaters; }
    IConnectionManagerR _GetConnections() override { return *m_connections; }
    void _OnRulesetCreated(RuleSetLocaterCR, PresentationRuleSetR) override {}
    void _OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetR) override {}
    void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override {}
    std::shared_ptr<INavNodesCache> _GetHierarchyCache(Utf8StringCR connectionId) const override {return m_nodesCache;}
public:
    RulesDrivenECPresentationManagerImplBase(ECPresentationManager::Impl::Params const& params)
        : m_connections(params.GetConnections())
        {
        static std::shared_ptr<TestPropertyFormatter const> const s_defaultPropertyFormatter = std::make_shared<TestPropertyFormatter>();
        static std::shared_ptr<TestCategorySupplier const> const s_defaultCategorySupplier = std::make_shared<TestCategorySupplier>();

        m_locaters = params.GetRulesetLocaters();
        m_userSettings = params.GetUserSettings();
        m_localState = params.GetLocalState();
        m_ecPropertyFormatter = params.GetECPropertyFormatter() ? params.GetECPropertyFormatter() : s_defaultPropertyFormatter;
        m_categorySupplier = params.GetCategorySupplier() ? params.GetCategorySupplier() : s_defaultCategorySupplier;
        m_ecInstanceChangeEventSources = params.GetECInstanceChangeEventSources();

        m_locaters->SetRulesetCallbacksHandler(this);
        m_userSettings->SetChangesListener(this);

        m_nodesCache = std::make_shared<TestNodesCache>();
        }
    TestNodesCache& GetNodesCache() {return *m_nodesCache;}
};

END_ECPRESENTATIONTESTS_NAMESPACE
