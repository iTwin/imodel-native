/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridSurfaceCreatesGridCurveHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Grids/gridsApi.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/DgnElementDependency.h>
#include <Geom/CurveVector.h>

BEGIN_GRIDS_NAMESPACE

struct GridSurfaceCreatesGridCurveHandler : Dgn::DgnElementDependencyHandler
{
private: 
    DOMAINHANDLER_DECLARE_MEMBERS (GRIDS_REL_GridSurfaceCreatesGridCurve, GridSurfaceCreatesGridCurveHandler, Dgn::DgnDomain::Handler, GRIDELEMENTS_EXPORT)

protected:
    void _OnRootChanged (Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, Dgn::DgnElementId source, Dgn::DgnElementId target) override;
    void _ProcessDeletedDependency (Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData) override;

    ICurvePrimitivePtr  static ComputeIntersection (GridSurfaceCPtr thisSurface, GridSurfaceCPtr otherSurface);

public:
    //! Returns relationship class
    //! @return a reference to GridSurfaceCreatesGridCurve relationship class
    GRIDELEMENTS_EXPORT static ECN::ECClassCR GetECClass (Dgn::DgnDbR db) { return *db.Schemas ().GetClass (GRIDS_SCHEMA_NAME, GRIDS_REL_GridSurfaceCreatesGridCurve); }
    
    //! Inserts relationship between grid surfaces and created grid curve
    //! @param[in]  db              db to create relationship in
    //! @param[in]  thisSurface     first surface creating grid intersection curve
    //! @param[in]  otherSurface    other surface creating grid intersection curve. This is the base surface in the relationship
    //! @param[in]  targetModel     model to create new grid curve in
    //! @return key to created relationship
    GRIDELEMENTS_EXPORT static BeSQLite::EC::ECInstanceKey Insert (Dgn::DgnDbR db, GridSurfaceCPtr thisSurface, GridSurfaceCPtr otherSurface, Dgn::DgnModelCR targetModel);
};

END_GRIDS_NAMESPACE