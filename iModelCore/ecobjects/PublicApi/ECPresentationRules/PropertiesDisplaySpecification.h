/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECPresentationRules/PropertiesDisplaySpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for specifying single property that should be displayed or hidden.
* @bsiclass                                     Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertiesDisplaySpecification
    {
    private:
        Utf8String m_propertyNames;
        int m_priority;
        bool m_isDisplayed;

    public:
        PropertiesDisplaySpecification() {}
        PropertiesDisplaySpecification(Utf8String propertyNames, int priority, bool isDisplayed)
            : m_propertyNames(propertyNames), m_priority(priority), m_isDisplayed(isDisplayed)
            {}

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;
        
        //! Get the property names.
        Utf8StringCR GetPropertyNames() const {return m_propertyNames;}

        //! Get the specification priority.
        int GetPriority() const {return m_priority;}

        //! Get whether the property is displayed or not.
        bool IsDisplayed() const {return m_isDisplayed;}
    };

END_BENTLEY_ECOBJECT_NAMESPACE
