/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/LabelOverride.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Label and Description override rule implementation. This rule is used to override default 
label and description generation algorithm.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LabelOverride : public PresentationRule
    {
    private:
        WString m_label;
        WString m_description;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT LabelOverride ();

        //! Constructor.
        ECOBJECTS_EXPORT LabelOverride (WStringCR condition, int priority, WStringCR label, WStringCR description);

        //! Label override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
        ECOBJECTS_EXPORT WStringCR        GetLabel (void) const;

        //! Description override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
        ECOBJECTS_EXPORT WStringCR        GetDescription (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE