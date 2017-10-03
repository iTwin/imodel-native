/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/PropertyEditorsSpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for specifying editor for a single property.
* @bsiclass                                     Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorsSpecification
    {
    private:
        Utf8String m_propertyName;
        Utf8String m_editorName;
        PropertyEditorParametersList m_parameters;

    public:
        PropertyEditorsSpecification() {}
        PropertyEditorsSpecification(Utf8String propertyName, Utf8String editorName)
            : m_propertyName(propertyName), m_editorName(editorName)
            {}

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;

        //! Get property name.
        Utf8StringCR GetPropertyName() const {return m_propertyName;}

        //! Get editor name.
        Utf8StringCR GetEditorName() const {return m_editorName;}
        
        //! Get parameters.
        PropertyEditorParametersList const& GetParameters() const {return m_parameters;}
        PropertyEditorParametersList& GetParametersR() {return m_parameters;}
    };

struct PropertyEditorJsonParameters;
struct PropertyEditorMultilineParameters;
struct PropertyEditorRangeParameters;
struct PropertyEditorSliderParameters;

/*---------------------------------------------------------------------------------**//**
* Base class for property editor parameters specification.
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorParametersSpecification
{
    struct Visitor
        {
        virtual ~Visitor() {}
        virtual void _Visit(PropertyEditorJsonParameters const&) {}
        virtual void _Visit(PropertyEditorMultilineParameters const&) {}
        virtual void _Visit(PropertyEditorRangeParameters const&) {}
        virtual void _Visit(PropertyEditorSliderParameters const&) {}
        };

protected:
    virtual Utf8CP _GetXmlElementName() const = 0;
    virtual bool _ReadXml(BeXmlNodeP xmlNode) = 0;
    virtual void _WriteXml(BeXmlNodeP parentXmlNode) const = 0;
    virtual void _Accept(Visitor&) const = 0;

public:
    virtual ~PropertyEditorParametersSpecification() {}

    //! Accepts a visitor.
    void Accept(Visitor& visitor) const {_Accept(visitor);}
    
    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    bool ReadXml(BeXmlNodeP xmlNode) {return _ReadXml(xmlNode);}

    //! Writes rule information to given XmlNode.
    void WriteXml(BeXmlNodeP parentXmlNode) const {_WriteXml(parentXmlNode);}
};

/*---------------------------------------------------------------------------------**//**
* Specification for storing property editor-specific JSON parameters.
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorJsonParameters : PropertyEditorParametersSpecification
{
private:
    Json::Value m_json;
protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP parentXmlNode) const override;
    void _Accept(Visitor& visitor) const override {visitor._Visit(*this);}
public:
    PropertyEditorJsonParameters() {}
    PropertyEditorJsonParameters(Json::Value json) : m_json(json) {}
    JsonValueCR GetJson() const {return m_json;}
};

/*---------------------------------------------------------------------------------**//**
* Parameters for property editors which want to be displayed as multiline text editors. 
* @note Should only be used on text type properties.
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorMultilineParameters : PropertyEditorParametersSpecification
{
private:
    uint32_t m_height;
protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP parentXmlNode) const override;
    void _Accept(Visitor& visitor) const override {visitor._Visit(*this);}
public:
    PropertyEditorMultilineParameters() : m_height(1) {}
    PropertyEditorMultilineParameters(uint32_t height) : m_height(height) {}
    uint32_t GetHeightInRows() const {return m_height;}
};

/*---------------------------------------------------------------------------------**//**
* Parameters for property editors which want to be displayed as multiline text editors. 
* @note Should only be used on text type properties.
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorRangeParameters : PropertyEditorParametersSpecification
{
private:
    double m_min;
    double m_max;
    bool m_isMinSet;
    bool m_isMaxSet;
protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP parentXmlNode) const override;
    void _Accept(Visitor& visitor) const override {visitor._Visit(*this);}
public:
    PropertyEditorRangeParameters() : m_min(0), m_isMinSet(false), m_max(0), m_isMaxSet(false) {}
    PropertyEditorRangeParameters(double min, double max) : m_min(min), m_isMinSet(true), m_max(max), m_isMaxSet(true) {}
    double const* GetMinimumValue() const {return m_isMinSet ? &m_min : nullptr;}
    double const* GetMaximumValue() const {return m_isMaxSet ? &m_max : nullptr;}
};

/*---------------------------------------------------------------------------------**//**
* Parameters for property editors which want to be displayed as multiline text editors. 
* @note Should only be used on text type properties.
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorSliderParameters : PropertyEditorParametersSpecification
{
private:
    double m_min;
    double m_max;
    uint32_t m_intervalsCount;
    uint32_t m_valueFactor;
    bool m_isVertical;
protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP parentXmlNode) const override;
    void _Accept(Visitor& visitor) const override {visitor._Visit(*this);}
public:
    PropertyEditorSliderParameters() : m_min(0), m_max(0), m_intervalsCount(1), m_valueFactor(1), m_isVertical(false) {}
    PropertyEditorSliderParameters(double min, double max, uint32_t intervalsCount = 1, uint32_t valueFactor = 1, bool isVertical = false) 
        : m_min(min), m_max(max), m_isVertical(isVertical), m_intervalsCount(intervalsCount), m_valueFactor(valueFactor)
        {}
    double GetMinimumValue() const {return m_min;}
    double GetMaximumValue() const {return m_max;}
    uint32_t GetIntervalsCount() const {return m_intervalsCount;}
    uint32_t GetValueFactor() const {return m_valueFactor;}
    bool IsVertical() const {return m_isVertical;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE