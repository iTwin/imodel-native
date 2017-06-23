/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/PublicApi/ElementConstrainsElementHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ConstraintModelMacros.h"
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/DgnElementDependency.h>

BEGIN_CONSTRAINTMODEL_NAMESPACE

struct ElementConstrainsElementHandler : Dgn::DgnElementDependencyHandler
{
private: 
    DOMAINHANDLER_DECLARE_MEMBERS (CONSTRAINTMODEL_REL_ElementConstrainsElement, ElementConstrainsElementHandler, Dgn::DgnDomain::Handler, CONSTRAINTMODEL_EXPORT)

protected:
    void _OnRootChanged (Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, Dgn::DgnElementId source, Dgn::DgnElementId target) override;
    void _ProcessDeletedDependency (Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData) override;

public:
    CONSTRAINTMODEL_EXPORT static ECN::ECClassCR GetECClass (Dgn::DgnDbR db) { return *db.Schemas ().GetClass (CONSTRAINTMODEL_SCHEMA_NAME, CONSTRAINTMODEL_REL_ElementConstrainsElement); }
    CONSTRAINTMODEL_EXPORT static BeSQLite::EC::ECInstanceKey Insert (Dgn::DgnDbR db, Dgn::DgnElementId root, Dgn::DgnElementId dependent, int geomid1, int geomid2);
    CONSTRAINTMODEL_EXPORT static bool IsConstrained (Dgn::DgnDbR db, Dgn::DgnElementId elementId);
};

END_CONSTRAINTMODEL_NAMESPACE