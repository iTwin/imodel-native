/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/UserSettingsGroup.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP  _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool    _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void    _WriteXml (BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT UserSettingsGroup ();

        //! Constructor.
        ECOBJECTS_EXPORT UserSettingsGroup (Utf8StringCR categoryLabel);

        //! Desctructor.
        ECOBJECTS_EXPORT                             ~UserSettingsGroup (void);

        //! Label of category that is used to group all the settings. If it is null, no category will be created.
        ECOBJECTS_EXPORT Utf8StringCR                GetCategoryLabel (void) const;

        //! Returns a list of UserSettingsItems.
        ECOBJECTS_EXPORT UserSettingsItemList&       GetSettingsItemsR (void);
        
        //! Returns a list of UserSettingsItems.
        ECOBJECTS_EXPORT UserSettingsItemList const& GetSettingsItems (void) const;

        //! Returns a list of nested UserSettingsGroup. This allows to create sub-categories.
        ECOBJECTS_EXPORT UserSettingsGroupList&      GetNestedSettingsR (void);
        
        //! Returns a list of nested UserSettingsGroup. This allows to create sub-categories.
        ECOBJECTS_EXPORT UserSettingsGroupList const& GetNestedSettings (void) const;
    };

/*---------------------------------------------------------------------------------**//**
Implementation of UserSettingsItem definition. It represents a single user setting in 
UserSettingsGroup.
* @bsiclass                                     Eligijus.Mauragas               01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserSettingsItem
    {
    private:
        Utf8String  m_id;
        Utf8String  m_label;
        Utf8String  m_options;
        Utf8String  m_defaultValue;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT UserSettingsItem ();

        //! Constructor.
        ECOBJECTS_EXPORT UserSettingsItem (Utf8StringCR id, Utf8StringCR label, Utf8StringCR options, Utf8StringCR defaultValue);

        //! Reads specification from xml.
        ECOBJECTS_EXPORT bool            ReadXml (BeXmlNodeP xmlNode);

        //! Writes specification to xml node.
        ECOBJECTS_EXPORT void            WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Id of the user setting that will be used to save and access settings value.
        ECOBJECTS_EXPORT Utf8StringCR    GetId (void) const;

        //! Label of the user settings that will be shown for the user in UserSettingsDialog.
        ECOBJECTS_EXPORT Utf8StringCR    GetLabel (void) const;

        //! Options for the user setting value.
        ECOBJECTS_EXPORT Utf8StringCR    GetOptions (void) const;

        //! Default user settings value.
        ECOBJECTS_EXPORT Utf8StringCR    GetDefaultValue (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
