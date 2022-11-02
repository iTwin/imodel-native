/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationTypes.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClass::IsEqual(RelatedClass const& other) const
    {
    return m_source == other.m_source
        && m_target == other.m_target
        && m_relationship == other.m_relationship
        && m_isForwardRelationship == other.m_isForwardRelationship
        && m_targetInstanceFilter == other.m_targetInstanceFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClass::FullComparer::operator()(RelatedClass const& lhs, RelatedClass const& rhs) const
    {
    NUMERIC_LESS_COMPARE(lhs.m_source->GetId(), rhs.m_source->GetId());
    NUMERIC_LESS_COMPARE(lhs.m_target, rhs.m_target);
    NUMERIC_LESS_COMPARE(lhs.m_relationship, rhs.m_relationship);
    NUMERIC_LESS_COMPARE(lhs.m_isForwardRelationship, rhs.m_isForwardRelationship);
    NUMERIC_LESS_COMPARE(lhs.m_isTargetOptional, rhs.m_isTargetOptional);
    STR_LESS_COMPARE(lhs.m_targetInstanceFilter.c_str(), rhs.m_targetInstanceFilter.c_str());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClass::ClassPointersComparer::operator()(RelatedClass const& lhs, RelatedClass const& rhs) const
    {
    NUMERIC_LESS_COMPARE(lhs.m_source->GetId(), rhs.m_source->GetId());
    NUMERIC_LESS_COMPARE(&lhs.m_target.GetClass(), &rhs.m_target.GetClass());
    NUMERIC_LESS_COMPARE(&lhs.m_relationship.GetClass(), &rhs.m_relationship.GetClass());
    NUMERIC_LESS_COMPARE(lhs.m_isForwardRelationship, rhs.m_isForwardRelationship);
    NUMERIC_LESS_COMPARE(lhs.m_isTargetOptional, rhs.m_isTargetOptional);
    STR_LESS_COMPARE(lhs.m_targetInstanceFilter.c_str(), rhs.m_targetInstanceFilter.c_str());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClass::ClassNamesComparer::operator()(RelatedClass const& lhs, RelatedClass const& rhs) const
    {
    NUMERIC_LESS_COMPARE(lhs.m_source->GetId(), rhs.m_source->GetId());
    STR_LESS_COMPARE(lhs.m_target.GetClass().GetFullName(), rhs.m_target.GetClass().GetFullName());
    STR_LESS_COMPARE(lhs.m_relationship.GetClass().GetFullName(), rhs.m_relationship.GetClass().GetFullName());
    NUMERIC_LESS_COMPARE(lhs.m_isForwardRelationship, rhs.m_isForwardRelationship);
    NUMERIC_LESS_COMPARE(lhs.m_isTargetOptional, rhs.m_isTargetOptional);
    STR_LESS_COMPARE(lhs.m_targetInstanceFilter.c_str(), rhs.m_targetInstanceFilter.c_str());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddProperties(bvector<ECPropertyCP>& properties, ECClassCR ecClass)
    {
    for (ECPropertyCP classProperty : ecClass.GetProperties(false))
        properties.push_back(classProperty);
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        AddProperties(properties, *baseClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECPropertyCP> GetProperties(ECClassCR ecClass)
    {
    bvector<ECPropertyCP> properties;
    AddProperties(properties, ecClass);
    return properties;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationECPropertyCP RelatedClass::GetNavigationProperty() const
    {
    if (!m_relationship.IsValid())
        return nullptr;

    ECClassCP source = m_isForwardRelationship ? m_source : &m_target.GetClass();
    ECClassCP target = m_isForwardRelationship ? &m_target.GetClass() : m_source;

    for (ECPropertyCP prop : GetProperties(*source))
        {
        if (prop->GetIsNavigation()
            && prop->GetAsNavigationProperty()->GetRelationshipClass() == &m_relationship.GetClass()
            && ECRelatedInstanceDirection::Forward == prop->GetAsNavigationProperty()->GetDirection())
            {
            return prop->GetAsNavigationProperty();
            }
        }

    for (ECPropertyCP prop : GetProperties(*target))
        {
        if (prop->GetIsNavigation()
            && prop->GetAsNavigationProperty()->GetRelationshipClass() == &m_relationship.GetClass()
            && ECRelatedInstanceDirection::Backward == prop->GetAsNavigationProperty()->GetDirection())
            {
            return prop->GetAsNavigationProperty();
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelatedClassPath::Reverse(Utf8CP resultTargetClassAlias, bool isResultTargetClassPolymorphic)
    {
    // first pass: reverse the order in list
    for (size_t i = 0; i < size() / 2; ++i)
        {
        RelatedClass& lhs = at(i);
        RelatedClass& rhs = at(size() - i - 1);
        RelatedClass tmp = lhs;
        lhs = rhs;
        rhs = tmp;
        }

    // second pass: reverse each spec
    for (size_t i = 0; i < size(); ++i)
        {
        RelatedClass& spec = at(i);

        if (!spec.GetTargetInstanceFilter().empty())
            {
            if (i > 0)
                {
                RelatedClassR prev = at(i - 1);
                prev.SetTargetInstanceFilter(spec.GetTargetInstanceFilter());
                }
            spec.SetTargetInstanceFilter("");
            }

        if (!spec.GetTargetIds().empty())
            {
            if (i == 0)
                {
                clear();
                return ERROR;
                }
            RelatedClassR prev = at(i - 1);
            prev.SetTargetIds(spec.GetTargetIds());
            erase(begin() + i, end());
            return SUCCESS;
            }

        ECClassCP tmp = spec.GetSourceClass();
        spec.SetIsForwardRelationship(!spec.IsForwardRelationship());
        spec.SetSourceClass(spec.GetTargetClass().GetClass());
        if (i < size() - 1)
            spec.SetTargetClass(SelectClassWithExcludes<ECClass>(*tmp, at(i + 1).GetTargetClass().GetAlias(), at(i + 1).GetTargetClass().IsSelectPolymorphic()));
        else
            spec.SetTargetClass(SelectClassWithExcludes<ECClass>(*tmp, resultTargetClassAlias, isResultTargetClassPolymorphic));
        if (spec.GetTargetClass().GetAlias().empty())
            spec.GetTargetClass().SetAlias(Utf8String(spec.GetTargetClass().GetClass().GetName()).ToLower());
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetAllBaseClasses(bset<ECClassCP>& baseClasses, ECClassCR derivedClass, std::function<bool(ECClassCR)> const& filter)
    {
    for (ECClassCP base : derivedClass.GetBaseClasses())
        {
        if (!filter(*base))
            continue;

        baseClasses.insert(base);
        GetAllBaseClasses(baseClasses, *base, filter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassCP FindBaseClassInSet(ECClassCR rhs, bset<ECClassCP> const& lookupClasses)
    {
    for (ECClassCP rhsBaseClass : rhs.GetBaseClasses())
        {
        if (lookupClasses.end() != lookupClasses.find(rhsBaseClass))
            return rhsBaseClass;
        }
    for (ECClassCP rhsBaseClass : rhs.GetBaseClasses())
        {
        ECClassCP foundClass = FindBaseClassInSet(*rhsBaseClass, lookupClasses);
        if (nullptr != foundClass)
            return foundClass;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassCP GetNearestCommonRelationshipEndClass(ECClassCR lhsClass, ECClassCR rhsClass, ECClassCR relationshipEnd)
    {
    bset<ECClassCP> lhsBaseClasses;
    lhsBaseClasses.insert(&lhsClass);
    GetAllBaseClasses(lhsBaseClasses, lhsClass, [&](ECClassCR baseClass){return baseClass.Is(&relationshipEnd);});

    if (lhsBaseClasses.end() != lhsBaseClasses.find(&rhsClass))
        return &rhsClass;

    return FindBaseClassInSet(rhsClass, lhsBaseClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClass::ClassesEqual(RelatedClassCR other) const
    {
    return &GetTargetClass().GetClass() == &other.GetTargetClass().GetClass()
        && GetSourceClass() == other.GetSourceClass()
        && &GetRelationship().GetClass() == &other.GetRelationship().GetClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClass::SourceAndRelationshipEqual(RelatedClassCR other) const
    {
    return GetSourceClass() == other.GetSourceClass()
        && &GetRelationship().GetClass() == &other.GetRelationship().GetClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AreTargetInstanceFiltersEqual(RelatedClassCR lhs, RelatedClassCR rhs)
    {
    if (lhs.GetTargetInstanceFilter().empty() && rhs.GetTargetInstanceFilter().empty())
        return true;

    Utf8String lhsFilter(lhs.GetTargetInstanceFilter());
    lhsFilter.ReplaceAll(lhs.GetTargetClass().GetAlias().c_str(), "this");

    Utf8String rhsFilter(rhs.GetTargetInstanceFilter());
    rhsFilter.ReplaceAll(rhs.GetTargetClass().GetAlias().c_str(), "this");

    return lhsFilter.Equals(rhsFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClass::Is(RelatedClassCR other) const
    {
    if (GetTargetIds() != other.GetTargetIds())
        return false;

    if (!other.GetTargetInstanceFilter().empty() && !AreTargetInstanceFiltersEqual(*this, other))
        return false;

    if (!GetSourceClass()->Is(other.GetSourceClass()))
        return false;

    if (GetRelationship().IsSelectPolymorphic() && other.GetRelationship().IsSelectPolymorphic())
        {
        if (!GetRelationship().GetClass().Is(&other.GetRelationship().GetClass()))
            return false;
        }
    else
        {
        if (&GetRelationship().GetClass() != &other.GetRelationship().GetClass())
            return false;
        }

    if (GetTargetClass().IsSelectPolymorphic() && other.GetTargetClass().IsSelectPolymorphic())
        {
        if (!GetTargetClass().GetClass().Is(&other.GetTargetClass().GetClass()))
            return false;
        }
    else
        {
        if (&GetTargetClass().GetClass() != &other.GetTargetClass().GetClass())
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClassPath::ClassesEqual(RelatedClassPathCR other) const
    {
    if (size() != other.size())
        return false;

    for (size_t i = 0; i < size(); ++i)
        {
        if (!at(i).ClassesEqual(other[i]))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelatedClass::Unify(RelatedClass& result, RelatedClassCR lhs, RelatedClassCR rhs)
    {
    if (lhs.GetTargetIds() != rhs.GetTargetIds() || lhs.GetTargetInstanceFilter() != rhs.GetTargetInstanceFilter())
        return ERROR;

    result = lhs;
    if (lhs.GetSourceClass() != rhs.GetSourceClass())
        {
        ECClassCP baseSource = GetNearestCommonRelationshipEndClass(*lhs.GetSourceClass(), *rhs.GetSourceClass(),
            lhs.IsForwardRelationship() ? *lhs.GetRelationship().GetClass().GetSource().GetAbstractConstraint() : *lhs.GetRelationship().GetClass().GetTarget().GetAbstractConstraint());
        result.SetSourceClass(*baseSource);
        }
    if (lhs.GetTargetClass() != rhs.GetTargetClass())
        {
        ECClassCP baseTarget = GetNearestCommonRelationshipEndClass(lhs.GetTargetClass().GetClass(), rhs.GetTargetClass().GetClass(),
            lhs.IsForwardRelationship() ? *lhs.GetRelationship().GetClass().GetTarget().GetAbstractConstraint() : *lhs.GetRelationship().GetClass().GetSource().GetAbstractConstraint());
        SelectClassWithExcludes<ECClass> targetSelectClass(lhs.GetTargetClass());
        targetSelectClass.SetClass(*baseTarget);
        targetSelectClass.SetIsSelectPolymorphic(true);
        result.SetTargetClass(targetSelectClass);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelatedClassPath::Unify(RelatedClassPath& result, RelatedClassPathCR lhs, RelatedClassPathCR rhs)
    {
    for (size_t i = 0; i < lhs.size() && i < rhs.size(); ++i)
        {
        RelatedClass unified;
        if (SUCCESS != RelatedClass::Unify(unified, lhs[i], rhs[i]))
            {
            result.clear();
            return ERROR;
            }
        result.push_back(unified);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedClassPath RelatedClassPath::Combine(RelatedClassPathCR lhs, RelatedClassPathCR rhs)
    {
    RelatedClassPath combined(lhs);
    for (auto const& rhsItem : rhs)
        combined.push_back(rhsItem);
    return combined;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedClassPath RelatedClassPath::Combine(RelatedClassPath lhs, RelatedClass rhs)
    {
    lhs.push_back(rhs);
    return lhs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClassPath::StartsWith(RelatedClassPathCR sub) const
    {
    if (sub.size() > size())
        return false;

    for (size_t i = 0; i < sub.size(); ++i)
        {
        if (!at(i).Is(sub[i]))
            return false;
        }
    return true;
    }
