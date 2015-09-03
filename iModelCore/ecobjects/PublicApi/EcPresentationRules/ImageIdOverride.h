/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ImageIdOverride.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRule.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
ImageId override rule implementation. This rule is used to override default ImageId
generation algorithm.
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageIdOverride : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        Utf8String m_imageIdExpression;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ImageIdOverride ();

        //! Constructor.
        ECOBJECTS_EXPORT ImageIdOverride (Utf8StringCR condition, int priority, Utf8StringCR imageIdExpression);

        //! Returns ImageId override ECExpression string.
        ECOBJECTS_EXPORT Utf8StringCR        GetImageId (void) const;

        //! Set imageId override ECExpression string.
        ECOBJECTS_EXPORT void             SetImageId (Utf8String value);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
