/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for selected items.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE SelectedNodeInstancesSpecification : ContentSpecification
{
    DEFINE_T_SUPER(ContentSpecification)

private:
    Utf8String  m_acceptableSchemaName;
    Utf8String  m_acceptableClassNames;
    bool        m_acceptablePolymorphically;

protected:
    //! Allows the visitor to visit this specification.
    ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Clones this content specification.
    ContentSpecification* _Clone() const override {return new SelectedNodeInstancesSpecification(*this);}

    //! Computes specification hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT SelectedNodeInstancesSpecification ();

    //! Constructor.
    ECPRESENTATION_EXPORT SelectedNodeInstancesSpecification (int priority, bool onlyIfNotHandled, Utf8StringCR acceptableSchemaName, Utf8StringCR acceptableClassNames, bool acceptablePolymorphically);

    //! Acceptable schema name of ECInstances that will be shown in the content.
    ECPRESENTATION_EXPORT Utf8StringCR                    GetAcceptableSchemaName (void) const;

    //! Sets the acceptable schema name of the specification.
    ECPRESENTATION_EXPORT void                         SetAcceptableSchemaName (Utf8StringCR value);

    //! Acceptable class names of ECInstances that will be shown in the content.
    ECPRESENTATION_EXPORT Utf8StringCR                    GetAcceptableClassNames (void) const;

    //! Sets the acceptable class names of the specification.
    ECPRESENTATION_EXPORT void                         SetAcceptableClassNames (Utf8StringCR value);

    //! Identifies whether AcceptableClasses should be check polymorphically.
    ECPRESENTATION_EXPORT bool                         GetAcceptablePolymorphically (void) const;
    //! Sets the AcceptablePolymorphically value for the specification.
    ECPRESENTATION_EXPORT void                         SetAcceptablePolymorphically (bool value);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
