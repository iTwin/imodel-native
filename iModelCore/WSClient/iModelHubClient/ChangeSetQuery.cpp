/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/ChangeSetQuery.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
ChangeSetQuery::ChangeSetQuery() : m_wsQuery(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet)
    {
    m_wsQuery.SetSelect("*");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterChangeSetsBetween(Utf8StringCR firstChangeSetId, Utf8StringCR secondChangeSetId)
    {
    if (Utf8String::IsNullOrEmpty(firstChangeSetId.c_str()) && Utf8String::IsNullOrEmpty(secondChangeSetId.c_str()))
        return false;

    Utf8String queryFilter;
    Utf8StringCR existingFilter = m_wsQuery.GetFilter();

    if (!existingFilter.empty())
        queryFilter.Sprintf("%s+and+", existingFilter.c_str());

    if (Utf8String::IsNullOrEmpty(firstChangeSetId.c_str()) || Utf8String::IsNullOrEmpty(secondChangeSetId.c_str()))
        {
        Utf8String notNullChangeSetId = Utf8String::IsNullOrEmpty(firstChangeSetId.c_str()) ? secondChangeSetId : firstChangeSetId;
        queryFilter.Sprintf("%s%s-backward-%s.%s+eq+'%s'", queryFilter.c_str(),
            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id,
            notNullChangeSetId.c_str());
        }
    else
        {
        queryFilter.Sprintf
        ("%s((%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s'))", queryFilter.c_str(),
            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, firstChangeSetId.c_str(),
            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, secondChangeSetId.c_str(),
            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, secondChangeSetId.c_str(),
            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, firstChangeSetId.c_str());
        }

    m_wsQuery.SetFilter(queryFilter);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterChangeSetsAfterId(Utf8StringCR changeSetId)
    {
    if (Utf8String::IsNullOrEmpty(changeSetId.c_str()))
        return false;

    Utf8String queryFilter;
    Utf8StringCR existingFilter = m_wsQuery.GetFilter();

    if (!existingFilter.empty())
        queryFilter.Sprintf("%s+and+", existingFilter.c_str());

    queryFilter.Sprintf("%s%s-backward-%s.%s+eq+'%s'", queryFilter.c_str(), ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet,
        ServerSchema::Property::Id, changeSetId.c_str());

    m_wsQuery.SetFilter(queryFilter);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterById(Utf8StringCR changeSetId)
    {
    if (Utf8String::IsNullOrEmpty(changeSetId.c_str()))
        return false;

    Utf8String queryFilter;
    Utf8StringCR existingFilter = m_wsQuery.GetFilter();

    if (!existingFilter.empty())
        queryFilter.Sprintf("%s+and+", existingFilter.c_str());

    queryFilter.Sprintf("%s$id+in+['%s']", queryFilter.c_str(), changeSetId.c_str());

    m_wsQuery.SetFilter(queryFilter);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             11/2019
//---------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterByIds(std::deque<ObjectId>& changeSetIds)
    {
    BeAssert(0 != changeSetIds.size() && "Query Ids in empty array is not supported.");
    if (0 == changeSetIds.size())
        return false;

    m_wsQuery.AddFilterIdsIn(changeSetIds, nullptr, 0, 0);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
void ChangeSetQuery::SelectBridgeProperties()
    {
    Utf8String select;
    Utf8StringCR existingSelect = m_wsQuery.GetSelect();
    select.Sprintf("%s,%s-%s-%s.*", existingSelect.c_str(), ServerSchema::Relationship::HasBridgeProperties, ServerSchema::RelationshipDirection::Forward, ServerSchema::Class::BridgeProperties);
    m_wsQuery.SetSelect(select);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
void ChangeSetQuery::SelectDownloadAccessKey()
    {
    Utf8String selectString(m_wsQuery.GetSelect());
    FileAccessKey::AddDownloadAccessKeySelect(selectString);
    m_wsQuery.SetSelect(selectString);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterBySeedFileId(BeSQLite::BeGuidCR seedFileId)
    {
    if (!seedFileId.IsValid())
        return false;

    Utf8String queryFilter;
    Utf8StringCR existingFilter = m_wsQuery.GetFilter();

    if (!existingFilter.empty())
        queryFilter.Sprintf("%s+and+", existingFilter.c_str());

    queryFilter.Sprintf("%s%s+eq+'%s'", queryFilter.c_str(), ServerSchema::Property::SeedFileId, seedFileId.ToString().c_str());
    m_wsQuery.SetFilter(queryFilter);

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             11/2019
//---------------------------------------------------------------------------------------
WSQuery ChangeSetQuery::GetWSQuery()
    {
    if (m_wsQuery.GetSelect().Equals("*"))
        m_wsQuery.SetSelect("");

    return m_wsQuery;
    }