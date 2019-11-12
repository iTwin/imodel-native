/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleCommonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/RelatedInstanceNodesSpecification.h>
#include <ECPresentation/RulesDriven/Rules/RelatedPropertiesSpecification.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RuleTargetTree CommonToolsInternal::ParseTargetTreeString (Utf8CP targetTreeString)
    {
    if (0 == strcmp (targetTreeString, COMMON_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE))
        return TargetTree_MainTree;
    else  if (0 == strcmp (targetTreeString, COMMON_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE))
        return TargetTree_SelectionTree;
    else  if (0 == strcmp (targetTreeString, COMMON_ATTRIBUTE_VALUE_TARGET_TREE_BOTH))
        return TargetTree_Both;
    else
        return TargetTree_MainTree; //default
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CommonToolsInternal::FormatTargetTreeString (RuleTargetTree targetTree)
    {
    if (TargetTree_MainTree == targetTree)
        return COMMON_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE;
    else if (TargetTree_SelectionTree == targetTree)
        return COMMON_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE;
    else if (TargetTree_Both == targetTree)
        return COMMON_ATTRIBUTE_VALUE_TARGET_TREE_BOTH;
    else
        return COMMON_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection CommonToolsInternal::ParseRequiredDirectionString (Utf8CP value)
    {
    if (0 == strcmp (value, COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH))
        return RequiredRelationDirection_Both;
    else  if (0 == strcmp (value, COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD))
        return RequiredRelationDirection_Forward;
    else  if (0 == strcmp (value, COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD))
        return RequiredRelationDirection_Backward;
    else
        return RequiredRelationDirection_Both; //default
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CommonToolsInternal::FormatRequiredDirectionString (RequiredRelationDirection direction)
    {
    if (RequiredRelationDirection_Both == direction)
        return COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH;
    else if (RequiredRelationDirection_Forward == direction)
        return COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD;
    else if (RequiredRelationDirection_Backward == direction)
        return COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD;
    else
        return COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMeaning CommonToolsInternal::ParseRelationshipMeaningString(Utf8CP value)
    {
    if (0 == strcmp(value, COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_SAMEINSTANCE))
        return RelationshipMeaning::SameInstance;
    if (0 == strcmp(value, COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_RELATEDINSTANCE))
        return RelationshipMeaning::RelatedInstance;
    return RelationshipMeaning::RelatedInstance; //default
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CommonToolsInternal::FormatRelationshipMeaningString(RelationshipMeaning meaning)
    {
    if (RelationshipMeaning::SameInstance == meaning)
        return COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_SAMEINSTANCE;
    if (RelationshipMeaning::RelatedInstance == meaning)
        return COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_RELATEDINSTANCE;
    return COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_RELATEDINSTANCE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> CommonToolsInternal::ParsePropertiesNames(Utf8StringCR properties)
    {
    bvector<Utf8String> propertiesNamesList;
    BeStringUtilities::Split(properties.c_str(), ",", propertiesNamesList);
    std::for_each(propertiesNamesList.begin(), propertiesNamesList.end(), [](Utf8StringR name){name.Trim();});
    return propertiesNamesList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CommonToolsInternal::SupportedSchemasToString(JsonValueCR json)
    {
    if (!json.isMember(SCHEMAS_SPECIFICATION_SCHEMANAMES) || !json[SCHEMAS_SPECIFICATION_SCHEMANAMES].isArray())
        return "";

    Utf8String str;
    if (json.isMember(SCHEMAS_SPECIFICATION_ISEXCLUDE) && json[SCHEMAS_SPECIFICATION_ISEXCLUDE].asBool())
        str.append("E:");

    for (Json::ArrayIndex i = 0; i < json[SCHEMAS_SPECIFICATION_SCHEMANAMES].size(); ++i)
        {
        if (i > 0)
            str.append(",");
        str.append(json[SCHEMAS_SPECIFICATION_SCHEMANAMES][i].asCString());
        }

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CommonToolsInternal::SupportedSchemasToJson(Utf8StringCR str)
    {
    Json::Value json;
    size_t pos = 0;
    if (str.StartsWithI("E:"))
        {
        json[SCHEMAS_SPECIFICATION_ISEXCLUDE] = true;
        pos = 2;
        }
    Utf8String schemaName;
    while (Utf8String::npos != (pos = str.GetNextToken(schemaName, ",", pos)))
        json[SCHEMAS_SPECIFICATION_SCHEMANAMES].append(schemaName.Trim());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CommonToolsInternal::ParseSchemaAndClassName(Utf8StringR schemaName, Utf8StringR className, JsonValueCR json)
    {
    if (!json.isMember(SINGLE_SCHEMA_CLASS_SPECIFICATION_SCHEMANAME) || !json[SINGLE_SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].isString())
        return;

    if (!json.isMember(SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME) || !json[SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME].isString())
        return;

    schemaName = json[SINGLE_SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].asCString();
    className = json[SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME].asCString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CommonToolsInternal::SchemaAndClassNameToString(JsonValueCR json)
    {
    Utf8String schemaName, className;
    ParseSchemaAndClassName(schemaName, className, json);
    if (schemaName.empty() || className.empty())
        return "";
    return schemaName.append(":").append(className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CommonToolsInternal::SchemaAndClassNameToJson(Utf8StringCR schemaName, Utf8StringCR className)
    {
    Json::Value json;
    json[SINGLE_SCHEMA_CLASS_SPECIFICATION_SCHEMANAME] = schemaName;
    json[SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME] = className;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CommonToolsInternal::SchemaAndClassNameToJson(Utf8StringCR str)
    {
    bvector<Utf8String> parts;
    BeStringUtilities::Split(str.c_str(), ":", parts);
    if (parts.size() != 2)
        {
        BeAssert(false);
        return Json::Value();
        }
    return SchemaAndClassNameToJson(parts[0], parts[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void ParseSchemaAndClassNames(JsonValueCR json, Utf8StringR schemaName, bvector<Utf8String>& includedClassNames, bvector<Utf8String>& excludedClassNames)
    {
    if (!json.isMember(MULTI_SCHEMA_CLASSES_SPECIFICATION_SCHEMANAME) || !json[MULTI_SCHEMA_CLASSES_SPECIFICATION_SCHEMANAME].isString())
        return;

    if (!json.isMember(MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES) || !json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES].isArray())
        return;

    schemaName = json[MULTI_SCHEMA_CLASSES_SPECIFICATION_SCHEMANAME].asCString();
    for (Json::ArrayIndex i = 0; i < json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES].size(); ++i)
        {
        Utf8CP className = json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES][i].asCString();
        if (0 == BeStringUtilities::Strnicmp("E:", className, 2))
            excludedClassNames.push_back(className + 2);
        else
            includedClassNames.push_back(className);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CommonToolsInternal::SchemaAndClassNamesToString(JsonValueCR json)
    {
    Utf8String str;
    if (json.isArray())
        {
        bmap<Utf8String, bvector<Utf8String>> schemaExcludes;
        for (Json::ArrayIndex i = 0; i < json.size(); ++i)
            {
            Utf8String schemaName;
            bvector<Utf8String> includes, excludes;
            ParseSchemaAndClassNames(json[i], schemaName, includes, excludes);
            if (!includes.empty())
                {
                if (!str.empty())
                    str.append(";");
                str.append(schemaName).append(":").append(BeStringUtilities::Join(includes, ","));
                }
            if (!excludes.empty())
                schemaExcludes.Insert(schemaName, excludes);
            }
        if (!schemaExcludes.empty())
            {
            if (!str.empty())
                str.append(";");
            str.append("E:");
            bool firstExclude = true;
            for (auto entry : schemaExcludes)
                {
                if (!firstExclude)
                    str.append(";");
                str.append(entry.first).append(":").append(BeStringUtilities::Join(entry.second, ","));
                firstExclude = false;
                }
            }
        }
    else
        {
        Utf8String schemaName;
        bvector<Utf8String> includes, excludes;
        ParseSchemaAndClassNames(json, schemaName, includes, excludes);
        if (!includes.empty())
            str.append(schemaName).append(":").append(BeStringUtilities::Join(includes, ","));
        if (!excludes.empty())
            str.append("E:").append(schemaName).append(":").append(BeStringUtilities::Join(excludes, ","));
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CommonToolsInternal::SchemaAndClassNamesToJson(Utf8StringCR str)
    {
    Json::Value json(Json::arrayValue);
    bool isExcludes = false;
    bmap<Utf8String, Json::ArrayIndex> schemaIndexes;
    bvector<Utf8String> parts;
    BeStringUtilities::Split(str.c_str(), ";", parts);
    for (Utf8StringCR part : parts)
        {
        size_t indexOffset = 0;
        bvector<Utf8String> schemaAndClasses;
        BeStringUtilities::Split(part.c_str(), ":", schemaAndClasses);
        if (schemaAndClasses.size() < 2)
            {
            BeAssert(false);
            continue;
            }
        else if (schemaAndClasses.size() == 3)
            {
            BeAssert(schemaAndClasses[0].EqualsI("E"));
            isExcludes = true;
            indexOffset = 1;
            }
        Utf8StringCR schemaName = schemaAndClasses[0 + indexOffset];
        bvector<Utf8String> classNames;
        BeStringUtilities::Split(schemaAndClasses[1 + indexOffset].c_str(), ",", classNames);

        Json::Value* schemaJsonPtr = nullptr;
        auto schemaIndexIter = schemaIndexes.find(schemaName);
        if (schemaIndexes.end() != schemaIndexIter)
            {
            schemaJsonPtr = &json[schemaIndexIter->second];
            }
        else
            {
            Json::Value schemaClasses;
            schemaClasses[MULTI_SCHEMA_CLASSES_SPECIFICATION_SCHEMANAME] = schemaName;
            schemaJsonPtr = &json.append(schemaClasses);
            schemaIndexes.Insert(schemaName, json.size() - 1);
            }
        for (Utf8StringR className : classNames)
            {
            if (isExcludes)
                className = Utf8String("E:").append(className);
            (*schemaJsonPtr)[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES].append(className);
            }
        }
    if (json.size() == 1)
        return json[0];
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void ReverseString(Utf8StringR str)
    {
    for (size_t i = 0; i < str.size() / 2; i++)
        std::swap(str[i], str[str.size() - i - 1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CommonTools::ToBase36String(uint64_t i)
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
* @bsimethod                                    Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t CommonTools::GetBriefcaseId(ECInstanceId id) {return (uint64_t)id.GetValueUnchecked() >> 40;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t CommonTools::GetLocalId(ECInstanceId id) {return (uint64_t)id.GetValueUnchecked() & (((uint64_t)1 << 40) - 1);}
