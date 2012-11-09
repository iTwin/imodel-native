/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/CheckBoxRule.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
Presentation rule for adding and configuring check boxes.
* @bsiclass                                     Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CheckBoxRule : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString                 m_propertyName;
        bool                    m_useInversedPropertyValue;
        bool                    m_defaultValue;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP      _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool        _ReadXml (BeXmlNodeP xmlNode) override;
        ECOBJECTS_EXPORT virtual void        _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT CheckBoxRule ()
            : PresentationRule (), m_propertyName (L""), m_useInversedPropertyValue (false), m_defaultValue (false)
            {
            }

        ECOBJECTS_EXPORT CheckBoxRule (WStringCR condition, int priority, bool onlyIfNotHandled, WStringCR propertyName, bool useInversedPropertyValue, bool defaultValue)
            : PresentationRule (condition, priority, onlyIfNotHandled), m_propertyName (propertyName), m_useInversedPropertyValue (useInversedPropertyValue), m_defaultValue (defaultValue)
            {
            }

        //! ECProperty name to bind check box state.
        ECOBJECTS_EXPORT WStringCR           GetPropertyName (void) const                { return m_propertyName; }

        //! Defines if inversed property value should be used for check box state.
        ECOBJECTS_EXPORT bool                GetUseInversedPropertyValue (void) const    { return m_useInversedPropertyValue; }

        //! Default check box value.
        ECOBJECTS_EXPORT bool                GetDefaultValue (void) const                { return m_defaultValue; }
    };

END_BENTLEY_ECOBJECT_NAMESPACE