/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/PresentationRuleSet.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::PresentationRuleSet (void)
    : m_ruleSetId (""), m_supportedSchemas (""), m_isSupplemental (false), m_supplementationPurpose (""), m_versionMajor (1), m_versionMinor (0), m_isSearchEnabled (true),
      m_extendedData (""), m_searchClasses ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::PresentationRuleSet
(
Utf8StringCR ruleSetId,
int          versionMajor,
int          versionMinor,
bool         isSupplemental,
Utf8StringCR supplementationPurpose,
Utf8StringCR supportedSchemas,
Utf8StringCR preferredImage,
bool         isSearchEnabled
) : m_ruleSetId (ruleSetId), m_supportedSchemas (supportedSchemas), m_isSupplemental (isSupplemental), m_supplementationPurpose (supplementationPurpose),
    m_versionMajor (versionMajor), m_versionMinor (versionMinor), m_preferredImage (preferredImage), m_isSearchEnabled (isSearchEnabled)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::~PresentationRuleSet ()
    {
    CommonToolsInternal::FreePresentationRules (m_rootNodesRules);
    CommonToolsInternal::FreePresentationRules (m_childNodesRules);
    CommonToolsInternal::FreePresentationRules (m_contentRules);
    CommonToolsInternal::FreePresentationRules (m_imageIdRules);
    CommonToolsInternal::FreePresentationRules (m_labelOverrides);
    CommonToolsInternal::FreePresentationRules (m_styleOverrides);
    CommonToolsInternal::FreePresentationRules (m_groupingRules);
    CommonToolsInternal::FreePresentationRules (m_localizationResourceKeyDefinitions);
    CommonToolsInternal::FreePresentationRules (m_userSettings);
    CommonToolsInternal::FreePresentationRules (m_checkBoxRules);
    CommonToolsInternal::FreePresentationRules (m_sortingRules);
    CommonToolsInternal::FreePresentationRules (m_contentModifiers);
    CommonToolsInternal::FreePresentationRules (m_instanceLabelOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::CreateInstance
(
Utf8StringCR ruleSetId,
int          versionMajor,
int          versionMinor,
bool         isSupplemental,
Utf8StringCR supplementationPurpose,
Utf8StringCR supportedSchemas,
Utf8StringCR preferredImage,
bool         isSearchEnabled
)
    {
    return new PresentationRuleSet (ruleSetId, versionMajor, versionMinor, isSupplemental, supplementationPurpose, supportedSchemas, preferredImage, isSearchEnabled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::CreateInstance
(
Utf8StringCR id,
int          versionMajor,
int          versionMinor,
Utf8StringCR supportedSchemas
)
    {
    return PresentationRuleSet::CreateInstance(id, versionMajor, versionMinor, false, "", supportedSchemas, "", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::CreateInstance
(
Utf8StringCR id,
Utf8StringCR supplementationPurpose,
int          versionMajor,
int          versionMinor,
Utf8StringCR supportedSchemas
)
    {
    return PresentationRuleSet::CreateInstance(id, versionMajor, versionMinor, true, supplementationPurpose, supportedSchemas, "", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationRuleSet::GetFullRuleSetId (void) const
    {
    Utf8Char fullId[255];

    if (GetIsSupplemental ())
        BeStringUtilities::Snprintf (fullId, "%s_supplemental_%s.%02d.%02d", GetRuleSetId ().c_str(), GetSupplementationPurpose ().c_str(), GetVersionMajor (), GetVersionMinor ());
    else
        BeStringUtilities::Snprintf (fullId, "%s.%02d.%02d", GetRuleSetId ().c_str(), GetVersionMajor (), GetVersionMinor ());

    return fullId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::ReadXml (BeXmlDomR xmlDom)
    {
    BeXmlNodeP ruleSetNode;
    if ( (BEXML_Success != xmlDom.SelectNode (ruleSetNode, "/" PRESENTATION_RULE_SET_XML_NODE_NAME, NULL, BeXmlDom::NODE_BIAS_First)) || (NULL == ruleSetNode) )
        {
        ECPRENSETATION_RULES_LOG.errorv ("Invalid PresentationRuleSetXML: Missing a top-level %s node", PRESENTATION_RULE_SET_XML_NODE_NAME);
        return false;
        }
    
    //Required:
    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_ruleSetId, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID))
        {
        ECPRENSETATION_RULES_LOG.errorv (INVALID_XML, PRESENTATION_RULE_SET_XML_NODE_NAME, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID);
        return false;
        }

    //Optional:
    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_supportedSchemas, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS))
        m_supportedSchemas = "";

    if (BEXML_Success != ruleSetNode->GetAttributeBooleanValue (m_isSupplemental, PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSUPPLEMENTAL))
        m_isSupplemental = false;

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_supplementationPurpose, PRESENTATION_RULE_SET_XML_ATTRIBUTE_SUPPLEMENTATIONPURPOSE))
        m_supplementationPurpose = "";

    if (BEXML_Success != ruleSetNode->GetAttributeInt32Value (m_versionMajor, PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMAJOR))
        m_versionMajor = 1;

    if (BEXML_Success != ruleSetNode->GetAttributeInt32Value (m_versionMinor, PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMINOR))
        m_versionMinor = 0;

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_preferredImage, PRESENTATION_RULE_SET_XML_ATTRIBUTE_PREFERREDIMAGE))
        m_preferredImage = "";

    if (BEXML_Success != ruleSetNode->GetAttributeBooleanValue (m_isSearchEnabled, PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSEARCHENABLED))
        m_isSearchEnabled = true;

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_searchClasses, PRESENTATION_RULE_SET_XML_ATTRIBUTE_SEARCHCLASSES))
        m_searchClasses = "";

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_extendedData, PRESENTATION_RULE_SET_XML_ATTRIBUTE_EXTENDEDDATA))
        m_extendedData = "";

    CommonToolsInternal::LoadRulesFromXmlNode <RootNodeRule>      (ruleSetNode, m_rootNodesRules,   ROOT_NODE_RULE_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <ChildNodeRule>     (ruleSetNode, m_childNodesRules,  CHILD_NODE_RULE_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <ContentRule>       (ruleSetNode, m_contentRules,     CONTENT_RULE_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <ImageIdOverride>   (ruleSetNode, m_imageIdRules,     IMAGE_ID_OVERRIDE_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <LabelOverride>     (ruleSetNode, m_labelOverrides,   LABEL_OVERRIDE_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <StyleOverride>     (ruleSetNode, m_styleOverrides,   STYLE_OVERRIDE_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <GroupingRule>      (ruleSetNode, m_groupingRules,    GROUPING_RULE_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <LocalizationResourceKeyDefinition> (ruleSetNode, m_localizationResourceKeyDefinitions, LOCALIZATION_DEFINITION_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <UserSettingsGroup> (ruleSetNode, m_userSettings,     USER_SETTINGS_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <CheckBoxRule>      (ruleSetNode, m_checkBoxRules,    CHECKBOX_RULE_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <SortingRule>       (ruleSetNode, m_sortingRules,     SORTING_RULE_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <ContentModifier>   (ruleSetNode, m_contentModifiers, CONTENTMODIEFIER_XML_NODE_NAME);
    CommonToolsInternal::LoadRulesFromXmlNode <InstanceLabelOverride> (ruleSetNode, m_instanceLabelOverrides, INSTANCE_LABEL_OVERRIDE_XML_NODE_NAME);
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSet::WriteXml (BeXmlDomR xmlDom) const
    {
    BeXmlNodeP ruleSetNode = xmlDom.AddNewElement (PRESENTATION_RULE_SET_XML_NODE_NAME, NULL, NULL);

    ruleSetNode->AddAttributeStringValue  (PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID,              m_ruleSetId.c_str ());
    ruleSetNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS,                      m_supportedSchemas.c_str ());
    ruleSetNode->AddAttributeBooleanValue (PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSUPPLEMENTAL,         m_isSupplemental);
    ruleSetNode->AddAttributeStringValue  (PRESENTATION_RULE_SET_XML_ATTRIBUTE_SUPPLEMENTATIONPURPOSE, m_supplementationPurpose.c_str ());
    ruleSetNode->AddAttributeInt32Value   (PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMAJOR,           m_versionMajor);
    ruleSetNode->AddAttributeInt32Value   (PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMINOR,           m_versionMinor);
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
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlString (Utf8CP xmlString)
    {
    ECPRENSETATION_RULES_LOG.debugv ("About to read PrsentationRuleSet from string.");

    BeXmlStatus xmlStatus;
    size_t stringSize = strlen (xmlString) * sizeof(Utf8Char);
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, xmlString, stringSize / sizeof (Utf8Char));
    
    if (BEXML_Success != xmlStatus)
        {
        ECPRENSETATION_RULES_LOG.errorv ("Failed to load PresentationRuleSet from xml string.");
        return NULL;
        }
    
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet ();
    if (ruleSet->ReadXml (*xmlDom.get ()))
        return ruleSet;

    ECPRENSETATION_RULES_LOG.errorv ("Failed to load PresentationRuleSet from xml string.");
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlFile (BeFileNameCR xmlFilePath)
    {
    ECPRENSETATION_RULES_LOG.debugv (L"About to read PrsentationRuleSet from file: fileName='%ls'", xmlFilePath.c_str());
        
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, xmlFilePath.c_str());
    if (xmlStatus != BEXML_Success || !xmlDom.IsValid ())
        {
        ECPRENSETATION_RULES_LOG.errorv (L"Failed to load PresentationRuleSet from file: fileName='%ls'", xmlFilePath.c_str());
        return NULL;
        }
    
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet ();
    if (ruleSet->ReadXml (*xmlDom.get ()))
        return ruleSet;

    ECPRENSETATION_RULES_LOG.errorv (L"Failed to load PresentationRuleSet from file: fileName='%ls'", xmlFilePath.c_str());
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
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
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::WriteToXmlFile (BeFileNameCR xmlFilePath) const
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();        
    WriteXml (*xmlDom.get());

    return BEXML_Success == xmlDom->ToFile(xmlFilePath.c_str(), (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Indent | BeXmlDom::TO_STRING_OPTION_Formatted), BeXmlDom::FILE_ENCODING_Utf8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::ReadJson(JsonValueCR json)
    {
    //Required
    m_ruleSetId = json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID].asCString("");
    if (m_ruleSetId.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "Ruleset", PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID);
        return false;
        }

    //Optional
    m_supportedSchemas = CommonToolsInternal::SupportedSchemasToString(json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS]);
    m_versionMajor = 1;
    m_versionMinor = 0;
    if (json.isMember(PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO))
        {
        m_isSupplemental = true;
        m_supplementationPurpose = json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO][PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO_PURPOSE].asCString("");
        }

    JsonValueCR rulesJson = json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES];
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_childNodesRules, CommonToolsInternal::LoadRuleFromJson<ChildNodeRule>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_rootNodesRules, CommonToolsInternal::LoadRuleFromJson<RootNodeRule>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_contentRules, CommonToolsInternal::LoadRuleFromJson<ContentRule>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_contentModifiers, CommonToolsInternal::LoadRuleFromJson<ContentModifier>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_checkBoxRules, CommonToolsInternal::LoadRuleFromJson<CheckBoxRule>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_groupingRules, CommonToolsInternal::LoadRuleFromJson<GroupingRule>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_imageIdRules, CommonToolsInternal::LoadRuleFromJson<ImageIdOverride>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_labelOverrides, CommonToolsInternal::LoadRuleFromJson<LabelOverride>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_sortingRules, CommonToolsInternal::LoadRuleFromJson<SortingRule>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_styleOverrides, CommonToolsInternal::LoadRuleFromJson<StyleOverride>);
    CommonToolsInternal::LoadFromJsonByPriority(rulesJson, m_instanceLabelOverrides, CommonToolsInternal::LoadRuleFromJson<InstanceLabelOverride>);
    CommonToolsInternal::LoadFromJsonByPriority(json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_USERSETTINGS], m_userSettings, CommonToolsInternal::LoadRuleFromJson<UserSettingsGroup>);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSet::WriteJson(JsonValueR json) const
    {
    json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID] = m_ruleSetId;
    if (!m_supportedSchemas.empty())
        json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS] = CommonToolsInternal::SupportedSchemasToJson(m_supportedSchemas);
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
    CommonToolsInternal::WriteRulesToJson<GroupingRule, GroupingRuleList>(rulesJson, m_groupingRules);
    CommonToolsInternal::WriteRulesToJson<UserSettingsGroup, UserSettingsGroupList>(rulesJson, m_userSettings);
    CommonToolsInternal::WriteRulesToJson<CheckBoxRule, CheckBoxRuleList>(rulesJson, m_checkBoxRules);
    CommonToolsInternal::WriteRulesToJson<SortingRule, SortingRuleList>(rulesJson, m_sortingRules);
    json[PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES] = rulesJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromJsonValue(JsonValueCR json)
    {
    ECPRENSETATION_RULES_LOG.debug("About to read PrsentationRuleSet from Json::Value");
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet();
    if (!json.isNull() && ruleSet->ReadJson(json))
        {
        ECPRENSETATION_RULES_LOG.debug("Successfully read PresentationRuleSet from Json::Value");
        return ruleSet;
        }

    ECPRENSETATION_RULES_LOG.error("Failed to load PresentationRuleSet from Json::Value");
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromJsonString(Utf8StringCR jsonString)
    {
    ECPRENSETATION_RULES_LOG.debug("About to read PrsentationRuleSet from json string");
    Json::Value json = Json::Reader::DoParse(jsonString);
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet();
    if (!json.isNull() && ruleSet->ReadJson(json))
        {
        ECPRENSETATION_RULES_LOG.debug("Successfully read PresentationRuleSet from json string");
        return ruleSet;
        }

    ECPRENSETATION_RULES_LOG.error("Failed to load PresentationRuleSet from json string");
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromJsonFile(BeFileNameCR jsonFilePath)
    {
    ECPRENSETATION_RULES_LOG.debugv(L"About to read PresentationRuleSet from json file: %s", jsonFilePath.c_str());

    BeFile f;
    if (BeFileStatus::Success != f.Open(jsonFilePath.c_str(), BeFileAccess::Read))
        {
        ECPRENSETATION_RULES_LOG.errorv(L"Failed to open json file for read: %s", jsonFilePath.c_str());
        return nullptr;
        }
    bvector<Byte> fileContents;
    if (BeFileStatus::Success != f.ReadEntireFile(fileContents))
        {
        ECPRENSETATION_RULES_LOG.errorv(L"Failed to read json file: %s", jsonFilePath.c_str());
        f.Close();
        return nullptr;
        }
    f.Close();

    Utf8String serializedJson((Utf8CP)fileContents.begin(), (Utf8CP)fileContents.end());
    PresentationRuleSetPtr ruleset = ReadFromJsonString(serializedJson);
    if (ruleset.IsValid())
        {
        ECPRENSETATION_RULES_LOG.debugv("Successfully read PresentationRuleSet '%s' from json file", ruleset->GetRuleSetId().c_str());
        return ruleset;
        }

    ECPRENSETATION_RULES_LOG.errorv("Failed to load PresentationRuleSet from json file: %s", jsonFilePath.c_str());
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PresentationRuleSet::WriteToJsonValue() const
    {
    Json::Value json;
    WriteJson(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::WriteToJsonFile(BeFileNameCR jsonFilePath) const
    {
    Json::Value json = WriteToJsonValue();
    Utf8String serializedJson = json.toStyledString();

    BeFile f;
    BeFileStatus status = BeFileStatus::UnknownError;
    if (BeFileStatus::Success != (status = f.Create(jsonFilePath.c_str())))
        return false;

    if (BeFileStatus::Success != (status = f.Write(nullptr, &serializedJson[0], serializedJson.size())))
        {
        f.Close();
        return false;
        }

    status = f.Close();
    BeAssert(BeFileStatus::Success == status);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetRuleSetId (void) const         { return m_ruleSetId;        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                   PresentationRuleSet::SetRuleSetId(Utf8StringCR id)     { m_ruleSetId = id;          }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int                    PresentationRuleSet::GetVersionMajor (void) const      { return m_versionMajor;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int                    PresentationRuleSet::GetVersionMinor (void) const      { return m_versionMinor;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool                   PresentationRuleSet::GetIsSupplemental (void) const    { return m_isSupplemental;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetSupplementationPurpose (void) const { return m_supplementationPurpose; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetSupportedSchemas (void) const  { return m_supportedSchemas; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetPreferredImage (void) const    { return m_preferredImage;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool                   PresentationRuleSet::GetIsSearchEnabled (void) const   { return m_isSearchEnabled;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetSearchClasses (void) const     { return m_searchClasses;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void                   PresentationRuleSet::SetSearchClasses (Utf8StringCR searchClasses) { m_searchClasses = searchClasses; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetExtendedData (void) const      { return m_extendedData;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void                   PresentationRuleSet::SetExtendedData (Utf8StringCR extendedData) { m_extendedData = extendedData; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRuleList const& PresentationRuleSet::GetRootNodesRules (void) const   { return m_rootNodesRules;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRuleList const& PresentationRuleSet::GetChildNodesRules (void) const { return m_childNodesRules;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRuleList const& PresentationRuleSet::GetContentRules (void) const      { return m_contentRules;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverrideList const& PresentationRuleSet::GetImageIdOverrides (void) const { return m_imageIdRules;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverrideList const& PresentationRuleSet::GetLabelOverrides (void) const  { return m_labelOverrides;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverrideList const& PresentationRuleSet::GetStyleOverrides (void) const  { return m_styleOverrides;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRuleList const& PresentationRuleSet::GetGroupingRules (void) const    { return m_groupingRules;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinitionList const& PresentationRuleSet::GetLocalizationResourceKeyDefinitions (void) const { return m_localizationResourceKeyDefinitions; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroupList const& PresentationRuleSet::GetUserSettings (void) const { return m_userSettings;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CheckBoxRuleList const& PresentationRuleSet::GetCheckBoxRules (void) const    { return m_checkBoxRules;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRuleList const& PresentationRuleSet::GetSortingRules (void) const      { return m_sortingRules;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifierList const& PresentationRuleSet::GetContentModifierRules (void) const { return m_contentModifiers; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceLabelOverrideList const& PresentationRuleSet::GetInstanceLabelOverrides (void) const { return m_instanceLabelOverrides; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PresentationRuleSet::_ComputeHash(Utf8CP) const
    {
    MD5 md5;
    md5.Add(m_ruleSetId.c_str(), m_ruleSetId.size());
    md5.Add(m_supportedSchemas.c_str(), m_supportedSchemas.size());
    md5.Add(&m_isSupplemental, sizeof(m_isSupplemental));
    md5.Add(m_supplementationPurpose.c_str(), m_supplementationPurpose.size());
    md5.Add(&m_versionMajor, sizeof(m_versionMajor));
    md5.Add(&m_versionMinor, sizeof(m_versionMinor));
    md5.Add(m_preferredImage.c_str(), m_preferredImage.size());
    md5.Add(&m_isSearchEnabled, sizeof(m_isSearchEnabled));
    md5.Add(m_extendedData.c_str(), m_extendedData.size());
    md5.Add(m_searchClasses.c_str(), m_searchClasses.size());

    Utf8String currentHash = md5.GetHashString();
    for (RootNodeRuleP rule : m_rootNodesRules)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (ChildNodeRuleP rule : m_childNodesRules)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (ContentRuleP rule : m_contentRules)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (ImageIdOverrideP rule : m_imageIdRules)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (LabelOverrideP rule : m_labelOverrides)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (StyleOverrideP rule : m_styleOverrides)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (GroupingRuleP rule : m_groupingRules)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (LocalizationResourceKeyDefinitionP rule : m_localizationResourceKeyDefinitions)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (CheckBoxRuleP rule : m_checkBoxRules)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (SortingRuleP rule : m_sortingRules)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (UserSettingsGroupP rule : m_userSettings)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (ContentModifierP rule : m_contentModifiers)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
    for (InstanceLabelOverrideP rule : m_instanceLabelOverrides)
        {
        Utf8StringCR ruleHash = rule->GetHash(currentHash.c_str());
        md5.Add(ruleHash.c_str(), ruleHash.size());
        }
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
END_BENTLEY_ECPRESENTATION_NAMESPACE
