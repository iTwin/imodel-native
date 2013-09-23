/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/CommonTools.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Helper class for commonly used functions
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CommonTools
{
public:
    //! Parses TargetTree string value
    static RuleTargetTree              ParseTargetTreeString (WCharCP targetTreeString);

    //! Formats TargetTree string value
    static WCharCP                     FormatTargetTreeString (RuleTargetTree targetTree);

    //! Parses RequiredDirection string value
    static RequiredRelationDirection   ParseRequiredDirectionString (WCharCP value);

    //! Formats RequiredDirection string value
    static WCharCP                     FormatRequiredDirectionString (RequiredRelationDirection direction);

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
        if (rule->ReadXml (ruleNode))
            rulesCollection.push_back (rule);
        else
            delete rule;
        }

    //! Load rules from parent XmlNode and adds to collection
    template<typename RuleType, typename RuleCollectionType>
    static void                        LoadRulesFromXmlNode (BeXmlNodeP xmlNode, RuleCollectionType& rulesCollection, char const* ruleXmlElementName)
        {
        BeXmlDom::IterableNodeSet ruleNodes;
        xmlNode->SelectChildNodes (ruleNodes, ruleXmlElementName);

        FOR_EACH (BeXmlNodeP& ruleNode, ruleNodes)
            LoadRuleFromXmlNode<RuleType, RuleCollectionType> (ruleNode, rulesCollection);
        }

    //! Write rules to XmlNode
    template<typename RuleType, typename RuleCollectionType>
    static void WriteRulesToXmlNode (BeXmlNodeP parentXmlNode, RuleCollectionType& rulesCollection)
        {
        FOR_EACH (RuleType* rule, rulesCollection)
            rule->WriteXml (parentXmlNode);
        }
};

END_BENTLEY_ECOBJECT_NAMESPACE
