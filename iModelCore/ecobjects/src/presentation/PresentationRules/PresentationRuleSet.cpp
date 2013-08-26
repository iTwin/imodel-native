/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/PresentationRuleSet.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
PresentationRuleSet::PresentationRuleSet (void)
    : m_ruleSetId (L""), m_supportedSchemas (L""), m_isSupplemental (false), m_supplementationPurpose (L""), m_versionMajor (1), m_versionMinor (0), m_isSearchEnabled (true)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSet::PresentationRuleSet
(
WStringCR ruleSetId,
int       versionMajor,
int       versionMinor,
bool      isSupplemental,
WStringCR supplementationPurpose,
WStringCR supportedSchemas,
WStringCR preferredImage,
bool      isSearchEnabled
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
WStringCR ruleSetId,
int       versionMajor,
int       versionMinor,
bool      isSupplemental,
WStringCR supplementationPurpose,
WStringCR supportedSchemas,
WStringCR preferredImage,
bool      isSearchEnabled
)
    {
    return new PresentationRuleSet (ruleSetId, versionMajor, versionMinor, isSupplemental, supplementationPurpose, supportedSchemas, preferredImage, isSearchEnabled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PresentationRuleSet::GetFullRuleSetId (void) const
    {
    wchar_t fullId[255];

    if (GetIsSupplemental ())
        BeStringUtilities::Snwprintf (fullId, L"%ls_supplemental_%ls.%02d.%02d", GetRuleSetId ().c_str(), GetSupplementationPurpose ().c_str(), GetVersionMajor (), GetVersionMinor ());
    else
        BeStringUtilities::Snwprintf (fullId, L"%ls.%02d.%02d", GetRuleSetId ().c_str(), GetVersionMajor (), GetVersionMinor ());

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
        LOG.errorv (L"Invalid PresentationRuleSetXML: Missing a top-level %hs node", PRESENTATION_RULE_SET_XML_NODE_NAME);
        return false;
        }
    
    //Required:
    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_ruleSetId, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID))
        {
        LOG.errorv (L"Invalid PresentationRuleSetXML: %hs element must contain a %hs attribute", PRESENTATION_RULE_SET_XML_NODE_NAME, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID);
        return false;
        }

    //Optional:
    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_supportedSchemas, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS))
        m_supportedSchemas = L"";

    if (BEXML_Success != ruleSetNode->GetAttributeBooleanValue (m_isSupplemental, PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSUPPLEMENTAL))
        m_isSupplemental = false;

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_supplementationPurpose, PRESENTATION_RULE_SET_XML_ATTRIBUTE_SUPPLEMENTATIONPURPOSE))
        m_supplementationPurpose = L"";

    if (BEXML_Success != ruleSetNode->GetAttributeInt32Value (m_versionMajor, PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMAJOR))
        m_versionMajor = 1;

    if (BEXML_Success != ruleSetNode->GetAttributeInt32Value (m_versionMinor, PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMINOR))
        m_versionMinor = 0;

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_preferredImage, PRESENTATION_RULE_SET_XML_ATTRIBUTE_PREFERREDIMAGE))
        m_preferredImage = L"";

    if (BEXML_Success != ruleSetNode->GetAttributeBooleanValue (m_isSearchEnabled, PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSEARCHENABLED))
        m_isSearchEnabled = true;

    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_extendedData, PRESENTATION_RULE_SET_XML_ATTRIBUTE_EXTENDEDDATA))
        m_extendedData = L"";

    CommonTools::LoadRulesFromXmlNode <RootNodeRule,      RootNodeRuleList>      (ruleSetNode, m_rootNodesRules,   ROOT_NODE_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <ChildNodeRule,     ChildNodeRuleList>     (ruleSetNode, m_childNodesRules,  CHILD_NODE_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <ContentRule,       ContentRuleList>       (ruleSetNode, m_contentRules,     CONTENT_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <ImageIdOverride,   ImageIdOverrideList>   (ruleSetNode, m_imageIdRules,     IMAGE_ID_OVERRIDE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <LabelOverride,     LabelOverrideList>     (ruleSetNode, m_labelOverrides,   LABEL_OVERRIDE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <StyleOverride,     StyleOverrideList>     (ruleSetNode, m_styleOverrides,   STYLE_OVERRIDE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <GroupingRule,      GroupingRuleList>      (ruleSetNode, m_groupingRules,    GROUPING_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <LocalizationResourceKeyDefinition, LocalizationResourceKeyDefinitionList> (ruleSetNode, m_localizationResourceKeyDefinitions, LOCALIZATION_DEFINITION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <UserSettingsGroup, UserSettingsGroupList> (ruleSetNode, m_userSettings,     USER_SETTINGS_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <CheckBoxRule,      CheckBoxRuleList>      (ruleSetNode, m_checkBoxRules,    CHECKBOX_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <RenameNodeRule,    RenameNodeRuleList>    (ruleSetNode, m_renameNodeRules,  RENAMENODE_RULE_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <SortingRule,       SortingRuleList>       (ruleSetNode, m_sortingRules,     SORTING_RULE_XML_NODE_NAME);

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
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlString (WCharCP xmlString)
    {
    LOG.debugv (L"About to read PrsentationRuleSet from string.");

    BeXmlStatus xmlStatus;
    size_t stringSize = wcslen (xmlString) * sizeof(WChar);
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, xmlString, stringSize);
    
    if (BEXML_Success != xmlStatus)
        {
        LOG.errorv (L"Failed to load PrsentationRuleSet from xml string.");
        return NULL;
        }
    
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet ();
    if (ruleSet->ReadXml (*xmlDom.get ()))
        return ruleSet;

    LOG.errorv (L"Failed to load PrsentationRuleSet from xml string.");
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
WString PresentationRuleSet::WriteToXmlString ()
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty ();        
    WriteXml (*xmlDom.get());

    WString presentationRuleSetXml = L"";
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

    return BEXML_Success == xmlDom->ToFile (xmlFilePath, BeXmlDom::TO_STRING_OPTION_Indent, BeXmlDom::FILE_ENCODING_Utf8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR              PresentationRuleSet::GetRuleSetId (void) const         { return m_ruleSetId;        }

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
WStringCR              PresentationRuleSet::GetSupplementationPurpose (void) const { return m_supplementationPurpose; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR              PresentationRuleSet::GetSupportedSchemas (void) const  { return m_supportedSchemas; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR              PresentationRuleSet::GetPreferredImage (void) const    { return m_preferredImage;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool                   PresentationRuleSet::GetIsSearchEnabled (void) const   { return m_isSearchEnabled;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR              PresentationRuleSet::GetExtendedData (void) const      { return m_extendedData;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void                   PresentationRuleSet::SetExtendedData (WStringCR extendedData) { m_extendedData = extendedData; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRuleList&      PresentationRuleSet::GetRootNodesRules (void)          { return m_rootNodesRules;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRuleList&     PresentationRuleSet::GetChildNodesRules (void)         { return m_childNodesRules;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRuleList&       PresentationRuleSet::GetContentRules (void)            { return m_contentRules;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverrideList&   PresentationRuleSet::GetImageIdOverrides (void)        { return m_imageIdRules;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverrideList&     PresentationRuleSet::GetLabelOverrides (void)          { return m_labelOverrides;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverrideList&     PresentationRuleSet::GetStyleOverrides (void)          { return m_styleOverrides;   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GroupingRuleList&      PresentationRuleSet::GetGroupingRules (void)           { return m_groupingRules;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinitionList&  PresentationRuleSet::GetLocalizationResourceKeyDefinitions (void) { return m_localizationResourceKeyDefinitions; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroupList& PresentationRuleSet::GetUserSettings (void)            { return m_userSettings;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CheckBoxRuleList&      PresentationRuleSet::GetCheckBoxRules (void)           { return m_checkBoxRules;    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RenameNodeRuleList&    PresentationRuleSet::GetRenameNodeRules (void)         { return m_renameNodeRules;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRuleList&       PresentationRuleSet::GetSortingRules (void)            { return m_sortingRules;     }
