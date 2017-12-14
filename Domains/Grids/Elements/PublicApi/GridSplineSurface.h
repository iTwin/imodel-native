/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridSplineSurface.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridSplineSurface : GridSurface
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridSplineSurface, GridSurface);
    DEFINE_T_SUPER (GridSurface);

protected:
    explicit GRIDELEMENTS_EXPORT GridSplineSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridSplineSurface (CreateParams const& params, ISolidPrimitivePtr surface);
    friend struct GridSplineSurfaceHandler;

    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const override;
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _Validate () const override;
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridSplineSurface, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a gridarc surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  surface       surface geometry
    //! @return                     gridarc surface
    //! Note: Only SolidPrimitives from DgnExtrusionDetail geometry with BSpline or InterpolationCurve passes as valid geometry
    GRIDELEMENTS_EXPORT static GridSplineSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, ISolidPrimitivePtr surface);

    //! Creates a gridarc surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  extDetail       surface geometry
    //! @return                     gridarc surface
    //! Note: Only DgnExtrusionDetail geometry with BSpline or InterpolationCurve passes as valid geometry
    GRIDELEMENTS_EXPORT static GridSplineSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, DgnExtrusionDetail extDetail);
};

//=======================================================================================
//! plan grid spline surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanGridSplineSurface : GridSplineSurface, IPlanGridSurface
    {
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_PlanGridSplineSurface, GridSplineSurface);
    DEFINE_T_SUPER (GridSplineSurface);
    public:

        struct CreateParams : T_Super::CreateParams, IPlanGridSurface::CreateParams
            {
            DEFINE_T_SUPER (GridSplineSurface::CreateParams);

            //! Creates create parameters for orthogonal grid
            //! @param[in] model              model for the PlanCartesianGridSurface
            CreateParams (Dgn::SpatialLocationModelCR model, Dgn::DgnClassId classId, Dgn::DgnElementId gridAxisId, double staElevation, double endElevation) :
                T_Super (model, classId, gridAxisId), IPlanGridSurface::CreateParams (staElevation, endElevation)
                {}

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in]      params   The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
                : T_Super (params), IPlanGridSurface::CreateParams (0.0, 0.0)
                {}
            };
    private:

    protected:
        //! Note: Only DgnExtrusionDetails with LineString, PointString or Line as base curve pass as valid geometry
        explicit GRIDELEMENTS_EXPORT PlanGridSplineSurface (CreateParams const& params);
        explicit GRIDELEMENTS_EXPORT PlanGridSplineSurface (CreateParams const& params, DgnExtrusionDetailCR  surface);

    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (PlanGridSplineSurface, GRIDELEMENTS_EXPORT)

    };

END_GRIDS_NAMESPACE