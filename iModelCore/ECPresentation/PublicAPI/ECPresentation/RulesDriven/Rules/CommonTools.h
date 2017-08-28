/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/CommonTools.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Helper class for commonly used functions
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CommonTools
{
private:
    CommonTools() {}

public:
    //! Parses TargetTree string value
    static RuleTargetTree              ParseTargetTreeString (Utf8CP targetTreeString);

    //! Formats TargetTree string value
    static Utf8CP                     FormatTargetTreeString (RuleTargetTree targetTree);

    //! Parses RequiredDirection string value
    static RequiredRelationDirection   ParseRequiredDirectionString (Utf8CP value);

    //! Formats RequiredDirection string value
    static Utf8CP                     FormatRequiredDirectionString (RequiredRelationDirection direction);

    //! Parses RelationshipMeaning string value
    static RelationshipMeaning ParseRelationshipMeaningString(Utf8CP value);

    //! Formats RelationshipMeaning string value
    static Utf8CP FormatRelationshipMeaningString(RelationshipMeaning meaning);

    //! Copies the rules in source vector into the target vector.
    template<typename T> 
    static void CopyRules(bvector<T*>& target, bvector<T*> const& source)
        {
        for (T* rule : source)
            target.push_back(new T(*rule));
        }
    
    //! Clones the rules in source vector into the target vector.
    template<typename T> 
    static void CloneRules(bvector<T*>& target, bvector<T*> const& source)
        {
        for (T const* rule : source)
            target.push_back(rule->Clone());
        }

    //! Frees and clears given list of objects
    template<typename T>
    static void                        FreePresentationRules (T& set)
        {
        for (typename T::const_iterator iter = set.begin (); iter != set.end (); ++iter)
            delete *iter;
        set.clear ();
        }

    //! Load rule from XmlNode and adds to collection
    template<typename RuleType, typename RuleCollectionType>
    static void                        LoadRuleFromXmlNode (BeXmlNodeP ruleNode, RuleCollectionType& rulesCollection)
        {
        RuleType* rule = new RuleType ();
        if (rule->ReadXml(ruleNode))
            AddToListByPriority(rulesCollection, *rule);
        else
            delete rule;
        }

    //! Load rules from parent XmlNode and adds to collection
    template<typename RuleType, typename RuleCollectionType>
    static void                        LoadRulesFromXmlNode (BeXmlNodeP xmlNode, RuleCollectionType& rulesCollection, char const* ruleXmlElementName)
        {
        BeXmlDom::IterableNodeSet ruleNodes;
        xmlNode->SelectChildNodes (ruleNodes, ruleXmlElementName);

        for (BeXmlNodeP& ruleNode: ruleNodes)
            LoadRuleFromXmlNode<RuleType, RuleCollectionType> (ruleNode, rulesCollection);
        }

    //! Load specification from XmlNode and adds to collection
    template<typename SpecificationType, typename SpecificationsCollectionType>
    static void                        LoadSpecificationFromXmlNode (BeXmlNodeP specificationNode, SpecificationsCollectionType& specificationsCollection)
        {
        SpecificationType* specification = new SpecificationType();
        if (specification->ReadXml (specificationNode))
            specificationsCollection.push_back(specification);
        else
            delete specification;
        }

    //! Load specifications from parent XmlNode and adds to collection
    template<typename SpecificationType, typename SpecificationsCollectionType>
    static void                        LoadSpecificationsFromXmlNode (BeXmlNodeP xmlNode, SpecificationsCollectionType& specificationsCollection, char const* specificationXmlElementName)
        {
        BeXmlDom::IterableNodeSet specificationNodes;
        xmlNode->SelectChildNodes (specificationNodes, specificationXmlElementName);

        for (BeXmlNodeP& specificationNode: specificationNodes)
            LoadSpecificationFromXmlNode<SpecificationType, SpecificationsCollectionType> (specificationNode, specificationsCollection);
        }

    //! Write rules to XmlNode
    template<typename RuleType, typename RuleCollectionType>
    static void WriteRulesToXmlNode (BeXmlNodeP parentXmlNode, RuleCollectionType const& rulesCollection)
        {
        for (RuleType const* rule: rulesCollection)
            rule->WriteXml (parentXmlNode);
        }

public:
    //! Adds an element to the specified list sorted by elements priority
    template<typename ElementType, typename ListType> static void AddToListByPriority(ListType& list, ElementType& element)
        {
        auto iter = list.rbegin();
        for (; iter != list.rend(); iter++)
            {
            if ((*iter)->GetPriority() >= element.GetPriority())
                break;
            }
        list.insert(iter.base(), &element);
        }

    //! Removes an element from the specified list
    template<typename ElementType, typename ListType> static void RemoveFromList(ListType& list, ElementType& element)
        {
        auto iter = list.begin();
        for (; iter != list.end(); iter++)
            {
            if (*iter == &element)
                break;
            }
        if (list.end() != iter)
            list.erase(iter);
        }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
