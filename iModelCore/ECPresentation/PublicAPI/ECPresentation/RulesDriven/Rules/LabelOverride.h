/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/LabelOverride.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Label and Description override rule implementation. This rule is used to override default 
label and description generation algorithm.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE LabelOverride : public CustomizationRule
    {
    private:
        Utf8String m_label;
        Utf8String m_description;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP   _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) const override;

        //! Accept nested customization rule visitor
        ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT LabelOverride ();

        //! Constructor.
        ECPRESENTATION_EXPORT LabelOverride (Utf8StringCR condition, int priority, Utf8StringCR label, Utf8StringCR description);

        //! Label override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
        ECPRESENTATION_EXPORT Utf8StringCR        GetLabel (void) const;

        //! Set label override value. Can be ECExpression string. 
        ECPRESENTATION_EXPORT void             SetLabel (Utf8String value);

        //! Description override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
        ECPRESENTATION_EXPORT Utf8StringCR        GetDescription (void) const;

        //! Set description override value. Can be ECExpression string. 
        ECPRESENTATION_EXPORT void             SetDescription (Utf8String value);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
