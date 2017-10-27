/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/ImageIdOverride.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
ImageId override rule implementation. This rule is used to override default ImageId
generation algorithm.
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageIdOverride : public CustomizationRule
    {
    private:
        Utf8String m_imageIdExpression;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP   _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) const override;

        //! Accecpt nested customization rule visitor
        ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const  override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ImageIdOverride ();

        //! Constructor.
        ECPRESENTATION_EXPORT ImageIdOverride (Utf8StringCR condition, int priority, Utf8StringCR imageIdExpression);

        //! Returns ImageId override ECExpression string.
        ECPRESENTATION_EXPORT Utf8StringCR        GetImageId (void) const;

        //! Set imageId override ECExpression string.
        ECPRESENTATION_EXPORT void             SetImageId (Utf8String value);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
