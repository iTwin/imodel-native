/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/* __PUBLISH_SECTION_START__ */

#include <ECPresentation/RulesDriven/Rules/ChildNodeSpecification.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns all instance nodes available in the repository.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE AllInstanceNodesSpecification : public ChildNodeSpecification
    {
    private:
        bool     m_groupByClass;
        bool     m_groupByLabel;
        Utf8String  m_supportedSchemas;

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
        ChildNodeSpecification* _Clone() const override {return new AllInstanceNodesSpecification(*this);}

        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT AllInstanceNodesSpecification ();

        //! Constructor.
        //! @deprecated Use AllInstanceNodesSpecification(int, ChildrenHint, bool, bool, bool, bool, Utf8StringCR)
        ECPRESENTATION_EXPORT AllInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
            bool groupByClass, bool groupByLabel, Utf8StringCR supportedSchemas);

        //! Constructor.
        ECPRESENTATION_EXPORT AllInstanceNodesSpecification (int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
            bool groupByClass, bool groupByLabel, Utf8StringCR supportedSchemas);

        //! Returns true if grouping by class should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByClass (void) const;

        //! Sets GroupByClass value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByClass (bool value);

        //! Returns true if grouping by label should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByLabel (void) const;

        //! Sets GroupByLabel value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByLabel (bool value);

        //! Returns supported schemas that should be used by this specification.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetSupportedSchemas (void) const;

        //! Sets SupportedSchemas value. Can be string.
        ECPRESENTATION_EXPORT void                         SetSupportedSchemas (Utf8StringCR value);

    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
