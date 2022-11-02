/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/Rules/PresentationRulesTypes.h>
#include <ECPresentation/Rules/RelationshipPathSpecification.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedInstanceSpecification : PresentationKey
{
    DEFINE_T_SUPER(PresentationKey)

private:
    RelationshipPathSpecification m_relationshipPath;
    Utf8String m_alias;
    bool m_isRequired;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    RelatedInstanceSpecification() : m_isRequired(false) {}
    RelatedInstanceSpecification(RequiredRelationDirection direction, Utf8String relationshipName, Utf8String className, Utf8String alias, bool isRequired = false)
        : m_alias(alias), m_isRequired(isRequired)
        {
        m_relationshipPath.AddStep(*new RelationshipStepSpecification(relationshipName, direction, className));
        }
    RelatedInstanceSpecification(RelationshipPathSpecification relationshipPath, Utf8String alias, bool isRequired = false)
        : m_relationshipPath(relationshipPath), m_alias(alias), m_isRequired(isRequired)
        {}

    //! Get specification of the relationship path to the related instance
    RelationshipPathSpecification const& GetRelationshipPath() const {return m_relationshipPath;}
    RelationshipPathSpecification& GetRelationshipPath() {return m_relationshipPath;}

    //! Related instance alias which will be used to access properties of this instance. Must be unique per parent specification.
    Utf8StringCR GetAlias() const {return m_alias;}

    //! @see GetAlias
    void SetAlias(Utf8String alias) {m_alias = alias; InvalidateHash();}

    //! Get whether related instance is required
    bool IsRequired() const {return m_isRequired;}

    //! @see IsRequired
    void SetIsRequired(bool value) {m_isRequired = value; InvalidateHash();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
