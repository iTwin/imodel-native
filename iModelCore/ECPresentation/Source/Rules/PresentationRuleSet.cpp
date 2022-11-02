/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Version const& PresentationRuleSet::GetCurrentRulesetSchemaVersion()
    {
    static Version s_currentRulesetSchemaVersion(PRESENTATION_RULES_SCHEMA_VERSION_MAJOR, PRESENTATION_RULES_SCHEMA_VERSION_MINOR, PRESENTATION_RULES_SCHEMA_VERSION_PATCH);
    return s_currentRulesetSchemaVersion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::PresentationRuleSet (void)
    : m_schemaVersion(GetCurrentRulesetSchemaVersion()), m_isSupplemental(false), m_isSearchEnabled(true)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::PresentationRuleSet(Version schemaVersion, Utf8String ruleSetId)
    : m_ruleSetId(ruleSetId), m_schemaVersion(schemaVersion), m_isSupplemental(false), m_isSearchEnabled(true)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::PresentationRuleSet(PresentationRuleSetCR other)
    : m_schemaVersion(other.m_schemaVersion)
    {
    m_schemaVersion = other.m_schemaVersion;
    m_ruleSetId = other.m_ruleSetId;
    m_rulesetVersion = other.m_rulesetVersion;
    m_supportedSchemas = other.m_supportedSchemas;
    CommonToolsInternal::CopyRules(m_requiredSchemas, other.m_requiredSchemas, this);
    m_isSupplemental = other.m_isSupplemental;
    m_supplementationPurpose = other.m_supplementationPurpose;
    m_preferredImage = other.m_preferredImage;
    m_isSearchEnabled = other.m_isSearchEnabled;
    m_extendedData = other.m_extendedData;
    m_searchClasses = other.m_searchClasses;

    CommonToolsInternal::CopyRules(m_rootNodesRules, other.m_rootNodesRules, this);
    CommonToolsInternal::CopyRules(m_childNodesRules, other.m_childNodesRules, this);
    CommonToolsInternal::CopyRules(m_contentRules, other.m_contentRules, this);
    CommonToolsInternal::CopyRules(m_imageIdRules, other.m_imageIdRules, this);
    CommonToolsInternal::CopyRules(m_labelOverrides, other.m_labelOverrides, this);
    CommonToolsInternal::CopyRules(m_styleOverrides, other.m_styleOverrides, this);
    CommonToolsInternal::CopyRules(m_groupingRules, other.m_groupingRules, this);
    CommonToolsInternal::CopyRules(m_localizationResourceKeyDefinitions, other.m_localizationResourceKeyDefinitions, this);
    CommonToolsInternal::CopyRules(m_checkBoxRules, other.m_checkBoxRules, this);
    CommonToolsInternal::CopyRules(m_sortingRules, other.m_sortingRules, this);
    CommonToolsInternal::CopyRules(m_userSettings, other.m_userSettings, this);
    CommonToolsInternal::CopyRules(m_contentModifiers, other.m_contentModifiers, this);
    CommonToolsInternal::CopyRules(m_instanceLabelOverrides, other.m_instanceLabelOverrides, this);
    CommonToolsInternal::CopyRules(m_extendedDataRules, other.m_extendedDataRules, this);
    CommonToolsInternal::CopyRules(m_nodeArtifactRules, other.m_nodeArtifactRules, this);
    CommonToolsInternal::CopyRules(m_defaultPropertyCategoryOverrides, other.m_defaultPropertyCategoryOverrides, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::PresentationRuleSet(PresentationRuleSet&& other)
    : m_schemaVersion(std::move(other.m_schemaVersion))
    {
    m_ruleSetId = std::move(other.m_ruleSetId);
    m_rulesetVersion = std::move(other.m_rulesetVersion);
    m_supportedSchemas = std::move(other.m_supportedSchemas);
    CommonToolsInternal::SwapRules(m_requiredSchemas, other.m_requiredSchemas, this);
    m_isSupplemental = std::move(other.m_isSupplemental);
    m_supplementationPurpose = std::move(other.m_supplementationPurpose);
    m_preferredImage = std::move(other.m_preferredImage);
    m_isSearchEnabled = std::move(other.m_isSearchEnabled);
    m_extendedData = std::move(other.m_extendedData);
    m_searchClasses = std::move(other.m_searchClasses);

    CommonToolsInternal::SwapRules(m_rootNodesRules, other.m_rootNodesRules, this);
    CommonToolsInternal::SwapRules(m_childNodesRules, other.m_childNodesRules, this);
    CommonToolsInternal::SwapRules(m_contentRules, other.m_contentRules, this);
    CommonToolsInternal::SwapRules(m_imageIdRules, other.m_imageIdRules, this);
    CommonToolsInternal::SwapRules(m_labelOverrides, other.m_labelOverrides, this);
    CommonToolsInternal::SwapRules(m_styleOverrides, other.m_styleOverrides, this);
    CommonToolsInternal::SwapRules(m_groupingRules, other.m_groupingRules, this);
    CommonToolsInternal::SwapRules(m_localizationResourceKeyDefinitions, other.m_localizationResourceKeyDefinitions, this);
    CommonToolsInternal::SwapRules(m_checkBoxRules, other.m_checkBoxRules, this);
    CommonToolsInternal::SwapRules(m_sortingRules, other.m_sortingRules, this);
    CommonToolsInternal::SwapRules(m_userSettings, other.m_userSettings, this);
    CommonToolsInternal::SwapRules(m_contentModifiers, other.m_contentModifiers, this);
    CommonToolsInternal::SwapRules(m_instanceLabelOverrides, other.m_instanceLabelOverrides, this);
    CommonToolsInternal::SwapRules(m_extendedDataRules, other.m_extendedDataRules, this);
    CommonToolsInternal::SwapRules(m_nodeArtifactRules, other.m_nodeArtifactRules, this);
    CommonToolsInternal::SwapRules(m_defaultPropertyCategoryOverrides, other.m_defaultPropertyCategoryOverrides, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::~PresentationRuleSet ()
    {
    CommonToolsInternal::FreePresentationRules(m_requiredSchemas);
    CommonToolsInternal::FreePresentationRules(m_rootNodesRules);
    CommonToolsInternal::FreePresentationRules(m_childNodesRules);
    CommonToolsInternal::FreePresentationRules(m_contentRules);
    CommonToolsInternal::FreePresentationRules(m_imageIdRules);
    CommonToolsInternal::FreePresentationRules(m_labelOverrides);
    CommonToolsInternal::FreePresentationRules(m_styleOverrides);
    CommonToolsInternal::FreePresentationRules(m_groupingRules);
    CommonToolsInternal::FreePresentationRules(m_localizationResourceKeyDefinitions);
    CommonToolsInternal::FreePresentationRules(m_userSettings);
    CommonToolsInternal::FreePresentationRules(m_checkBoxRules);
    CommonToolsInternal::FreePresentationRules(m_sortingRules);
    CommonToolsInternal::FreePresentationRules(m_contentModifiers);
    CommonToolsInternal::FreePresentationRules(m_instanceLabelOverrides);
    CommonToolsInternal::FreePresentationRules(m_extendedDataRules);
    CommonToolsInternal::FreePresentationRules(m_nodeArtifactRules);
    CommonToolsInternal::FreePresentationRules(m_defaultPropertyCategoryOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::CreateInstance(Utf8String id)
    {
    return new PresentationRuleSet(GetCurrentRulesetSchemaVersion(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TRule>
static void CopyRulesByPriority(bvector<TRule*>& target, bvector<TRule*> const& source, HashableBase* parentHashable)
    {
    for (TRule const* rule : source)
        {
        TRule* copy = new TRule(*rule);
        copy->SetParent(parentHashable);
        CommonToolsInternal::AddToCollectionByPriority<TRule>(target, copy);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSet::MergeWith(PresentationRuleSetCR other)
    {
    CopyRulesByPriority(m_rootNodesRules, other.m_rootNodesRules, this);
    CopyRulesByPriority(m_childNodesRules, other.m_childNodesRules, this);
    CopyRulesByPriority(m_contentRules, other.m_contentRules, this);
    CopyRulesByPriority(m_imageIdRules, other.m_imageIdRules, this);
    CopyRulesByPriority(m_labelOverrides, other.m_labelOverrides, this);
    CopyRulesByPriority(m_instanceLabelOverrides, other.m_instanceLabelOverrides, this);
    CopyRulesByPriority(m_styleOverrides, other.m_styleOverrides, this);
    CopyRulesByPriority(m_groupingRules, other.m_groupingRules, this);
    CopyRulesByPriority(m_localizationResourceKeyDefinitions, other.m_localizationResourceKeyDefinitions, this);
    CopyRulesByPriority(m_userSettings, other.m_userSettings, this);
    CopyRulesByPriority(m_checkBoxRules, other.m_checkBoxRules, this);
    CopyRulesByPriority(m_sortingRules, other.m_sortingRules, this);
    CopyRulesByPriority(m_contentModifiers, other.m_contentModifiers, this);
    CopyRulesByPriority(m_extendedDataRules, other.m_extendedDataRules, this);
    CopyRulesByPriority(m_nodeArtifactRules, other.m_nodeArtifactRules, this);
    CopyRulesByPriority(m_defaultPropertyCategoryOverrides, other.m_defaultPropertyCategoryOverrides, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationRuleSet::GetFullRuleSetId (void) const
    {
    Utf8String fullId;
    if (GetIsSupplemental())
        fullId.Sprintf("%s_supplemental_%s", GetRuleSetId().c_str(), GetSupplementationPurpose().c_str());
    else
        fullId = GetRuleSetId();

    if (GetRulesetVersion().IsValid())
        fullId.append(".").append(GetRulesetVersion().Value().ToString());

    return fullId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::ReadXml (BeXmlDomR xmlDom)
    {
    BeXmlNodeP ruleSetNode;
    if ((BEXML_Success != xmlDom.SelectNode(ruleSetNode, "/" PRESENTATION_RULE_SET_XML_NODE_NAME, NULL, BeXmlDom::NODE_BIAS_First)) || (NULL == ruleSetNode))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid XML: Missing a top-level `%s` node", PRESENTATION_RULE_SET_XML_NODE_NAME));
        return false;
        }

    // required:
    if (BEXML_Success != ruleSetNode->GetAttributeStringValue(m_ruleSetId, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString(INVALID_XML, PRESENTATION_RULE_SET_XML_NODE_NAME, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID));
        return false;
        }

    // optional:
    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_supportedSchemas, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS))
        m_supportedSchemas = "";

    if (BEXML_Success != ruleSetNode->GetAttributeBooleanValue (m_isSupplemental, PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSUPPLEMENTAL))
        m_isSupplemental = false;

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_supplementationPurpose, PRESENTATION_RULE_SET_XML_ATTRIBUTE_SUPPLEMENTATIONPURPOSE))
        m_supplementationPurpose = "";

    int versionMajor, versionMinor;
    if (BEXML_Success != ruleSetNode->GetAttributeInt32Value (versionMajor, PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMAJOR))
        versionMajor = 1;
    if (BEXML_Success != ruleSetNode->GetAttributeInt32Value (versionMinor, PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMINOR))
        versionMinor = 0;
    m_rulesetVersion = Version(versionMajor, versionMinor, 0);

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_preferredImage, PRESENTATION_RULE_SET_XML_ATTRIBUTE_PREFERREDIMAGE))
        m_preferredImage = "";

    if (BEXML_Success != ruleSetNode->GetAttributeBooleanValue (m_isSearchEnabled, PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSEARCHENABLED))
        m_isSearchEnabled = true;

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_searchClasses, PRESENTATION_RULE_SET_XML_ATTRIBUTE_SEARCHCLASSES))
        m_searchClasses = "";

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_extendedData, PRESENTATION_RULE_SET_XML_ATTRIBUTE_EXTENDEDDATA))
        m_extendedData = "";

    CommonToolsInternal::LoadRulesFromXmlNode <RootNodeRule>      (ruleSetNode, m_rootNodesRules,   ROOT_NODE_RULE_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <ChildNodeRule>     (ruleSetNode, m_childNodesRules,  CHILD_NODE_RULE_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <ContentRule>       (ruleSetNode, m_contentRules,     CONTENT_RULE_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <ImageIdOverride>   (ruleSetNode, m_imageIdRules,     IMAGE_ID_OVERRIDE_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <LabelOverride>     (ruleSetNode, m_labelOverrides,   LABEL_OVERRIDE_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <StyleOverride>     (ruleSetNode, m_styleOverrides,   STYLE_OVERRIDE_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <GroupingRule>      (ruleSetNode, m_groupingRules,    GROUPING_RULE_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <LocalizationResourceKeyDefinition> (ruleSetNode, m_localizationResourceKeyDefinitions, LOCALIZATION_DEFINITION_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <UserSettingsGroup> (ruleSetNode, m_userSettings,     USER_SETTINGS_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <CheckBoxRule>      (ruleSetNode, m_checkBoxRules,    CHECKBOX_RULE_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <SortingRule>       (ruleSetNode, m_sortingRules,     SORTING_RULE_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <ContentModifier>   (ruleSetNode, m_contentModifiers, CONTENTMODIFIER_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode <InstanceLabelOverride> (ruleSetNode, m_instanceLabelOverrides, INSTANCE_LABEL_OVERRIDE_XML_NODE_NAME, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSet::WriteXml (BeXmlDomR xmlDom) const
    {
    BeXmlNodeP ruleSetNode = xmlDom.AddNewElement (PRESENTATION_RULE_SET_XML_NODE_NAME, NULL, NULL);

    ruleSetNode->AddAttributeStringValue  (PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID,              m_ruleSetId.c_str ());
    ruleSetNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS,                      m_supportedSchemas.c_str ());
    ruleSetNode->AddAttributeBooleanValue (PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSUPPLEMENTAL,         m_isSupplemental);
    ruleSetNode->AddAttributeStringValue  (PRESENTATION_RULE_SET_XML_ATTRIBUTE_SUPPLEMENTATIONPURPOSE, m_supplementationPurpose.c_str ());
    ruleSetNode->AddAttributeInt32Value   (PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMAJOR,           m_rulesetVersion.IsValid() ? m_rulesetVersion.Value().GetMajor() : 1);
    ruleSetNode->AddAttributeInt32Value   (PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMINOR,           m_rulesetVersion.IsValid() ? m_rulesetVersion.Value().GetMinor() : 0);
    ruleSetNode->AddAttributeStringValue  (PRESENTATION_RULE_SET_XML_ATTRIBUTE_PREFERREDIMAGE,         m_preferredImage.c_str ());
    ruleSetNode->AddAttributeBooleanValue (PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSEARCHENABLED,        m_isSearchEnabled);
    ruleSetNode->AddAttributeStringValue  (PRESENTATION_RULE_SET_XML_ATTRIBUTE_SEARCHCLASSES,          m_searchClasses.c_str ());
    ruleSetNode->AddAttributeStringValue  (PRESENTATION_RULE_SET_XML_ATTRIBUTE_EXTENDEDDATA,           m_extendedData.c_str ());

    CommonToolsInternal::WriteRulesToXmlNode<RootNodeRule,      RootNodeRuleList>      (ruleSetNode, m_rootNodesRules);
    CommonToolsInternal::WriteRulesToXmlNode<ChildNodeRule,     ChildNodeRuleList>     (ruleSetNode, m_childNodesRules);
    CommonToolsInternal::WriteRulesToXmlNode<ContentRule,       ContentRuleList>       (ruleSetNode, m_contentRules);
    CommonToolsInternal::WriteRulesToXmlNode<ImageIdOverride,   ImageIdOverrideList>   (ruleSetNode, m_imageIdRules);
    CommonToolsInternal::WriteRulesToXmlNode<LabelOverride,     LabelOverrideList>     (ruleSetNode, m_labelOverrides);
    CommonToolsInternal::WriteRulesToXmlNode<StyleOverride,     StyleOverrideList>     (ruleSetNode, m_styleOverrides);
    CommonToolsInternal::WriteRulesToXmlNode<GroupingRule,      GroupingRuleList>      (ruleSetNode, m_groupingRules);
    CommonToolsInternal::WriteRulesToXmlNode<LocalizationResourceKeyDefinition, LocalizationResourceKeyDefinitionList> (ruleSetNode, m_localizationResourceKeyDefinitions);
    CommonToolsInternal::WriteRulesToXmlNode<UserSettingsGroup, UserSettingsGroupList> (ruleSetNode, m_userSettings);
    CommonToolsInternal::WriteRulesToXmlNode<CheckBoxRule,      CheckBoxRuleList>      (ruleSetNode, m_checkBoxRules);
    CommonToolsInternal::WriteRulesToXmlNode<SortingRule,       SortingRuleList>       (ruleSetNode, m_sortingRules);
    CommonToolsInternal::WriteRulesToXmlNode<ContentModifier,   ContentModifierList>   (ruleSetNode, m_contentModifiers);
    CommonToolsInternal::WriteRulesToXmlNode<InstanceLabelOverride,InstanceLabelOverrideList> (ruleSetNode, m_instanceLabelOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlString (Utf8CP xmlString)
    {
    auto scope = Diagnostics::Scope::Create("Read PresentationRuleSet from XML string");

    BeXmlStatus xmlStatus;
    size_t stringSize = strlen (xmlString) * sizeof(Utf8Char);
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString, stringSize / sizeof(Utf8Char));
    if (BEXML_Success != xmlStatus)
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, "Failed to read XML from string. Is XML valid?");
        return NULL;
        }

    PresentationRuleSetPtr ruleSet = new PresentationRuleSet();
    if (ruleSet->ReadXml(*xmlDom.get()))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_INFO, "Successfully read presentation rules");
        return ruleSet;
        }

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, "Failed to read presentation rules from XML.");
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlFile (BeFileNameCR xmlFilePath)
    {
    auto scope = Diagnostics::Scope::Create("Read PresentationRuleSet from XML file");

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, xmlFilePath.c_str());
    if (xmlStatus != BEXML_Success || !xmlDom.IsValid())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, "Failed to read XML from file. Is XML valid?");
        return NULL;
        }

    PresentationRuleSetPtr ruleSet = new PresentationRuleSet();
    if (ruleSet->ReadXml(*xmlDom.get()))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_INFO, "Successfully read presentation rules");
        return ruleSet;
        }

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, "Failed to read presentation rules from XML.");
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationRuleSet::WriteToXmlString () const
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();
    WriteXml (*xmlDom.get());

    Utf8String presentationRuleSetXml;
    xmlDom->ToString (presentationRuleSetXml, BeXmlDom::TO_STRING_OPTION_Default);

    return presentationRuleSetXml;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::WriteToXmlFile (BeFileNameCR xmlFilePath) const
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();
    WriteXml (*xmlDom.get());

    return BEXML_Success == xmlDom->ToFile(xmlFilePath.c_str(), (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Indent | BeXmlDom::TO_STRING_OPTION_Formatted), BeXmlDom::FILE_ENCODING_Utf8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::ReadJson(JsonValueCR json)
    {
    // required:
    m_ruleSetId = json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_ruleSetId.empty(), "PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID, json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID], "non-empty string"))
        return false;

    // optional:
    m_supportedSchemas = CommonToolsInternal::SupportedSchemasToString(json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS]);

    if (json.isMember(COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS))
        CommonToolsInternal::LoadFromJson("PresentationRuleSet", COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS, json[COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS], m_requiredSchemas, CommonToolsInternal::LoadRuleFromJson<RequiredSchemaSpecification>, this);

    if (json.isMember(PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SCHEMA_VERSION))
        {
        if (SUCCESS != Version::FromString(m_schemaVersion, json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SCHEMA_VERSION].asCString()))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid value for `%s.%s`: `%s`. Expected %s.",
                "PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SCHEMA_VERSION, json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SCHEMA_VERSION].ToString().c_str(),
                "version string in format {major}.{minor}.{patch}"));
            m_schemaVersion = GetCurrentRulesetSchemaVersion();
            }
        }

    if (json.isMember(PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESET_VERSION))
        {
        Version rulesetVersion;
        if (SUCCESS != Version::FromString(rulesetVersion, json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESET_VERSION].asCString()))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid value for `%s.%s`: `%s`. Expected %s.",
                "PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESET_VERSION, json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESET_VERSION].ToString().c_str(),
                "version string in format {major}.{minor}.{patch}"));
            }
        else
            m_rulesetVersion = rulesetVersion;
        }

    if (json.isMember(PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO))
        {
        m_isSupplemental = true;
        m_supplementationPurpose = json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO][PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO_PURPOSE].asCString("");
        }

    if (json.isMember(PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES))
        {
        JsonValueCR rulesJson = json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES];
        if (CommonToolsInternal::ValidateJsonValueType("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, Json::arrayValue))
            {
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_childNodesRules, CommonToolsInternal::LoadRuleFromJson<ChildNodeRule>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_rootNodesRules, CommonToolsInternal::LoadRuleFromJson<RootNodeRule>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_contentRules, CommonToolsInternal::LoadRuleFromJson<ContentRule>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_contentModifiers, CommonToolsInternal::LoadRuleFromJson<ContentModifier>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_checkBoxRules, CommonToolsInternal::LoadRuleFromJson<CheckBoxRule>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_groupingRules, CommonToolsInternal::LoadRuleFromJson<GroupingRule>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_imageIdRules, CommonToolsInternal::LoadRuleFromJson<ImageIdOverride>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_labelOverrides, CommonToolsInternal::LoadRuleFromJson<LabelOverride>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_sortingRules, CommonToolsInternal::LoadRuleFromJson<SortingRule>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_styleOverrides, CommonToolsInternal::LoadRuleFromJson<StyleOverride>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_instanceLabelOverrides, CommonToolsInternal::LoadRuleFromJson<InstanceLabelOverride>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_extendedDataRules, CommonToolsInternal::LoadRuleFromJson<ExtendedDataRule>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_nodeArtifactRules, CommonToolsInternal::LoadRuleFromJson<NodeArtifactsRule>, this);
            CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES, rulesJson, m_defaultPropertyCategoryOverrides, CommonToolsInternal::LoadRuleFromJson<DefaultPropertyCategoryOverride>, this);
            }
        }

    if (json.isMember(PRESENTATION_RULE_SET_JSON_ATTRIBUTE_USERSETTINGS))
        CommonToolsInternal::LoadFromJsonByPriority("PresentationRuleSet", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_USERSETTINGS, json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_USERSETTINGS], m_userSettings, CommonToolsInternal::LoadRuleFromJson<UserSettingsGroup>, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSet::WriteJson(JsonValueR json) const
    {
    json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID] = m_ruleSetId;
    if (!m_supportedSchemas.empty())
        json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS] = CommonToolsInternal::SupportedSchemasToJson(m_supportedSchemas);
    if (!m_requiredSchemas.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RequiredSchemaSpecification, RequiredSchemaSpecificationsList>
            (json[COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS], m_requiredSchemas);
        }
    if (m_isSupplemental)
        json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO][PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO_PURPOSE] = m_supplementationPurpose;

    Json::Value rulesJson(Json::arrayValue);
    CommonToolsInternal::WriteRulesToJson<RootNodeRule, RootNodeRuleList>(rulesJson, m_rootNodesRules);
    CommonToolsInternal::WriteRulesToJson<ChildNodeRule, ChildNodeRuleList>(rulesJson, m_childNodesRules);
    CommonToolsInternal::WriteRulesToJson<ContentRule, ContentRuleList>(rulesJson, m_contentRules);
    CommonToolsInternal::WriteRulesToJson<ContentModifier, ContentModifierList>(rulesJson, m_contentModifiers);
    CommonToolsInternal::WriteRulesToJson<ImageIdOverride, ImageIdOverrideList>(rulesJson, m_imageIdRules);
    CommonToolsInternal::WriteRulesToJson<InstanceLabelOverride,InstanceLabelOverrideList>(rulesJson, m_instanceLabelOverrides);
    CommonToolsInternal::WriteRulesToJson<LabelOverride, LabelOverrideList>(rulesJson, m_labelOverrides);
    CommonToolsInternal::WriteRulesToJson<StyleOverride, StyleOverrideList>(rulesJson, m_styleOverrides);
    CommonToolsInternal::WriteRulesToJson<ExtendedDataRule, ExtendedDataRuleList>(rulesJson, m_extendedDataRules);
    CommonToolsInternal::WriteRulesToJson<NodeArtifactsRule, NodeArtifactsRuleList>(rulesJson, m_nodeArtifactRules);
    CommonToolsInternal::WriteRulesToJson<GroupingRule, GroupingRuleList>(rulesJson, m_groupingRules);
    CommonToolsInternal::WriteRulesToJson<CheckBoxRule, CheckBoxRuleList>(rulesJson, m_checkBoxRules);
    CommonToolsInternal::WriteRulesToJson<SortingRule, SortingRuleList>(rulesJson, m_sortingRules);
    CommonToolsInternal::WriteRulesToJson<DefaultPropertyCategoryOverride, DefaultPropertyCategoryOverridesList>(rulesJson, m_defaultPropertyCategoryOverrides);
    json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES] = rulesJson;

    if (!m_userSettings.empty())
        CommonToolsInternal::WriteRulesToJson<UserSettingsGroup, UserSettingsGroupList>(json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_USERSETTINGS], m_userSettings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromJsonValue(JsonValueCR json)
    {
    auto scope = Diagnostics::Scope::Create("Read PresentationRuleSet from JSON value");

    PresentationRuleSetPtr ruleSet = new PresentationRuleSet();
    if (!json.isNull() && ruleSet->ReadJson(json))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Successfully read: %s", ruleSet->WriteToJsonValue().ToString().c_str()));
        return ruleSet;
        }

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Failed to read: ", json.ToString().c_str()));
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromJsonString(Utf8StringCR jsonString)
    {
    auto scope = Diagnostics::Scope::Create("Read PresentationRuleSet from JSON string");
    Json::Value json = Json::Reader::DoParse(jsonString);
    return ReadFromJsonValue(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromJsonFile(BeFileNameCR jsonFilePath)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Read PresentationRuleSet from JSON file: '%s'", jsonFilePath.GetNameUtf8().c_str()));

    BeFile f;
    if (BeFileStatus::Success != f.Open(jsonFilePath.c_str(), BeFileAccess::Read))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_WARNING, LOG_ERROR, "Failed to open file for read");
        return nullptr;
        }

    bvector<Byte> fileContents;
    if (BeFileStatus::Success != f.ReadEntireFile(fileContents))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_WARNING, LOG_ERROR, "Failed to read file contents");
        f.Close();
        return nullptr;
        }
    f.Close();

    Utf8String serializedJson((Utf8CP)&*fileContents.begin(), (Utf8CP)&*fileContents.end());
    return ReadFromJsonString(serializedJson);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PresentationRuleSet::WriteToJsonValue() const
    {
    Json::Value json;
    WriteJson(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::WriteToJsonFile(BeFileNameCR jsonFilePath) const
    {
    Json::Value json = WriteToJsonValue();
    Utf8String serializedJson = json.toStyledString();

    BeFile f;
    BeFileStatus status = BeFileStatus::UnknownError;
    if (BeFileStatus::Success != (status = f.Create(jsonFilePath.c_str())))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to create the file '%s'. Status: %d", jsonFilePath.c_str(), (int)status));

    if (BeFileStatus::Success != (status = f.Write(nullptr, &serializedJson[0], serializedJson.size())))
        {
        f.Close();
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to write to the file '%s'. Status: %d", jsonFilePath.c_str(), (int)status));
        }

    status = f.Close();
    if (BeFileStatus::Success != status)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to close the file. Status: %d", (int)status));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetRuleSetId (void) const         { return m_ruleSetId;        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                   PresentationRuleSet::SetRuleSetId(Utf8StringCR id)     { m_ruleSetId = id;          }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSet::ClearRequiredSchemaSpecifications()
    {
    CommonToolsInternal::FreePresentationRules(m_requiredSchemas);
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSet::AddRequiredSchemaSpecification(RequiredSchemaSpecificationR spec) { ADD_HASHABLE_CHILD(m_requiredSchemas, spec); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetPreferredImage (void) const    { return m_preferredImage;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool                   PresentationRuleSet::GetIsSearchEnabled (void) const   { return m_isSearchEnabled;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetSearchClasses (void) const     { return m_searchClasses;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                   PresentationRuleSet::SetSearchClasses (Utf8StringCR searchClasses) { m_searchClasses = searchClasses; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetExtendedData (void) const      { return m_extendedData;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                   PresentationRuleSet::SetExtendedData (Utf8StringCR extendedData) { m_extendedData = extendedData; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRuleList const& PresentationRuleSet::GetRootNodesRules (void) const   { return m_rootNodesRules;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRuleList const& PresentationRuleSet::GetChildNodesRules (void) const { return m_childNodesRules;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRuleList const& PresentationRuleSet::GetContentRules (void) const      { return m_contentRules;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverrideList const& PresentationRuleSet::GetImageIdOverrides (void) const { return m_imageIdRules;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverrideList const& PresentationRuleSet::GetLabelOverrides (void) const  { return m_labelOverrides;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverrideList const& PresentationRuleSet::GetStyleOverrides (void) const  { return m_styleOverrides;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRuleList const& PresentationRuleSet::GetGroupingRules (void) const    { return m_groupingRules;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinitionList const& PresentationRuleSet::GetLocalizationResourceKeyDefinitions (void) const { return m_localizationResourceKeyDefinitions; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroupList const& PresentationRuleSet::GetUserSettings (void) const { return m_userSettings;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CheckBoxRuleList const& PresentationRuleSet::GetCheckBoxRules (void) const    { return m_checkBoxRules;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRuleList const& PresentationRuleSet::GetSortingRules (void) const      { return m_sortingRules;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifierList const& PresentationRuleSet::GetContentModifierRules (void) const { return m_contentModifiers; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverrideList const& PresentationRuleSet::GetInstanceLabelOverrides (void) const { return m_instanceLabelOverrides; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExtendedDataRuleList const& PresentationRuleSet::GetExtendedDataRules() const { return m_extendedDataRules; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodeArtifactsRuleList const& PresentationRuleSet::GetNodeArtifactRules() const { return m_nodeArtifactRules; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PresentationRuleSet::_ComputeHash() const
    {
    MD5 md5;

    if (!m_ruleSetId.empty())
        ADD_STR_VALUE_TO_HASH(md5, PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID, m_ruleSetId);

    if (!m_supportedSchemas.empty())
        ADD_STR_VALUE_TO_HASH(md5, PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS, m_supportedSchemas);
    ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS, m_requiredSchemas);

    if (m_isSupplemental)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, "IsSupplemental", m_isSupplemental);
    if (!m_supplementationPurpose.empty())
        ADD_STR_VALUE_TO_HASH(md5, PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO_PURPOSE, m_supplementationPurpose);

    if (!m_preferredImage.empty())
        ADD_STR_VALUE_TO_HASH(md5, PRESENTATION_RULE_SET_XML_ATTRIBUTE_PREFERREDIMAGE, m_preferredImage);
    if (m_isSearchEnabled)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSEARCHENABLED, m_isSearchEnabled);
    if (!m_extendedData.empty())
        ADD_STR_VALUE_TO_HASH(md5, PRESENTATION_RULE_SET_XML_ATTRIBUTE_EXTENDEDDATA, m_extendedData);
    if (!m_searchClasses.empty())
        ADD_STR_VALUE_TO_HASH(md5, PRESENTATION_RULE_SET_XML_ATTRIBUTE_SEARCHCLASSES, m_searchClasses);

    ADD_RULES_TO_HASH(md5, "RootNodeRules", m_rootNodesRules);
    ADD_RULES_TO_HASH(md5, "ChildNodeRules", m_childNodesRules);
    ADD_RULES_TO_HASH(md5, "ContentRules", m_contentRules);
    ADD_RULES_TO_HASH(md5, "ImageIdRules", m_imageIdRules);
    ADD_RULES_TO_HASH(md5, "LabelOverrides", m_labelOverrides);
    ADD_RULES_TO_HASH(md5, "StyleOverrides", m_styleOverrides);
    ADD_RULES_TO_HASH(md5, "GroupingRules", m_groupingRules);
    ADD_RULES_TO_HASH(md5, "LocalizationResourceKeyDefinitions", m_localizationResourceKeyDefinitions);
    ADD_RULES_TO_HASH(md5, "CheckboxRules", m_checkBoxRules);
    ADD_RULES_TO_HASH(md5, "SortingRules", m_sortingRules);
    ADD_RULES_TO_HASH(md5, "UserSettings", m_userSettings);
    ADD_RULES_TO_HASH(md5, "ContentModifiers", m_contentModifiers);
    ADD_RULES_TO_HASH(md5, "InstanceLabelOverrides", m_instanceLabelOverrides);
    ADD_RULES_TO_HASH(md5, "ExtendedDataRules", m_extendedDataRules);
    ADD_RULES_TO_HASH(md5, "NodeArtifactRules", m_nodeArtifactRules);
    ADD_RULES_TO_HASH(md5, "DefaultPropertyCategoryOverrides", m_defaultPropertyCategoryOverrides);

    return md5;
    }

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
template<> RootNodeRuleList* PresentationRuleSet::GetRules<RootNodeRule>() {return &m_rootNodesRules;}
template<> ChildNodeRuleList* PresentationRuleSet::GetRules<ChildNodeRule>() {return &m_childNodesRules;}
template<> ContentRuleList* PresentationRuleSet::GetRules<ContentRule>() {return &m_contentRules;}
template<> ImageIdOverrideList* PresentationRuleSet::GetRules<ImageIdOverride>() {return &m_imageIdRules;}
template<> LabelOverrideList* PresentationRuleSet::GetRules<LabelOverride>() {return &m_labelOverrides;}
template<> StyleOverrideList* PresentationRuleSet::GetRules<StyleOverride>() {return &m_styleOverrides;}
template<> GroupingRuleList* PresentationRuleSet::GetRules<GroupingRule>() {return &m_groupingRules;}
template<> CheckBoxRuleList* PresentationRuleSet::GetRules<CheckBoxRule>() {return &m_checkBoxRules;}
template<> SortingRuleList* PresentationRuleSet::GetRules<SortingRule>() {return &m_sortingRules;}
template<> ContentModifierList* PresentationRuleSet::GetRules<ContentModifier>() {return &m_contentModifiers;}
template<> UserSettingsGroupList* PresentationRuleSet::GetRules<UserSettingsGroup>() {return &m_userSettings;}
template<> InstanceLabelOverrideList* PresentationRuleSet::GetRules<InstanceLabelOverride>() {return &m_instanceLabelOverrides;}
template<> LocalizationResourceKeyDefinitionList* PresentationRuleSet::GetRules<LocalizationResourceKeyDefinition>() {return &m_localizationResourceKeyDefinitions;}
template<> ExtendedDataRuleList* PresentationRuleSet::GetRules<ExtendedDataRule>() { return &m_extendedDataRules; }
template<> NodeArtifactsRuleList* PresentationRuleSet::GetRules<NodeArtifactsRule>() { return &m_nodeArtifactRules; }
template<> DefaultPropertyCategoryOverridesList* PresentationRuleSet::GetRules<DefaultPropertyCategoryOverride>() {return &m_defaultPropertyCategoryOverrides;}
END_BENTLEY_ECPRESENTATION_NAMESPACE
