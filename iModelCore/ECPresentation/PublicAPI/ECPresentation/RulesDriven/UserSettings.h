/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/UserSettings.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/RuleSetLocater.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define USER_SETTINGS_LOCALSTATE

struct ILocalizationProvider;
struct UserSettingsManager;

//=======================================================================================
//! An interface for a listener that's notified when a user setting changes.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                04/2016
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
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                01/2016
//=======================================================================================
struct IUserSettings
{
protected:
    virtual ~IUserSettings() {}

    virtual Json::Value _GetPresentationInfo() const = 0;

    virtual bool _HasSetting(Utf8CP id) const = 0;

    virtual void _SetSettingValue(Utf8CP id, Utf8CP value) = 0;
    virtual void _SetSettingIntValue(Utf8CP id, int64_t value) = 0;
    virtual void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) = 0;
    virtual void _SetSettingBoolValue(Utf8CP id, bool value) = 0;

    virtual Utf8String _GetSettingValue(Utf8CP id) const = 0;
    virtual int64_t _GetSettingIntValue(Utf8CP id) const = 0;
    virtual bvector<int64_t> _GetSettingIntValues(Utf8CP id) const = 0;
    virtual bool _GetSettingBoolValue(Utf8CP id) const = 0;

public:
    //! Get the presentation info for user settings stored in this instance.
    Json::Value GetPresentationInfo() const {return _GetPresentationInfo();}

    //! Is there a user setting with the specified id.
    bool HasSetting(Utf8CP id) const {return _HasSetting(id);}

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
};

#ifdef USER_SETTINGS_DB
//=======================================================================================
//! Handles user settings' storage and retrieval.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                01/2016
//=======================================================================================
struct UserSettings : IUserSettings
{
friend struct UserSettingsManager;

private:
    BeSQLite::Db& m_settingsDb;
    BeSQLite::StatementCache& m_statements;
    Utf8String m_rulesetId;
    ILocalizationProvider const* m_localizationProvider;
    Json::Value m_presentationInfo;
    IUserSettingsChangeListener const* m_changeListener;
    bool m_isInitializing;
    
private:
    bool HasValue(Utf8CP settingId) const;
    Json::Value GetValue(Utf8CP settingId, bool isText) const;
    void DeleteValues(Utf8CP settingId);
    void InsertValue(Utf8CP settingId, JsonValueCR settingValue, bool updateIfExists, bool notifyListeners);

    void AddValues(JsonValueR json) const;
    Utf8String GetLocalizedLabel(Utf8StringCR nonLocalizedLabel) const;
    void InitFrom(UserSettingsGroupList const& rules, JsonValueR presentationObj);

protected:
    ECPRESENTATION_EXPORT Json::Value _GetPresentationInfo() const override;
    ECPRESENTATION_EXPORT bool _HasSetting(Utf8CP id) const override;
    ECPRESENTATION_EXPORT void _SetSettingValue(Utf8CP id, Utf8CP value) override;
    ECPRESENTATION_EXPORT void _SetSettingIntValue(Utf8CP id, int64_t value) override;
    ECPRESENTATION_EXPORT void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) override;
    ECPRESENTATION_EXPORT void _SetSettingBoolValue(Utf8CP id, bool value) override;
    ECPRESENTATION_EXPORT Utf8String _GetSettingValue(Utf8CP id) const override;
    ECPRESENTATION_EXPORT int64_t _GetSettingIntValue(Utf8CP id) const override;
    ECPRESENTATION_EXPORT bvector<int64_t> _GetSettingIntValues(Utf8CP id) const override;
    ECPRESENTATION_EXPORT bool _GetSettingBoolValue(Utf8CP id) const override;

public:
    UserSettings(BeSQLite::Db& settingsDb, BeSQLite::StatementCache& statements, Utf8CP rulesetId, IUserSettingsChangeListener const* changeListener = nullptr) 
        : m_settingsDb(settingsDb), m_statements(statements), m_changeListener(changeListener), m_rulesetId(rulesetId), m_localizationProvider(nullptr), m_isInitializing(false)
        {}
    void SetLocalizationProvider(ILocalizationProvider const* provider) {m_localizationProvider = provider;}
    ECPRESENTATION_EXPORT void InitFrom(UserSettingsGroupList const& rules);
};
#endif

#ifdef USER_SETTINGS_LOCALSTATE
//=======================================================================================
//! Handles user settings' storage and retrieval.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                01/2016
//=======================================================================================
struct UserSettings : IUserSettings
{
friend struct UserSettingsManager;

private:
    Utf8String m_rulesetId;
    IJsonLocalState* m_localState;
    ILocalizationProvider const* m_localizationProvider;
    NativeLogging::ILogger* m_logger;
    Json::Value m_presentationInfo;
    IUserSettingsChangeListener const* m_changeListener;
    bool m_isInitializing;
    
private:
    void AddValues(JsonValueR json) const;
    Utf8String GetLocalizedLabel(Utf8StringCR nonLocalizedLabel) const;
    void InitFrom(UserSettingsGroupList const& rules, JsonValueR presentationObj);
    
protected:
    ECPRESENTATION_EXPORT Json::Value _GetPresentationInfo() const override;
    ECPRESENTATION_EXPORT bool _HasSetting(Utf8CP id) const override;
    ECPRESENTATION_EXPORT void _SetSettingValue(Utf8CP id, Utf8CP value) override;
    ECPRESENTATION_EXPORT void _SetSettingIntValue(Utf8CP id, int64_t value) override;
    ECPRESENTATION_EXPORT void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) override;
    ECPRESENTATION_EXPORT void _SetSettingBoolValue(Utf8CP id, bool value) override;
    ECPRESENTATION_EXPORT Utf8String _GetSettingValue(Utf8CP id) const override;
    ECPRESENTATION_EXPORT int64_t _GetSettingIntValue(Utf8CP id) const override;
    ECPRESENTATION_EXPORT bvector<int64_t> _GetSettingIntValues(Utf8CP id) const override;
    ECPRESENTATION_EXPORT bool _GetSettingBoolValue(Utf8CP id) const override;

public:
    //! Constructor.
    //! @param[in] rulesetId The ID of the ruleset whose settings are stored in this instance.
    //! @param[in] changeListener Listener that's notified when a setting in this instance changes.
    UserSettings(Utf8CP rulesetId, IUserSettingsChangeListener const* changeListener = nullptr) 
        : m_changeListener(changeListener), m_rulesetId(rulesetId), m_localState(nullptr), m_localizationProvider(nullptr), m_logger(nullptr), m_isInitializing(false)
        {}
    void SetLocalState(IJsonLocalState* localState) {m_localState = localState;}
    void SetLocalizationProvider(ILocalizationProvider const* provider) {m_localizationProvider = provider;}
    void SetLogger(NativeLogging::ILogger* logger) {m_logger = logger;}
    ECPRESENTATION_EXPORT void InitFrom(UserSettingsGroupList const& rules);
};
#endif

//=======================================================================================
//! Manages @ref UserSettings instances for each ruleset.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                01/2016
//=======================================================================================
struct IUserSettingsManager
{
protected:
    virtual IUserSettings& _GetSettings(Utf8StringCR) const = 0;

public:
    virtual ~IUserSettingsManager() {}

    //! Get a writable @ref IUserSettings instance.
    //! @param[in] rulesetId The ID of the ruleset whose settings should be returned.
    IUserSettings& GetSettings(Utf8StringCR rulesetId) const {return _GetSettings(rulesetId);}
};

//=======================================================================================
//! Manages @ref UserSettings instances for each ruleset.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                01/2016
//=======================================================================================
struct UserSettingsManager : NonCopyableClass, IUserSettingsManager, IRulesetCallbacksHandler, IUserSettingsChangeListener
{
private:
    mutable bmap<Utf8String, UserSettings*> m_settings;
#ifdef USER_SETTINGS_DB
    BeFileName m_settingsDbPath;
    BeSQLite::Db m_settingsDb;
    BeSQLite::StatementCache m_statements;
    bset<BeSQLite::BeGuid> m_attachedConnections;
#endif
#ifdef USER_SETTINGS_LOCALSTATE
    IJsonLocalState* m_localState;
#endif
    ILocalizationProvider const* m_localizationProvider;
    IUserSettingsChangeListener const* m_changeListener;
    
#ifdef USER_SETTINGS_DB
private:
    void InitSettingsDb(BeFileNameCR localStateDirectory);
    void InitSettingsTable();
#endif
    
protected:
    IUserSettings& _GetSettings(Utf8StringCR rulesetId) const override {return GetSettings(rulesetId);}
    void _OnRulesetDispose(PresentationRuleSetCR ruleset) override;
    void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override;

public:
    ECPRESENTATION_EXPORT UserSettings& GetSettings(Utf8StringCR rulesetId) const;
    void SetLocalizationProvider(ILocalizationProvider const* provider);
#ifdef USER_SETTINGS_DB
    BeSQLite::Db& GetSettingsDb() {return m_settingsDb;}
    void InitForConnection(ECDbR);
    void TerminateForConnection(ECDbR);
#endif
#ifdef USER_SETTINGS_LOCALSTATE
    ECPRESENTATION_EXPORT void SetLocalState(IJsonLocalState* localState);
#endif

public:
    //! Constructor.
    //! @param[in] temporaryDirectory Path to a temporary directory which can be used to store temp files.
    //! @param[in] changeListener Listener that's notified when a user setting changes.
    ECPRESENTATION_EXPORT UserSettingsManager(BeFileNameCR temporaryDirectory, IUserSettingsChangeListener const* changeListener = nullptr);
    ECPRESENTATION_EXPORT ~UserSettingsManager();
};
typedef UserSettingsManager const& UserSettingsManagerCR;

END_BENTLEY_ECPRESENTATION_NAMESPACE
