/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct RelationshipUtils
    {
    private:
        RelationshipUtils() {}

    public:
        BUILDINGSHAREDDGNUTILS_EXPORT static ElementIdIterator MakeTargetIterator(Dgn::DgnDbR db, ECN::ECRelationshipClassCR relationshipClass, Dgn::DgnElementId sourceId);
        BUILDINGSHAREDDGNUTILS_EXPORT static ElementIdIterator MakeSourceIterator(Dgn::DgnDbR db, ECN::ECRelationshipClassCR relationshipClass, Dgn::DgnElementId targetId);

        BUILDINGSHAREDDGNUTILS_EXPORT static BeSQLite::EC::ECInstanceKey InsertRelationship(Dgn::DgnDbR db, ECN::ECRelationshipClassCR relationshipClass, Dgn::DgnElementId sourceId, Dgn::DgnElementId targetId);
        BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::DgnDbStatus DeleteRelationships(Dgn::DgnDbR db, ECN::ECRelationshipClassCR relationshipClass, Dgn::DgnElementId sourceId, Dgn::DgnElementId targetId);
        BUILDINGSHAREDDGNUTILS_EXPORT static bool RelationshipExists(Dgn::DgnDbR db, ECN::ECRelationshipClassCR relationshipClass, Dgn::DgnElementId sourceId, Dgn::DgnElementId targetId);
    };

END_BUILDING_SHARED_NAMESPACE