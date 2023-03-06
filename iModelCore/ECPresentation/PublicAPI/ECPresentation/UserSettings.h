/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RuleSetLocater.h>
#include <ECPresentation/Rules/PresentationRules.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct UserSettingsManager;

//=======================================================================================
//! An interface for a listener that's notified when a user setting changes.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
struct IUserSettingsChangeListener
    {
    //! Virtual destructor.
    virtual ~IUserSettingsChangeListener() {}

    //! Called when a user setting changes.
    //! @param[in] rulesetId The ID of the ruleset whose user setting changed.
    //! @param[in] settingId The ID of the user setting that changed.
    virtual void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const = 0;
    };

//=======================================================================================
//! Handles user settings' storage and retrieval.
//! @note The implementation must be thread safe.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
struct IUserSettings
{
protected:
    virtual ~IUserSettings() {}

    virtual BeJsDocument _GetPresentationInfo() const = 0;

    virtual bvector<bpair<Utf8String, Utf8String>> _GetSettings() const = 0;

    virtual bool _HasSetting(Utf8CP id) const = 0;

    virtual void _InitFrom(UserSettingsGroupList const&) = 0;
    virtual void _UnsetValue(Utf8CP id) = 0;
    virtual void _SetSettingValue(Utf8CP id, Utf8CP value) = 0;
    virtual void _SetSettingIntValue(Utf8CP id, int64_t value) = 0;
    virtual void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) = 0;
    virtual void _SetSettingBoolValue(Utf8CP id, bool value) = 0;

    virtual Utf8String _GetSettingValue(Utf8CP id) const = 0;
    virtual int64_t _GetSettingIntValue(Utf8CP id) const = 0;
    virtual bvector<int64_t> _GetSettingIntValues(Utf8CP id) const = 0;
    virtual bool _GetSettingBoolValue(Utf8CP id) const = 0;
    virtual BeJsDocument _GetSettingValueAsJson(Utf8CP id) const = 0;

public:
    //! Get the presentation info for user settings stored in this instance.
    BeJsDocument GetPresentationInfo() const {return _GetPresentationInfo();}

    //! Get existing settings ids and types
    bvector<bpair<Utf8String, Utf8String>> GetSettings() const {return _GetSettings();}

    //! Is there a user setting with the specified id.
    bool HasSetting(Utf8CP id) const {return _HasSetting(id);}

    //! Initialize settings from presentation rules.
    void InitFrom(UserSettingsGroupList const& rules) { _InitFrom(rules); }
    //! Unset a value
    void UnsetValue(Utf8CP id) {_UnsetValue(id);}
    //! Set a setting value.
    void SetSettingValue(Utf8CP id, Utf8CP value) {_SetSettingValue(id, value);}
    //! Set a setting value.
    void SetSettingIntValue(Utf8CP id, int64_t value) {_SetSettingIntValue(id, value);}
    //! Set a setting value.
    void SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) {_SetSettingIntValues(id, values);}
    //! Set a setting value.
    void SetSettingBoolValue(Utf8CP id, bool value) {_SetSettingBoolValue(id, value);}

    //! Get a setting value.
    Utf8String GetSettingValue(Utf8CP id) const {return _GetSettingValue(id);}
    //! Get a setting value.
    int64_t GetSettingIntValue(Utf8CP id) const {return _GetSettingIntValue(id);}
    //! Get a setting value.
    bvector<int64_t> GetSettingIntValues(Utf8CP id) const {return _GetSettingIntValues(id);}
    //! Get a setting value.
    bool GetSettingBoolValue(Utf8CP id) const {return _GetSettingBoolValue(id);}
    //! Get a setting value.
    BeJsDocument GetSettingValueAsJson(Utf8CP id) const {return _GetSettingValueAsJson(id);}
};

