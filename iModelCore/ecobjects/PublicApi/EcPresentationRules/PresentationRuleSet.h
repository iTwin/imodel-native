/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/PresentationRuleSet.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/CommonTools.h>
#include <ECPresentationRules/PresentationRulesTypes.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
PresentationRuleSet is a container of all presentation rules for particular type of the tree
and particular set of schemas .
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationRuleSet : public RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        Utf8String                             m_ruleSetId;
        Utf8String                             m_supportedSchemas;
        bool                                   m_isSupplemental;
        Utf8String                             m_supplementationPurpose;
        int                                    m_versionMajor;
        int                                    m_versionMinor;
        Utf8String                             m_preferredImage;
        bool                                   m_isSearchEnabled;
        Utf8String                             m_extendedData;
        Utf8String                             m_searchClasses;

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

    private:
        //Private constructor. This class instance should be creates using static helper methods.
        PresentationRuleSet
            (
            Utf8StringCR ruleSetId,
            int          versionMajor,
            int          versionMinor,
            bool         isSupplemental,
            Utf8StringCR supplementationPurpose,
            Utf8StringCR supportedSchemas,
            Utf8StringCR preferredImage,
            bool         isSearchEnabled
            );

        //Reads PresentationRuleSet from XML. Returns false if it is not able to load it.
        bool                                            ReadXml (BeXmlDomR xmlDom);

        //Writes PresentationRuleSet to XML.
        void                                            WriteXml (BeXmlDomR xmlDom);

    /*__PUBLISH_SECTION_START__*/
    private:
        //Private constructor. This class instance should be creates using static helper methods.
        PresentationRuleSet (void);

        //Private destructor.
        ~PresentationRuleSet (void);

        //! The generic implementation of GetRules. See below for function specializations
        template<typename RuleType> bvector<RuleType*>* GetRules() {BeAssert(false); return nullptr;}

    public:
        //! Creates an instance of PresentationRuleSet.
        ECOBJECTS_EXPORT static PresentationRuleSetPtr  CreateInstance 
            (
            Utf8StringCR ruleSetId,
            int          versionMajor,
            int          versionMinor,
            bool         isSupplemental,
            Utf8StringCR supplementationPurpose,
            Utf8StringCR supportedSchemas,
            Utf8StringCR preferredImage,
            bool         isSearchEnabled
            );

        //! Adds a presentation rule to this rule set.
        template<typename RuleType> void AddPresentationRule(RuleType& rule)
            {
            bvector<RuleType*>* list = GetRules<RuleType>();
            if (nullptr != list)
                CommonTools::AddToListByPriority(*list, rule);
            }

        //! Removes the presentation rule from this rule set.
        template<typename RuleType> void RemovePresentationRule(RuleType& rule)
            {
            bvector<RuleType*>* list = GetRules<RuleType>();
            if (nullptr != list)
                CommonTools::RemoveFromList(*list, rule);
            }

        //! Reads PresentationRuleSet from XmlString.
        ECOBJECTS_EXPORT static PresentationRuleSetPtr  ReadFromXmlString (Utf8CP xmlString);

        //! Reads PresentationRuleSet from XmlFile.
        ECOBJECTS_EXPORT static PresentationRuleSetPtr  ReadFromXmlFile (WCharCP xmlFilePath);

        //! Writes PresentationRuleSet to XmlString.
        ECOBJECTS_EXPORT Utf8String                     WriteToXmlString ();

        //! Writes PresentationRuleSet to XmlFile.
        ECOBJECTS_EXPORT bool                           WriteToXmlFile (WCharCP xmlFilePath);

        //! PresentationRuleSet identification.
        ECOBJECTS_EXPORT Utf8StringCR                   GetRuleSetId (void) const;

        //! Full id of PresentationRuleSet that includes RuleSetId, Version, and IsSupplemental flag.
        ECOBJECTS_EXPORT Utf8String                     GetFullRuleSetId (void) const;

        //! Major version of the PresentationRuleSet. This will be used in the future if we add some incompatible changes to the system.
        ECOBJECTS_EXPORT int                            GetVersionMajor (void) const;

        //! Minor version of the PresentationRuleSet. This can be used to identify compatible changes in the PresentationRuleSet.
        ECOBJECTS_EXPORT int                            GetVersionMinor (void) const;

        //! Returns true if PresentationRuleSet is supplemental. This allows to add additional rules for already existing PresentationRuleSet.
        ECOBJECTS_EXPORT bool                           GetIsSupplemental (void) const;

        //! Purpose of supplementation. There can be one RuleSet with the same version and same supplementation purpose.
        ECOBJECTS_EXPORT Utf8StringCR                   GetSupplementationPurpose (void) const;

        //! Schemas list for which rules should be applied
        ECOBJECTS_EXPORT Utf8StringCR                   GetSupportedSchemas (void) const;

        //! Preferred ImageId for the tree that is configured using this presentation rule set.
        ECOBJECTS_EXPORT Utf8StringCR                   GetPreferredImage (void) const;

        //! Returns true if search should be enabled for the tree that uses this presentation rule set.
        ECOBJECTS_EXPORT bool                           GetIsSearchEnabled (void) const;

        //! Allowed classes for the search.
        ECOBJECTS_EXPORT Utf8StringCR                   GetSearchClasses (void) const;

        //! Set allowed classes for the search.
        ECOBJECTS_EXPORT void                           SetSearchClasses (Utf8StringCR searchClasses);

        //! Extended data of a rule set.
        ECOBJECTS_EXPORT Utf8StringCR                   GetExtendedData (void) const;

        //! Set extended data of a rule set.
        ECOBJECTS_EXPORT void                           SetExtendedData (Utf8StringCR extendedData);

        //! Collection of rules, which should be used when root nodes needs to be populated.
        ECOBJECTS_EXPORT RootNodeRuleList const&        GetRootNodesRules (void) const;

        //! Collection of rules, which should be used when child nodes needs to be populated.
        ECOBJECTS_EXPORT ChildNodeRuleList const&       GetChildNodesRules (void) const;

        //! Collection of rules, which should be used when content for selected nodes needs to be populated.
        ECOBJECTS_EXPORT ContentRuleList const&         GetContentRules (void) const;

        //! Collection of rules, which should be used when default ImageId for nodes should be overridden.
        ECOBJECTS_EXPORT ImageIdOverrideList const&     GetImageIdOverrides (void) const;

        //! Collection of rules, which should be used when default Label or Description for nodes should be overridden.
        ECOBJECTS_EXPORT LabelOverrideList const&       GetLabelOverrides (void) const;

        //! Collection of rules, which should be used when default style for nodes should be overridden.
        ECOBJECTS_EXPORT StyleOverrideList const&       GetStyleOverrides (void) const;

        //! Collection of rules, which should be used when advanced grouping should be applied for particular classes.
        ECOBJECTS_EXPORT GroupingRuleList const&        GetGroupingRules (void) const;

        //! Collection of rules, which should be used when localization resource key definition is predefined.
        ECOBJECTS_EXPORT LocalizationResourceKeyDefinitionList const& GetLocalizationResourceKeyDefinitions (void) const;

        //! Collection of user settings definitions, that can affect behavior of the hierarchy. These settings will be shown in UserSettingsDialog.
        ECOBJECTS_EXPORT UserSettingsGroupList const&   GetUserSettings (void) const;

        //! Collection of rules, which should be used when check boxes for particular nodes should be displayed.
        ECOBJECTS_EXPORT CheckBoxRuleList const&        GetCheckBoxRules (void) const;

        //! Collection of rules, which should be used when particular nodes can be renamed.
        ECOBJECTS_EXPORT RenameNodeRuleList const&      GetRenameNodeRules (void) const;

        //! Collection of rules, which should be used for configuring sorting of ECInstances.
        ECOBJECTS_EXPORT SortingRuleList const&         GetSortingRules (void) const;
    };

