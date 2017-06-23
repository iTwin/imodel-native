/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/PublicApi/CoincidenceHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ConstraintModelMacros.h"
#include "ElementConstrainsElementHandler.h"
#include <DgnPlatform/DgnElementDependency.h>

BEGIN_CONSTRAINTMODEL_NAMESPACE

struct CoincidenceHandler : ElementConstrainsElementHandler
{
private: 
    DOMAINHANDLER_DECLARE_MEMBERS (CONSTRAINTMODEL_REL_ElementCoincidesElement, CoincidenceHandler, ElementConstrainsElementHandler, CONSTRAINTMODEL_EXPORT)

protected:
    void _OnRootChanged (Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, Dgn::DgnElementId source, Dgn::DgnElementId target) override;
    void _ProcessDeletedDependency (Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData) override;

public:
    CONSTRAINTMODEL_EXPORT static ECN::ECClassCR GetECClass (Dgn::DgnDbR db) { return *db.Schemas ().GetClass (CONSTRAINTMODEL_SCHEMA_NAME, CONSTRAINTMODEL_REL_ElementCoincidesElement); }
    CONSTRAINTMODEL_EXPORT static BeSQLite::EC::ECInstanceKey Insert (Dgn::DgnDbR db, Dgn::DgnElementId source, Dgn::DgnElementId target, int geomid1, int geomid2);
};

END_CONSTRAINTMODEL_NAMESPACE