/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Rule used to inject some extended data into presentation data
* objects (nodes, records).
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ExtendedDataRule : ConditionalCustomizationRule
{
    DEFINE_T_SUPER(ConditionalCustomizationRule)

private:
    bmap<Utf8String, Utf8String> m_items;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT void _WriteJson(BeJsValue json) const override;

    //! Accecpt nested customization rule visitor
    void _Accept(CustomizationRuleVisitor& visitor) const override { visitor._Visit(*this); }

    //! Computes rule hash
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    //! Clones rule.
    CustomizationRule* _Clone() const override {return new ExtendedDataRule(*this);}
public:
    ExtendedDataRule() {}
    ExtendedDataRule(Utf8StringCR condition, bmap<Utf8String, Utf8String> itemsMap = bmap<Utf8String, Utf8String>())
        : ConditionalCustomizationRule(condition, 1000, false), m_items(itemsMap)
        {}
    ExtendedDataRule(Utf8StringCR condition, int priority, bool onlyIfNotHandled, bmap<Utf8String, Utf8String> itemsMap = bmap<Utf8String, Utf8String>())
        : ConditionalCustomizationRule(condition, priority, onlyIfNotHandled), m_items(itemsMap)
        {}

    //! Get key-value pairs for extended data value definitions in this rule
    bmap<Utf8String, Utf8String> const& GetItemsMap() const { return m_items; }

    //! Set key-value pairs for extended data value definitions in this rule
    ECPRESENTATION_EXPORT void SetItemsMap(bmap<Utf8String, Utf8String> map);

    //! Set a single extended data value definition. The `key` property must be
    //! unique. The `value` property is an ECExpression.
    ECPRESENTATION_EXPORT void AddItem(Utf8String key, Utf8String value);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
