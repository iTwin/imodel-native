/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/CommonTools.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "CommonTools.h"

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RuleTargetTree CommonTools::ParseTargetTreeString (WCharCP targetTreeString)
    {
    if (0 == wcscmp (targetTreeString, COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE))
        return TargetTree_MainTree;
    else  if (0 == wcscmp (targetTreeString, COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE))
        return TargetTree_SelectionTree;
    else  if (0 == wcscmp (targetTreeString, COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_BOTH))
        return TargetTree_Both;
    else
        return TargetTree_MainTree; //default
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP CommonTools::FormatTargetTreeString (RuleTargetTree targetTree)
    {
    if (TargetTree_MainTree == targetTree)
        return COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE;
    else if (TargetTree_SelectionTree == targetTree)
        return COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE;
    else if (TargetTree_Both == targetTree)
        return COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_BOTH;
    else
        return COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection CommonTools::ParseRequiredDirectionString (WCharCP value)
    {
    if (0 == wcscmp (value, COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH))
        return RequiredRelationDirection_Both;
    else  if (0 == wcscmp (value, COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD))
        return RequiredRelationDirection_Forward;
    else  if (0 == wcscmp (value, COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD))
        return RequiredRelationDirection_Backward;
    else
        return RequiredRelationDirection_Both; //default
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP CommonTools::FormatRequiredDirectionString (RequiredRelationDirection direction)
    {
    if (RequiredRelationDirection_Both == direction)
        return COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH;
    else if (RequiredRelationDirection_Forward == direction)
        return COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD;
    else if (RequiredRelationDirection_Backward == direction)
        return COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD;
    else
        return COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH;
    }