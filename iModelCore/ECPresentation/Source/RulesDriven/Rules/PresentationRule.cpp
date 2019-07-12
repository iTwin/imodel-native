/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HashableBase::GetHash(Utf8CP parentHash)
    {
    BeMutexHolder lock(m_mutex);
    m_hash = _ComputeHash(parentHash).GetHashString();
    return m_hash;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HashableBase::GetHash() const
    {
    BeMutexHolder lock(m_mutex);
    if (m_hash.empty())
        {
        const_cast<HashableBase*>(this)->ComputeHash();
        BeAssert(!m_hash.empty());
        }
    return m_hash;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HashableBase::InvalidateHash()
    {
    BeMutexHolder lock(m_mutex);
    if (nullptr != m_parent)
        m_parent->InvalidateHash();
    m_hash.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HashableBase::ComputeHash()
    {
    BeMutexHolder lock(m_mutex);

    // go up rules and specifications hierarchy and start computing hashes from the top
    if (nullptr != m_parent)
        {
        m_parent->ComputeHash();
        return;
        }

    // we are at the top, start computing hashes
    m_hash = _ComputeHash(nullptr).GetHashString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationKey::PresentationKey () : m_priority (1000)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationKey::PresentationKey (int priority) : m_priority (priority)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationKey::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_priority, COMMON_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    //Make sure we call protected override
    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationKey::WriteXml (BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP ruleNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());

    ruleNode->AddAttributeInt32Value (COMMON_XML_ATTRIBUTE_PRIORITY, m_priority);

    //Make sure we call protected override
    _WriteXml (ruleNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationKey::ReadJson (JsonValueCR json)
    {
    m_priority = json[COMMON_JSON_ATTRIBUTE_PRIORITY].asInt(1000);
    return _ReadJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PresentationKey::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    if (1000 != m_priority)
        json[COMMON_JSON_ATTRIBUTE_PRIORITY] = m_priority;
    _WriteJson(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int PresentationKey::GetPriority (void) const { return m_priority; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PresentationKey::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(&m_priority, sizeof(m_priority));
    Utf8CP name = _GetXmlElementName();
    md5.Add(name, strlen(name));
    if (parentHash != nullptr)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule ()
    : m_onlyIfNotHandled (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule (int priority, bool onlyIfNotHandled)
    : PresentationKey (priority), m_onlyIfNotHandled (onlyIfNotHandled)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_onlyIfNotHandled, COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED))
        m_onlyIfNotHandled = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED, m_onlyIfNotHandled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRule::_ReadJson (JsonValueCR json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    if (!json.isMember(COMMON_JSON_ATTRIBUTE_RULETYPE) || !json[COMMON_JSON_ATTRIBUTE_RULETYPE].isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PresentationRule", COMMON_JSON_ATTRIBUTE_RULETYPE);
        return false;
        }
    if (nullptr != _GetJsonElementType() && 0 != strcmp(json[COMMON_JSON_ATTRIBUTE_RULETYPE].asCString(), _GetJsonElementType()))
        return false;

    m_onlyIfNotHandled = json[COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED].asBool(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRule::_WriteJson(JsonValueR json) const
    {
    PresentationKey::_WriteJson(json);
    if (nullptr != _GetJsonElementType())
        json[COMMON_JSON_ATTRIBUTE_RULETYPE] = _GetJsonElementType();
    if (m_onlyIfNotHandled)
        json[COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED] = m_onlyIfNotHandled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRule::GetOnlyIfNotHandled (void) const     { return m_onlyIfNotHandled; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PresentationRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = PresentationKey::_ComputeHash(parentHash);
    md5.Add(&m_onlyIfNotHandled, sizeof(m_onlyIfNotHandled));
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ConditionalPresentationRule::GetCondition() const { return m_condition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalPresentationRule::SetCondition(Utf8String value) { m_condition = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConditionalPresentationRule::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_condition, PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION))
        m_condition = "";

    return PresentationRule::_ReadXml(xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalPresentationRule::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue(PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION, m_condition.c_str());
    PresentationRule::_WriteXml(xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConditionalPresentationRule::_ReadJson(JsonValueCR json)
    {
    if (!PresentationRule::_ReadJson(json))
        return false;
    m_condition = json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalPresentationRule::_WriteJson(JsonValueR json) const
    {
    PresentationRule::_WriteJson(json);
    if (!m_condition.empty())
        json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION] = m_condition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ConditionalPresentationRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = PresentationRule::_ComputeHash(parentHash);
    md5.Add(m_condition.c_str(), m_condition.size());
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSpecification::Accept(PresentationRuleSpecificationVisitor& visitor) const {_Accept(visitor);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PresentationRuleSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSpecification::WriteXml (BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement(_GetXmlElementName());
    _WriteXml(specificationNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRuleSpecification::ReadJson(JsonValueCR json)
    {
    if (!json.isMember(COMMON_JSON_ATTRIBUTE_SPECTYPE) || !json[COMMON_JSON_ATTRIBUTE_SPECTYPE].isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PresentationRuleSpecification", COMMON_JSON_ATTRIBUTE_SPECTYPE);
        return false;
        }
    if (0 != strcmp(json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString(), _GetJsonElementType()))
        return false;
    return _ReadJson(json);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PresentationRuleSpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[COMMON_JSON_ATTRIBUTE_SPECTYPE] = _GetJsonElementType();
    _WriteJson(json);
    return json;
    } 
