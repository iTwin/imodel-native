/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/PresentationRule.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HashableBase::GetHash(Utf8CP parentHash)
    {
    m_hash = _ComputeHash(parentHash).GetHashString();
    return m_hash;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HashableBase::GetHash() const
    {
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
    if (nullptr != m_parent)
        m_parent->InvalidateHash();
    m_hash.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HashableBase::ComputeHash()
    {
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
    CharCP name = _GetXmlElementName();
    md5.Add(name, strlen(name));
    if (parentHash != nullptr)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule ()
    : PresentationKey (), m_condition (""), m_onlyIfNotHandled (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled)
    : PresentationKey (priority), m_condition (condition), m_onlyIfNotHandled (onlyIfNotHandled)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_condition, PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION))
        m_condition = "";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_onlyIfNotHandled, COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED))
        m_onlyIfNotHandled = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue (PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION, m_condition.c_str ());
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED, m_onlyIfNotHandled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR PresentationRule::GetCondition (void) const       { return m_condition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRule::SetCondition (Utf8String value)         { m_condition = value; }

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
    md5.Add(m_condition.c_str(), m_condition.size());
    md5.Add(&m_onlyIfNotHandled, sizeof(m_onlyIfNotHandled));
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