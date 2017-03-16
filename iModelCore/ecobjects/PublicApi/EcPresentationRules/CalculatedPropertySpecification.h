/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/CalculatedPropertySpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for specifying single property that is Calculated.
* @bsiclass                                     Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct CalculatedPropertiesSpecification
    {
    private:
        Utf8String m_label;
        int32_t m_priority;
        Utf8String m_value;

    public:
        CalculatedPropertiesSpecification() {}
        CalculatedPropertiesSpecification(Utf8String label, int priority, Utf8String value)
            : m_label(label), m_priority(priority), m_value(value)
            {}

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;

        //! Get label expression.
        Utf8StringCR GetLabel() const {return m_label;}

        //! Get property priority.
        int GetPriority() const {return m_priority;}

        //! Get property value expression.
        Utf8StringCR GetValue() const {return m_value;}
    };

END_BENTLEY_ECOBJECT_NAMESPACE
