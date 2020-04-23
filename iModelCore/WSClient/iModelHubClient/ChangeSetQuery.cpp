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

    Utf8String queryFilter(m_wsQuery.GetFilter());

    if (Utf8String::IsNullOrEmpty(firstChangeSetId.c_str()) || Utf8String::IsNullOrEmpty(secondChangeSetId.c_str()))
        {
        Utf8String notNullChangeSetId = Utf8String::IsNullOrEmpty(firstChangeSetId.c_str()) ? secondChangeSetId : firstChangeSetId;
        AppendFilter(queryFilter, "%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::CumulativeChangeSet,
            ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, notNullChangeSetId.c_str());
        }
    else
        {
        AppendFilter(queryFilter,
            "((%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s'))",
            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, firstChangeSetId.c_str(),
            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, secondChangeSetId.c_str(),
            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, secondChangeSetId.c_str(),
            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, firstChangeSetId.c_str());
        }

    m_wsQuery.SetFilter(queryFilter);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             04/2020
//---------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterChangeSetsAfterId(Utf8StringCR changeSetId)
    {
    if (Utf8String::IsNullOrEmpty(changeSetId.c_str()))
        return false;

    Utf8String queryFilter(m_wsQuery.GetFilter());
    AppendFilter(queryFilter, "%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet,
        ServerSchema::Property::Id, changeSetId.c_str());

    m_wsQuery.SetFilter(queryFilter);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             04/2020
//---------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterChangeSetsAfterVersion(Utf8StringCR versionId)
    {
    if (Utf8String::IsNullOrEmpty(versionId.c_str()))
        return false;

    Utf8String queryFilter(m_wsQuery.GetFilter());
    AppendFilter(queryFilter, "%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version,
        ServerSchema::Property::Id, versionId.c_str());

    m_wsQuery.SetFilter(queryFilter);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             04/2020
//---------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterChangeSetsBetweenVersionAndChangeSet(Utf8StringCR versionId, Utf8StringCR changeSetId)
    {
    Utf8String queryFilter(m_wsQuery.GetFilter());

    if (Utf8String::IsNullOrEmpty(changeSetId.c_str()))
        {
        AppendFilter(queryFilter, "%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::CumulativeChangeSet, 
            ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str());
        }
    else
        {
        AppendFilter(queryFilter, 
            "(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')",
            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),
            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, changeSetId.c_str(),
            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),
            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, changeSetId.c_str());
        }

    m_wsQuery.SetFilter(queryFilter);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             04/2020
//--------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterCumulativeChangeSetsByChangeSetId(Utf8StringCR changeSetId)
    {
    if (Utf8String::IsNullOrEmpty(changeSetId.c_str()))
        return false;

    Utf8String queryFilter(m_wsQuery.GetFilter());
    AppendFilter(queryFilter, "%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet,
        ServerSchema::Property::Id, changeSetId.c_str());

    m_wsQuery.SetFilter(queryFilter);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             04/2020
//--------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterCumulativeChangeSetsByVersionId(Utf8StringCR versionId)
    {
    if (Utf8String::IsNullOrEmpty(versionId.c_str()))
        return false;

    Utf8String queryFilter(m_wsQuery.GetFilter());
    AppendFilter(queryFilter, "%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version,
        ServerSchema::Property::Id, versionId.c_str());

    m_wsQuery.SetFilter(queryFilter);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             04/2020
//--------------------------------------------------------------------------------------
bool ChangeSetQuery::FilterChangeSetsBetweenVersions(Utf8StringCR firstVersionId, Utf8StringCR secondVersionId)
    {
    Utf8String queryFilter(m_wsQuery.GetFilter());
    AppendFilter(queryFilter, 
        "(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')",
        ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, firstVersionId.c_str(),
        ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, secondVersionId.c_str(),
        ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, secondVersionId.c_str(),
        ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, firstVersionId.c_str());

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

    Utf8String queryFilter(m_wsQuery.GetFilter());
    AppendFilter(queryFilter, "$id+in+['%s']", changeSetId.c_str());

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
//@bsimethod                                     Algirdas.Mikoliunas             04/2020
//---------------------------------------------------------------------------------------
void ChangeSetQuery::AppendFilter(Utf8StringR filterString, Utf8CP format, ...)
    {
    if (!filterString.empty())
        filterString += "+and+";

    va_list args;
    va_start(args, format);
    filterString += Utf8PrintfString::CreateFromVaList(format, args);
    va_end(args);
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
//@bsimethod                                     Algirdas.Mikoliunas             11/2019
//---------------------------------------------------------------------------------------
WSQuery ChangeSetQuery::GetWSQuery()
    {
    if (m_wsQuery.GetSelect().Equals("*"))
        m_wsQuery.SetSelect("");

    return m_wsQuery;
    }