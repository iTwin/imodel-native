/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/PublicApi/DimensionHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ConstraintModelMacros.h"
#include "ElementConstrainsElementHandler.h"
#include <DgnPlatform/DgnElementDependency.h>

BEGIN_CONSTRAINTMODEL_NAMESPACE

struct DimensionHandler : ElementConstrainsElementHandler
{
private: 
    DOMAINHANDLER_DECLARE_MEMBERS (CONSTRAINTMODEL_REL_ElementOffsetsElement, DimensionHandler, ElementConstrainsElementHandler, CONSTRAINTMODEL_EXPORT)

protected:
    void _OnRootChanged (Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, Dgn::DgnElementId source, Dgn::DgnElementId target) override;
    void _ProcessDeletedDependency (Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData) override;

public:
    CONSTRAINTMODEL_EXPORT static ECN::ECClassCR GetECClass (Dgn::DgnDbR db) { return *db.Schemas ().GetClass (CONSTRAINTMODEL_SCHEMA_NAME, CONSTRAINTMODEL_REL_ElementOffsetsElement); }
    CONSTRAINTMODEL_EXPORT static BeSQLite::EC::ECInstanceKey Insert (Dgn::DgnDbR db, Dgn::DgnElementId root, Dgn::DgnElementId dependent, int geomid1, int geomid2, DVec3d direction, double offset);
    CONSTRAINTMODEL_EXPORT static bvector<BeSQLite::EC::ECInstanceId> IsDimensioned (Dgn::DgnDbR db, Dgn::DgnElementId elementId);
    CONSTRAINTMODEL_EXPORT static bvector<BeSQLite::EC::ECInstanceId> GetDimensioningRelationshipInstances (Dgn::DgnDbR db, Dgn::DgnElementId elementId, bool elementIsSource = false);
    CONSTRAINTMODEL_EXPORT static bool AdjustOffsetByVector (Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, DVec3d delta, bool adjustSingle = false);


};

END_CONSTRAINTMODEL_NAMESPACE