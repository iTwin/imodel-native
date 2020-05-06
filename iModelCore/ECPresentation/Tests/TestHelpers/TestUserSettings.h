/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    mutable BeMutex m_mutex;

    void NotifySettingChanged(Utf8CP settingId)
        {
        BeMutexHolder lock(m_mutex);
        if (nullptr != m_changesListener)
            m_changesListener->_OnSettingChanged("", settingId);
        }

    Utf8String GetSettingType(JsonValueCR setting) const
        {
        switch (setting.type())
            {
            case Json::ValueType::intValue:
            case Json::ValueType::uintValue:
                return "int";
            case Json::ValueType::booleanValue:
                return "bool";
            case Json::ValueType::arrayValue:
                return "ints";
            case Json::ValueType::stringValue:
                return "string";
            default:
                BeAssert(false);
                return "";
            }
        }
    
protected:
    Json::Value _GetPresentationInfo(Utf8StringCR) const override {return Json::Value();}

    bvector<bpair<Utf8String, Utf8String>> _GetSettings() const override 
        {
        bvector<bpair<Utf8String, Utf8String>> settings;
        for (Utf8StringCR name : m_values.getMemberNames())
            settings.push_back(bpair<Utf8String, Utf8String>(name, GetSettingType(m_values[name])));
        return settings;
        }
    bool _HasSetting(Utf8CP id) const override {BeMutexHolder lock(m_mutex); return m_values.isMember(id);}

    void _InitFrom(UserSettingsGroupList const&) override {}
    void _SetSettingValue(Utf8CP id, Utf8CP value) override {BeMutexHolder lock(m_mutex); m_values[id] = value; NotifySettingChanged(id);}
    void _SetSettingBoolValue(Utf8CP id, bool value) override {BeMutexHolder lock(m_mutex); m_values[id] = value; NotifySettingChanged(id);}
    void _SetSettingIntValue(Utf8CP id, int64_t value) override {BeMutexHolder lock(m_mutex); m_values[id] = Json::Value(value); NotifySettingChanged(id);}
    void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) override
        {
        BeMutexHolder lock(m_mutex);
        for (int64_t v : values)
            m_values[id].append(Json::Value(v));
        NotifySettingChanged(id);
        }    

    Utf8String _GetSettingValue(Utf8CP id) const override {BeMutexHolder lock(m_mutex); return m_values.isMember(id) ? m_values[id].asCString() : "";}
    bool _GetSettingBoolValue(Utf8CP id) const override {BeMutexHolder lock(m_mutex); return m_values.isMember(id) ? m_values[id].asBool() : false;}
    int64_t _GetSettingIntValue(Utf8CP id) const override {BeMutexHolder lock(m_mutex); return m_values.isMember(id) ? m_values[id].asInt64() : 0;}
    bvector<int64_t> _GetSettingIntValues(Utf8CP id) const override
        {
        BeMutexHolder lock(m_mutex);
        bvector<int64_t> values;
        if (m_values.isMember(id))
            {
            JsonValueCR jsonArr = m_values[id];
            for (Json::ArrayIndex i = 0; i < jsonArr.size(); ++i)
                values.push_back(jsonArr[i].asInt64());
            }
        return values;
        }    
    Json::Value _GetSettingValueAsJson(Utf8CP id) const override {BeMutexHolder lock(m_mutex); return m_values.isMember(id) ? m_values[id] : Json::Value(Json::nullValue);}

public:
    TestUserSettings() : m_changesListener(nullptr) {}
    TestUserSettings(TestUserSettings const& other) : m_values(other.m_values), m_changesListener(other.m_changesListener) {}
    TestUserSettings& operator=(TestUserSettings const& other)
        {
        m_values = other.m_values;
        m_changesListener = other.m_changesListener;
        return *this;
        }
    void SetChangesListener(IUserSettingsChangeListener* listener) {BeMutexHolder lock(m_mutex); m_changesListener = listener;}
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
