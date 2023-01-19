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
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct CommonToolsInternal
{
private:
    CommonToolsInternal() {}

public:
    //! Parses TargetTree string value
    static RuleTargetTree ParseTargetTreeString(Utf8CP targetTreeString, Utf8CP attributeIdentifier);

    //! Formats TargetTree string value
    static Utf8CP FormatTargetTreeString(RuleTargetTree targetTree);

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

    static Utf8String SupportedSchemasToString(JsonValueCR json);
    static Json::Value SupportedSchemasToJson(Utf8StringCR str);

    static void ParseSchemaAndClassName(Utf8StringR schemaName, Utf8StringR className, JsonValueCR json, Utf8CP attributeIdentifier);
    static Utf8String SchemaAndClassNameToString(JsonValueCR json, Utf8CP attributeIdentifier);
    static Json::Value SchemaAndClassNameToJson(Utf8StringCR str);
    static Json::Value SchemaAndClassNameToJson(Utf8StringCR schemaName, Utf8StringCR className);

    static bool ParseMultiSchemaClassesFromJson(JsonValueCR json, bool defaultPoly, bvector<MultiSchemaClass*>& classes, HashableBase* parent);
    static bool ParseMultiSchemaClassesFromClassNamesString(Utf8StringCR classNames, bool defaultPolymorphism, bvector<MultiSchemaClass*>& classes, HashableBase* parent);

    static Json::Value MultiSchemaClassesToJson(bvector<MultiSchemaClass*> const& multiSchemaClasses);

    static Utf8String SchemaAndClassNamesToString(JsonValueCR json);
    static Json::Value SchemaAndClassNamesToJson(Utf8StringCR str);

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

    //! Load rule from XmlNode and adds to collection
    template<typename RuleType, typename RuleCollectionType>
    static void LoadRuleFromXmlNode(BeXmlNodeP ruleNode, RuleCollectionType& rulesCollection, HashableBase* parentHashable)
        {
        RuleType* rule = new RuleType ();
        if (rule->ReadXml(ruleNode))
            {
            rule->SetParent(parentHashable);
            CommonTools::AddToListByPriority(rulesCollection, *rule);
            }
        else
            delete rule;
        }

    //! Load rules from parent XmlNode and adds to collection
    template<typename RuleType, typename RuleCollectionType>
    static void LoadRulesFromXmlNode(BeXmlNodeP xmlNode, RuleCollectionType& rulesCollection, char const* ruleXmlElementName, HashableBase* parentHashable)
        {
        BeXmlDom::IterableNodeSet ruleNodes;
        xmlNode->SelectChildNodes (ruleNodes, ruleXmlElementName);

        for (BeXmlNodeP& ruleNode: ruleNodes)
            LoadRuleFromXmlNode<RuleType, RuleCollectionType> (ruleNode, rulesCollection, parentHashable);
        }

    //! Load specification from XmlNode and adds to collection
    template<typename SpecificationType, typename SpecificationsCollectionType>
    static void LoadSpecificationFromXmlNode (BeXmlNodeP specificationNode, SpecificationsCollectionType& specificationsCollection, HashableBase* parentHashable)
        {
        SpecificationType* specification = new SpecificationType();
        if (specification->ReadXml(specificationNode))
            {
            specification->SetParent(parentHashable);
            specificationsCollection.push_back(specification);
            }
        else
            delete specification;
        }

    //! Load specifications from parent XmlNode and adds to collection
    template<typename SpecificationType, typename SpecificationsCollectionType>
    static void LoadSpecificationsFromXmlNode (BeXmlNodeP xmlNode, SpecificationsCollectionType& specificationsCollection, char const* specificationXmlElementName, HashableBase* parentHashable)
        {
        BeXmlDom::IterableNodeSet specificationNodes;
        xmlNode->SelectChildNodes (specificationNodes, specificationXmlElementName);

        for (BeXmlNodeP& specificationNode: specificationNodes)
            LoadSpecificationFromXmlNode<SpecificationType, SpecificationsCollectionType> (specificationNode, specificationsCollection, parentHashable);
        }

    //! Write rules to XmlNode
    template<typename RuleType, typename RuleCollectionType>
    static void WriteRulesToXmlNode (BeXmlNodeP parentXmlNode, RuleCollectionType const& rulesCollection)
        {
        for (RuleType const* rule: rulesCollection)
            rule->WriteXml (parentXmlNode);
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

    template<typename TRule> static TRule* LoadRuleFromJson(JsonValueCR json)
        {
        return LoadRuleFromJson<TRule>(json, &TRule::ReadJson);
        }

    template<typename TRule> static TRule* LoadRuleFromJson(JsonValueCR json, bool(TRule::*reader)(JsonValueCR))
        {
        TRule* rule = new TRule();
        if (!(rule->*reader)(json))
            DELETE_AND_CLEAR(rule);
        return rule;
        }

    //! Load rules from json array and add them to collection
    template<typename TRule>
    static void LoadFromJson(Utf8CP ruleName, Utf8CP attributeName, JsonValueCR json, bvector<TRule*>& collection, TRule*(*factory)(JsonValueCR), HashableBase* parentHashable)
        {
        if (!ValidateJsonValueType(ruleName, attributeName, json, Json::arrayValue))
            return;

        for (Json::ArrayIndex i = 0; i < json.size(); i++)
            {
            TRule* rule = factory(json[i]);
            if (nullptr != rule)
                rule->SetParent(parentHashable);
            AddToCollection(collection, rule);
            }
        }

    //! Load rules from json array and add them to collection by priority
    template<typename TRule>
    static void LoadFromJsonByPriority(Utf8CP ruleName, Utf8CP attributeName, JsonValueCR json, bvector<TRule*>& collection, TRule*(*factory)(JsonValueCR), HashableBase* parentHashable)
        {
        if (!ValidateJsonValueType(ruleName, attributeName, json, Json::arrayValue))
            return;

        for (Json::ArrayIndex i = 0; i < json.size(); i++)
            {
            TRule* rule = factory(json[i]);
            if (nullptr != rule)
                rule->SetParent(parentHashable);
            AddToCollectionByPriority(collection, rule);
            }
        }

    //! Write rules to json
    template<typename TRule, typename TCollection>
    static void WriteRulesToJson(JsonValueR rulesList, TCollection const& rulesCollection)
        {
        for (TRule const* rule : rulesCollection)
            rulesList.append(rule->WriteJson());
        }

    static Utf8CP GetJsonValueTypeStr(Json::ValueType type)
        {
        switch (type)
            {
            case Json::nullValue: return "null";
            case Json::intValue: return "int";
            case Json::uintValue: return "uint";
            case Json::realValue: return "real";
            case Json::stringValue: return "string";
            case Json::booleanValue: return "boolean";
            case Json::arrayValue: return "array";
            case Json::objectValue: return "object";
            };
        return "<invalid>";
        }

    static bool ValidateJsonValueType(Utf8CP ruleName, Utf8CP attributeName, JsonValueCR attributeValue, Json::ValueType expectedType)
        {
        if (attributeValue.type() == expectedType)
            return true;

        Utf8String fullAttributeName(ruleName ? ruleName : "");
        if (attributeName && *attributeName)
            {
            if (!fullAttributeName.empty())
                fullAttributeName.append(".");
            fullAttributeName.append(attributeName);
            }

        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid type for `%s`: `%s`. Expected `%s`.",
            fullAttributeName.c_str(), GetJsonValueTypeStr(attributeValue.type()), GetJsonValueTypeStr(expectedType)));
        return false;
        }

    static bool CheckRuleIssue(bool issueCondition, Utf8CP ruleName, Utf8CP attributeName, JsonValueCR attributeValue, Utf8CP expectation)
        {
        if (!issueCondition)
            return false;

        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid value for `%s.%s`: `%s`. Expected %s.",
            ruleName, attributeName, attributeValue.ToString().c_str(), expectation));
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
