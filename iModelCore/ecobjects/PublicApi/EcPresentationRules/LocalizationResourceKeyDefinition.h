/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/LocalizationResourceKeyDefinition.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Implementation of localization resource key definition. This rule is used to define 
additional settings for the resource key.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalizationResourceKeyDefinition : public PresentationKey
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        Utf8String m_id;
        Utf8String m_key;
        Utf8String m_defaultValue;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP  _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool    _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void    _WriteXml (BeXmlNodeP xmlNode);

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT LocalizationResourceKeyDefinition ();

        //! Constructor.
        ECOBJECTS_EXPORT LocalizationResourceKeyDefinition (int priority, Utf8StringCR id, Utf8StringCR key, Utf8StringCR defaultValue);

        //! Id of the resource to localize.
        ECOBJECTS_EXPORT Utf8StringCR    GetId (void) const;

        //! New key of the resource.
        ECOBJECTS_EXPORT Utf8StringCR    GetKey (void) const;

        //! Default value which is used if the resource is not found.
        ECOBJECTS_EXPORT Utf8StringCR    GetDefaultValue (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
