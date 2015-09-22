/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DgnChangeSummary.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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
    BentleyStatus GetElementsWithAspectUpdates(DgnElementIdSet& elementIds, Utf8CP elementClassFullName, Utf8CP aspectRelationshipClassFullName, Utf8CP aspectClassFullName);

public:
    //! Constructor
    DgnChangeSummary(DgnDbR dgndb) : BeSQLite::EC::ChangeSummary(dgndb), m_dgndb(dgndb), m_statementCache(5) {}

    //! Get elements that have changed.  
    DGNPLATFORM_EXPORT void GetChangedElements(DgnElementIdSet& elementIds, BeSQLite::EC::ChangeSummary::QueryDbOpcode queryOpcode);

    //! Get elements that have updated geometries
    DGNPLATFORM_EXPORT void GetElementsWithGeometryUpdates(DgnElementIdSet& elementIds);

    //! Get elements that have updated items
    DGNPLATFORM_EXPORT void GetElementsWithItemUpdates(DgnElementIdSet& elementIds);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

