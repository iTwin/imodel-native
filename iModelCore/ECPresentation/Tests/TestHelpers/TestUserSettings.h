/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/TestUserSettings.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationTest.h"
#include <ECPresentation/RulesDriven/UserSettings.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2017
+===============+===============+===============+===============+===============+======*/
struct TestUserSettings : IUserSettings
    {
    Json::Value _GetPresentationInfo() const override {return Json::Value();}

    bool _HasSetting(Utf8CP id) const override {return false;}

    void _SetSettingValue(Utf8CP id, Utf8CP value) override {}
    void _SetSettingIntValue(Utf8CP id, int64_t value) override {}
    void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) override {}
    void _SetSettingBoolValue(Utf8CP id, bool value) override {}

    Utf8String _GetSettingValue(Utf8CP id) const override {return "";}
    int64_t _GetSettingIntValue(Utf8CP id) const override {return 0;}
    bvector<int64_t> _GetSettingIntValues(Utf8CP id) const override {return bvector<int64_t>();}
    bool _GetSettingBoolValue(Utf8CP id) const override {return false;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2017
+===============+===============+===============+===============+===============+======*/
struct TestUserSettingsManager : IUserSettingsManager
{
private:
    mutable bmap<Utf8String, TestUserSettings> m_settings;
protected:
    IUserSettings& _GetSettings(Utf8StringCR id) const override
        {
        auto iter = m_settings.find(id);
        if (m_settings.end() == iter)
            iter = m_settings.Insert(id, TestUserSettings()).first;
        return iter->second;
        }
};