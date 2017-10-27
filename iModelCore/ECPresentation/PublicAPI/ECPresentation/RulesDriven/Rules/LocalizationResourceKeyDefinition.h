/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/LocalizationResourceKeyDefinition.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Implementation of localization resource key definition. This rule is used to define 
additional settings for the resource key.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalizationResourceKeyDefinition : public PresentationKey
    {
    private:
        Utf8String m_id;
        Utf8String m_key;
        Utf8String m_defaultValue;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP  _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool    _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void    _WriteXml (BeXmlNodeP xmlNode) const override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT LocalizationResourceKeyDefinition ();

        //! Constructor.
        ECPRESENTATION_EXPORT LocalizationResourceKeyDefinition (int priority, Utf8StringCR id, Utf8StringCR key, Utf8StringCR defaultValue);

        //! Id of the resource to localize.
        ECPRESENTATION_EXPORT Utf8StringCR    GetId (void) const;

        //! New key of the resource.
        ECPRESENTATION_EXPORT Utf8StringCR    GetKey (void) const;

        //! Default value which is used if the resource is not found.
        ECPRESENTATION_EXPORT Utf8StringCR    GetDefaultValue (void) const;

    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
