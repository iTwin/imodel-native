/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "LocalizationHelper.h"
#include "RulesPreprocessor.h"
#include <regex>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalizationHelper::LocalizeString(Utf8StringR str) const
    {
    std::regex expression("(@[\\w\\d\\-_]+:[\\w\\d\\-\\._]+?@)", std::regex_constants::ECMAScript);
    std::cmatch matches;
    std::regex_search(str.c_str(), matches, expression);

    if (0 == matches.size())
        return true;

    bool didLocalize = false;
    for (size_t i = 1; i < matches.size(); i++)
        {
        std::csub_match const& match = matches[i];

        Utf8String defaultValue;
        Utf8String key(match.first, match.second);
        key.Trim("@");

        if (nullptr != m_ruleset)
            {
            LocalizationResourceKeyDefinitionCP localizationResouceKeyDefinition = RulesPreprocessor::GetLocalizationResourceKeyDefinition(key, *m_ruleset);
            if (nullptr != localizationResouceKeyDefinition)
                {
                key = localizationResouceKeyDefinition->GetKey();
                defaultValue = localizationResouceKeyDefinition->GetDefaultValue();
                }
            }

        Utf8String localizedString;
        if (!m_provider.GetString(m_locale, key, localizedString))
            localizedString = defaultValue;

        if (!localizedString.empty())
            {
            size_t replacementsCount = str.ReplaceAll(match.str().c_str(), localizedString.c_str());
            BeAssert(replacementsCount > 0);
            UNUSED_VARIABLE(replacementsCount);
            didLocalize = true;
            }
        }

    return didLocalize;
    }
