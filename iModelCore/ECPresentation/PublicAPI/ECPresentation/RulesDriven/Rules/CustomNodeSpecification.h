/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
        ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationRuleSpecification const& other) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //! Clones this specification.
        ChildNodeSpecification* _Clone() const override {return new CustomNodeSpecification(*this);}

        //! Computes specification hash
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

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