//! Template specialization for root node rules.
template<> RootNodeRuleList* PresentationRuleSet::GetRules<RootNodeRule>();
template ECOBJECTS_EXPORT RootNodeRuleList* PresentationRuleSet::GetRules<RootNodeRule>();

//! Template specialization for child node rules.
template<> ChildNodeRuleList* PresentationRuleSet::GetRules<ChildNodeRule>();
template ECOBJECTS_EXPORT ChildNodeRuleList* PresentationRuleSet::GetRules<ChildNodeRule>();

//! Template specialization for content rules.
template<> ContentRuleList* PresentationRuleSet::GetRules<ContentRule>();
template ECOBJECTS_EXPORT ContentRuleList* PresentationRuleSet::GetRules<ContentRule>();

//! Template specialization for image id overrides.
template<> ImageIdOverrideList* PresentationRuleSet::GetRules<ImageIdOverride>();
template ECOBJECTS_EXPORT ImageIdOverrideList* PresentationRuleSet::GetRules<ImageIdOverride>();

//! Template specialization for label overrides.
template<> LabelOverrideList* PresentationRuleSet::GetRules<LabelOverride>();
template ECOBJECTS_EXPORT LabelOverrideList* PresentationRuleSet::GetRules<LabelOverride>();

//! Template specialization for style overrides.
template<> StyleOverrideList* PresentationRuleSet::GetRules<StyleOverride>();
template ECOBJECTS_EXPORT StyleOverrideList* PresentationRuleSet::GetRules<StyleOverride>();

//! Template specialization for grouping rules.
template<> GroupingRuleList* PresentationRuleSet::GetRules<GroupingRule>();
template ECOBJECTS_EXPORT GroupingRuleList* PresentationRuleSet::GetRules<GroupingRule>();

//! Template specialization for check box rules.
template<> CheckBoxRuleList* PresentationRuleSet::GetRules<CheckBoxRule>();
template ECOBJECTS_EXPORT CheckBoxRuleList* PresentationRuleSet::GetRules<CheckBoxRule>();

//! Template specialization for rename node rules.
template<> RenameNodeRuleList* PresentationRuleSet::GetRules<RenameNodeRule>();
template ECOBJECTS_EXPORT RenameNodeRuleList* PresentationRuleSet::GetRules<RenameNodeRule>();

//! Template specialization for sorting rules.
template<> SortingRuleList* PresentationRuleSet::GetRules<SortingRule>();
template ECOBJECTS_EXPORT SortingRuleList* PresentationRuleSet::GetRules<SortingRule>();

//! Template specialization for user settings group rules.
template<> UserSettingsGroupList* PresentationRuleSet::GetRules<UserSettingsGroup>();
template ECOBJECTS_EXPORT UserSettingsGroupList* PresentationRuleSet::GetRules<UserSettingsGroup>();

//! Template specialization for localization resource key definitions.
template<> LocalizationResourceKeyDefinitionList* PresentationRuleSet::GetRules<LocalizationResourceKeyDefinition>();
template ECOBJECTS_EXPORT LocalizationResourceKeyDefinitionList* PresentationRuleSet::GetRules<LocalizationResourceKeyDefinition>();

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
