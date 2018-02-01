/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/PresentationRuleSet.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
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
    CommonTools::FreePresentationRules (m_contentModifiers);
    CommonTools::FreePresentationRules (m_instanceLabelOverrides);
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
        ECPRENSETATION_RULES_LOG.errorv ("Invalid PresentationRuleSetXML: Missing a top-level %s node", PRESENTATION_RULE_SET_XML_NODE_NAME);
        return false;
        }
    
    //Required:
    if (BEXML_Success != ruleSetNode->GetAttributeStringValue (m_ruleSetId, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID))
        {
        ECPRENSETATION_RULES_LOG.errorv ("Invalid PresentationRuleSetXML: %s element must contain a %s attribute", PRESENTATION_RULE_SET_XML_NODE_NAME, PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID);
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
    CommonTools::LoadRulesFromXmlNode <ContentModifier>   (ruleSetNode, m_contentModifiers, CONTENTMODIEFIER_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode <InstanceLabelOverride> (ruleSetNode, m_instanceLabelOverrides, INSTANCE_LABEL_OVERRIDE_XML_NODE_NAME);
    
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
    CommonTools::WriteRulesToXmlNode<ContentModifier,   ContentModifierList>   (ruleSetNode, m_contentModifiers);
    CommonTools::WriteRulesToXmlNode<InstanceLabelOverride,InstanceLabelOverrideList> (ruleSetNode, m_instanceLabelOverrides);
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
PresentationRuleSetPtr PresentationRuleSet::ReadFromXmlFile (WCharCP xmlFilePath)
    {
    ECPRENSETATION_RULES_LOG.debugv (L"About to read PrsentationRuleSet from file: fileName='%ls'", xmlFilePath);
        
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, xmlFilePath);
    if (xmlStatus != BEXML_Success || !xmlDom.IsValid ())
        {
        ECPRENSETATION_RULES_LOG.errorv (L"Failed to load PresentationRuleSet from file: fileName='%ls'", xmlFilePath);
        return NULL;
        }
    
    PresentationRuleSetPtr ruleSet = new PresentationRuleSet ();
    if (ruleSet->ReadXml (*xmlDom.get ()))
        return ruleSet;

    ECPRENSETATION_RULES_LOG.errorv (L"Failed to load PresentationRuleSet from file: fileName='%ls'", xmlFilePath);
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
RenameNodeRuleList const& PresentationRuleSet::GetRenameNodeRules (void) const { return m_renameNodeRules;  }

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
    for (RenameNodeRuleP rule : m_renameNodeRules)
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
template<> RenameNodeRuleList* PresentationRuleSet::GetRules<RenameNodeRule>() {return &m_renameNodeRules;}
template<> SortingRuleList* PresentationRuleSet::GetRules<SortingRule>() {return &m_sortingRules;}
template<> ContentModifierList* PresentationRuleSet::GetRules<ContentModifier>() {return &m_contentModifiers;}
template<> UserSettingsGroupList* PresentationRuleSet::GetRules<UserSettingsGroup>() {return &m_userSettings;}
template<> InstanceLabelOverrideList* PresentationRuleSet::GetRules<InstanceLabelOverride>() {return &m_instanceLabelOverrides;}
template<> LocalizationResourceKeyDefinitionList* PresentationRuleSet::GetRules<LocalizationResourceKeyDefinition>() {return &m_localizationResourceKeyDefinitions;}
END_BENTLEY_ECPRESENTATION_NAMESPACE
