/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/CustomNodeSpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns customly defined nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE CustomNodeSpecification : public ChildNodeSpecification
    {
    private:
        Utf8String  m_type;
        Utf8String  m_label;
        Utf8String  m_description;
        Utf8String  m_imageId;

    protected:
        //! Allows the visitor to visit this specification.
        ECPRESENTATION_EXPORT virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP               _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) const override;
        
        //! Clones this specification.
        virtual ChildNodeSpecification* _Clone() const override {return new CustomNodeSpecification(*this);}

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT CustomNodeSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT CustomNodeSpecification (int priority, bool hideIfNoChildren, Utf8StringCR type, Utf8StringCR label, Utf8StringCR description, Utf8StringCR imageId);

        //! Returns type of the custom node.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetNodeType (void) const;

        //! Sets the Node type. Can be string.
        ECPRESENTATION_EXPORT void                         SetNodeType (Utf8StringCR value);

        //! Returns label of the custom node.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetLabel (void) const;

        //! Sets the label. Can be string.
        ECPRESENTATION_EXPORT void                         SetLabel (Utf8StringCR value);

        //! Returns description of the custom node.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetDescription (void) const;

        //! Sets the description. Can be string.
        ECPRESENTATION_EXPORT void                         SetDescription (Utf8StringCR value);

        //! Returns ImageId of the custom node.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetImageId (void) const;
        //! Sets the ImageId. Can be string
        ECPRESENTATION_EXPORT void                         SetImageId (Utf8StringCR value);

    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
