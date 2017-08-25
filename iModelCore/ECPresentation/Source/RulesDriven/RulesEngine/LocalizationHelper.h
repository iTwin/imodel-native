/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/LocalizationHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Localization.h>
#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define RULESENGINE_LOCALIZEDSTRING(str)            PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, str)
#define RULESENGINE_LOCALIZEDSTRING_NotSpecified    RULESENGINE_LOCALIZEDSTRING(RulesEngineL10N::LABEL_General_NotSpecified().m_str)
#define RULESENGINE_LOCALIZEDSTRING_Other           RULESENGINE_LOCALIZEDSTRING(RulesEngineL10N::LABEL_General_Other().m_str)

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE SQLangLocalizationProvider : ILocalizationProvider
{
private:
    static StatusInt ParseKey(Utf8StringR ns, Utf8StringR id, Utf8StringCR key);
protected:
    ECPRESENTATION_EXPORT bool _GetString(Utf8StringCR key, Utf8StringR localizedValue) const override;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct LocalizationHelper
{
private:
    ECN::PresentationRuleSetCP m_ruleset;
    ILocalizationProvider const& m_provider;

public:
    LocalizationHelper(ILocalizationProvider const& provider, ECN::PresentationRuleSetCP ruleset = nullptr) : m_ruleset(ruleset), m_provider(provider) {}
    ECPRESENTATION_EXPORT bool LocalizeString(Utf8StringR str) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
