/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecificationP ContentSpecification::Create(JsonValueCR json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString("");
    ContentSpecificationP spec = nullptr;
    if (0 == strcmp(SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_TYPE, type))
        spec = new SelectedNodeInstancesSpecification();
    else if (0 == strcmp(CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE, type))
        spec = new ContentInstancesOfSpecificClassesSpecification();
    else if (0 == strcmp(CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_TYPE, type))
        spec = new ContentRelatedInstancesSpecification();
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification() : m_priority(1000), m_showImages(false) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(int priority, bool showImages)
    : m_priority(priority), m_showImages(showImages)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(ContentSpecificationCR other)
    : m_priority(other.m_priority), m_showImages(other.m_showImages), m_modifiers(other.m_modifiers)
    {
    CommonToolsInternal::CopyRules(m_relatedInstances, other.m_relatedInstances, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(ContentSpecification&& other)
    : m_priority(other.m_priority), m_showImages(other.m_showImages), m_modifiers(std::move(other.m_modifiers))
    {
    CommonToolsInternal::SwapRules(m_relatedInstances, other.m_relatedInstances, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::~ContentSpecification ()
    {
    CommonToolsInternal::FreePresentationRules(m_relatedInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationRuleSpecification::_ReadXml(xmlNode))
        return false;

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeInt32Value(m_priority, COMMON_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_showImages, CONTENT_SPECIFICATION_XML_ATTRIBUTE_SHOWIMAGES))
        m_showImages = false;

    CommonToolsInternal::LoadSpecificationsFromXmlNode<RelatedInstanceSpecification, RelatedInstanceSpecificationList> (xmlNode, m_relatedInstances, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, this);
    return m_modifiers.ReadXml(xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::_WriteXml (BeXmlNodeP specificationNode) const
    {
    PresentationRuleSpecification::_WriteXml(specificationNode);
    specificationNode->AddAttributeInt32Value(COMMON_XML_ATTRIBUTE_PRIORITY, m_priority);
    specificationNode->AddAttributeBooleanValue(CONTENT_SPECIFICATION_XML_ATTRIBUTE_SHOWIMAGES, m_showImages);
    CommonToolsInternal::WriteRulesToXmlNode<RelatedInstanceSpecification, RelatedInstanceSpecificationList>(specificationNode, m_relatedInstances);
    m_modifiers.WriteXml(specificationNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecification::_ReadJson(JsonValueCR json)
    {
    if (!PresentationRuleSpecification::_ReadJson(json))
        return false;

    m_priority = json[COMMON_JSON_ATTRIBUTE_PRIORITY].asInt(1000);
    m_showImages = json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_SHOWIMAGES].asBool(false);
    CommonToolsInternal::LoadFromJson(json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION],
        m_relatedInstances, CommonToolsInternal::LoadRuleFromJson<RelatedInstanceSpecification>, this);
    return m_modifiers.ReadJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::_WriteJson(JsonValueR json) const
    {
    PresentationRuleSpecification::_WriteJson(json);
    if (1000 != m_priority)
        json[COMMON_JSON_ATTRIBUTE_PRIORITY] = m_priority;
    if (m_showImages)
        json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_SHOWIMAGES] = m_showImages;
    if (!m_relatedInstances.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedInstanceSpecification, RelatedInstanceSpecificationList>
            (json[CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION], m_relatedInstances);
        }
    m_modifiers.WriteJson(json);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::SetPriority(int value) { InvalidateHash(); m_priority = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::SetShowImages(bool value) { InvalidateHash(); m_showImages = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::AddRelatedInstance(RelatedInstanceSpecificationR relatedInstance) { ADD_HASHABLE_CHILD(m_relatedInstances, relatedInstance); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = PresentationRuleSpecification::_ComputeHash(parentHash);
    md5.Add(&m_priority, sizeof(m_priority));
    md5.Add(&m_showImages, sizeof(m_showImages));
    Utf8CP name = _GetXmlElementName();
    md5.Add(name, strlen(name));

    Utf8String currentHash = md5.GetHashString();
    for (RelatedInstanceSpecificationP spec : m_relatedInstances)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    Utf8StringCR modifiersHash = m_modifiers.GetHash(currentHash.c_str());
    md5.Add(modifiersHash.c_str(), modifiersHash.size());

    return md5;
    }