//=======================================================================================
//! Handles user settings' storage and retrieval.
//! @note The implementation must be thread safe.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
struct UserSettings : IUserSettings
{
friend struct UserSettingsManager;

private:
    Utf8String m_rulesetId;
    IJsonLocalState* m_localState;
    BeJsDocument m_presentationInfo;
    IUserSettingsChangeListener const* m_changeListener;
    bool m_isInitializing;
    UserSettingsGroupList const* m_initializedFrom;
    bset<bpair<Utf8String, Utf8String>> m_settingIds;
    mutable BeMutex m_mutex;

private:
    void AddValues(BeJsValue) const;
    void InitFromJson(UserSettingsGroupList const& rules, BeJsValue presentationObj);

protected:
    ECPRESENTATION_EXPORT BeJsDocument _GetPresentationInfo() const override;
    ECPRESENTATION_EXPORT bvector<bpair<Utf8String, Utf8String>> _GetSettings() const override;
    ECPRESENTATION_EXPORT bool _HasSetting(Utf8CP id) const override;
    ECPRESENTATION_EXPORT void _InitFrom(UserSettingsGroupList const&) override;
    ECPRESENTATION_EXPORT void _UnsetValue(Utf8CP id) override;
    ECPRESENTATION_EXPORT void _SetSettingValue(Utf8CP id, Utf8CP value) override;
    ECPRESENTATION_EXPORT void _SetSettingIntValue(Utf8CP id, int64_t value) override;
    ECPRESENTATION_EXPORT void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) override;
    ECPRESENTATION_EXPORT void _SetSettingBoolValue(Utf8CP id, bool value) override;
    ECPRESENTATION_EXPORT Utf8String _GetSettingValue(Utf8CP id) const override;
    ECPRESENTATION_EXPORT int64_t _GetSettingIntValue(Utf8CP id) const override;
    ECPRESENTATION_EXPORT bvector<int64_t> _GetSettingIntValues(Utf8CP id) const override;
    ECPRESENTATION_EXPORT bool _GetSettingBoolValue(Utf8CP id) const override;
    ECPRESENTATION_EXPORT BeJsDocument _GetSettingValueAsJson(Utf8CP id) const override;

public:
    //! Constructor.
    //! @param[in] rulesetId The ID of the ruleset whose settings are stored in this instance.
    //! @param[in] changeListener Listener that's notified when a setting in this instance changes.
    UserSettings(Utf8CP rulesetId, IUserSettingsChangeListener const* changeListener = nullptr)
        : m_changeListener(changeListener), m_rulesetId(rulesetId), m_localState(nullptr), m_isInitializing(false), m_initializedFrom(nullptr)
        {}
    void SetLocalState(IJsonLocalState* localState) {m_localState = localState;}
};

//=======================================================================================
//! An interface for @ref UserSettings manager.
//! @note The implementation must be thread safe.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
struct IUserSettingsManager
{
private:
    IJsonLocalState* m_localState;
    IUserSettingsChangeListener* m_changeListener;
    mutable BeMutex m_mutex;

protected:
    BeMutex& GetMutex() const {return m_mutex;}
    virtual IUserSettings& _GetSettings(Utf8StringCR) const = 0;
    virtual void _OnLocalStateChanged() {}
    virtual void _OnChangesListenerChanged() {}

public:
    IUserSettingsManager() : m_localState(nullptr), m_changeListener(nullptr) {}
    virtual ~IUserSettingsManager() {}

    IJsonLocalState* GetLocalState() const {return m_localState;}
    void SetLocalState(IJsonLocalState* localState) {m_localState = localState; _OnLocalStateChanged();}

    IUserSettingsChangeListener* GetChangesListener() const {return m_changeListener;}
    void SetChangesListener(IUserSettingsChangeListener* listener) {m_changeListener = listener; _OnChangesListenerChanged();}

    //! Get a writable @ref IUserSettings instance.
    //! @param[in] rulesetId The ID of the ruleset whose settings should be returned.
    IUserSettings& GetSettings(Utf8StringCR rulesetId) const {return _GetSettings(rulesetId);}
};

//=======================================================================================
//! Manages @ref UserSettings instances for each ruleset.
//! @note The implementation must be thread safe.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
struct UserSettingsManager : NonCopyableClass, IUserSettingsManager, IRulesetCallbacksHandler, IUserSettingsChangeListener
{
private:
    mutable bmap<Utf8String, UserSettings*> m_settings;

protected:
    // IUserSettingsManager
    IUserSettings& _GetSettings(Utf8StringCR rulesetId) const override {return GetSettings(rulesetId);}
    void _OnLocalStateChanged() override;

    // IUserSettingsChangeListener
    void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override;

    // IRulesetCallbacksHandler
    void _OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetR ruleset) override;

public:
    //! Constructor.
    //! @param[in] temporaryDirectory Path to a temporary directory which can be used to store temp files.
    ECPRESENTATION_EXPORT UserSettingsManager(BeFileNameCR temporaryDirectory);
    ECPRESENTATION_EXPORT ~UserSettingsManager();

    //! Get a @ref UserSettings instance.
    //! @param[in] rulesetId The ID of the ruleset whose settings should be returned.
    ECPRESENTATION_EXPORT UserSettings& GetSettings(Utf8StringCR rulesetId) const;
};
typedef UserSettingsManager const& UserSettingsManagerCR;

END_BENTLEY_ECPRESENTATION_NAMESPACE
