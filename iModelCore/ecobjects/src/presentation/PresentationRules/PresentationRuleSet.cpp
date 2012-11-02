/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/PresentationRuleSet.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "CommonTools.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::~PresentationRuleSet ()
    {
    CommonTools::FreePresentationRules (m_rootNodesRules);
    CommonTools::FreePresentationRules (m_childNodesRules);
    CommonTools::FreePresentationRules (m_contentRules);
    CommonTools::FreePresentationRules (m_imageIdRules);
    CommonTools::FreePresentationRules (m_labelOverrides);
    CommonTools::FreePresentationRules (m_styleOverrides);
    CommonTools::FreePresentationRules (m_groupingRules);
    CommonTools::FreePresentationRules (m_localizationResourceKeyDefinitions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::CreateInstance
(
WStringCR ruleSetId,
WStringCR supportedSchemas,
bool      isSupplemental,
int       version,
WStringCR preferredImage
)
    {
    return new PresentationRuleSet (ruleSetId, supportedSchemas, isSupplemental, version, preferredImage);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PresentationRuleSet::GetFullRuleSetId (void) const
    {
    wchar_t fullId[255];

    if (GetIsSupplemental ())
        BeStringUtilities::Snwprintf (fullId, L"%ls_supplemental.%02d", GetRuleSetId ().c_str(), GetVersion ());
    else
        BeStringUtilities::Snwprintf (fullId, L"%ls.%02d", GetRuleSetId ().c_str(), GetVersion ());

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
        ECObjectsLogger::Log()->errorv (L"Invalid PresentationRuleSetXML: Missing a top-level %hs node", PRESENTATION_RULE_SET_XML_NODE_NAME);
        return false;
        }
    
    //Required:
    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_ruleSetId, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid PresentationRuleSetXML: %hs element must contain a %hs attribute", PRESENTATION_RULE_SET_XML_NODE_NAME, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID);
        return false;
        }

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_supportedSchemas, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid PresentationRuleSetXML: %hs element must contain a %hs attribute", PRESENTATION_RULE_SET_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS);
        return false;
        }

    //Optional:
    if (BEXML_Success != ruleSetNode->GetAttributeBooleanValue (m_isSupplemental, PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSUPPLEMENTAL))
        m_isSupplemental = false;

    if (BEXML_Success != ruleSetNode->GetAttributeInt32Value (m_version, PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSION))
        m_version = 1;

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_preferredImage, PRESENTATION_RULE_SET_XML_ATTRIBUTE_PREFERREDIMAGE))
        m_preferredImage = L"";

    CommonTools::LoadRulesFromXmlNode <RootNodeRule,    RootNodeRuleList>    (ruleSetNode, m_rootNodesRules,   ROOT_NODE_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <ChildNodeRule,   ChildNodeRuleList>   (ruleSetNode, m_childNodesRules,  CHILD_NODE_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <ContentRule,     ContentRuleList>     (ruleSetNode, m_contentRules,     CONTENT_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <ImageIdOverride, ImageIdOverrideList> (ruleSetNode, m_imageIdRules,     IMAGE_ID_OVERRIDE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <LabelOverride,   LabelOverrideList>   (ruleSetNode, m_labelOverrides,   LABEL_OVERRIDE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <StyleOverride,   StyleOverrideList>   (ruleSetNode, m_styleOverrides,   STYLE_OVERRIDE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <GroupingRule,    GroupingRuleList>    (ruleSetNode, m_groupingRules,    GROUPING_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <LocalizationResourceKeyDefinition, LocalizationResourceKeyDefinitionList> (ruleSetNode, m_localizationResourceKeyDefinitions, LOCALIZATION_DEFINITION_XML_NODE_NAME);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSet::WriteXml (BeXmlDomR xmlDom)
    {
    BeXmlNodeP ruleSetNode = xmlDom.AddNewElement (PRESENTATION_RULE_SET_XML_NODE_NAME, NULL, NULL);

    ruleSetNode->AddAttributeStringValue  (PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID,      m_ruleSetId.c_str ());
    ruleSetNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS,              m_supportedSchemas.c_str ());
    ruleSetNode->AddAttributeBooleanValue (PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSUPPLEMENTAL, m_isSupplemental);
    ruleSetNode->AddAttributeInt32Value   (PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSION,        m_version);
    ruleSetNode->AddAttributeStringValue  (PRESENTATION_RULE_SET_XML_ATTRIBUTE_PREFERREDIMAGE, m_preferredImage.c_str ());

    CommonTools::WriteRulesToXmlNode<RootNodeRule,    RootNodeRuleList>    (ruleSetNode, m_rootNodesRules);
    CommonTools::WriteRulesToXmlNode<ChildNodeRule,   ChildNodeRuleList>   (ruleSetNode, m_childNodesRules);
    CommonTools::WriteRulesToXmlNode<ContentRule,     ContentRuleList>     (ruleSetNode, m_contentRules);
    CommonTools::WriteRulesToXmlNode<ImageIdOverride, ImageIdOverrideList> (ruleSetNode, m_imageIdRules);
    CommonTools::WriteRulesToXmlNode<LabelOverride,   LabelOverrideList>   (ruleSetNode, m_labelOverrides);
    CommonTools::WriteRulesToXmlNode<StyleOverride,   StyleOverrideList>   (ruleSetNode, m_styleOverrides);
    CommonTools::WriteRulesToXmlNode<GroupingRule,    GroupingRuleList>    (ruleSetNode, m_groupingRules);
    CommonTools::WriteRulesToXmlNode<LocalizationResourceKeyDefinition, LocalizationResourceKeyDefinitionList> (ruleSetNode, m_localizationResourceKeyDefinitions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlString (WCharCP xmlString)
    {
    ECObjectsLogger::Log()->debugv (L"About to read PrsentationRuleSet from string.");

    BeXmlStatus xmlStatus;
    size_t stringSize = wcslen (xmlString) * sizeof(WChar);
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, xmlString, stringSize);
    
    if (BEXML_Success != xmlStatus)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to load PrsentationRuleSet from xml string.");
        return NULL;
        }
    
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet ();
    if (ruleSet->ReadXml (*xmlDom.get ()))
        return ruleSet;

    ECObjectsLogger::Log()->errorv (L"Failed to load PrsentationRuleSet from xml string.");
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlFile (WCharCP xmlFilePath)
    {
    ECObjectsLogger::Log()->debugv (L"About to read PrsentationRuleSet from file: fileName='%ls'", xmlFilePath);
        
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, xmlFilePath);
    if (xmlStatus != BEXML_Success || !xmlDom.IsValid ())
        {
        ECObjectsLogger::Log()->errorv (L"Failed to load PresentationRuleSet from file: fileName='%ls'", xmlFilePath);
        return NULL;
        }
    
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet ();
    if (ruleSet->ReadXml (*xmlDom.get ()))
        return ruleSet;

    ECObjectsLogger::Log()->errorv (L"Failed to load PresentationRuleSet from file: fileName='%ls'", xmlFilePath);
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PresentationRuleSet::WriteToXmlString ()
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty ();        
    WriteXml (*xmlDom.get());

    WString presentationRuleSetXml = L"";
    xmlDom->ToString (presentationRuleSetXml, BeXmlDom::TO_STRING_OPTION_OmitByteOrderMark);

    return presentationRuleSetXml;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::WriteToXmlFile (WCharCP xmlFilePath)
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();        
    WriteXml (*xmlDom.get());

    return BEXML_Success == xmlDom->ToFile (xmlFilePath, BeXmlDom::TO_STRING_OPTION_Indent, BeXmlDom::FILE_ENCODING_Utf8);
    }
