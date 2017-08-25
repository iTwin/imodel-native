/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/TestLocalizationProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../../../Source/RulesDriven/RulesEngine/LocalizationHelper.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct TestLocalizationProvider : ILocalizationProvider
{
private:
    std::function<bool(Utf8StringCR,Utf8StringR)> m_handler;

protected:
    bool _GetString(Utf8StringCR key, Utf8StringR localizedValue) const override
        {
        if (nullptr == m_handler)
            return false;

        return m_handler(key, localizedValue);
        }

public:
    void SetHandler(std::function<bool(Utf8StringCR,Utf8StringR)> const& handler) {m_handler = handler;}
};
