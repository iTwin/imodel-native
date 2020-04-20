/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

typedef bvector<UserSettingsItemP>    UserSettingsItemList;

/*---------------------------------------------------------------------------------**//**
Implementation of UserSettingsGroup definition. This rule is used to define user settings
that can affect behavior of the hierarchy. These settings will be shown in UserSettingsDialog.
* @bsiclass                                     Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsGroup : public PresentationKey
    {
    private:
        Utf8String            m_categoryLabel;
        UserSettingsGroupList m_nestedSettings;
        UserSettingsItemList  m_settingsItems;

    protected:
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

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
* @bsiclass                                     Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsItem : HashableBase
    {
    private:
        Utf8String  m_id;
        Utf8String  m_label;
        Utf8String  m_options;
        Utf8String  m_defaultValue;

    protected:
        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT UserSettingsItem ();

        //! Constructor.
        ECPRESENTATION_EXPORT UserSettingsItem (Utf8StringCR id, Utf8StringCR label, Utf8StringCR options, Utf8StringCR defaultValue);

        //! Reads specification from xml.
        ECPRESENTATION_EXPORT bool ReadXml (BeXmlNodeP xmlNode);

        //! Writes specification to xml node.
        ECPRESENTATION_EXPORT void WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Reads specification from json.
        ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);

        //! Writes specification to json.
        ECPRESENTATION_EXPORT Json::Value WriteJson() const;

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
