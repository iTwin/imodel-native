/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/CheckBoxRule.h $
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
Presentation rule for adding and configuring check boxes.
* @bsiclass                                     Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CheckBoxRule : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        Utf8String              m_propertyName;
        bool                    m_useInversedPropertyValue;
        bool                    m_defaultValue;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP      _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool        _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void        _WriteXml (BeXmlNodeP xmlNode) override;

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT CheckBoxRule ();

        //! Constructor.
        ECOBJECTS_EXPORT CheckBoxRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, Utf8StringCR propertyName, bool useInversedPropertyValue, bool defaultValue);

        //! ECProperty name to bind check box state.
        ECOBJECTS_EXPORT Utf8StringCR        GetPropertyName (void) const;

        //! Defines if inversed property value should be used for check box state.
        ECOBJECTS_EXPORT bool                GetUseInversedPropertyValue (void) const;

        //! Default check box value.
        ECOBJECTS_EXPORT bool                GetDefaultValue (void) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
