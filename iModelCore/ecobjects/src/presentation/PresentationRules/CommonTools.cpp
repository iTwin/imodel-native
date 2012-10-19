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
RuleTargetTree CommonTools::ParseTargetTreeString (WString targetTreeString)
    {
    if (0 == wcscmp (targetTreeString.c_str (), COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE))
        return TargetTree_MainTree;
    else  if (0 == wcscmp (targetTreeString.c_str (), COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE))
        return TargetTree_SelectionTree;
    else  if (0 == wcscmp (targetTreeString.c_str (), COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_BOTH))
        return TargetTree_Both;
    else
        return TargetTree_MainTree; //default
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection CommonTools::ParseRequiredDirectionString (WString value)
    {
    if (0 == wcscmp (value.c_str (), COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH))
        return RequiredRelationDirection_Both;
    else  if (0 == wcscmp (value.c_str (), COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD))
        return RequiredRelationDirection_Forward;
    else  if (0 == wcscmp (value.c_str (), COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD))
        return RequiredRelationDirection_Backward;
    else
        return RequiredRelationDirection_Both; //default
    }

