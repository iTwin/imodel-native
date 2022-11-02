/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

typedef bvector<UserSettingsItemP>    UserSettingsItemList;

/*---------------------------------------------------------------------------------**//**
Implementation of UserSettingsGroup definition. This rule is used to define user settings
that can affect behavior of the hierarchy. These settings will be shown in UserSettingsDialog.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsGroup : PrioritizedPresentationKey
{
    DEFINE_T_SUPER(PrioritizedPresentationKey)

private:
    Utf8String            m_categoryLabel;
    UserSettingsGroupList m_nestedSettings;
    UserSettingsItemList  m_settingsItems;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Computes rule hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT UserSettingsGroup();

    //! Constructor.
    ECPRESENTATION_EXPORT UserSettingsGroup(Utf8StringCR categoryLabel);

    //! Constructor.
    ECPRESENTATION_EXPORT UserSettingsGroup(UserSettingsGroupCR);

    //! Desctructor.
    ECPRESENTATION_EXPORT ~UserSettingsGroup (void);

    //! Label of category that is used to group all the settings. If it is null, no category will be created.
    ECPRESENTATION_EXPORT Utf8StringCR                GetCategoryLabel (void) const;

    //! Add UserSettingsItem.
    ECPRESENTATION_EXPORT void AddSettingsItem (UserSettingsItemR item);

    //! Returns a list of UserSettingsItems.
    ECPRESENTATION_EXPORT UserSettingsItemList const& GetSettingsItems (void) const;

    //! Add nested UserSettingGroup.
    ECPRESENTATION_EXPORT void AddNestedSettings (UserSettingsGroupR group);

    //! Returns a list of nested UserSettingsGroup. This allows to create sub-categories.
    ECPRESENTATION_EXPORT UserSettingsGroupList const& GetNestedSettings (void) const;
};

/*---------------------------------------------------------------------------------**//**
Implementation of UserSettingsItem definition. It represents a single user setting in
UserSettingsGroup.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsItem : PresentationKey
{
    DEFINE_T_SUPER(PresentationKey)

private:
    Utf8String  m_id;
    Utf8String  m_label;
    Utf8String  m_options;
    Utf8String  m_defaultValue;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT UserSettingsItem ();

    //! Constructor.
    ECPRESENTATION_EXPORT UserSettingsItem (Utf8StringCR id, Utf8StringCR label, Utf8StringCR options, Utf8StringCR defaultValue);

    //! Id of the user setting that will be used to save and access settings value.
    ECPRESENTATION_EXPORT Utf8StringCR GetId (void) const;

    //! Label of the user settings that will be shown for the user in UserSettingsDialog.
    ECPRESENTATION_EXPORT Utf8StringCR GetLabel (void) const;

    //! Options for the user setting value.
    ECPRESENTATION_EXPORT Utf8StringCR GetOptions (void) const;

    //! Default user settings value.
    ECPRESENTATION_EXPORT Utf8StringCR GetDefaultValue (void) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
