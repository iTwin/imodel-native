/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/RenameNodeRule.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              dmitrijus.tiazlovas                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RenameNodeRule::RenameNodeRule () : CustomizationRule()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              dmitrijus.tiazlovas                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RenameNodeRule::RenameNodeRule (Utf8StringCR condition, int priority)
    : CustomizationRule(condition, priority, false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              dmitrijus.tiazlovas                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP RenameNodeRule::_GetXmlElementName () const
    {
    return RENAMENODE_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              dmitrijus.tiazlovas                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RenameNodeRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              dmitrijus.tiazlovas                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RenameNodeRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Aidas.Vaiksnoras                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RenameNodeRule::_Accept(CustomizationRuleVisitor& visitor) const {visitor._Visit(*this);}
