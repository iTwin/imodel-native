/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnDb.h>
#include <BuildingShared/BuildingSharedMacros.h>
#include <BuildingShared/DgnUtils/BuildingUtils.h>
#include "ForwardDeclarations.h"

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               06/2018
//=======================================================================================
struct GridCurveBundle : Dgn::DriverBundleElement
    {
    DGNELEMENT_DECLARE_MEMBERS(GRIDS_CLASS_GridCurveBundle, Dgn::DriverBundleElement)

    private:
        void UpdateGridCurve() const;
        ICurvePrimitivePtr ComputeIntersection(GridSurfaceCR thisSurface, GridSurfaceCR otherSurface) const;

        static ECN::ECClassId GetCurvesSetRelClassId(Dgn::DgnDbR);
        void SetCurvesSet(GridCurvesSetCR portion);

        BE_PROP_NAME(CurvesSet)

    protected:
        explicit GridCurveBundle(CreateParams params) : T_Super(params) {}

        //Dgn::IDependencyGraphNode
        GRIDELEMENTS_EXPORT virtual void _OnAllInputsHandled(Dgn::ElementDependency::Graph const& graph, Dgn::ElementDependency::Edge const& edge) const override;
        GRIDELEMENTS_EXPORT virtual void _OnBeforeOutputsHandled(Dgn::ElementDependency::Graph const& graph, Dgn::ElementDependency::Edge const& edge) const override;

        GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnDelete() const override;

        friend struct GridCurveBundleHandler;

    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(GridCurveBundle, GRIDELEMENTS_EXPORT)

        GRIDELEMENTS_EXPORT Dgn::DgnElementId GetCurvesSetId() const;
        GRIDELEMENTS_EXPORT GridCurveCPtr GetGridCurve() const;

        BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::ElementIdIterator MakeDrivingSurfaceIterator() const;
        static BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::ElementIdIterator MakeDrivingSurfaceIterator(GridCurveCR curve);

        static GridCurveBundlePtr CreateAndInsert(Dgn::DgnDbR db, GridCurvesSetCR portion, GridSurfaceCR surface1, GridSurfaceCR surface2);
        static Dgn::ElementIterator MakeGridCurveBundleIterator(GridCurvesSetCR portion);
    };

END_GRIDS_NAMESPACE