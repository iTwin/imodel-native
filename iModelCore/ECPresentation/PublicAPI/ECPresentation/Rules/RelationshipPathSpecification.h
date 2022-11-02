/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/ContentSpecification.h>
#include <ECPresentation/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelationshipStepSpecification : NoXmlSupport<PresentationKey>
{
    DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)

private:
    Utf8String m_relationshipClassName;
    RequiredRelationDirection m_direction;
    Utf8String m_targetClassName;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    RelationshipStepSpecification() : m_direction(RequiredRelationDirection_Forward) {}
    RelationshipStepSpecification(RelationshipStepSpecification const& other)
        : T_Super(other), m_relationshipClassName(other.m_relationshipClassName), m_direction(other.m_direction), m_targetClassName(other.m_targetClassName)
        {}
    RelationshipStepSpecification(RelationshipStepSpecification&& other)
        : T_Super(std::move(other)), m_relationshipClassName(std::move(other.m_relationshipClassName)), m_direction(other.m_direction), m_targetClassName(std::move(other.m_targetClassName))
        {}
    RelationshipStepSpecification(Utf8String relationshipClassName, RequiredRelationDirection direction, Utf8String targetClassName = "")
        : m_relationshipClassName(relationshipClassName), m_direction(direction), m_targetClassName(targetClassName)
        {}
    RelationshipStepSpecification& operator=(RelationshipStepSpecification const& other)
        {
        m_relationshipClassName = other.m_relationshipClassName;
        m_direction = other.m_direction;
        m_targetClassName = other.m_targetClassName;
        return *this;
        }
    RelationshipStepSpecification& operator=(RelationshipStepSpecification&& other)
        {
        m_relationshipClassName = std::move(other.m_relationshipClassName);
        m_direction = std::move(other.m_direction);
        m_targetClassName = std::move(other.m_targetClassName);
        return *this;
        }
    bool operator==(RelationshipStepSpecification const& other) const
        {
        return m_relationshipClassName == other.m_relationshipClassName
            && m_targetClassName == other.m_targetClassName
            && m_direction == other.m_direction;
        }
    RequiredRelationDirection GetRelationDirection() const {return m_direction;}
    void SetRelationDirection(RequiredRelationDirection direction) {m_direction = direction; InvalidateHash();}
    Utf8StringCR GetRelationshipClassName() const {return m_relationshipClassName;}
    void SetRelationshipClassName(Utf8String value) {m_relationshipClassName = value; InvalidateHash();}
    Utf8StringCR GetTargetClassName() const {return m_targetClassName;}
    void SetTargetClassName(Utf8String value) {m_targetClassName = value; InvalidateHash();}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelationshipPathSpecification : NoXmlSupport<PresentationKey>
{
    DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)

private:
    bvector<RelationshipStepSpecification*> m_steps;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    RelationshipPathSpecification() {}
    RelationshipPathSpecification(RelationshipStepSpecification& step) {m_steps.push_back(&step);}
    RelationshipPathSpecification(bvector<RelationshipStepSpecification*> steps) : m_steps(steps) {}
    ECPRESENTATION_EXPORT RelationshipPathSpecification(RelationshipPathSpecification const&);
    ECPRESENTATION_EXPORT RelationshipPathSpecification(RelationshipPathSpecification&&);
    ECPRESENTATION_EXPORT ~RelationshipPathSpecification();
    ECPRESENTATION_EXPORT RelationshipPathSpecification& operator=(RelationshipPathSpecification const&);
    ECPRESENTATION_EXPORT RelationshipPathSpecification& operator=(RelationshipPathSpecification&&);
    bool operator==(RelationshipPathSpecification const& other) const
        {
        return m_steps.size() == other.m_steps.size() && std::equal(m_steps.begin(), m_steps.end(), other.m_steps.begin(),
            [](RelationshipStepSpecification const* lhs, RelationshipStepSpecification const* rhs){return *lhs == *rhs;});
        }
    bvector<RelationshipStepSpecification*> const& GetSteps() const {return m_steps;}
    ECPRESENTATION_EXPORT void AddStep(RelationshipStepSpecification& step);
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepeatableRelationshipStepSpecification : RelationshipStepSpecification
{
    DEFINE_T_SUPER(RelationshipStepSpecification)

private:
    int m_count;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    RepeatableRelationshipStepSpecification() : RelationshipStepSpecification(), m_count(1) {}
    RepeatableRelationshipStepSpecification(RepeatableRelationshipStepSpecification const& other)
        : RelationshipStepSpecification(other), m_count(other.m_count)
        {}
    RepeatableRelationshipStepSpecification(RepeatableRelationshipStepSpecification&& other)
        : RelationshipStepSpecification(std::forward<RelationshipStepSpecification>(other)), m_count(other.m_count)
        {}
    RepeatableRelationshipStepSpecification(Utf8String relationshipClassName, RequiredRelationDirection direction, Utf8String targetClassName = "", int count = 1)
        : RelationshipStepSpecification(relationshipClassName, direction, targetClassName), m_count(count)
        {}
    RepeatableRelationshipStepSpecification& operator=(RepeatableRelationshipStepSpecification const& other)
        {
        RelationshipStepSpecification::operator=(other);
        m_count = other.m_count;
        return *this;
        }
    RepeatableRelationshipStepSpecification& operator=(RepeatableRelationshipStepSpecification&& other)
        {
        RelationshipStepSpecification::operator=(std::forward<RelationshipStepSpecification>(other));
        m_count = std::move(other.m_count);
        return *this;
        }
    bool operator==(RepeatableRelationshipStepSpecification const& other) const
        {
        return RelationshipStepSpecification::operator==(other)
            && m_count == other.m_count;
        }
    int GetCount() const { return m_count; }
    void SetCount(int value) { m_count = value; InvalidateHash(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepeatableRelationshipPathSpecification : NoXmlSupport<PresentationKey>
{
    DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)

private:
    bvector<RepeatableRelationshipStepSpecification*> m_steps;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    RepeatableRelationshipPathSpecification() {}
    RepeatableRelationshipPathSpecification(RepeatableRelationshipStepSpecification& step) { m_steps.push_back(&step); }
    RepeatableRelationshipPathSpecification(bvector<RepeatableRelationshipStepSpecification*> steps) : m_steps(steps) {}
    ECPRESENTATION_EXPORT RepeatableRelationshipPathSpecification(RepeatableRelationshipPathSpecification const&);
    ECPRESENTATION_EXPORT RepeatableRelationshipPathSpecification(RepeatableRelationshipPathSpecification&&);
    ECPRESENTATION_EXPORT ~RepeatableRelationshipPathSpecification();
    ECPRESENTATION_EXPORT RepeatableRelationshipPathSpecification& operator=(RepeatableRelationshipPathSpecification const&);
    ECPRESENTATION_EXPORT RepeatableRelationshipPathSpecification& operator=(RepeatableRelationshipPathSpecification&&);
    bool operator==(RepeatableRelationshipPathSpecification const& other) const
        {
        return m_steps.size() == other.m_steps.size() && std::equal(m_steps.begin(), m_steps.end(), other.m_steps.begin(),
            [](RelationshipStepSpecification const* lhs, RelationshipStepSpecification const* rhs) {return *lhs == *rhs; });
        }
    bvector<RepeatableRelationshipStepSpecification*> const& GetSteps() const { return m_steps; }
    ECPRESENTATION_EXPORT void AddStep(RepeatableRelationshipStepSpecification& step);
    ECPRESENTATION_EXPORT void ClearSteps();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
