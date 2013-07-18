/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ImageIdOverride.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
ImageId override rule implementation. This rule is used to override default ImageId
generation algorithm.
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageIdOverride : public PresentationRule
    {
    private:
        WString m_imageIdExpression;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ImageIdOverride ();

        //! Constructor.
        ECOBJECTS_EXPORT ImageIdOverride (WStringCR condition, int priority, WStringCR imageIdExpression);

        //! Returns ImageId override ECExpression string.
        ECOBJECTS_EXPORT WStringCR        GetImageId (void) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE