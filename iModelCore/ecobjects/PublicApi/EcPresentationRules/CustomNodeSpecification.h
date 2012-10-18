/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/CustomNodeSpecification.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

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
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT CustomNodeSpecification ()
            : ChildNodeSpecification (), m_type (L""), m_label (L""), m_description (L""), m_imageId (L"")
            {
            }

        ECOBJECTS_EXPORT CustomNodeSpecification (int priority, WStringCR type, WStringCR label, WStringCR description, WStringCR imageId)
            : ChildNodeSpecification (priority, true, false), 
            m_type (type), m_label (label), m_description (description), m_imageId (imageId)
            {
            }

        //! Returns type of the custom node.
        ECOBJECTS_EXPORT WStringCR                    GetNodeType (void) const      { return m_type; }

        //! Returns label of the custom node.
        ECOBJECTS_EXPORT WStringCR                    GetLabel (void) const         { return m_label; }

        //! Returns description of the custom node.
        ECOBJECTS_EXPORT WStringCR                    GetDescription (void) const   { return m_description; }

        //! Returns ImageId of the custom node.
        ECOBJECTS_EXPORT WStringCR                    GetImageId (void) const       { return m_imageId; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE