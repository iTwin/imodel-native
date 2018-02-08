/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/CommonTools.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/RelatedInstanceNodesSpecification.h>
#include <ECPresentation/RulesDriven/Rules/RelatedPropertiesSpecification.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> CommonTools::ParsePropertiesNames(Utf8StringCR properties)
    {
    bvector<Utf8String> propertiesNamesList;
    BeStringUtilities::Split(properties.c_str(), ",", propertiesNamesList);
    std::for_each(propertiesNamesList.begin(), propertiesNamesList.end(), [](Utf8StringR name){name.Trim();});
    return propertiesNamesList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void ReverseString(Utf8StringR str)
    {
    for (size_t i = 0; i < str.size() / 2; i++)
        std::swap(str[i], str[str.size() - i - 1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String ToBase36String(uint64_t i)
    {
    static Utf8CP chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    Utf8String encoded;
    while (i != 0)
        {
        encoded.push_back(chars[i % 36]);
        i /= 36;
        }
    ReverseString(encoded);
    return !encoded.empty() ? encoded : "0";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CommonTools::GetDefaultDisplayLabel(Utf8StringCR className, uint64_t id)
    {
    Utf8String label = className;
    label.append(" ");
    label.append(ToBase36String(id));
    return label;
    }
