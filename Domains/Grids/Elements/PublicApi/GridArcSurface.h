/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridArcSurface.h $
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

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridArcSurface)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridArcSurface : GridSurface
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridArcSurface, GridSurface);

protected:
    explicit GRIDELEMENTS_EXPORT GridArcSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridArcSurface (CreateParams const& params, ISolidPrimitivePtr surface);
    friend struct GridArcSurfaceHandler;

    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const override;
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _Validate () const override;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridArcSurface, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a gridarc surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  extDetail       surface geometry
    //! @return                     gridarc surface
    //! Note: Only solid primitives with DgnExtrusionDetails with arcs as base curves pass as valid geometry
    GRIDELEMENTS_EXPORT static GridArcSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, ISolidPrimitivePtr surface);

    //! Creates a gridarc surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  extDetail       surface geometry
    //! @return                     gridarc surface
    //! Note: Only DgnExtrusionDetails with arcs as base curves pass as valid geometry
    GRIDELEMENTS_EXPORT static GridArcSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, DgnExtrusionDetail extDetail);
};


//=======================================================================================
//! plan grid arc surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanGridArcSurface : GridArcSurface, IPlanGridSurface
    {
    DEFINE_T_SUPER (GridArcSurface);
    public:

        struct CreateParams : T_Super::CreateParams, IPlanGridSurface::CreateParams
            {
            DEFINE_T_SUPER (GridArcSurface::CreateParams);

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
        explicit GRIDELEMENTS_EXPORT PlanGridArcSurface (CreateParams const& params);
        explicit GRIDELEMENTS_EXPORT PlanGridArcSurface (CreateParams const& params, DgnExtrusionDetailCR  surface);

    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (PlanGridArcSurface, GRIDELEMENTS_EXPORT)

    };

//=======================================================================================
//! plan grid planar surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SketchArcGridSurface : PlanGridArcSurface
    {
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_SketchArcGridSurface, PlanGridArcSurface);
    DEFINE_T_SUPER (PlanGridArcSurface);
    public:
        struct CreateParams : T_Super::CreateParams
            {
            DEFINE_T_SUPER (SketchArcGridSurface::T_Super::CreateParams);
            DEllipse3d m_arc;

            //! Creates create parameters for orthogonal grid
            //! @param[in] model              model for the PlanCartesianGridSurface
            CreateParams (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, double staElevation, double endElevation, DEllipse3d arc) :
                T_Super::CreateParams (model, QueryClassId (model.GetDgnDb ()), gridAxis.GetElementId (), staElevation, endElevation)
                {
                m_arc = arc;
                }

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in]      params   The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
                : T_Super (params)
                {
                DPoint3d center = DPoint3d::FromZero();
                m_arc = DEllipse3d::FromCenterRadiusXY(center, 0);
                }
            };

    private:
        BE_PROP_NAME (Arc2d)


    protected:
        explicit GRIDELEMENTS_EXPORT SketchArcGridSurface (CreateParams const& params);
        friend struct SketchArcGridSurfaceHandler;

    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (SketchArcGridSurface, GRIDELEMENTS_EXPORT)

        //! Creates a SketchArcGridSurface surface
        //! @param[in]  params           params to create SketchArcGridSurface
        GRIDELEMENTS_EXPORT static  SketchArcGridSurfacePtr Create (CreateParams const& params);

        //! Gets Arc2d of this SketchArcGridSurface
        //! @param[out]  arc        base arc of gridSurface in local coordinates, on zero Z plane
        //! @return base arc of this SketchArcGridSurface
        GRIDELEMENTS_EXPORT BentleyStatus       GetBaseArc (DEllipse3dR arc) const;

        //! Sets Line2d of this SketchArcGridSurface
        //! @param[in]   arc        base arc of gridSurface in local coordinates, on zero Z plane
        //! @Note: Only arcs on zero Z plane pass as valid geometry
        GRIDELEMENTS_EXPORT void                SetBaseArc (DEllipse3d arc);
    };

END_GRIDS_NAMESPACE