/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationTypes.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClass::IsEqual(RelatedClass const& other) const
    {
    return m_source == other.m_source
        && m_target == other.m_target
        && m_targetAlias.Equals(other.m_targetAlias)
        && m_relationship == other.m_relationship
        && m_relationshipAlias.Equals(other.m_relationshipAlias)
        && m_isForwardRelationship == other.m_isForwardRelationship;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedClass::operator<(RelatedClass const& other) const
    {
    if (m_source->GetId() < other.m_source->GetId())
        return true;
    if (m_source->GetId() > other.m_source->GetId())
        return false;
    if (m_target < other.m_target)
        return true;
    if (other.m_target < m_target)
        return false;
    if (m_relationship->GetId() < other.m_relationship->GetId())
        return true;
    if (m_relationship->GetId() > other.m_relationship->GetId())
        return false;
    return !m_isForwardRelationship && other.m_isForwardRelationship;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationECPropertyCP RelatedClass::GetNavigationProperty() const
    {
    ECClassCP source = m_isForwardRelationship ? m_source : &m_target.GetClass();
    ECClassCP target = m_isForwardRelationship ? &m_target.GetClass() : m_source;

    ECPropertyIterable sourceIterable = source->GetProperties(true);
    for (ECPropertyCP prop : sourceIterable)
        {
        if (prop->GetIsNavigation()
            && prop->GetAsNavigationProperty()->GetRelationshipClass() == m_relationship
            && ECRelatedInstanceDirection::Forward == prop->GetAsNavigationProperty()->GetDirection())
            {
            return prop->GetAsNavigationProperty();
            }
        }

    ECPropertyIterable targetIterable = target->GetProperties(true);
    for (ECPropertyCP prop : targetIterable)
        {
        if (prop->GetIsNavigation()
            && prop->GetAsNavigationProperty()->GetRelationshipClass() == m_relationship
            && ECRelatedInstanceDirection::Backward == prop->GetAsNavigationProperty()->GetDirection())
            {
            return prop->GetAsNavigationProperty();
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedClassPath& RelatedClassPath::Reverse(Utf8CP resultTargetClassAlias, bool isResultTargetClassPolymorphic)
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

        if (!spec.GetTargetIds().empty())
            {
            if (i > 0)
                {
                RelatedClassR prev = at(i - 1);
                prev.SetTargetIds(spec.GetTargetIds());
                erase(begin() + i, end());
                }
            else
                {
                clear();
                }
            break;
            }

        ECClassCP tmp = spec.GetSourceClass();
        spec.SetIsForwardRelationship(!spec.IsForwardRelationship());
        spec.SetSourceClass(spec.GetTargetClass().GetClass());
        spec.SetTargetClass(SelectClass(*tmp, (i < size() - 1) ? at(i + 1).GetTargetClass().IsSelectPolymorphic() : isResultTargetClassPolymorphic));
        spec.SetTargetClassAlias((i < size() - 1) ? at(i + 1).GetTargetClassAlias() : resultTargetClassAlias);
        if (nullptr == spec.GetTargetClassAlias() || 0 == *spec.GetTargetClassAlias())
            spec.SetTargetClassAlias(Utf8String(spec.GetTargetClass().GetClass().GetName()).ToLower());
        }

    return *this;
    }
