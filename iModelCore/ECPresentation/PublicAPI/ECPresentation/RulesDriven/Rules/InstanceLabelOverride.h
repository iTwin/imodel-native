/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/InstanceLabelOverride.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* CustomizationRule to override labels for instances of specified class 
* @bsiclass                                     Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE InstanceLabelOverride : public CustomizationRule
    {
    private:
        Utf8String m_className;
        bvector<Utf8String> m_properties;

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

        ECPRESENTATION_EXPORT InstanceLabelOverride(int priority, bool onlyIfNotHandled, Utf8String className, Utf8StringCR propertyNames);

        ECPRESENTATION_EXPORT Utf8StringCR GetClassName() const;

        ECPRESENTATION_EXPORT bvector<Utf8String> const& GetPropertyNames() const;
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
