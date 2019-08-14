/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
PresentationRuleSet is a container of all presentation rules for particular type of the tree
and particular set of schemas .
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationRuleSet : public RefCountedBase, HashableBase
    {
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
        SortingRuleList                        m_sortingRules;
        UserSettingsGroupList                  m_userSettings;
        ContentModifierList                    m_contentModifiers;
        InstanceLabelOverrideList              m_instanceLabelOverrides;
        ExtendedDataRuleList                   m_extendedDataRules;
        NodeArtifactsRuleList                  m_nodeArtifactRules;

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
        bool ReadXml (BeXmlDomR xmlDom);

        //Writes PresentationRuleSet to XML.
        void WriteXml (BeXmlDomR xmlDom) const;

        //Reads PresentationRuleSet from Json. Returns false if it is not able to load it.
        bool ReadJson(JsonValueCR json);

        //Writes PresentationRuleSet to JSON.
        void WriteJson(JsonValueR json) const;

    protected:
        //! Computes rules set hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    private:
        //Private constructor. This class instance should be creates using static helper methods.
        PresentationRuleSet (void);

        //Private destructor.
        ~PresentationRuleSet (void);

        //! The generic implementation of GetRules. See below for function specializations
        template<typename RuleType> bvector<RuleType*>* GetRules() {BeAssert(false); return nullptr;}

    public:
        //! Creates an instance of PresentationRuleSet.
        ECPRESENTATION_EXPORT static PresentationRuleSetPtr  CreateInstance 
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

        //! Creates an instance of PresentationRuleSet.
        ECPRESENTATION_EXPORT static PresentationRuleSetPtr CreateInstance 
            (
            Utf8StringCR id,
            int          versionMajor = 1,
            int          versionMinor = 0,
            Utf8StringCR supportedSchemas = ""
            );
        
        //! Creates an instance of supplemental PresentationRuleSet.
        ECPRESENTATION_EXPORT static PresentationRuleSetPtr CreateInstance 
            (
            Utf8StringCR id,
            Utf8StringCR supplementationPurpose,
            int          versionMajor = 1,
            int          versionMinor = 0,
            Utf8StringCR supportedSchemas = ""
            );

        //! Adds a presentation rule to this rule set.
        template<typename RuleType> void AddPresentationRule(RuleType& rule)
            {
            bvector<RuleType*>* list = GetRules<RuleType>();
            if (nullptr != list)
                {
                InvalidateHash();
                rule.SetParent(this);
                CommonTools::AddToListByPriority(*list, rule);
                }
            }

        //! Removes the presentation rule from this rule set.
        template<typename RuleType> void RemovePresentationRule(RuleType& rule)
            {
            bvector<RuleType*>* list = GetRules<RuleType>();
            if (nullptr != list)
                {
                InvalidateHash();
                CommonTools::RemoveFromList(*list, rule);
                }
            }

        //! Reads PresentationRuleSet from XmlString.
        ECPRESENTATION_EXPORT static PresentationRuleSetPtr  ReadFromXmlString (Utf8CP xmlString);

        //! Reads PresentationRuleSet from XmlFile.
        ECPRESENTATION_EXPORT static PresentationRuleSetPtr  ReadFromXmlFile (BeFileNameCR xmlFilePath);

        //! Writes PresentationRuleSet to XmlString.
        ECPRESENTATION_EXPORT Utf8String                     WriteToXmlString () const;

        //! Writes PresentationRuleSet to XmlFile.
        ECPRESENTATION_EXPORT bool                           WriteToXmlFile (BeFileNameCR xmlFilePath) const;
        
        //! Reads PresentationRuleSet from Json::Value.
        ECPRESENTATION_EXPORT static PresentationRuleSetPtr  ReadFromJsonValue(JsonValueCR json);

        //! Reads PresentationRuleSet from JSON string.
        ECPRESENTATION_EXPORT static PresentationRuleSetPtr  ReadFromJsonString(Utf8StringCR jsonString);

        //! Reads PresentationRuleSet from JSON file.
        ECPRESENTATION_EXPORT static PresentationRuleSetPtr  ReadFromJsonFile(BeFileNameCR jsonFilePath);
        
        //! Writes PresentationRuleSet to Json::Value.
        ECPRESENTATION_EXPORT Json::Value                    WriteToJsonValue() const;

        //! Writes PresentationRuleSet to JSON file.
        ECPRESENTATION_EXPORT bool                           WriteToJsonFile(BeFileNameCR jsonFilePath) const;

        //! PresentationRuleSet identification.
        ECPRESENTATION_EXPORT Utf8StringCR                   GetRuleSetId (void) const;

        //! Sets new PresentationRuleSet identification.
        ECPRESENTATION_EXPORT void                           SetRuleSetId (Utf8StringCR);

        //! Full id of PresentationRuleSet that includes RuleSetId, Version, and IsSupplemental flag.
        ECPRESENTATION_EXPORT Utf8String                     GetFullRuleSetId (void) const;

        //! Major version of the PresentationRuleSet. This will be used in the future if we add some incompatible changes to the system.
        ECPRESENTATION_EXPORT int                            GetVersionMajor (void) const;

        //! Minor version of the PresentationRuleSet. This can be used to identify compatible changes in the PresentationRuleSet.
        ECPRESENTATION_EXPORT int                            GetVersionMinor (void) const;

        //! Returns true if PresentationRuleSet is supplemental. This allows to add additional rules for already existing PresentationRuleSet.
        ECPRESENTATION_EXPORT bool                           GetIsSupplemental (void) const;

        //! Purpose of supplementation. There can be one RuleSet with the same version and same supplementation purpose.
        ECPRESENTATION_EXPORT Utf8StringCR                   GetSupplementationPurpose (void) const;

        //! Schemas list for which rules should be applied
        ECPRESENTATION_EXPORT Utf8StringCR                   GetSupportedSchemas (void) const;

        //! Preferred ImageId for the tree that is configured using this presentation rule set.
        ECPRESENTATION_EXPORT Utf8StringCR                   GetPreferredImage (void) const;

        //! Returns true if search should be enabled for the tree that uses this presentation rule set.
        ECPRESENTATION_EXPORT bool                           GetIsSearchEnabled (void) const;

        //! Allowed classes for the search.
        ECPRESENTATION_EXPORT Utf8StringCR                   GetSearchClasses (void) const;

        //! Set allowed classes for the search.
        ECPRESENTATION_EXPORT void                           SetSearchClasses (Utf8StringCR searchClasses);

        //! Extended data of a rule set.
        ECPRESENTATION_EXPORT Utf8StringCR                   GetExtendedData (void) const;

        //! Set extended data of a rule set.
        ECPRESENTATION_EXPORT void                           SetExtendedData (Utf8StringCR extendedData);

        //! Collection of rules, which should be used when root nodes needs to be populated.
        ECPRESENTATION_EXPORT RootNodeRuleList const&        GetRootNodesRules (void) const;

        //! Collection of rules, which should be used when child nodes needs to be populated.
        ECPRESENTATION_EXPORT ChildNodeRuleList const&       GetChildNodesRules (void) const;

        //! Collection of rules, which should be used when content for selected nodes needs to be populated.
        ECPRESENTATION_EXPORT ContentRuleList const&         GetContentRules (void) const;

        //! Collection of rules, which should be used when default ImageId for nodes should be overridden.
        ECPRESENTATION_EXPORT ImageIdOverrideList const&     GetImageIdOverrides (void) const;

        //! Collection of rules, which should be used when default Label or Description for nodes should be overridden.
        ECPRESENTATION_EXPORT LabelOverrideList const&       GetLabelOverrides (void) const;

        //! Collection of rules, which should be used when default style for nodes should be overridden.
        ECPRESENTATION_EXPORT StyleOverrideList const&       GetStyleOverrides (void) const;

        //! Collection of rules, which should be used when advanced grouping should be applied for particular classes.
        ECPRESENTATION_EXPORT GroupingRuleList const&        GetGroupingRules (void) const;

        //! Collection of rules, which should be used when localization resource key definition is predefined.
        ECPRESENTATION_EXPORT LocalizationResourceKeyDefinitionList const& GetLocalizationResourceKeyDefinitions (void) const;

        //! Collection of user settings definitions, that can affect behavior of the hierarchy. These settings will be shown in UserSettingsDialog.
        ECPRESENTATION_EXPORT UserSettingsGroupList const&   GetUserSettings (void) const;

        //! Collection of rules, which should be used when check boxes for particular nodes should be displayed.
        ECPRESENTATION_EXPORT CheckBoxRuleList const&        GetCheckBoxRules (void) const;

        //! Collection of rules, which should be used for configuring sorting of ECInstances.
        ECPRESENTATION_EXPORT SortingRuleList const&         GetSortingRules (void) const;

        //! Collection of rules, which should be for supplementing ruleset with additional rules
        ECPRESENTATION_EXPORT ContentModifierList const&     GetContentModifierRules(void) const;

        //! Collection of rules, which should override instance label of specific class.
        ECPRESENTATION_EXPORT InstanceLabelOverrideList const& GetInstanceLabelOverrides(void) const;

        //! Collection of rules, which add extended data into presentation objects.
        ECPRESENTATION_EXPORT ExtendedDataRuleList const& GetExtendedDataRules() const;

        //! Collection of rules, which add artifacts into nodes.
        ECPRESENTATION_EXPORT NodeArtifactsRuleList const& GetNodeArtifactRules() const;
    };

template<> ECPRESENTATION_EXPORT RootNodeRuleList* PresentationRuleSet::GetRules<RootNodeRule>();
template<> ECPRESENTATION_EXPORT ChildNodeRuleList* PresentationRuleSet::GetRules<ChildNodeRule>();
template<> ECPRESENTATION_EXPORT ContentRuleList* PresentationRuleSet::GetRules<ContentRule>();
template<> ECPRESENTATION_EXPORT ImageIdOverrideList* PresentationRuleSet::GetRules<ImageIdOverride>();
template<> ECPRESENTATION_EXPORT LabelOverrideList* PresentationRuleSet::GetRules<LabelOverride>();
template<> ECPRESENTATION_EXPORT StyleOverrideList* PresentationRuleSet::GetRules<StyleOverride>();
template<> ECPRESENTATION_EXPORT GroupingRuleList* PresentationRuleSet::GetRules<GroupingRule>();
template<> ECPRESENTATION_EXPORT CheckBoxRuleList* PresentationRuleSet::GetRules<CheckBoxRule>();
template<> ECPRESENTATION_EXPORT SortingRuleList* PresentationRuleSet::GetRules<SortingRule>();
template<> ECPRESENTATION_EXPORT UserSettingsGroupList* PresentationRuleSet::GetRules<UserSettingsGroup>();
template<> ECPRESENTATION_EXPORT ContentModifierList* PresentationRuleSet::GetRules<ContentModifier>();
template<> ECPRESENTATION_EXPORT InstanceLabelOverrideList* PresentationRuleSet::GetRules<InstanceLabelOverride>();
template<> ECPRESENTATION_EXPORT LocalizationResourceKeyDefinitionList* PresentationRuleSet::GetRules<LocalizationResourceKeyDefinition>();
template<> ECPRESENTATION_EXPORT ExtendedDataRuleList* PresentationRuleSet::GetRules<ExtendedDataRule>();
template<> ECPRESENTATION_EXPORT NodeArtifactsRuleList* PresentationRuleSet::GetRules<NodeArtifactsRule>();

END_BENTLEY_ECPRESENTATION_NAMESPACE
