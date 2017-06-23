/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/GridSurfaceCreatesGridCurveHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../../Domain/PublicApi/GridsMacros.h"
#include "../../Elements/PublicApi/GridSurface.h"
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/DgnElementDependency.h>
#include <Geom/CurveVector.h>

BEGIN_GRIDS_NAMESPACE

struct GridSurfaceCreatesGridCurveHandler : Dgn::DgnElementDependencyHandler
{
private: 
    DOMAINHANDLER_DECLARE_MEMBERS (GRIDS_REL_GridSurfaceCreatesGridCurve, GridSurfaceCreatesGridCurveHandler, Dgn::DgnDomain::Handler, GRIDHANDLERS_EXPORT)

protected:
    void _OnRootChanged (Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, Dgn::DgnElementId source, Dgn::DgnElementId target) override;
    void _ProcessDeletedDependency (Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData) override;

    ICurvePrimitivePtr  static ComputeIntersection (GridSurfaceCPtr thisSurface, GridSurfaceCPtr otherSurface);

public:
    GRIDHANDLERS_EXPORT static ECN::ECClassCR GetECClass (Dgn::DgnDbR db) { return *db.Schemas ().GetClass (GRIDS_SCHEMA_NAME, GRIDS_REL_GridSurfaceCreatesGridCurve); }
    GRIDHANDLERS_EXPORT static BeSQLite::EC::ECInstanceKey Insert (Dgn::DgnDbR db, GridSurfaceCPtr thisSurface, GridSurfaceCPtr otherSurface, Dgn::DgnModelCR targetModel);
};

END_GRIDS_NAMESPACE