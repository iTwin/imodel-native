/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/ContentSpecification.h>
#include <ECPresentation/RulesDriven/Rules/RelatedInstanceNodesSpecification.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelationshipStepSpecification : HashableBase
{
private:
    Utf8String m_relationshipClassName;
    RequiredRelationDirection m_direction;
    Utf8String m_targetClassName;
protected:
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;
public:
    RelationshipStepSpecification() : m_direction(RequiredRelationDirection_Forward) {}
    RelationshipStepSpecification(RelationshipStepSpecification const& other)
        : m_relationshipClassName(other.m_relationshipClassName), m_direction(other.m_direction), m_targetClassName(other.m_targetClassName)
        {}
    RelationshipStepSpecification(RelationshipStepSpecification&& other)
        : m_relationshipClassName(std::move(other.m_relationshipClassName)), m_direction(other.m_direction), m_targetClassName(std::move(other.m_targetClassName))
        {}
    RelationshipStepSpecification(Utf8String relationshipClassName, RequiredRelationDirection direction, Utf8String targetClassName = "")
        : m_relationshipClassName(relationshipClassName), m_direction(direction), m_targetClassName(targetClassName)
        {}
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;
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
    Utf8StringCR GetRelationshipClassName() const {return m_relationshipClassName;}
    Utf8StringCR GetTargetClassName() const {return m_targetClassName;}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelationshipPathSpecification : HashableBase
{
private:
    bvector<RelationshipStepSpecification*> m_steps;
protected:
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const;        
public:
    RelationshipPathSpecification() {}
    RelationshipPathSpecification(RelationshipStepSpecification& step) {m_steps.push_back(&step);}
    RelationshipPathSpecification(bvector<RelationshipStepSpecification*> steps) : m_steps(steps) {}
    ECPRESENTATION_EXPORT RelationshipPathSpecification(RelationshipPathSpecification const&);
    ECPRESENTATION_EXPORT RelationshipPathSpecification(RelationshipPathSpecification&&);
    ECPRESENTATION_EXPORT ~RelationshipPathSpecification();
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;
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
* @bsiclass                                     Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepeatableRelationshipStepSpecification : RelationshipStepSpecification
{
private:
    int m_count;
protected:
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;
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
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;
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
    void SetCount(int value) { m_count = value; }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepeatableRelationshipPathSpecification : HashableBase
{
private:
    bvector<RepeatableRelationshipStepSpecification*> m_steps;
protected:
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const;
public:
    RepeatableRelationshipPathSpecification() {}
    RepeatableRelationshipPathSpecification(RepeatableRelationshipStepSpecification& step) { m_steps.push_back(&step); }
    RepeatableRelationshipPathSpecification(bvector<RepeatableRelationshipStepSpecification*> steps) : m_steps(steps) {}
    ECPRESENTATION_EXPORT RepeatableRelationshipPathSpecification(RepeatableRelationshipPathSpecification const&);
    ECPRESENTATION_EXPORT RepeatableRelationshipPathSpecification(RepeatableRelationshipPathSpecification&&);
    ECPRESENTATION_EXPORT ~RepeatableRelationshipPathSpecification();
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;
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
