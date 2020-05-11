/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>
#include "TestHelpers.h"
#include "../../../Source/RulesDriven/RulesEngine/PresentationManagerImpl.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImplBase : RulesDrivenECPresentationManager::Impl, IRulesetCallbacksHandler, IUserSettingsChangeListener
{
private:
    std::shared_ptr<IConnectionManager> m_connections;
    std::shared_ptr<IRulesetLocaterManager> m_locaters;
    std::shared_ptr<IUserSettingsManager> m_userSettings;
    IJsonLocalState* m_localState;
    IECPropertyFormatter const* m_ecPropertyFormatter;
    IPropertyCategorySupplier const* m_categorySupplier;
    ILocalizationProvider const* m_localizationProvider;
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> m_ecInstanceChangeEventSources;
protected:
    IPropertyCategorySupplier const& _GetCategorySupplier() const override { return *m_categorySupplier; }
    IECPropertyFormatter const& _GetECPropertyFormatter() const override { return *m_ecPropertyFormatter; }
    IUserSettingsManager& _GetUserSettingsManager() const override { return *m_userSettings; }
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& _GetECInstanceChangeEventSources() const override { return m_ecInstanceChangeEventSources; }
    ILocalizationProvider const* _GetLocalizationProvider() const override { return m_localizationProvider; }
    IJsonLocalState* _GetLocalState() const override { return m_localState; }
    IRulesetLocaterManager& _GetLocaters() const override { return *m_locaters; }
    IConnectionManagerR _GetConnections() override { return *m_connections; }
    void _OnRulesetCreated(RuleSetLocaterCR, PresentationRuleSetR) override {}
    void _OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetR) override {}
    void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override {}
public:
    RulesDrivenECPresentationManagerImplBase(RulesDrivenECPresentationManager::Impl::Params const& params)
        : m_connections(params.GetConnections())
        {
        static const TestPropertyFormatter s_defaultPropertyFormatter;
        static const TestCategorySupplier s_defaultCategorySupplier;

        m_locaters = params.GetRulesetLocaters();
        m_userSettings = params.GetUserSettings();
        m_localState = params.GetLocalState();
        m_ecPropertyFormatter = params.GetECPropertyFormatter() ? params.GetECPropertyFormatter() : &s_defaultPropertyFormatter;
        m_categorySupplier = params.GetCategorySupplier() ? params.GetCategorySupplier() : &s_defaultCategorySupplier;
        m_localizationProvider = params.GetLocalizationProvider();
        m_ecInstanceChangeEventSources = params.GetECInstanceChangeEventSources();

        m_locaters->SetRulesetCallbacksHandler(this);
        m_userSettings->SetChangesListener(this);
        }
};

END_ECPRESENTATIONTESTS_NAMESPACE
