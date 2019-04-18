/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Localization.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define RULESENGINE_LOCALIZEDSTRING(str)            PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, str)
#define RULESENGINE_LOCALIZEDSTRING_NotSpecified    RULESENGINE_LOCALIZEDSTRING(RulesEngineL10N::LABEL_General_NotSpecified().m_str)
#define RULESENGINE_LOCALIZEDSTRING_Other           RULESENGINE_LOCALIZEDSTRING(RulesEngineL10N::LABEL_General_Other().m_str)

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct LocalizationHelper
{
private:
    PresentationRuleSetCP m_ruleset;
    ILocalizationProvider const& m_provider;
    Utf8StringCR m_locale;

public:
    LocalizationHelper(ILocalizationProvider const& provider, Utf8StringCR locale, PresentationRuleSetCP ruleset = nullptr) 
        : m_ruleset(ruleset), m_locale(locale), m_provider(provider)
        {}
    ECPRESENTATION_EXPORT bool LocalizeString(Utf8StringR str) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
