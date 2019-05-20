/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct CommonToolsInternal
{
private:
    CommonToolsInternal() {}

public:
    //! Parses TargetTree string value
    static RuleTargetTree ParseTargetTreeString(Utf8CP targetTreeString);

    //! Formats TargetTree string value
    static Utf8CP FormatTargetTreeString(RuleTargetTree targetTree);

    //! Parses RequiredDirection string value
    static RequiredRelationDirection ParseRequiredDirectionString(Utf8CP value);

    //! Formats RequiredDirection string value
    static Utf8CP FormatRequiredDirectionString(RequiredRelationDirection direction);

    //! Parses RelationshipMeaning string value
    static RelationshipMeaning ParseRelationshipMeaningString(Utf8CP value);

    //! Formats RelationshipMeaning string value
    static Utf8CP FormatRelationshipMeaningString(RelationshipMeaning meaning);

    //! Parse properties names from string value to vector of properties names.
    static bvector<Utf8String> ParsePropertiesNames(Utf8StringCR value);

    static Utf8String SupportedSchemasToString(JsonValueCR json);
    static Json::Value SupportedSchemasToJson(Utf8StringCR str);

    static void ParseSchemaAndClassName(Utf8StringR schemaName, Utf8StringR className, JsonValueCR json);
    static Utf8String SchemaAndClassNameToString(JsonValueCR json);
    static Json::Value SchemaAndClassNameToJson(Utf8StringCR str);
    static Json::Value SchemaAndClassNameToJson(Utf8StringCR schemaName, Utf8StringCR className);
    
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

    //! Load specification from Json object and adds to collection
    template<typename TRule> static TRule* LoadRuleFromJson(JsonValueCR json)
        {
        TRule* rule = new TRule();
        if (!rule->ReadJson(json))
            DELETE_AND_CLEAR(rule);
        return rule;
        }

    //! Load rules from json array and add them to collection
    template<typename TRule>
    static void LoadFromJson(JsonValueCR json, bvector<TRule*>& collection, TRule*(*factory)(JsonValueCR), HashableBase* parentHashable)
        {
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
    static void LoadFromJsonByPriority(JsonValueCR json, bvector<TRule*>& collection, TRule*(*factory)(JsonValueCR), HashableBase* parentHashable)
        {
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
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
