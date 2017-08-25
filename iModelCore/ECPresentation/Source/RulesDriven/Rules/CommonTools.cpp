/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/CommonTools.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/CommonTools.h>
#include <ECPresentationRules/RelatedInstanceNodesSpecification.h>
#include <ECPresentationRules/RelatedPropertiesSpecification.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RuleTargetTree CommonTools::ParseTargetTreeString (Utf8CP targetTreeString)
    {
    if (0 == strcmp (targetTreeString, COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE))
        return TargetTree_MainTree;
    else  if (0 == strcmp (targetTreeString, COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE))
        return TargetTree_SelectionTree;
    else  if (0 == strcmp (targetTreeString, COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_BOTH))
        return TargetTree_Both;
    else
        return TargetTree_MainTree; //default
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CommonTools::FormatTargetTreeString (RuleTargetTree targetTree)
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
RequiredRelationDirection CommonTools::ParseRequiredDirectionString (Utf8CP value)
    {
    if (0 == strcmp (value, COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH))
        return RequiredRelationDirection_Both;
    else  if (0 == strcmp (value, COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD))
        return RequiredRelationDirection_Forward;
    else  if (0 == strcmp (value, COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD))
        return RequiredRelationDirection_Backward;
    else
        return RequiredRelationDirection_Both; //default
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CommonTools::FormatRequiredDirectionString (RequiredRelationDirection direction)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMeaning CommonTools::ParseRelationshipMeaningString(Utf8CP value)
    {
    if (0 == strcmp(value, COMMON_XML_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_SAMEINSTANCE))
        return RelationshipMeaning::SameInstance;
    if (0 == strcmp(value, COMMON_XML_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_RELATEDINSTANCE))
        return RelationshipMeaning::RelatedInstance;
    return RelationshipMeaning::RelatedInstance; //default
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CommonTools::FormatRelationshipMeaningString(RelationshipMeaning meaning)
    {
    if (RelationshipMeaning::SameInstance == meaning)
        return COMMON_XML_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_SAMEINSTANCE;
    if (RelationshipMeaning::RelatedInstance == meaning)
        return COMMON_XML_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_RELATEDINSTANCE;
    return COMMON_XML_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_RELATEDINSTANCE;
    }
