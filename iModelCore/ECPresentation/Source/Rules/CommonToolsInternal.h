/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/Rules/PresentationRulesTypes.h>
#include <ECPresentation/Rules/PresentationRule.h>
#include <ECPresentation/Rules/CommonTools.h>
#include <ECPresentation/Rules/MultiSchemaClass.h>
#include <ECPresentation/Diagnostics.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct CommonToolsInternal
{
private:
    CommonToolsInternal() {}

public:
    //! Parses RequiredDirection string value
    static RequiredRelationDirection ParseRequiredDirectionString(Utf8CP value, Utf8CP attributeIdentifier);

    //! Formats RequiredDirection string value
    static Utf8CP FormatRequiredDirectionString(RequiredRelationDirection direction);

    //! Parses RelationshipMeaning string value
    static RelationshipMeaning ParseRelationshipMeaningString(Utf8CP value, Utf8CP attributeIdentifier);

    //! Formats RelationshipMeaning string value
    static Utf8CP FormatRelationshipMeaningString(RelationshipMeaning meaning);

    //! Parse properties names from string value to vector of properties names.
    static bvector<Utf8String> ParsePropertiesNames(Utf8StringCR value);

    static Utf8String SupportedSchemasToString(BeJsConst json);
    static void WriteSupportedSchemasToJson(BeJsValue json, Utf8StringCR str);

    static void ParseSchemaAndClassName(Utf8StringR schemaName, Utf8StringR className, BeJsConst json, Utf8CP attributeIdentifier);
    static Utf8String SchemaAndClassNameToString(BeJsConst json, Utf8CP attributeIdentifier);
    static void WriteSchemaAndClassNameToJson(BeJsValue json, Utf8StringCR str);
    static void WriteSchemaAndClassNameToJson(BeJsValue json, Utf8StringCR schemaName, Utf8StringCR className);

    static bool ParseMultiSchemaClassesFromJson(BeJsConst json, bool defaultPoly, bvector<MultiSchemaClass*>& classes, HashableBase* parent);
    static bool ParseMultiSchemaClassesFromClassNamesString(Utf8StringCR classNames, bool defaultPolymorphism, bvector<MultiSchemaClass*>& classes, HashableBase* parent);

    static BeJsDocument WriteMultiSchemaClassesToJson(bvector<MultiSchemaClass*> const& multiSchemaClasses);
    static void WriteMultiSchemaClassesToJson(BeJsValue json, bvector<MultiSchemaClass*> const& multiSchemaClasses);

    static Utf8String SchemaAndClassNamesToString(BeJsConst json);
    static BeJsDocument WriteSchemaAndClassNamesToJson(Utf8StringCR str);
    static void WriteSchemaAndClassNamesToJson(BeJsValue json, Utf8StringCR str);

    //! Copies the rules in source vector into the target vector.
    template<typename T>
    static void CopyRules(bvector<T*>& target, bvector<T*> const& source, HashableBase* parentHashable)
        {
        for (T* rule : source)
            {
            T* copy = new T(*rule);
            copy->SetParent(parentHashable);
            target.push_back(copy);
            }
        }

    //! Clones the rules in source vector into the target vector.
    template<typename T>
    static void CloneRules(bvector<T*>& target, bvector<T*> const& source, HashableBase* parentHashable)
        {
        for (T const* rule : source)
            {
            T* clone = rule->Clone();
            clone->SetParent(parentHashable);
            target.push_back(clone);
            }
        }

    //! Copies the rules in source vector into the target vector.
    template<typename T>
    static void SwapRules(bvector<T*>& target, bvector<T*>& source, HashableBase* parentHashable)
        {
        target.swap(source);
        for (T* rule : target)
            rule->SetParent(parentHashable);
        }

    //! Frees and clears given list of objects
    template<typename T>
    static void FreePresentationRules(T& set)
        {
        for (typename T::const_iterator iter = set.begin (); iter != set.end (); ++iter)
            delete *iter;
        set.clear ();
        }

    template<typename TRule, typename TCollection>
    static void AddToCollection(TCollection& collection, TRule* rule)
        {
        if (nullptr != rule)
            collection.push_back(rule);
        }

    template<typename TRule, typename TCollection>
    static void AddToCollectionByPriority(TCollection& collection, TRule* rule)
        {
        if (nullptr != rule)
            CommonTools::AddToListByPriority(collection, *rule);
        }

    template<typename TRule> static TRule* LoadRuleFromJson(BeJsConst json)
        {
        return LoadRuleFromJson<TRule>(json, &TRule::ReadJson);
        }

    template<typename TRule> static TRule* LoadRuleFromJson(BeJsConst json, bool(TRule::*reader)(BeJsConst))
        {
        TRule* rule = new TRule();
        if (!(rule->*reader)(json))
            DELETE_AND_CLEAR(rule);
        return rule;
        }

    //! Load rules from json array and add them to collection
    template<typename TRule>
    static void LoadFromJson(Utf8CP ruleName, Utf8CP attributeName, BeJsConst json, bvector<TRule*>& collection, TRule*(*factory)(BeJsConst), HashableBase* parentHashable)
        {
        if (!ValidateJsonArrayValueType(ruleName, attributeName, json))
            return;

        for (BeJsConst::ArrayIndex i = 0; i < json.size(); i++)
            {
            TRule* rule = factory(json[i]);
            if (nullptr != rule)
                rule->SetParent(parentHashable);
            AddToCollection(collection, rule);
            }
        }

    //! Load rules from json array and add them to collection by priority
    template<typename TRule>
    static void LoadFromJsonByPriority(Utf8CP ruleName, Utf8CP attributeName, BeJsConst json, bvector<TRule*>& collection, TRule*(*factory)(BeJsConst), HashableBase* parentHashable)
        {
        if (!ValidateJsonArrayValueType(ruleName, attributeName, json))
            return;

        for (BeJsConst::ArrayIndex i = 0; i < json.size(); i++)
            {
            TRule* rule = factory(json[i]);
            if (nullptr != rule)
                rule->SetParent(parentHashable);
            AddToCollectionByPriority(collection, rule);
            }
        }

    //! Write rules to json
    template<typename TRule, typename TCollection>
    static void WriteRulesToJson(BeJsValue rulesList, TCollection const& rulesCollection)
        {
        for (TRule const* rule : rulesCollection)
            rule->WriteJson(rulesList[rulesList.size()]);
        }

    static Utf8CP GetJsonTypeStr(BeJsConst json)
        {
        if (json.isNull())
            return "null";
        if (json.isNumeric())
            return "number";
        if (json.isString())
            return "string";
        if (json.isBool())
            return "bool";
        if (json.isArray())
            return "array";
        if (json.isObject())
            return "object";
        return "<invalid>";
        }

    static bool ValidateJsonArrayValueType(Utf8CP ruleName, Utf8CP attributeName, BeJsConst attributeValue)
        {
        if (attributeValue.isArray())
            return true;

        Utf8String fullAttributeName(ruleName ? ruleName : "");
        if (attributeName && *attributeName)
            {
            if (!fullAttributeName.empty())
                fullAttributeName.append(".");
            fullAttributeName.append(attributeName);
            }

        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Invalid type for `%s`: `%s`. Expected `%s`.",
            fullAttributeName.c_str(), GetJsonTypeStr(attributeValue), "array"));
        return false;
        }

    static bool CheckRuleIssue(bool issueCondition, Utf8CP ruleName, Utf8CP attributeName, BeJsConst attributeValue, Utf8CP expectation)
        {
        if (!issueCondition)
            return false;

        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Invalid value for `%s.%s`: `%s`. Expected %s.",
            ruleName, attributeName, attributeValue.Stringify().c_str(), expectation));
        return true;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum ClassSelectionFlags
    {
    CLASS_SELECTION_FLAG_Include = 1 << 0,
    CLASS_SELECTION_FLAG_Exclude = 1 << 1,
    CLASS_SELECTION_FLAG_Polymorphic = 1 << 2,
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ClassNamesParser
{
    struct Entry
    {
    private:
        Utf8CP m_schemaName;
        Utf8CP m_className;
        int m_flags;
    public:
        Entry(Utf8CP schemaName, Utf8CP className, int flags)
            : m_schemaName(schemaName), m_className(className), m_flags(flags)
            {}
        Utf8CP GetSchemaName() const {return m_schemaName;}
        Utf8CP GetClassName() const {return m_className;}
        int GetFlags() const {return m_flags;}
        bool IsInclude() const {return 0 != ((int)CLASS_SELECTION_FLAG_Include & m_flags);}
        bool IsExclude() const {return 0 != ((int)CLASS_SELECTION_FLAG_Exclude & m_flags);}
        bool IsPolymorphic() const {return 0 != ((int)CLASS_SELECTION_FLAG_Polymorphic & m_flags);}
    };
    struct Iterator
    {
    private:
        ClassNamesParser const& m_parser;
        Utf8CP m_startPos;
        Utf8CP m_currentPos;
        int m_currentFlags;
        Utf8String m_currentSchemaName;
        Utf8String m_currentClassName;
    private:
        ECPRESENTATION_EXPORT void Advance();
    public:
        Iterator(ClassNamesParser const& parser, Utf8CP initialPos)
            : m_parser(parser), m_startPos(initialPos), m_currentPos(initialPos), m_currentFlags(CLASS_SELECTION_FLAG_Include | CLASS_SELECTION_FLAG_Polymorphic)
            { Advance(); }
        Entry operator*() const { return Entry(m_currentSchemaName.c_str(), m_currentClassName.c_str(), m_currentFlags); }
        Iterator& operator=(Iterator const& rhs) { m_startPos = rhs.m_startPos; m_currentPos = rhs.m_currentPos; return *this; }
        Iterator& operator++() { Advance(); return *this; }
        bool operator==(Iterator const& rhs) const { return m_startPos == rhs.m_startPos; }
        bool operator!=(Iterator const& rhs) const { return m_startPos != rhs.m_startPos; }
    };
private:
    Utf8CP m_str;
    bool m_supportExclusion;
public:
    ClassNamesParser(Utf8CP input, bool supportExclusion) : m_str(input), m_supportExclusion(supportExclusion) {}
    Iterator begin() const { return Iterator(*this, m_str); }
    Iterator end() const { return Iterator(*this, m_str + strlen(m_str)); }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
