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

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Invalid `%s` attribute value: `%s`. Expected \"" COMMON_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE "\", "
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

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Invalid `%s` attribute value: `%s`. Expected \"" COMMON_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH "\", "
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

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Invalid `%s` attribute value: `%s`. Expected \"" COMMON_ATTRIBUTE_VALUE_RELATIONSHIP_MEANING_SAMEINSTANCE "\" or "
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
Utf8String CommonToolsInternal::SupportedSchemasToString(BeJsConst json)
    {
    if (!json.isMember(SCHEMAS_SPECIFICATION_SCHEMANAMES) || !json[SCHEMAS_SPECIFICATION_SCHEMANAMES].isArray())
        return "";

    Utf8String str;
    if (json.isMember(SCHEMAS_SPECIFICATION_ISEXCLUDE) && json[SCHEMAS_SPECIFICATION_ISEXCLUDE].asBool())
        str.append("E:");

    for (BeJsConst::ArrayIndex i = 0; i < json[SCHEMAS_SPECIFICATION_SCHEMANAMES].size(); ++i)
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
void CommonToolsInternal::WriteSupportedSchemasToJson(BeJsValue json, Utf8StringCR str)
    {
    size_t pos = 0;
    if (str.StartsWithI("E:"))
        {
        json[SCHEMAS_SPECIFICATION_ISEXCLUDE] = true;
        pos = 2;
        }
    Utf8String schemaName;

    BeJsValue schemaNames = json[SCHEMAS_SPECIFICATION_SCHEMANAMES];
    while (Utf8String::npos != (pos = str.GetNextToken(schemaName, ",", pos)))
        {
        schemaNames[schemaNames.size()] = schemaName.Trim();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CommonToolsInternal::ParseSchemaAndClassName(Utf8StringR schemaName, Utf8StringR className, BeJsConst json, Utf8CP attributeIdentifier)
    {
    bool hasIssues = false;
    if (!json.isMember(SCHEMA_CLASS_SPECIFICATION_SCHEMANAME) || !json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].isString())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Value for `%s.%s` is either missing or invalid. Expected non-empty string", attributeIdentifier, SCHEMA_CLASS_SPECIFICATION_SCHEMANAME));
        hasIssues = true;
        }
    if (!json.isMember(SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME) || !json[SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME].isString())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Value for `%s.%s` is either missing or invalid. Expected non-empty string", attributeIdentifier, SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME));
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
Utf8String CommonToolsInternal::SchemaAndClassNameToString(BeJsConst json, Utf8CP attributeIdentifier)
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
void CommonToolsInternal::WriteSchemaAndClassNameToJson(BeJsValue json, Utf8StringCR schemaName, Utf8StringCR className)
    {
    json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME] = schemaName;
    json[SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME] = className;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CommonToolsInternal::WriteSchemaAndClassNameToJson(BeJsValue json, Utf8StringCR str)
    {
    bvector<Utf8String> parts;
    BeStringUtilities::Split(str.c_str(), ":", parts);
    if (parts.size() != 2)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, Utf8PrintfString("Failed to serialize schema and class name to JSON: '%s'", str.c_str()));

    WriteSchemaAndClassNameToJson(json, parts[0], parts[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ParseSchemaAndClassNames(BeJsConst json, Utf8StringR schemaName, bvector<Utf8String>& includedClassNames, bvector<Utf8String>& excludedClassNames)
    {
    if (!json.isMember(SCHEMA_CLASS_SPECIFICATION_SCHEMANAME) || !json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].isString())
        return;

    if (!json.isMember(MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES) || !json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES].isArray())
        return;

    schemaName = json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].asCString();
    for (BeJsConst::ArrayIndex i = 0; i < json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES].size(); ++i)
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
Utf8String CommonToolsInternal::SchemaAndClassNamesToString(BeJsConst json)
    {
    Utf8String str;
    if (json.isArray())
        {
        bmap<Utf8String, bvector<Utf8String>> schemaExcludes;
        for (BeJsConst::ArrayIndex i = 0; i < json.size(); ++i)
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
bool CommonToolsInternal::ParseMultiSchemaClassesFromJson(BeJsConst json, bool defaultPoly, bvector<MultiSchemaClass*>& classes, HashableBase* parent)
    {
    if (json.isNull())
        return false;
    auto readMultiSchemaClassFromJson = [&](BeJsConst jsonElement)
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
        auto iterationStopped = json.ForEachArrayMember([&](BeJsConst::ArrayIndex i, BeJsConst schemaClass)
            {
            return !readMultiSchemaClassFromJson(schemaClass);
            });
        return !iterationStopped;
        }

    if (json.isObject())
        return readMultiSchemaClassFromJson(json);

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, Utf8PrintfString("Failed to parse schema and class names from JSON. Expected array or object, got: '%s'", json.Stringify().c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeJsDocument CommonToolsInternal::WriteMultiSchemaClassesToJson(bvector<MultiSchemaClass*> const& multiSchemaClasses)
    {
    BeJsDocument json;
    CommonToolsInternal::WriteMultiSchemaClassesToJson(json, multiSchemaClasses);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CommonToolsInternal::WriteMultiSchemaClassesToJson(BeJsValue json, bvector<MultiSchemaClass*> const& multiSchemaClasses)
    {
    json.SetEmptyArray();
    for (auto const& multiSchemaClass : multiSchemaClasses)
        multiSchemaClass->WriteJson(json[json.size()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CommonToolsInternal::ParseMultiSchemaClassesFromClassNamesString(Utf8StringCR classNames, bool defaultPolymorphism, bvector<MultiSchemaClass*>& classes, HashableBase* parent)
    {
    return CommonToolsInternal::ParseMultiSchemaClassesFromJson(CommonToolsInternal::WriteSchemaAndClassNamesToJson(classNames), defaultPolymorphism, classes, parent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeJsDocument CommonToolsInternal::WriteSchemaAndClassNamesToJson(Utf8StringCR str)
    {
    BeJsDocument json;
    WriteSchemaAndClassNamesToJson(json, str);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CommonToolsInternal::WriteSchemaAndClassNamesToJson(BeJsValue json, Utf8StringCR str)
    {
    bool isExcludes = false;
    bmap<Utf8String, BeJsConst::ArrayIndex> schemaIndexes;
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

        auto schemaIndexIter = schemaIndexes.find(schemaName);
        auto getSchemaJson = [&](BeJsConst jsonElement)
            {
            if (schemaIndexes.end() != schemaIndexIter)
                {
                return json[schemaIndexIter->second];
                }
            json[json.size()][SCHEMA_CLASS_SPECIFICATION_SCHEMANAME] = schemaName;
            schemaIndexes.Insert(schemaName, json.size() - 1);
            return json[json.size() - 1];
            };

        BeJsValue schemaJson = getSchemaJson(json);
        for (Utf8StringR className : classNames)
            {
            if (isExcludes)
                className = Utf8String("E:").append(className);
            BeJsValue classNamesJson = schemaJson[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES];
            classNamesJson[classNamesJson.size()] = className;
            }
        }
    if (json.size() == 1)
        json.From(json[0]);
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
