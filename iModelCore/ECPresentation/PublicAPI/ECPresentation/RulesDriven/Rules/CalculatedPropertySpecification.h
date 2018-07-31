/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/CalculatedPropertySpecification.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for specifying single property that is Calculated.
* @bsiclass                                     Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct CalculatedPropertiesSpecification : HashableBase
    {
    private:
        Utf8String m_label;
        int32_t m_priority;
        Utf8String m_value;

    protected:
        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        CalculatedPropertiesSpecification(): m_priority(1000) {}
        CalculatedPropertiesSpecification(Utf8String label, int priority, Utf8String value)
            : m_label(label), m_priority(priority), m_value(value)
            {}

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;

        //! Reads rule information from Json, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);
    
        //! Writes specification to json.
        ECPRESENTATION_EXPORT Json::Value WriteJson() const;

        //! Get label expression.
        Utf8StringCR GetLabel() const {return m_label;}

        //! Get property priority.
        int GetPriority() const {return m_priority;}

        //! Get property value expression.
        Utf8StringCR GetValue() const {return m_value;}
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
