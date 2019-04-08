/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridCurveBundleCreatesGridCurveHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               06/2018
//=======================================================================================
struct GridCurveBundleCreatesGridCurveHandler : Dgn::DgnElementDependencyHandler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(GRIDS_REL_GridCurveBundleCreatesGridCurve, GridCurveBundleCreatesGridCurveHandler, Dgn::DgnElementDependencyHandler, GRIDELEMENTS_EXPORT)
    
    private:
        static ECN::ECRelationshipClassCR GetECClass(Dgn::DgnDbCR db);

    protected:
        virtual void _OnRootChanged(Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, Dgn::DgnElementId source, Dgn::DgnElementId target) override;
        virtual void _ProcessDeletedDependency(Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData) override;

    public:
        static BeSQLite::EC::ECInstanceKey Insert(Dgn::DgnDbR db, GridCurveBundleCR source, GridCurveCR target);

        static GridCurveBundleCPtr GetBundle(GridCurveCR target);

        static GridCurveCPtr GetGridCurve(GridCurveBundleCR source);
        static GridCurvePtr GetGridCurveForEdit(GridCurveBundleCR source);
    };

END_GRIDS_NAMESPACE