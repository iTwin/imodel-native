/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRule.h>
#include <ECPresentation/Rules/CustomizationRules.h>
#include <ECPresentation/Rules/RelationshipPathSpecification.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct InstanceLabelOverrideValueSpecification;

/*---------------------------------------------------------------------------------**//**
* CustomizationRule to override labels for instances of specified class
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE InstanceLabelOverride : CustomizationRule
{
    DEFINE_T_SUPER(CustomizationRule)

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
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

    //! Clones rule.
    CustomizationRule* _Clone() const override {return new InstanceLabelOverride(*this);}

public:
    InstanceLabelOverride() {}
    InstanceLabelOverride(int priority, bool onlyIfNotHandled, Utf8String className, bvector<InstanceLabelOverrideValueSpecification*> valueSpecifications)
        : T_Super(priority, onlyIfNotHandled), m_className(className), m_valueSpecifications(valueSpecifications)
        {}
    //! Deprecated. Use `InstanceLabelOverride(int, bool, Utf8String, bvector<InstanceLabelOverrideValueSpecification*>)`
    ECPRESENTATION_EXPORT InstanceLabelOverride(int priority, bool onlyIfNotHandled, Utf8String className, Utf8StringCR propertyNames);
    ECPRESENTATION_EXPORT InstanceLabelOverride(InstanceLabelOverride const&);
    ECPRESENTATION_EXPORT InstanceLabelOverride(InstanceLabelOverride&&);
    ECPRESENTATION_EXPORT ~InstanceLabelOverride();
    Utf8StringCR GetClassName() const { return m_className; }
    bvector<InstanceLabelOverrideValueSpecification*> const& GetValueSpecifications() const { return m_valueSpecifications; }
    ECPRESENTATION_EXPORT void AddValueSpecification(InstanceLabelOverrideValueSpecification&);
};

struct InstanceLabelOverrideCompositeValueSpecification;
struct InstanceLabelOverridePropertyValueSpecification;
struct InstanceLabelOverrideClassNameValueSpecification;
struct InstanceLabelOverrideClassLabelValueSpecification;
struct InstanceLabelOverrideBriefcaseIdValueSpecification;
struct InstanceLabelOverrideLocalIdValueSpecification;
struct InstanceLabelOverrideStringValueSpecification;
struct InstanceLabelOverrideRelatedInstanceLabelSpecification;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE InstanceLabelOverrideValueSpecificationVisitor
{
    friend struct InstanceLabelOverrideCompositeValueSpecification;
    friend struct InstanceLabelOverridePropertyValueSpecification;
    friend struct InstanceLabelOverrideClassNameValueSpecification;
    friend struct InstanceLabelOverrideClassLabelValueSpecification;
    friend struct InstanceLabelOverrideBriefcaseIdValueSpecification;
    friend struct InstanceLabelOverrideLocalIdValueSpecification;
    friend struct InstanceLabelOverrideStringValueSpecification;
    friend struct InstanceLabelOverrideRelatedInstanceLabelSpecification;
protected:
    virtual void _Visit(InstanceLabelOverrideCompositeValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverridePropertyValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideClassNameValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideClassLabelValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideBriefcaseIdValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideLocalIdValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideStringValueSpecification const&) {}
    virtual void _Visit(InstanceLabelOverrideRelatedInstanceLabelSpecification const&) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideValueSpecification : NoXmlSupport<PresentationKey>
{
    DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)

protected:
    virtual void _Accept(InstanceLabelOverrideValueSpecificationVisitor&) const = 0;
    virtual InstanceLabelOverrideValueSpecification* _Clone() const = 0;
    virtual bool _ShallowEqual(PresentationKeyCR other) const override {return true;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementTypeAttributeName() const override;

public:
    static InstanceLabelOverrideValueSpecification* Create(JsonValueCR json);

    //! Allows the visitor to visit this specification.
    void Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const { _Accept(visitor); }

    InstanceLabelOverrideValueSpecification* Clone() const { return _Clone(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideCompositeValueSpecification : InstanceLabelOverrideValueSpecification
{
    DEFINE_T_SUPER(InstanceLabelOverrideValueSpecification)

    struct Part : NoXmlSupport<PresentationKey>
    {
        DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)
    private:
        InstanceLabelOverrideValueSpecification* m_specification;
        bool m_isRequired;
    protected:
        Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
        Utf8CP _GetJsonElementType() const override {return nullptr;}
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
        ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    public:
        Part() : m_specification(nullptr), m_isRequired(false) {}
        Part(InstanceLabelOverrideValueSpecification& spec, bool isRequired = false) : m_specification(&spec), m_isRequired(isRequired) {}
        Part(Part const& other) : T_Super(other), m_specification(other.m_specification->Clone()), m_isRequired(other.m_isRequired) {}
        Part(Part&& other) : T_Super(std::move(other)), m_specification(other.m_specification), m_isRequired(other.m_isRequired) { other.m_specification = nullptr; }
        ~Part() { DELETE_AND_CLEAR(m_specification); }
        bool IsRequired() const { return m_isRequired; }
        InstanceLabelOverrideValueSpecification* GetSpecification() const { return m_specification; }
    };

private:
    Utf8String m_separator;
    bvector<Part*> m_parts;
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideCompositeValueSpecification(*this); }
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
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
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverridePropertyValueSpecification : InstanceLabelOverrideValueSpecification
{
    DEFINE_T_SUPER(InstanceLabelOverrideValueSpecification)
private:
    RelationshipPathSpecification m_pathToRelatedInstanceSpec;
    Utf8String m_propertyName;
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverridePropertyValueSpecification(*this); }
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
public:
    InstanceLabelOverridePropertyValueSpecification() {}
    InstanceLabelOverridePropertyValueSpecification(Utf8String propertyName, RelationshipPathSpecification spec = RelationshipPathSpecification())
        : m_propertyName(propertyName.Trim()), m_pathToRelatedInstanceSpec(std::move(spec))
        {}
    Utf8StringCR GetPropertyName() const {return m_propertyName;}
    RelationshipPathSpecification const& GetPathToRelatedInstanceSpecification() const {return m_pathToRelatedInstanceSpec;}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideClassNameValueSpecification : InstanceLabelOverrideValueSpecification
{
    DEFINE_T_SUPER(InstanceLabelOverrideValueSpecification)
private:
    bool m_full;
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideClassNameValueSpecification(*this); }
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
public:
    InstanceLabelOverrideClassNameValueSpecification() : m_full(false) {}
    InstanceLabelOverrideClassNameValueSpecification(bool full) : m_full(full) {}
    bool ShouldUseFullName() const { return m_full; }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideClassLabelValueSpecification : InstanceLabelOverrideValueSpecification
{
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideClassLabelValueSpecification(*this); }
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideBriefcaseIdValueSpecification : InstanceLabelOverrideValueSpecification
{
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideBriefcaseIdValueSpecification(*this); }
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideLocalIdValueSpecification : InstanceLabelOverrideValueSpecification
{
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideLocalIdValueSpecification(*this); }
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideStringValueSpecification : InstanceLabelOverrideValueSpecification
{
    DEFINE_T_SUPER(InstanceLabelOverrideValueSpecification)
private:
    Utf8String m_value;
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideStringValueSpecification(*this); }
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
public:
    InstanceLabelOverrideStringValueSpecification() {}
    InstanceLabelOverrideStringValueSpecification(Utf8String value) : m_value(value) {}
    Utf8StringCR GetValue() const { return m_value; }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideRelatedInstanceLabelSpecification : InstanceLabelOverrideValueSpecification
{
    DEFINE_T_SUPER(InstanceLabelOverrideValueSpecification)
private:
    RelationshipPathSpecification m_pathToRelatedInstanceSpec;
protected:
    void _Accept(InstanceLabelOverrideValueSpecificationVisitor& visitor) const override { visitor._Visit(*this); }
    InstanceLabelOverrideValueSpecification* _Clone() const override { return new InstanceLabelOverrideRelatedInstanceLabelSpecification(*this); }
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
public:
    InstanceLabelOverrideRelatedInstanceLabelSpecification() {}
    InstanceLabelOverrideRelatedInstanceLabelSpecification(RelationshipPathSpecification spec)
        : m_pathToRelatedInstanceSpec(std::move(spec))
        {}
    RelationshipPathSpecification const& GetPathToRelatedInstanceSpecification() const {return m_pathToRelatedInstanceSpec;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
