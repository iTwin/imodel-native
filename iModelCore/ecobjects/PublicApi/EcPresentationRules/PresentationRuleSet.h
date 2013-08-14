/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/PresentationRuleSet.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef bvector<RootNodeRuleP>                       RootNodeRuleList;
typedef bvector<ChildNodeRuleP>                      ChildNodeRuleList;
typedef bvector<ContentRuleP>                        ContentRuleList;
typedef bvector<ImageIdOverrideP>                    ImageIdOverrideList;
typedef bvector<LabelOverrideP>                      LabelOverrideList;
typedef bvector<StyleOverrideP>                      StyleOverrideList;
typedef bvector<GroupingRuleP>                       GroupingRuleList;
typedef bvector<LocalizationResourceKeyDefinitionP>  LocalizationResourceKeyDefinitionList;
typedef bvector<UserSettingsGroupP>                  UserSettingsGroupList;
typedef bvector<CheckBoxRuleP>                       CheckBoxRuleList;
typedef bvector<RenameNodeRuleP>                     RenameNodeRuleList;
typedef bvector<SortingRuleP>                        SortingRuleList;

/*---------------------------------------------------------------------------------**//**
PresentationRuleSet is a container of all presentation rules for particular type of the tree
and particular set of schemas .
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationRuleSet : public RefCountedBase
    {
    private:
        WString                                m_ruleSetId;
        WString                                m_supportedSchemas;
        bool                                   m_isSupplemental;
        WString                                m_supplementationPurpose;
        int                                    m_versionMajor;
        int                                    m_versionMinor;
        WString                                m_preferredImage;
        bool                                   m_isSearchEnabled;

        RootNodeRuleList                       m_rootNodesRules;
        ChildNodeRuleList                      m_childNodesRules;
        ContentRuleList                        m_contentRules;
        ImageIdOverrideList                    m_imageIdRules;
        LabelOverrideList                      m_labelOverrides;
        StyleOverrideList                      m_styleOverrides;
        GroupingRuleList                       m_groupingRules;
        LocalizationResourceKeyDefinitionList  m_localizationResourceKeyDefinitions;
        CheckBoxRuleList                       m_checkBoxRules;
        RenameNodeRuleList                     m_renameNodeRules;
        SortingRuleList                        m_sortingRules;
        UserSettingsGroupList                  m_userSettings;

        //Private constructor. This class instance should be creates using static helper methods.
        PresentationRuleSet (void);

        //Private constructor. This class instance should be creates using static helper methods.
        PresentationRuleSet
            (
            WStringCR ruleSetId,
            int       versionMajor,
            int       versionMinor,
            bool      isSupplemental,
            WStringCR supplementationPurpose,
            WStringCR supportedSchemas,
            WStringCR preferredImage,
            bool      isSearchEnabled
            );

        //Reads PresentationRuleSet from XML. Returns false if it is not able to load it.
        bool                                            ReadXml (BeXmlDomR xmlDom);

        //Writes PresentationRuleSet to XML.
        void                                            WriteXml (BeXmlDomR xmlDom);

        //Private destructor.
                                                        ~PresentationRuleSet (void);

    public:
        //! Creates an instance of PresentationRuleSet.
        ECOBJECTS_EXPORT static PresentationRuleSetPtr  CreateInstance 
            (
            WStringCR ruleSetId,
            int       versionMajor,
            int       versionMinor,
            bool      isSupplemental,
            WStringCR supplementationPurpose,
            WStringCR supportedSchemas,
            WStringCR preferredImage,
            bool      isSearchEnabled
            );

        //! Reads PresentationRuleSet from XmlString.
        ECOBJECTS_EXPORT static PresentationRuleSetPtr  ReadFromXmlString (WCharCP xmlString);

        //! Reads PresentationRuleSet from XmlFile.
        ECOBJECTS_EXPORT static PresentationRuleSetPtr  ReadFromXmlFile (WCharCP xmlFilePath);

        //! Writes PresentationRuleSet to XmlString.
        ECOBJECTS_EXPORT WString                        WriteToXmlString ();

        //! Writes PresentationRuleSet to XmlFile.
        ECOBJECTS_EXPORT bool                           WriteToXmlFile (WCharCP xmlFilePath);

        //! PresentationRuleSet identification.
        ECOBJECTS_EXPORT WStringCR                      GetRuleSetId (void) const;

        //! Full id of PresentationRuleSet that includes RuleSetId, Version, and IsSupplemental flag.
        ECOBJECTS_EXPORT WString                        GetFullRuleSetId (void) const;

        //! Major version of the PresentationRuleSet. This will be used in the future if we add some incompatible changes to the system.
        ECOBJECTS_EXPORT int                            GetVersionMajor (void) const;

        //! Minor version of the PresentationRuleSet. This can be used to identify compatible changes in the PresentationRuleSet.
        ECOBJECTS_EXPORT int                            GetVersionMinor (void) const;

        //! Returns true if PresentationRuleSet is supplemental. This allows to add additional rules for already existing PresentationRuleSet.
        ECOBJECTS_EXPORT bool                           GetIsSupplemental (void) const;

        //! Purpose of supplementation. There can be one RuleSet with the same version and same supplementation purpose.
        ECOBJECTS_EXPORT WStringCR                      GetSupplementationPurpose (void) const;

        //! Schemas list for which rules should be applied
        ECOBJECTS_EXPORT WStringCR                      GetSupportedSchemas (void) const;

        //! Preferred ImageId for the tree that is configured using this presentation rule set.
        ECOBJECTS_EXPORT WStringCR                      GetPreferredImage (void) const;

        //! Returns true if search should be enabled for the tree that uses this presentation rule set.
        ECOBJECTS_EXPORT bool                           GetIsSearchEnabled (void) const;

        //! Collection of rules, which should be used when root nodes needs to be populated.
        ECOBJECTS_EXPORT RootNodeRuleList&              GetRootNodesRules (void);

        //! Collection of rules, which should be used when child nodes needs to be populated.
        ECOBJECTS_EXPORT ChildNodeRuleList&             GetChildNodesRules (void);

        //! Collection of rules, which should be used when content for selected nodes needs to be populated.
        ECOBJECTS_EXPORT ContentRuleList&               GetContentRules (void);

        //! Collection of rules, which should be used when default ImageId for nodes should be overridden.
        ECOBJECTS_EXPORT ImageIdOverrideList&           GetImageIdOverrides (void);

        //! Collection of rules, which should be used when default Label or Description for nodes should be overridden.
        ECOBJECTS_EXPORT LabelOverrideList&             GetLabelOverrides (void);

        //! Collection of rules, which should be used when default style for nodes should be overridden.
        ECOBJECTS_EXPORT StyleOverrideList&             GetStyleOverrides (void);

        //! Collection of rules, which should be used when advanced grouping should be applied for particular classes.
        ECOBJECTS_EXPORT GroupingRuleList&              GetGroupingRules (void);

        //! Collection of rules, which should be used when localization resource key definition is predefined.
        ECOBJECTS_EXPORT LocalizationResourceKeyDefinitionList&  GetLocalizationResourceKeyDefinitions (void);

        //! Collection of user settings definitions, that can affect behavior of the hierarchy. These settings will be shown in UserSettingsDialog.
        ECOBJECTS_EXPORT UserSettingsGroupList&         GetUserSettings (void);

        //! Collection of rules, which should be used when check boxes for particular nodes should be displayed.
        ECOBJECTS_EXPORT CheckBoxRuleList&              GetCheckBoxRules (void);

        //! Collection of rules, which should be used when particular nodes can be renamed.
        ECOBJECTS_EXPORT RenameNodeRuleList&            GetRenameNodeRules (void);

        //! Collection of rules, which should be used for configuring sorting of ECInstances.
        ECOBJECTS_EXPORT SortingRuleList&               GetSortingRules (void);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
