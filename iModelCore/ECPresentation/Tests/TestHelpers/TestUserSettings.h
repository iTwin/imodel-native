/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/TestUserSettings.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationTest.h"
#include <ECPresentation/RulesDriven/UserSettings.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct TestUserSettings : IUserSettings
{
private:
    Json::Value m_values;
    IUserSettingsChangeListener* m_changesListener;

    void NotifySettingChanged(Utf8CP settingId)
        {
        if (nullptr != m_changesListener)
            m_changesListener->_OnSettingChanged("", settingId);
        }
    
protected:
    Json::Value _GetPresentationInfo(Utf8StringCR) const override {return Json::Value();}

    bool _HasSetting(Utf8CP id) const override {return m_values.isMember(id);}

    void _InitFrom(UserSettingsGroupList const&) override {}
    void _SetSettingValue(Utf8CP id, Utf8CP value) override {m_values[id] = value; NotifySettingChanged(id);}
    void _SetSettingBoolValue(Utf8CP id, bool value) override {m_values[id] = value; NotifySettingChanged(id);}
    void _SetSettingIntValue(Utf8CP id, int64_t value) override {m_values[id] = Json::Value(value); NotifySettingChanged(id);}
    void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) override
        {
        for (int64_t v : values)
            m_values[id].append(Json::Value(v));
        NotifySettingChanged(id);
        }    

    Utf8String _GetSettingValue(Utf8CP id) const override {return m_values.isMember(id) ? m_values[id].asCString() : "";}
    bool _GetSettingBoolValue(Utf8CP id) const override {return m_values.isMember(id) ? m_values[id].asBool() : false;}
    int64_t _GetSettingIntValue(Utf8CP id) const override {return m_values.isMember(id) ? m_values[id].asInt64() : 0;}
    bvector<int64_t> _GetSettingIntValues(Utf8CP id) const override
        {
        bvector<int64_t> values;
        if (m_values.isMember(id))
            {
            JsonValueCR jsonArr = m_values[id];
            for (Json::ArrayIndex i = 0; i < jsonArr.size(); ++i)
                values.push_back(jsonArr[i].asInt64());
            }
        return values;
        }    
    Json::Value _GetSettingValueAsJson(Utf8CP id) const override {return m_values.isMember(id) ? m_values[id] : Json::Value(Json::nullValue);}

public:
    TestUserSettings() : m_changesListener(nullptr) {}
    void SetChangesListener(IUserSettingsChangeListener* listener) {m_changesListener = listener;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2017
+===============+===============+===============+===============+===============+======*/
struct TestUserSettingsManager : IUserSettingsManager
{
private:
    mutable BeMutex m_mutex;
    mutable bmap<Utf8String, TestUserSettings> m_settings;
protected:
    IUserSettings& _GetSettings(Utf8StringCR id) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_settings.find(id);
        if (m_settings.end() == iter)
            iter = m_settings.Insert(id, TestUserSettings()).first;
        return iter->second;
        }
};

END_ECPRESENTATIONTESTS_NAMESPACE
