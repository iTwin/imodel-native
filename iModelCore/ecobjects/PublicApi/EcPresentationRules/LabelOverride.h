/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/LabelOverride.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
Label and Description override rule implementation. This rule is used to override default 
label and description generation algorithm.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LabelOverride : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString m_label;
        WString m_description;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT LabelOverride ()
            : PresentationRule (), m_label (L""), m_description (L"")
            {
            }

        ECOBJECTS_EXPORT LabelOverride (WStringCR condition, int priority, WStringCR label, WStringCR description)
            : PresentationRule (condition, priority, false), m_label (label), m_description (description)
            {
            }

        //! Label override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
        ECOBJECTS_EXPORT WStringCR        GetLabel (void) const          { return m_label; }

        //! Description override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
        ECOBJECTS_EXPORT WStringCR        GetDescription (void) const    { return m_description; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE