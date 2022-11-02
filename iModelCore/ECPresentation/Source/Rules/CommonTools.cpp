/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleCommonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/CommonTools.h>
#include <ECPresentation/Rules/RelatedInstanceNodesSpecification.h>
#include <ECPresentation/Rules/RelatedPropertiesSpecification.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Version::FromString(Version& version, Utf8CP str)
    {
    version = Version();
    if (Utf8String::IsNullOrEmpty(str))
        return ERROR;

    bvector<Utf8String> tokens;
    BeStringUtilities::Split(str, ".", tokens);
    if (tokens.size() < 1)
        return ERROR;

    Utf8P end = nullptr;
    unsigned major = strtoul(tokens[0].c_str(), &end, 10);
    if (end == tokens[0].c_str())
        return ERROR;

    unsigned minor = 0;
    if (tokens.size() > 1)
        minor = strtoul(tokens[1].c_str(), nullptr, 10);

    unsigned patch = 0;
    if (tokens.size() > 2)
        patch = strtoul(tokens[2].c_str(), nullptr, 10);

    version = Version(major, minor, patch);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Version::ToString() const
    {
    return Utf8PrintfString("%" PRIu32 ".%" PRIu32 ".%" PRIu32, m_major, m_minor, m_patch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RuleTargetTree CommonToolsInternal::ParseTargetTreeString(Utf8CP targetTreeString, Utf8CP attributeIdentifier)
    {
    if (0 == strcmp(targetTreeString, COMMON_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE))
        return TargetTree_MainTree;
    if (0 == strcmp(targetTreeString, COMMON_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE))
        return TargetTree_SelectionTree;
    if (0 == strcmp(targetTreeString, COMMON_ATTRIBUTE_VALUE_TARGET_TREE_BOTH))
        return TargetTree_Both;

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid `%s` attribute value: `%s`. Expected \"" COMMON_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE "\", "
        "\"" COMMON_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE "\" or \"" COMMON_ATTRIBUTE_VALUE_TARGET_TREE_BOTH "\".", attributeIdentifier, targetTreeString));
    return TargetTree_MainTree;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CommonToolsInternal::FormatTargetTreeString(RuleTargetTree targetTree)
    {
    if (TargetTree_MainTree == targetTree)
        return COMMON_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE;
    if (TargetTree_SelectionTree == targetTree)
        return COMMON_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE;
    if (TargetTree_Both == targetTree)
        return COMMON_ATTRIBUTE_VALUE_TARGET_TREE_BOTH;
    return COMMON_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection CommonToolsInternal::ParseRequiredDirectionString(Utf8CP value, Utf8CP attributeIdentifier)
    {
    if (0 == strcmp(value, COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH))
        return RequiredRelationDirection_Both;
    if (0 == strcmp(value, COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD))
        return RequiredRelationDirection_Forward;
    if (0 == strcmp(value, COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD))
        return RequiredRelationDirection_Backward;

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid `%s` attribute value: `%s`. Expected \"" COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH "\", "
        "\"" COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD "\" or \"" COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD "\".", attributeIdentifier, value));
    return RequiredRelationDirection_Both;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CommonToolsInternal::FormatRequiredDirectionString(RequiredRelationDirection direction)
    {
    if (RequiredRelationDirection_Both == direction)
        return COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH;
    if (RequiredRelationDirection_Forward == direction)
        return COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD;
    if (RequiredRelationDirection_Backward == direction)
        return COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD;
    return COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMeaning CommonToolsInternal::ParseRelationshipMeaningString(Utf8CP value, Utf8CP attributeIdentifier)
    {
    if (0 == strcmp(value, COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_SAMEINSTANCE))
        return RelationshipMeaning::SameInstance;
    if (0 == strcmp(value, COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_RELATEDINSTANCE))
        return RelationshipMeaning::RelatedInstance;

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid `%s` attribute value: `%s`. Expected \"" COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_SAMEINSTANCE "\" or "
        "\"" COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_RELATEDINSTANCE "\".", attributeIdentifier, value));
    return RelationshipMeaning::RelatedInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> CommonToolsInternal::ParsePropertiesNames(Utf8StringCR properties)
    {
    bvector<Utf8String> propertiesNamesList;
    BeStringUtilities::Split(properties.c_str(), ",", propertiesNamesList);
    std::for_each(propertiesNamesList.begin(), propertiesNamesList.end(), [](Utf8StringR name) {name.Trim(); });
    return propertiesNamesList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CommonToolsInternal::ParseSchemaAndClassName(Utf8StringR schemaName, Utf8StringR className, JsonValueCR json, Utf8CP attributeIdentifier)
    {
    bool hasIssues = false;
    if (!json.isMember(SCHEMA_CLASS_SPECIFICATION_SCHEMANAME) || !json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].isString())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Value for `%s.%s` is either missing or invalid. Expected non-empty string", attributeIdentifier, SCHEMA_CLASS_SPECIFICATION_SCHEMANAME));
        hasIssues = true;
        }
    if (!json.isMember(SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME) || !json[SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME].isString())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Value for `%s.%s` is either missing or invalid. Expected non-empty string", attributeIdentifier, SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME));
        hasIssues = true;
        }
    if (hasIssues)
        return;

    schemaName = json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].asCString();
    className = json[SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME].asCString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CommonToolsInternal::SchemaAndClassNameToString(JsonValueCR json, Utf8CP attributeIdentifier)
    {
    Utf8String schemaName, className;
    ParseSchemaAndClassName(schemaName, className, json, attributeIdentifier);
    if (schemaName.empty() || className.empty())
        return "";
    return schemaName.append(":").append(className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CommonToolsInternal::SchemaAndClassNameToJson(Utf8StringCR schemaName, Utf8StringCR className)
    {
    Json::Value json;
    json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME] = schemaName;
    json[SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME] = className;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CommonToolsInternal::SchemaAndClassNameToJson(Utf8StringCR str)
    {
    bvector<Utf8String> parts;
    BeStringUtilities::Split(str.c_str(), ":", parts);
    if (parts.size() != 2)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, Utf8PrintfString("Failed to serialize schema and class name to JSON: '%s'", str.c_str()));

    return SchemaAndClassNameToJson(parts[0], parts[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ParseSchemaAndClassNames(JsonValueCR json, Utf8StringR schemaName, bvector<Utf8String>& includedClassNames, bvector<Utf8String>& excludedClassNames)
    {
    if (!json.isMember(SCHEMA_CLASS_SPECIFICATION_SCHEMANAME) || !json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].isString())
        return;

    if (!json.isMember(MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES) || !json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES].isArray())
        return;

    schemaName = json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].asCString();
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CommonToolsInternal::ParseMultiSchemaClassesFromJson(JsonValueCR json, bool defaultPoly, bvector<MultiSchemaClass*>& classes, HashableBase* parent)
    {
    auto readMultiSchemaClassFromJson = [&](JsonValueCR jsonElement)
        {
        auto schemaClass = MultiSchemaClass::LoadFromJson(jsonElement, defaultPoly);
        if (nullptr != schemaClass)
            {
            schemaClass->SetParent(parent);
            classes.push_back(schemaClass);
            return true;
            }
        return false;
        };

    if (json.isArray())
        {
        for (auto const& schemaClass : json)
            {
            auto success = readMultiSchemaClassFromJson(schemaClass);
            if (!success)
                return false;
            }
        return true;
        }

    if (json.isObject())
        return readMultiSchemaClassFromJson(json);

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, Utf8PrintfString("Failed to parse schema and class names from JSON. Expected array or object, got: '%s'", json.ToString().c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CommonToolsInternal::MultiSchemaClassesToJson(bvector<MultiSchemaClass*> const& multiSchemaClasses)
    {
    Json::Value json(Json::arrayValue);
    for (auto const& multiSchemaClass : multiSchemaClasses)
        json.append(multiSchemaClass->WriteJson());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CommonToolsInternal::ParseMultiSchemaClassesFromClassNamesString(Utf8StringCR classNames, bool defaultPolymorphism, bvector<MultiSchemaClass*>& classes, HashableBase* parent)
    {
    return CommonToolsInternal::ParseMultiSchemaClassesFromJson(CommonToolsInternal::SchemaAndClassNamesToJson(classNames), defaultPolymorphism, classes, parent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, Utf8PrintfString("Failed to serialize schema and class names to JSON: '%s'", part.c_str()));
        if (schemaAndClasses.size() == 3)
            {
            if (!schemaAndClasses[0].EqualsI("E"))
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, Utf8PrintfString("Failed to serialize schema and class names to JSON: '%s'. Found 3 parts and the first one does not equal to 'E'.", part.c_str()));
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
            schemaClasses[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME] = schemaName;
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ReverseString(Utf8StringR str)
    {
    for (size_t i = 0; i < str.size() / 2; i++)
        std::swap(str[i], str[str.size() - i - 1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t CommonTools::GetBriefcaseId(ECInstanceId id) { return (uint64_t)id.GetValueUnchecked() >> 40; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t CommonTools::GetLocalId(ECInstanceId id) { return (uint64_t)id.GetValueUnchecked() & (((uint64_t)1 << 40) - 1); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void SkipSpaces(Utf8CP& pos)
    {
    while (*pos == ' ')
        ++pos;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int ReadFlags(Utf8CP& pos)
    {
    SkipSpaces(pos);
    if (*pos == 'P' && *(pos + 1) == 'E' && *(pos + 2) == ':')
        {
        pos += 3;
        return CLASS_SELECTION_FLAG_Polymorphic | CLASS_SELECTION_FLAG_Exclude;
        }
    if (*pos == 'E' && *(pos + 1) == ':')
        {
        pos += 2;
        return CLASS_SELECTION_FLAG_Exclude;
        }
    return 0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String TakeSchemaName(Utf8CP& pos)
    {
    SkipSpaces(pos);
    Utf8CP front = pos;
    while (*pos && *pos != ':')
        ++pos;
    return Utf8String(front, pos).Trim();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String TakeClassName(Utf8CP& pos)
    {
    SkipSpaces(pos);
    Utf8CP front = pos;
    while (*pos && *pos != ',' && *pos != ';')
        ++pos;
    return Utf8String(front, pos).Trim();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassNamesParser::Iterator::Advance()
    {
    m_startPos = m_currentPos;
    SkipSpaces(m_currentPos);
    if (!*m_currentPos)
        return;

    if (*m_currentPos == ',' || *m_currentPos == ':')
        {
        // if we get here at comma or a colon, always expect a class name
        ++m_currentPos;
        m_currentClassName = TakeClassName(m_currentPos);
        }
    else
        {
        // otherwise we might be at the beginning or at a semi-colon - in that case
        // we should expect optional flags and schema name
        if (*m_currentPos == ';')
            ++m_currentPos;
        int flags = ReadFlags(m_currentPos);
        if (flags != 0 && m_parser.m_supportExclusion)
            m_currentFlags = flags;
        m_currentSchemaName = TakeSchemaName(m_currentPos);
        Advance(); // we should now be at a colon after a schema name - advance to read the class name
        }

    if (m_currentClassName.empty())
        m_startPos = m_currentPos;
    }
