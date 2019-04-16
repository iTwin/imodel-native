/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Presentation rule for adding and configuring check boxes.
* @bsiclass                                     Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CheckBoxRule : public ConditionalCustomizationRule
    {
    private:
        Utf8String              m_propertyName;
        bool                    m_useInversedPropertyValue;
        bool                    m_defaultValue;
        Utf8String              m_isEnabled;

    protected:
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //! Accept nested cutomization rule visitor
        ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

        //! Clones rule.
        CustomizationRule* _Clone() const override {return new CheckBoxRule(*this);}

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT CheckBoxRule ();

        //! Constructor.
        ECPRESENTATION_EXPORT CheckBoxRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, Utf8StringCR propertyName, bool useInversedPropertyValue, bool defaultValue, Utf8StringCR isEnabled);

        //! ECProperty name to bind check box state.
        ECPRESENTATION_EXPORT Utf8StringCR        GetPropertyName (void) const;

        //! Defines if inversed property value should be used for check box state.
        ECPRESENTATION_EXPORT bool                GetUseInversedPropertyValue (void) const;

        //! Default check box value.
        ECPRESENTATION_EXPORT bool                GetDefaultValue (void) const;

        //! An ECExpression indicating whether check box should be enabled
        ECPRESENTATION_EXPORT Utf8StringCR        GetIsEnabled(void) const;
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
