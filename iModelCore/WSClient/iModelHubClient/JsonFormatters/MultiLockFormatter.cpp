/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "MultiLockFormatter.h"
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

# define LOCKS_GROUPS_COUNT 15

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Json::Value CreateLockInstanceJson
(
bvector<BeInt64Id> const& ids,
BeBriefcaseId            briefcaseId,
BeGuidCR                 seedFileId,
Utf8StringCR             releasedWithChangeSetId,
LockableType             type,
LockLevel                level,
bool                     queryOnly
)
    {
    Json::Value properties;

    properties[ServerSchema::Property::BriefcaseId]           = briefcaseId.GetValue();
    properties[ServerSchema::Property::SeedFileId]            = seedFileId.ToString();
    properties[ServerSchema::Property::ReleasedWithChangeSet] = releasedWithChangeSetId;
    properties[ServerSchema::Property::QueryOnly]             = queryOnly;
    RepositoryJson::LockableTypeToJson(properties[ServerSchema::Property::LockType], type);
    RepositoryJson::LockLevelToJson(properties[ServerSchema::Property::LockLevel], level);

    properties[ServerSchema::Property::ObjectIds] = Json::arrayValue;
    int i = 0;
    for (auto const& id : ids)
        {
        properties[ServerSchema::Property::ObjectIds][i++] = FormatBeInt64Id(id);
        }

    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
void AddToInstance
(
WSChangeset&                     changeset,
WSChangeset::ChangeState const&  changeState,
bvector<BeInt64Id> const&        ids,
BeBriefcaseId                    briefcaseId,
BeGuidCR                         seedFileId,
Utf8StringCR                     releasedWithChangeSetId,
LockableType                     type,
LockLevel                        level,
bool                             queryOnly
)
    {
    if (ids.empty())
        return;
    ObjectId lockObject(ServerSchema::Schema::iModel, ServerSchema::Class::MultiLock, "MultiLock");
    auto json = std::make_shared<Json::Value>(CreateLockInstanceJson(ids, briefcaseId, seedFileId, releasedWithChangeSetId, type, level, queryOnly));
    changeset.AddInstance(lockObject, changeState, json);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             03/2018
//---------------------------------------------------------------------------------------
void ClearBuffer(bvector<BeInt64Id> objects[])
    {
    for (int i = 0; i < LOCKS_GROUPS_COUNT; i++)
        {
        objects[i].clear();
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             03/2018
//---------------------------------------------------------------------------------------
void FlushLocksToChangeset
(
std::shared_ptr<WSChangeset>    changeset,
const WSChangeset::ChangeState& changeState,
bvector<BeInt64Id>              objects[],
BeBriefcaseId                   briefcaseId,
BeGuidCR                        seedFileId,
Utf8StringCR                    releasedWithChangeSetId,
bool                            queryOnly
)
    {
    for (int i = 0; i < LOCKS_GROUPS_COUNT; ++i)
        AddToInstance(*changeset, changeState, objects[i], briefcaseId, seedFileId, releasedWithChangeSetId, static_cast<LockableType>(i / 3),
            static_cast<LockLevel>(i % 3), queryOnly);

    ClearBuffer(objects);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             03/2018
//---------------------------------------------------------------------------------------
void AddLockToBuffer(bvector<BeInt64Id> objects[], DgnLock lock)
    {
    int index = static_cast<int32_t>(lock.GetType()) * 3 + static_cast<int32_t>(lock.GetLevel());
    if (index >= 0 && index < LOCKS_GROUPS_COUNT)
        objects[index].push_back(lock.GetId());
    else
        BeAssert(false && "Lock index out of bounds! New lock type might have been added.");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             03/2018
//---------------------------------------------------------------------------------------
void MultiLockFormatter::SetToChunkedChangeset
(
const Dgn::DgnLockSet&          locks, 
BeSQLite::BeBriefcaseId         briefcaseId, 
BeSQLite::BeGuidCR              seedFileId, 
Utf8StringCR                    releasedWithChangeSetId, 
ChunkedWSChangeset&             chunkedChangeset, 
const WSChangeset::ChangeState& changeState, 
bool                            includeOnlyExclusive, 
bool                            queryOnly
)
    {
    bvector<BeInt64Id> objects[LOCKS_GROUPS_COUNT];

    for (auto& lock : locks)
        {
        if (includeOnlyExclusive && LockLevel::Exclusive != lock.GetLevel())
            continue;

        if (!chunkedChangeset.AddInstance())
            {
            FlushLocksToChangeset(chunkedChangeset.GetCurrentChangeset(), changeState, objects, briefcaseId, seedFileId, releasedWithChangeSetId, queryOnly);
            chunkedChangeset.StartNewChangeset();
            }

        AddLockToBuffer(objects, lock);
        }

    FlushLocksToChangeset(chunkedChangeset.GetCurrentChangeset(), changeState, objects, briefcaseId, seedFileId, releasedWithChangeSetId, queryOnly);
    }
