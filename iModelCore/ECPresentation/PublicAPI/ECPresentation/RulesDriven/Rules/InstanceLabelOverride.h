/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>
#include <ECPresentation/RulesDriven/Rules/CustomizationRules.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct InstanceLabelOverrideValueSpecification;

/*---------------------------------------------------------------------------------**//**
* CustomizationRule to override labels for instances of specified class 
* @bsiclass                                     Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE InstanceLabelOverride : CustomizationRule
{
private:
    Utf8String m_className;
    bvector<InstanceLabelOverrideValueSpecification*> m_valueSpecifications;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Accept nested customization rule visitor
    ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const override;

    //! Computes rule hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    //! Clones rule.
    CustomizationRule* _Clone() const override {return new InstanceLabelOverride(*this);}

public:
    InstanceLabelOverride() {}
    InstanceLabelOverride(int priority, bool onlyIfNotHandled, Utf8String className, bvector<InstanceLabelOverrideValueSpecification*> valueSpecifications)
        : CustomizationRule(priority, onlyIfNotHandled), m_className(className), m_valueSpecifications(valueSpecifications)
        {}
    //! Deprecated. Use `InstanceLabelOverride(int, bool, Utf8String, bvector<InstanceLabelOverrideValueSpecification*>)`
    ECPRESENTATION_EXPORT InstanceLabelOverride(int priority, bool onlyIfNotHandled, Utf8String className, Utf8StringCR propertyNames);
    ECPRESENTATION_EXPORT InstanceLabelOverride(InstanceLabelOverride const&);
    ECPRESENTATION_EXPORT InstanceLabelOverride(InstanceLabelOverride&&);
    ECPRESENTATION_EXPORT ~InstanceLabelOverride();
    Utf8StringCR GetClassName() const { return m_className; }
    bvector<InstanceLabelOverrideValueSpecification*> const& GetValueSpeficications() const { return m_valueSpecifications; }
    ECPRESENTATION_EXPORT void AddValueSpecification(InstanceLabelOverrideValueSpecification&);
};

struct InstanceLabelOverrideCompositeValueSpecification;
struct InstanceLabelOverridePropertyValueSpecification;
struct InstanceLabelOverrideClassNameValueSpecification;
struct InstanceLabelOverrideClassLabelValueSpecification;
struct InstanceLabelOverrideBriefcaseIdValueSpecification;
struct InstanceLabelOverrideLocalIdValueSpecification;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE InstanceLabelOverrideValueSpecificationVisitor
{
    friend struct InstanceLabelOverrideCompositeValueSpecification;
    friend struct InstanceLabelOverridePropertyValueSpecification;
    friend struct InstanceLabelOverrideClassNameValueSpecification;
    friend struct InstanceLabelOverrideClassLabelValueSpecification;
    friend struct InstanceLabelOverrideBriefcaseIdValueSpecification;
    friend struct InstanceLabelOverrideLocalIdValueSpecification;
protected:
    virtual void _Visit(InstanceLabelOverrideCompositeValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverridePropertyValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideClassNameValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideClassLabelValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideBriefcaseIdValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideLocalIdValueSpecification const&) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideValueSpecification : HashableBase
{
protected:
    virtual void _Accept(InstanceLabelOverrideValueSpecificationVisitor&) const = 0;
    virtual InstanceLabelOverrideValueSpecification* _Clone() const = 0;
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;
    virtual Utf8CP _GetJsonElementType() const = 0;
    virtual bool _ReadJson(JsonValueCR json) { return true; }
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const;

public:
    static InstanceLabelOverrideValueSpecification* Create(JsonValueCR json);
    virtual ~InstanceLabelOverrideValueSpecification() {}

    //! Allows the visitor to visit this specification.
    void Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const { _Accept(visitor); }

    InstanceLabelOverrideValueSpecification* Clone() const { return _Clone(); }

    //! Reads specification from json
    bool ReadJson(JsonValueCR json) { return _ReadJson(json); }

    //! Writes specification to json
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideCompositeValueSpecification : InstanceLabelOverrideValueSpecification
{
    struct Part : HashableBase
    {
    private:
        InstanceLabelOverrideValueSpecification* m_specification;
        bool m_isRequired;
    protected:
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;
    public:
        Part() : m_specification(nullptr), m_isRequired(false) {}
        Part(InstanceLabelOverrideValueSpecification& spec, bool isRequired = false) : m_specification(&spec), m_isRequired(isRequired) {}
        Part(Part const& other): m_specification(other.m_specification->Clone()), m_isRequired(other.m_isRequired) {}
        Part(Part&& other) : m_specification(other.m_specification), m_isRequired(other.m_isRequired) { other.m_specification = nullptr; }
        ~Part() { DELETE_AND_CLEAR(m_specification); }
        ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);
        ECPRESENTATION_EXPORT Json::Value WriteJson() const;
        bool IsRequired() const { return m_isRequired; }
        InstanceLabelOverrideValueSpecification* GetSpecification() const { return m_specification; }
    };

private:
    bvector<Part*> m_parts;
    Utf8String m_separator;
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideCompositeValueSpecification(*this); }
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
public:
    InstanceLabelOverrideCompositeValueSpecification(): m_separator(" ") {}
    InstanceLabelOverrideCompositeValueSpecification(bvector<Part*> parts, Utf8String separator = " "): m_parts(parts), m_separator(separator) {}
    ECPRESENTATION_EXPORT InstanceLabelOverrideCompositeValueSpecification(InstanceLabelOverrideCompositeValueSpecification const&);
    ECPRESENTATION_EXPORT InstanceLabelOverrideCompositeValueSpecification(InstanceLabelOverrideCompositeValueSpecification&&);
    ECPRESENTATION_EXPORT ~InstanceLabelOverrideCompositeValueSpecification();
    Utf8StringCR GetSeparator() const { return m_separator; }
    bvector<Part*> const GetValueParts() const { return m_parts; }
    ECPRESENTATION_EXPORT void AddValuePart(InstanceLabelOverrideValueSpecification& partSpecification, bool isRequired = false);
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverridePropertyValueSpecification : InstanceLabelOverrideValueSpecification
{
private:
    Utf8String m_propertyName;
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverridePropertyValueSpecification(*this); }
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
public:
    InstanceLabelOverridePropertyValueSpecification() {}
    InstanceLabelOverridePropertyValueSpecification(Utf8String propertyName): m_propertyName(propertyName.Trim()) {}
    Utf8StringCR GetPropertyName() const { return m_propertyName; }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideClassNameValueSpecification : InstanceLabelOverrideValueSpecification
{
private:
    bool m_full;
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideClassNameValueSpecification(*this); }
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
public:
    InstanceLabelOverrideClassNameValueSpecification() : m_full(false) {}
    InstanceLabelOverrideClassNameValueSpecification(bool full) : m_full(full) {}
    bool ShouldUseFullName() const { return m_full; }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideClassLabelValueSpecification : InstanceLabelOverrideValueSpecification
{
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideClassLabelValueSpecification(*this); }
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideBriefcaseIdValueSpecification : InstanceLabelOverrideValueSpecification
{
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideBriefcaseIdValueSpecification(*this); }
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideLocalIdValueSpecification : InstanceLabelOverrideValueSpecification
{
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideLocalIdValueSpecification(*this); }
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
