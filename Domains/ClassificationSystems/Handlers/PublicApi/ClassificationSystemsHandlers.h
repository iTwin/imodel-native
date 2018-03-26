/*--------------------------------------------------------------------------------------+
|
|     $Source: Handlers/PublicApi/ClassificationSystemsHandlers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_EGRESS_NAMESPACE

struct EgressPathHandler : Dgn::dgn_ElementHandler::SpatialLocation, BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::IElementChangedHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(EGRESS_CLASS_EgressPath, EgressPath, EgressPathHandler, Dgn::dgn_ElementHandler::SpatialLocation, EGRESSHANDLERS_EXPORT)

    protected:
    //IElementChangedHandler
    EGRESSHANDLERS_EXPORT virtual void OnElementInserted(Dgn::DgnDbR db, Dgn::DgnElementId elementId) override;
    EGRESSHANDLERS_EXPORT virtual void OnElementUpdated(Dgn::DgnDbR db, Dgn::DgnElementId elementId) override;
    EGRESSHANDLERS_EXPORT virtual void OnElementDeleted(Dgn::DgnDbR db, Dgn::DgnElementId elementId) override;
    public:
    
    EGRESSHANDLERS_EXPORT static BentleyStatus FindShortestPathsForAllSpacesInAFloor(bmap<bpair<Dgn::DgnElementId, Dgn::DgnElementId>, EgressPathPtr>& shortestPaths, bmap<bpair<Dgn::DgnElementId, Dgn::DgnElementId>, EgressPathPtr>& secondaryShortestPaths, Dgn::DgnModelP floorSubModel);
    EGRESSHANDLERS_EXPORT static BentleyStatus UpdateAllShortestPathsInModel(Dgn::DgnModelP model);
    };

END_EGRESS_NAMESPACE