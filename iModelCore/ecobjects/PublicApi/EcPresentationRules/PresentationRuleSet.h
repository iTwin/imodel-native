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

/*__PUBLISH_SECTION_START__*/

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
    /*__PUBLISH_SECTION_END__*/
    private:
        WString                                m_ruleSetId;
        WString                                m_supportedSchemas;
        bool                                   m_isSupplemental;
        int                                    m_version;
        WString                                m_preferredImage;

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
        PresentationRuleSet (void)
            : m_ruleSetId (L""), m_supportedSchemas (L""), m_isSupplemental (false), m_version (1)
            {
            }

        //Private constructor. This class instance should be creates using static helper methods.
        PresentationRuleSet
            (
            WStringCR ruleSetId,
            WStringCR supportedSchemas,
            bool      isSupplemental,
            int       version,
            WStringCR preferredImage
            )
            : m_ruleSetId (ruleSetId), m_supportedSchemas (supportedSchemas), m_isSupplemental (isSupplemental), m_version (version), m_preferredImage (preferredImage)
            {
            }

        //Reads PresentationRuleSet from XML. Returns false if it is not able to load it.
        bool                                            ReadXml (BeXmlDomR xmlDom);

        //Writes PresentationRuleSet to XML.
        void                                            WriteXml (BeXmlDomR xmlDom);

        //Private destructor.
                                                        ~PresentationRuleSet (void);

    public:
    /*__PUBLISH_SECTION_START__*/
        //! Creates an instance of PresentationRuleSet.
        ECOBJECTS_EXPORT static PresentationRuleSetPtr  CreateInstance 
            (
            WStringCR ruleSetId,
            WStringCR supportedSchemas,
            bool      isSupplemental,
            int       version,
            WStringCR preferredImage
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
        ECOBJECTS_EXPORT WStringCR                      GetRuleSetId (void) const         { return m_ruleSetId;        }

        //! Full id of PresentationRuleSet that includes RuleSetId, Version, and IsSupplemental flag.
        ECOBJECTS_EXPORT WString                        GetFullRuleSetId (void) const;

        //! Schemas list for which rules should be applied
        ECOBJECTS_EXPORT WStringCR                      GetSupportedSchemas (void) const  { return m_supportedSchemas; }

        //! Returns true if PresentationRuleSet is supplemental. This allows to add additional rules for already existing PresentationRuleSet.
        ECOBJECTS_EXPORT bool                           GetIsSupplemental (void) const    { return m_isSupplemental;   }

        //! Version fo the PresentationRuleSet. This will be used in the future if we add some incompatible changes to the system.
        ECOBJECTS_EXPORT int                            GetVersion (void) const           { return m_version;          }

        //! Preferred ImageId for the tree that is configured using this presentation rule set.
        ECOBJECTS_EXPORT WStringCR                      GetPreferredImage (void) const    { return m_preferredImage;   }

        //! Collection of rules, which should be used when root nodes needs to be populated.
        ECOBJECTS_EXPORT RootNodeRuleList&              GetRootNodesRules (void)          { return m_rootNodesRules;   }

        //! Collection of rules, which should be used when child nodes needs to be populated.
        ECOBJECTS_EXPORT ChildNodeRuleList&             GetChildNodesRules (void)         { return m_childNodesRules;  }

        //! Collection of rules, which should be used when content for selected nodes needs to be populated.
        ECOBJECTS_EXPORT ContentRuleList&               GetContentRules (void)            { return m_contentRules;     }

        //! Collection of rules, which should be used when default ImageId for nodes should be overridden.
        ECOBJECTS_EXPORT ImageIdOverrideList&           GetImageIdOverrides (void)        { return m_imageIdRules;     }

        //! Collection of rules, which should be used when default Label or Description for nodes should be overridden.
        ECOBJECTS_EXPORT LabelOverrideList&             GetLabelOverrides (void)          { return m_labelOverrides;   }

        //! Collection of rules, which should be used when default style for nodes should be overridden.
        ECOBJECTS_EXPORT StyleOverrideList&             GetStyleOverrides (void)          { return m_styleOverrides;   }

        //! Collection of rules, which should be used when advanced grouping should be applied for particular classes.
        ECOBJECTS_EXPORT GroupingRuleList&              GetGroupingRules (void)           { return m_groupingRules;    }

        //! Collection of rules, which should be used when localization resource key definition is predefined.
        ECOBJECTS_EXPORT LocalizationResourceKeyDefinitionList&  GetLocalizationResourceKeyDefinitions (void) { return m_localizationResourceKeyDefinitions; }

        //! Collection of user settings definitions, that can affect behavior of the hierarchy. These settings will be shown in UserSettingsDialog.
        ECOBJECTS_EXPORT UserSettingsGroupList&         GetUserSettings (void)            { return m_userSettings;     }

        //! Collection of rules, which should be used when check boxes for particular nodes should be displayed.
        ECOBJECTS_EXPORT CheckBoxRuleList&              GetCheckBoxRules (void)           { return m_checkBoxRules;    }

        //! Collection of rules, which should be used when particular nodes can be renamed.
        ECOBJECTS_EXPORT RenameNodeRuleList&            GetRenameNodeRules (void)         { return m_renameNodeRules;  }

        //! Collection of rules, which should be used for configuring sorting of ECInstances.
        ECOBJECTS_EXPORT SortingRuleList&               GetSortingRules (void)            { return m_sortingRules;     }
    };

END_BENTLEY_ECOBJECT_NAMESPACE
