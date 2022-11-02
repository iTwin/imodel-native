/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct PropertyEditorJsonParameters;
struct PropertyEditorMultilineParameters;
struct PropertyEditorRangeParameters;
struct PropertyEditorSliderParameters;

/*---------------------------------------------------------------------------------**//**
* Base class for property editor parameters specification.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorParametersSpecification : PresentationKey
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
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementTypeAttributeName() const override;
    virtual void _Accept(Visitor&) const = 0;
    virtual PropertyEditorParametersSpecification* _Clone() const = 0;

public:
    static PropertyEditorParametersSpecification* Create(JsonValueCR);
    PropertyEditorParametersSpecification* Clone() const {return _Clone();}
    void Accept(Visitor& visitor) const {_Accept(visitor);}
};

/*---------------------------------------------------------------------------------**//**
* Specification for storing property editor-specific JSON parameters.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorJsonParameters : PropertyEditorParametersSpecification
{
    DEFINE_T_SUPER(PropertyEditorParametersSpecification)

private:
    Json::Value m_json;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP parentXmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    void _Accept(Visitor& visitor) const override {visitor._Visit(*this);}
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    PropertyEditorParametersSpecification* _Clone() const override {return new PropertyEditorJsonParameters(*this);}

public:
    PropertyEditorJsonParameters() {}
    PropertyEditorJsonParameters(Json::Value json) : m_json(json) {}
    JsonValueCR GetJson() const {return m_json;}
};

/*---------------------------------------------------------------------------------**//**
* Parameters for property editors which want to be displayed as multiline text editors.
* @note Should only be used on text type properties.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorMultilineParameters : PropertyEditorParametersSpecification
{
    DEFINE_T_SUPER(PropertyEditorParametersSpecification)

private:
    uint32_t m_height;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP parentXmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    void _Accept(Visitor& visitor) const override {visitor._Visit(*this);}
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    PropertyEditorParametersSpecification* _Clone() const override {return new PropertyEditorMultilineParameters(*this);}

public:
    PropertyEditorMultilineParameters() : m_height(1) {}
    PropertyEditorMultilineParameters(uint32_t height) : m_height(height) {}
    uint32_t GetHeightInRows() const {return m_height;}
};

/*---------------------------------------------------------------------------------**//**
* Parameters for property editors which want to be displayed as multiline text editors.
* @note Should only be used on text type properties.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorRangeParameters : PropertyEditorParametersSpecification
{
    DEFINE_T_SUPER(PropertyEditorParametersSpecification)

private:
    Nullable<double> m_min;
    Nullable<double> m_max;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP parentXmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    void _Accept(Visitor& visitor) const override {visitor._Visit(*this);}
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    PropertyEditorParametersSpecification* _Clone() const override {return new PropertyEditorRangeParameters(*this);}

public:
    PropertyEditorRangeParameters()  {}
    PropertyEditorRangeParameters(double min, double max) : m_min(min), m_max(max) {}
    Nullable<double> const& GetMinimumValue() const {return m_min;}
    Nullable<double> const& GetMaximumValue() const {return m_max;}
};

/*---------------------------------------------------------------------------------**//**
* Parameters for property editors which want to be displayed as multiline text editors.
* @note Should only be used on text type properties.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorSliderParameters : PropertyEditorParametersSpecification
{
    DEFINE_T_SUPER(PropertyEditorParametersSpecification)

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

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    void _Accept(Visitor& visitor) const override {visitor._Visit(*this);}
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    PropertyEditorParametersSpecification* _Clone() const override {return new PropertyEditorSliderParameters(*this);}

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

/*---------------------------------------------------------------------------------**//**
* Specification for specifying editor for a single property.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorSpecification : PresentationKey
{
    DEFINE_T_SUPER(PresentationKey)

private:
    Utf8String m_name;
    PropertyEditorParametersList m_parameters;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    PropertyEditorSpecification() {}
    PropertyEditorSpecification(Utf8String editorName) : m_name(editorName) {}
    ECPRESENTATION_EXPORT PropertyEditorSpecification(PropertyEditorSpecification const&);
    ECPRESENTATION_EXPORT PropertyEditorSpecification(PropertyEditorSpecification&&);
    ECPRESENTATION_EXPORT ~PropertyEditorSpecification();

    //! Get editor name.
    Utf8StringCR GetEditorName() const {return m_name;}
    void SetEditorName(Utf8String value) {m_name = value; InvalidateHash();}

    //! Get parameters.
    PropertyEditorParametersList const& GetParameters() const {return m_parameters;}
    ECPRESENTATION_EXPORT void AddParameter(PropertyEditorParametersSpecificationR specification);
    ECPRESENTATION_EXPORT void ClearParameters();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
