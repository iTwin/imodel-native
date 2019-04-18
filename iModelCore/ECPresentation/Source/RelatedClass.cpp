/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
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
        && m_isForwardRelationship == other.m_isForwardRelationship
        && m_isPolymorphic == other.m_isPolymorphic;
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
    if (m_target->GetId() < other.m_target->GetId())
        return true;
    if (m_target->GetId() > other.m_target->GetId())
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
    ECClassCP source = m_isForwardRelationship ? m_source : m_target;
    ECClassCP target = m_isForwardRelationship ? m_target : m_source;

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