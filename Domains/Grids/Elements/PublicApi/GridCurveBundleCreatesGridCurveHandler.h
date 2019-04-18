/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               06/2018
//=======================================================================================
struct GridCurveBundleCreatesGridCurveHandler : Dgn::ElementDependency::Handler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(GRIDS_REL_GridCurveBundleCreatesGridCurve, GridCurveBundleCreatesGridCurveHandler, Dgn::ElementDependency::Handler, GRIDELEMENTS_EXPORT)
    
    private:
        static ECN::ECRelationshipClassCR GetECClass(Dgn::DgnDbCR db);

    protected:
        virtual void _OnRootChanged(Dgn::ElementDependency::Graph const& graph, Dgn::ElementDependency::Edge const& edge) override;
        virtual void _OnDeletedDependency(Dgn::ElementDependency::Graph const& graph, Dgn::ElementDependency::Edge const& edge) override;

    public:
        static BeSQLite::EC::ECInstanceKey Insert(Dgn::DgnDbR db, GridCurveBundleCR source, GridCurveCR target);

        static GridCurveBundleCPtr GetBundle(GridCurveCR target);

        static GridCurveCPtr GetGridCurve(GridCurveBundleCR source);
        static GridCurvePtr GetGridCurveForEdit(GridCurveBundleCR source);
    };

END_GRIDS_NAMESPACE