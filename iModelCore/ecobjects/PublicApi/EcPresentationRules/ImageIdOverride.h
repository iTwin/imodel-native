/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ImageIdOverride.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
ImageId override rule implementation. This rule is used to override default ImageId
generation algorithm.
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageIdOverride : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString m_imageIdExpression;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT ImageIdOverride ()
            : PresentationRule (), m_imageIdExpression (L"")
            {
            }

        ECOBJECTS_EXPORT ImageIdOverride (WStringCR condition, int priority, WStringCR imageIdExpression)
            : PresentationRule (condition, priority, false), m_imageIdExpression (imageIdExpression)
            {
            }

        //! Returns ImageId override ECExpression string.
        ECOBJECTS_EXPORT WStringCR        GetImageId (void) const    { return m_imageIdExpression; }
    };

END_BENTLEY_ECOBJECT_NAMESPACE