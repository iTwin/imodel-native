/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/LocalizationResourceKeyDefinition.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
Implementation of localization resource key definition. This rule is used to define 
additional settings for the resource key.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalizationResourceKeyDefinition : public PresentationKey
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString m_id;
        WString m_key;
        WString m_defaultValue;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual bool _ReadXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT LocalizationResourceKeyDefinition ()
            : PresentationKey (), m_id (L""), m_key (L""), m_defaultValue (L"")
            {
            }

        ECOBJECTS_EXPORT LocalizationResourceKeyDefinition (int priority, WStringCR id, WStringCR key, WStringCR defaultValue)
            : PresentationKey (priority), m_id (m_id), m_key (key), m_defaultValue (defaultValue)
            {
            }

        //! Id of the resource to localize.
        ECOBJECTS_EXPORT WStringCR    GetId (void) const             { return m_id; }

        //! New key of the resource.
        ECOBJECTS_EXPORT WStringCR    GetKey (void) const            { return m_key; }

        //! Default value which is used if the resource is not found.
        ECOBJECTS_EXPORT WStringCR    GetDefaultValue (void) const   { return m_defaultValue; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE