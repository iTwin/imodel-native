/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/CustomNodeSpecification.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns customly defined nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomNodeSpecification : public ChildNodeSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString  m_type;
        WString  m_label;
        WString  m_description;
        WString  m_imageId;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT CustomNodeSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT CustomNodeSpecification (int priority, bool hideIfNoChildren, WStringCR type, WStringCR label, WStringCR description, WStringCR imageId);

        //! Returns type of the custom node.
        ECOBJECTS_EXPORT WStringCR                    GetNodeType (void) const;

        //! Returns label of the custom node.
        ECOBJECTS_EXPORT WStringCR                    GetLabel (void) const;

        //! Returns description of the custom node.
        ECOBJECTS_EXPORT WStringCR                    GetDescription (void) const;

        //! Returns ImageId of the custom node.
        ECOBJECTS_EXPORT WStringCR                    GetImageId (void) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
