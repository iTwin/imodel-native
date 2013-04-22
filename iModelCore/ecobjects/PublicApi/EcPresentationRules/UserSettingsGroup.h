/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/UserSettingsGroup.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

typedef bvector<UserSettingsItemP>    UserSettingsItemList;

/*---------------------------------------------------------------------------------**//**
Implementation of UserSettingsGroup definition. This rule is used to define user settings
that can affect behavior of the hierarchy. These settings will be shown in UserSettingsDialog.
* @bsiclass                                     Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsGroup : public PresentationKey
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString               m_categoryLabel;
        UserSettingsGroupList m_nestedSettings;
        UserSettingsItemList  m_settingsItems;

    protected:
    /*__PUBLISH_SECTION_START__*/
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP  _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool    _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void    _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT UserSettingsGroup ()
            : PresentationKey (), m_categoryLabel (L"")
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT UserSettingsGroup (WStringCR categoryLabel)
            : PresentationKey (1000), m_categoryLabel (categoryLabel)
            {
            }

        //! Desctructor.
        ECOBJECTS_EXPORT                             ~UserSettingsGroup (void);

        //! Label of category that is used to group all the settings. If it is null, no category will be created.
        ECOBJECTS_EXPORT WStringCR                   GetCategoryLabel (void) const            { return m_categoryLabel; }

        //! Returns a list of UserSettingsItems.
        ECOBJECTS_EXPORT UserSettingsItemList&       GetSettingsItems (void)                  { return m_settingsItems; }

        //! Returns a list of nested UserSettingsGroup. This allows to create sub-categories.
        ECOBJECTS_EXPORT UserSettingsGroupList&      GetNestedSettings (void)                 { return m_nestedSettings; }

    };

/*---------------------------------------------------------------------------------**//**
Implementation of UserSettingsItem definition. It represents a single user setting in 
UserSettingsGroup.
* @bsiclass                                     Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsItem
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString  m_id;
        WString  m_label;
        WString  m_options;
        WString  m_defaultValue;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT UserSettingsItem ()
            : m_id (L""), m_label (L""), m_options (L""), m_defaultValue (L"")
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT UserSettingsItem (WStringCR id, WStringCR label, WStringCR options, WStringCR defaultValue)
            : m_id (id), m_label (label), m_options (options), m_defaultValue (defaultValue)
            {
            }

        //! Reads specification from xml.
        ECOBJECTS_EXPORT bool            ReadXml (BeXmlNodeP xmlNode);

        //! Writes specification to xml node.
        ECOBJECTS_EXPORT void            WriteXml (BeXmlNodeP parentXmlNode);

        //! Id of the user setting that will be used to save and access settings value.
        ECOBJECTS_EXPORT WStringCR       GetId (void) const               { return m_id; }

        //! Label of the user settings that will be shown for the user in UserSettingsDialog.
        ECOBJECTS_EXPORT WStringCR       GetLabel (void) const            { return m_label; }

        //! Options for the user setting value.
        ECOBJECTS_EXPORT WStringCR       GetOptions (void) const          { return m_options; }

        //! Default user settings value.
        ECOBJECTS_EXPORT WStringCR       GetDefaultValue (void) const     { return m_defaultValue; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE