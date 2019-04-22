/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDbTables.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct LockRequest;

//=======================================================================================
//! Utility to extract Dgn related information from ChangeSet-s
//=======================================================================================
struct DgnChangeSummary : BeSQLite::EC::ChangeSummary
{
private:
    DgnDbR m_dgndb;
    BeSQLite::EC::ECSqlStatementCache m_statementCache;

    void FindChangedRelationshipEndIds(BeSQLite::EC::ECInstanceIdSet& endInstanceIds, Utf8CP relationshipSchemaName, Utf8CP relationshipClassName, ECN::ECRelationshipEnd relationshipEnd);
    void FindUpdatedInstanceIds(BeSQLite::EC::ECInstanceIdSet& updatedInstanceIds, Utf8CP schemaName, Utf8CP className);
    void FindRelatedInstanceIds(DgnElementIdSet& relatedElements, Utf8CP ecsql, BeSQLite::EC::ECInstanceIdSet const& inInstances);

    BentleyStatus ParseClassFullName(Utf8StringR schemaName, Utf8StringR className, Utf8CP classFullName);

public:
    BentleyStatus GetElementsWithAspectUpdates(DgnElementIdSet& elementIds, Utf8CP elementClassFullName, Utf8CP aspectRelationshipClassFullName, Utf8CP aspectClassFullName);

    //! Constructor
    DgnChangeSummary(DgnDbR dgndb) : BeSQLite::EC::ChangeSummary(dgndb), m_dgndb(dgndb), m_statementCache(5) {}

    //! Get elements that have changed.  
    DGNPLATFORM_EXPORT void GetChangedElements(DgnElementIdSet& elementIds, BeSQLite::EC::ChangeSummary::QueryDbOpcode queryOpcode);

    //! Get elements that have updated geometries
    DGNPLATFORM_EXPORT void GetElementsWithGeometryUpdates(DgnElementIdSet& elementIds);

    //! Returns the DgnDb for which this change summary was created
    DgnDbR GetDgnDb() const { return m_dgndb; }

    //! Gets all changed elements between changesets and two db states
    //! This takes into account changes to aspects and goes back to the element that those
    //! aspects relate to
    //! Note: This is expensive, so if this is being used, call it once and cache the results
    //! for each comparison between two versions
    //! @param[in] currentDb the current Db being used by the application
    //! @param[in] targetDb a temporary target Db that is rolled to the target version
    //! @param[in] changesets Vector of changesets used to obtain the changed elements]
    //! @param[out] elementIds Ids of the elements that changed
    //! @param[out] ecclassIds ECClassIds of all elements that changed
    //! @param[out] opcodes DbOpcodes of each element denoting which type of change happened
    DGNPLATFORM_EXPORT static StatusInt    GetChangedElements(DgnDbR currentDb, DgnDbPtr targetDb, bvector<DgnRevisionPtr> const & changesets, bvector<DgnElementId>& elementIds, bvector<ECN::ECClassId>& ecclassIds, bvector<BeSQLite::DbOpcode>& opcodes);

};

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

