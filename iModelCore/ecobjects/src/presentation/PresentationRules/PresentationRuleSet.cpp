/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/PresentationRuleSet.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/CommonTools.h>
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_EC

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
    CommonTools::FreePresentationRules (m_rootNodesRules);
    CommonTools::FreePresentationRules (m_childNodesRules);
    CommonTools::FreePresentationRules (m_contentRules);
    CommonTools::FreePresentationRules (m_imageIdRules);
    CommonTools::FreePresentationRules (m_labelOverrides);
    CommonTools::FreePresentationRules (m_styleOverrides);
    CommonTools::FreePresentationRules (m_groupingRules);
    CommonTools::FreePresentationRules (m_localizationResourceKeyDefinitions);
    CommonTools::FreePresentationRules (m_userSettings);
    CommonTools::FreePresentationRules (m_checkBoxRules);
    CommonTools::FreePresentationRules (m_renameNodeRules);
    CommonTools::FreePresentationRules (m_sortingRules);
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
        LOG.errorv ("Invalid PresentationRuleSetXML: Missing a top-level %s node", PRESENTATION_RULE_SET_XML_NODE_NAME);
        return false;
        }
    
    //Required:
    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_ruleSetId, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID))
        {
        LOG.errorv ("Invalid PresentationRuleSetXML: %s element must contain a %s attribute", PRESENTATION_RULE_SET_XML_NODE_NAME, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID);
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

    CommonTools::LoadRulesFromXmlNode <RootNodeRule>      (ruleSetNode, m_rootNodesRules,   ROOT_NODE_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <ChildNodeRule>     (ruleSetNode, m_childNodesRules,  CHILD_NODE_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <ContentRule>       (ruleSetNode, m_contentRules,     CONTENT_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <ImageIdOverride>   (ruleSetNode, m_imageIdRules,     IMAGE_ID_OVERRIDE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <LabelOverride>     (ruleSetNode, m_labelOverrides,   LABEL_OVERRIDE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <StyleOverride>     (ruleSetNode, m_styleOverrides,   STYLE_OVERRIDE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <GroupingRule>      (ruleSetNode, m_groupingRules,    GROUPING_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <LocalizationResourceKeyDefinition> (ruleSetNode, m_localizationResourceKeyDefinitions, LOCALIZATION_DEFINITION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <UserSettingsGroup> (ruleSetNode, m_userSettings,     USER_SETTINGS_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <CheckBoxRule>      (ruleSetNode, m_checkBoxRules,    CHECKBOX_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <RenameNodeRule>    (ruleSetNode, m_renameNodeRules,  RENAMENODE_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <SortingRule>       (ruleSetNode, m_sortingRules,     SORTING_RULE_XML_NODE_NAME);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSet::WriteXml (BeXmlDomR xmlDom)
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

    CommonTools::WriteRulesToXmlNode<RootNodeRule,      RootNodeRuleList>      (ruleSetNode, m_rootNodesRules);
    CommonTools::WriteRulesToXmlNode<ChildNodeRule,     ChildNodeRuleList>     (ruleSetNode, m_childNodesRules);
    CommonTools::WriteRulesToXmlNode<ContentRule,       ContentRuleList>       (ruleSetNode, m_contentRules);
    CommonTools::WriteRulesToXmlNode<ImageIdOverride,   ImageIdOverrideList>   (ruleSetNode, m_imageIdRules);
    CommonTools::WriteRulesToXmlNode<LabelOverride,     LabelOverrideList>     (ruleSetNode, m_labelOverrides);
    CommonTools::WriteRulesToXmlNode<StyleOverride,     StyleOverrideList>     (ruleSetNode, m_styleOverrides);
    CommonTools::WriteRulesToXmlNode<GroupingRule,      GroupingRuleList>      (ruleSetNode, m_groupingRules);
    CommonTools::WriteRulesToXmlNode<LocalizationResourceKeyDefinition, LocalizationResourceKeyDefinitionList> (ruleSetNode, m_localizationResourceKeyDefinitions);
    CommonTools::WriteRulesToXmlNode<UserSettingsGroup, UserSettingsGroupList> (ruleSetNode, m_userSettings);
    CommonTools::WriteRulesToXmlNode<CheckBoxRule,      CheckBoxRuleList>      (ruleSetNode, m_checkBoxRules);
    CommonTools::WriteRulesToXmlNode<RenameNodeRule,    RenameNodeRuleList>    (ruleSetNode, m_renameNodeRules);
    CommonTools::WriteRulesToXmlNode<SortingRule,       SortingRuleList>       (ruleSetNode, m_sortingRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlString (Utf8CP xmlString)
    {
    LOG.debugv ("About to read PrsentationRuleSet from string.");

    BeXmlStatus xmlStatus;
    size_t stringSize = strlen (xmlString) * sizeof(Utf8Char);
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, xmlString, stringSize / sizeof (Utf8Char));
    
    if (BEXML_Success != xmlStatus)
        {
        LOG.errorv ("Failed to load PresentationRuleSet from xml string.");
        return NULL;
        }
    
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet ();
    if (ruleSet->ReadXml (*xmlDom.get ()))
        return ruleSet;

    LOG.errorv ("Failed to load PresentationRuleSet from xml string.");
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlFile (WCharCP xmlFilePath)
    {
    LOG.debugv (L"About to read PrsentationRuleSet from file: fileName='%ls'", xmlFilePath);
        
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, xmlFilePath);
    if (xmlStatus != BEXML_Success || !xmlDom.IsValid ())
        {
        LOG.errorv (L"Failed to load PresentationRuleSet from file: fileName='%ls'", xmlFilePath);
        return NULL;
        }
    
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet ();
    if (ruleSet->ReadXml (*xmlDom.get ()))
        return ruleSet;

    LOG.errorv (L"Failed to load PresentationRuleSet from file: fileName='%ls'", xmlFilePath);
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationRuleSet::WriteToXmlString ()
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty ();        
    WriteXml (*xmlDom.get());

    Utf8String presentationRuleSetXml;
    xmlDom->ToString (presentationRuleSetXml, BeXmlDom::TO_STRING_OPTION_Default);

    return presentationRuleSetXml;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSet::WriteToXmlFile (WCharCP xmlFilePath)
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();        
    WriteXml (*xmlDom.get());

    return BEXML_Success == xmlDom->ToFile(xmlFilePath, (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Indent | BeXmlDom::TO_STRING_OPTION_Formatted), BeXmlDom::FILE_ENCODING_Utf8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR           PresentationRuleSet::GetRuleSetId (void) const         { return m_ruleSetId;        }

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
RenameNodeRuleList const& PresentationRuleSet::GetRenameNodeRules (void) const { return m_renameNodeRules;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRuleList const& PresentationRuleSet::GetSortingRules (void) const      { return m_sortingRules;     }

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
template<> RootNodeRuleList* PresentationRuleSet::GetRules<RootNodeRule>() {return &m_rootNodesRules;}
template<> ChildNodeRuleList* PresentationRuleSet::GetRules<ChildNodeRule>() {return &m_childNodesRules;}
template<> ContentRuleList* PresentationRuleSet::GetRules<ContentRule>() {return &m_contentRules;}
template<> ImageIdOverrideList* PresentationRuleSet::GetRules<ImageIdOverride>() {return &m_imageIdRules;}
template<> LabelOverrideList* PresentationRuleSet::GetRules<LabelOverride>() {return &m_labelOverrides;}
template<> StyleOverrideList* PresentationRuleSet::GetRules<StyleOverride>() {return &m_styleOverrides;}
template<> GroupingRuleList* PresentationRuleSet::GetRules<GroupingRule>() {return &m_groupingRules;}
template<> CheckBoxRuleList* PresentationRuleSet::GetRules<CheckBoxRule>() {return &m_checkBoxRules;}
template<> RenameNodeRuleList* PresentationRuleSet::GetRules<RenameNodeRule>() {return &m_renameNodeRules;}
template<> SortingRuleList* PresentationRuleSet::GetRules<SortingRule>() {return &m_sortingRules;}
template<> UserSettingsGroupList* PresentationRuleSet::GetRules<UserSettingsGroup>() {return &m_userSettings;}
template<> LocalizationResourceKeyDefinitionList* PresentationRuleSet::GetRules<LocalizationResourceKeyDefinition>() {return &m_localizationResourceKeyDefinitions;}
END_BENTLEY_ECOBJECT_NAMESPACE