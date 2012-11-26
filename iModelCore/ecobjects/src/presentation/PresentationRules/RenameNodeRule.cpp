/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/RenameNodeRule.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              dmitrijus.tiazlovas                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP RenameNodeRule::_GetXmlElementName ()
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
void RenameNodeRule::_WriteXml (BeXmlNodeP xmlNode)
    {
    PresentationRule::_WriteXml (xmlNode);
    }