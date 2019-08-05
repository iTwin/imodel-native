/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for specifying single property that should be displayed or hidden.
* @bsiclass                                     Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertiesDisplaySpecification : HashableBase
    {
    private:
        Utf8String m_propertyNames;
        int m_priority;
        bool m_isDisplayed;

    protected:
        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        PropertiesDisplaySpecification(): m_priority(1000), m_isDisplayed(true) {}
        PropertiesDisplaySpecification(Utf8String propertyNames, int priority, bool isDisplayed)
            : m_propertyNames(propertyNames), m_priority(priority), m_isDisplayed(isDisplayed)
            {}

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;

        //! Reads rule information from Json, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);

        //! Reads rule information from Json, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT Json::Value WriteJson() const;
        
        //! Get the property names.
        Utf8StringCR GetPropertyNames() const {return m_propertyNames;}

        //! Get the specification priority.
        int GetPriority() const {return m_priority;}

        //! Get whether the property is displayed or not.
        bool IsDisplayed() const {return m_isDisplayed;}
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
