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
    static RuleTargetTree ParseTargetTreeString (WString targetTreeString);

    //! Parses RequiredDirection string value
    static RequiredRelationDirection ParseRequiredDirectionString (WString value);

    //! Frees and clears given list of objects
    template<typename T>
    static void FreePresentationRules (T& set)
        {
        for (T::const_iterator iter = set.begin (); iter != set.end (); ++iter)
            delete *iter;
        set.clear ();
        }

    //! Load rule from XmlNode and adds to collection
    template<typename RuleType, typename RuleCollectionType>
    static void LoadRulesFromXmlNode (BeXmlNodeP xmlNode, RuleCollectionType& rulesCollection, char* ruleXmlElementName)
        {
        BeXmlDom::IterableNodeSet ruleNodes;
        xmlNode->SelectChildNodes (ruleNodes, ruleXmlElementName);

        FOR_EACH (BeXmlNodeP& ruleNode, ruleNodes)
            {
            RuleType* rule = new RuleType ();
            if (rule->ReadXml (ruleNode))
                rulesCollection.push_back (rule);
            else
                delete rule;
            }
        }
};

END_BENTLEY_ECOBJECT_NAMESPACE