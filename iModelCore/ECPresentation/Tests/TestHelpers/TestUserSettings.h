/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECPresentationTest.h"
#include <ECPresentation/UserSettings.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestUserSettings : IUserSettings
{
private:
    BeJsDocument m_values;
    IUserSettingsChangeListener* m_changesListener;
    mutable BeMutex m_mutex;

    void NotifySettingChanged(Utf8CP settingId)
        {
        BeMutexHolder lock(m_mutex);
        if (nullptr != m_changesListener)
            m_changesListener->_OnSettingChanged("", settingId);
        }

    Utf8String GetSettingType(BeJsConst json) const
        {
        if (json.isNumeric()) 
            return "int";
        if (json.isString()) 
            return "string";
        if (json.isBool()) 
            return "bool";
        if (json.isArray()) 
            return "ints";
        BeAssert(false);
        return "";
        }

protected:
    BeJsDocument _GetPresentationInfo() const override {return BeJsDocument();}

    bvector<bpair<Utf8String, Utf8String>> _GetSettings() const override
        {
        bvector<bpair<Utf8String, Utf8String>> settings;
        m_values.ForEachProperty(
            [&](Utf8CP name, BeJsConst json)
            {
            settings.push_back(bpair<Utf8String, Utf8String>(name, GetSettingType(json)));
            return false;
            }
        );
        return settings;
        }
    bool _HasSetting(Utf8CP id) const override {BeMutexHolder lock(m_mutex); return m_values.isMember(id);}

    void _InitFrom(UserSettingsGroupList const&) override {}
    void _UnsetValue(Utf8CP id) override { BeMutexHolder lock(m_mutex); m_values.removeMember(id); NotifySettingChanged(id); }
    void _SetSettingValue(Utf8CP id, Utf8CP value) override { BeMutexHolder lock(m_mutex); m_values[id] = value; NotifySettingChanged(id); }
    void _SetSettingBoolValue(Utf8CP id, bool value) override {BeMutexHolder lock(m_mutex); m_values[id] = value; NotifySettingChanged(id);}
    void _SetSettingIntValue(Utf8CP id, int64_t value) override {BeMutexHolder lock(m_mutex); m_values[id] = value; NotifySettingChanged(id);}
    void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) override
        {
        BeMutexHolder lock(m_mutex);
        for (int64_t v : values)
            m_values[id][m_values[id].size()] = v;
        NotifySettingChanged(id);
        }

    Utf8String _GetSettingValue(Utf8CP id) const override {BeMutexHolder lock(m_mutex); return m_values.isMember(id) ? m_values[id].asCString() : "";}
    bool _GetSettingBoolValue(Utf8CP id) const override {BeMutexHolder lock(m_mutex); return m_values.isMember(id) ? m_values[id].asBool() : false;}
    int64_t _GetSettingIntValue(Utf8CP id) const override {BeMutexHolder lock(m_mutex); return m_values.isMember(id) ? m_values[id].asInt64() : 0;}
    bvector<int64_t> _GetSettingIntValues(Utf8CP id) const override
        {
        BeMutexHolder lock(m_mutex);
        bvector<int64_t> values;
        if (m_values.hasMember(id))
            {
            BeJsConst jsonArr = m_values[id];
            for (BeJsConst::ArrayIndex i = 0; i < jsonArr.size(); ++i)
                values.push_back(jsonArr[i].asInt64());
            }
        return values;
        }
    BeJsDocument _GetSettingValueAsJson(Utf8CP id) const override {
        BeMutexHolder lock(m_mutex);
        BeJsDocument json;
        if (m_values.hasMember(id))
            {
            json.From(m_values[id]);
            }
        return json;
        }

public:
    TestUserSettings() : m_changesListener(nullptr) {}
    TestUserSettings(TestUserSettings const& other) : m_changesListener(other.m_changesListener) { m_values.From(other.m_values); }
    TestUserSettings& operator=(TestUserSettings const& other)
        {
        m_values.From(other.m_values);
        m_changesListener = other.m_changesListener;
        return *this;
        }
    void SetChangesListener(IUserSettingsChangeListener* listener) {BeMutexHolder lock(m_mutex); m_changesListener = listener;}
};

/*=================================================================================**//**
* @bsiclass
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
