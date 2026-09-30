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
struct RelatedInstanceTargetInstancesSpecification : PresentationKey
{
    DEFINE_T_SUPER(PresentationKey)

private:
    Utf8String m_className;
    bvector<ECInstanceId> m_instanceIds;

protected:
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT void _WriteJson(BeJsValue) const override;

public:
    RelatedInstanceTargetInstancesSpecification() {}
    RelatedInstanceTargetInstancesSpecification(Utf8String className, bvector<ECInstanceId> instanceIds)
        : m_className(className), m_instanceIds(instanceIds)
        {}
    Utf8StringCR GetClassName() const {return m_className;}
    bvector<ECInstanceId> const GetInstanceIds() const {return m_instanceIds;}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedInstanceSpecification : PresentationKey
{
    DEFINE_T_SUPER(PresentationKey)

private:
    RelationshipPathSpecification m_relationshipPath;
    std::unique_ptr<RelatedInstanceTargetInstancesSpecification> m_targetInstancesSpecification;
    Utf8String m_alias;
    bool m_isRequired;

protected:
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT void _WriteJson(BeJsValue) const override;

public:
    RelatedInstanceSpecification() : m_isRequired(false) {}
    RelatedInstanceSpecification(RelatedInstanceSpecification const& other)
        : m_relationshipPath(other.m_relationshipPath), m_alias(other.m_alias), m_isRequired(other.m_isRequired)
        {
        if (other.m_targetInstancesSpecification)
            m_targetInstancesSpecification = std::make_unique<RelatedInstanceTargetInstancesSpecification>(*other.m_targetInstancesSpecification);
        }
    RelatedInstanceSpecification(RelatedInstanceSpecification&& other)
        : m_relationshipPath(std::move(other.m_relationshipPath)), m_targetInstancesSpecification(std::move(other.m_targetInstancesSpecification)), m_alias(std::move(other.m_alias)), m_isRequired(other.m_isRequired)
        {}
    RelatedInstanceSpecification(RequiredRelationDirection direction, Utf8String relationshipName, Utf8String className, Utf8String alias, bool isRequired = false)
        : m_alias(alias), m_isRequired(isRequired)
        {
        m_relationshipPath.AddStep(*new RelationshipStepSpecification(relationshipName, direction, className));
        }
    RelatedInstanceSpecification(RelationshipPathSpecification relationshipPath, Utf8String alias, bool isRequired = false)
        : m_relationshipPath(relationshipPath), m_alias(alias), m_isRequired(isRequired)
        {}
    RelatedInstanceSpecification(std::unique_ptr<RelatedInstanceTargetInstancesSpecification> targetInstancesSpecification, Utf8String alias, bool isRequired = false)
        : m_targetInstancesSpecification(std::move(targetInstancesSpecification)), m_alias(alias), m_isRequired(isRequired)
        {}

    //! Get specification of the relationship path to the related instance
    RelationshipPathSpecification const& GetRelationshipPath() const {return m_relationshipPath;}
    RelationshipPathSpecification& GetRelationshipPath() {return m_relationshipPath;}

    //! Defines a way to access related instance through a class and instance IDs. This should be mutually exclusive
    //! with relationship path.
    RelatedInstanceTargetInstancesSpecification const* GetTargetInstancesSpecification() const {return m_targetInstancesSpecification.get();}

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
