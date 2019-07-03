/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridSplineSurface.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! An abstract curved grid-surface element that is created from a spline.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridSplineSurface : GridSurface
{
    DEFINE_T_SUPER (GridSurface);

protected:
    explicit GRIDELEMENTS_EXPORT GridSplineSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridSplineSurface (CreateParams const& params, ISolidPrimitivePtr surface);
    friend struct GridSplineSurfaceHandler;

    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const override;
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _Validate () const override;
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridSplineSurface, GRIDELEMENTS_EXPORT)

};

//=======================================================================================
//! An abstract curved and upwards facing grid surface element that is created from a spline.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanGridSplineSurface : GridSplineSurface, IPlanGridSurface
    {
    DEFINE_T_SUPER (GridSplineSurface);
    public:

        struct CreateParams : T_Super::CreateParams, IPlanGridSurface::CreateParams
            {
            DEFINE_T_SUPER (GridSplineSurface::CreateParams);

            //! Creates create parameters for a plan grid spline surface
            //! @param[in] model            Model of the grid that will contain this surface
            //! @param[in] classId          ClassId of the grid that will contain this surface
            //! @param[in] gridAxisId       Element id of the axis that this surface is being created from
            //! @param[in] startElevation   Starting elevation for the surface
            //! @param[in] endElevation     End elevation for the surface
            CreateParams (Dgn::SpatialLocationModelCR model, Dgn::DgnClassId classId, Dgn::DgnElementId gridAxisId, double startElevation, double endElevation) :
                T_Super (model, classId, gridAxisId), IPlanGridSurface::CreateParams (startElevation, endElevation)
                {}

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in] params           The base element parameters
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

//=======================================================================================
//! A curved and upwards facing grid surface element that is created from a spline.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SketchSplineGridSurface : PlanGridSplineSurface
    {
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_SketchSplineGridSurface, PlanGridSplineSurface);
    DEFINE_T_SUPER (PlanGridSplineSurface);
    public:
        struct CreateParams : T_Super::CreateParams
            {
            DEFINE_T_SUPER (SketchSplineGridSurface::T_Super::CreateParams);
            ICurvePrimitivePtr m_splinePrimitive;

            //! Creates create parameters for a sketch spline grid surface
            //! @param[in] model            Model of the grid that will contain this surface
            //! @param[in] gridAxis         Axis that this surface is being created from
            //! @param[in] startElevation   Starting elevation for the surface
            //! @param[in] endElevation     End elevation for the surface
            //! @param[in] splinePrimitive  Spline curve that defined the shape of a surface
            CreateParams (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, double startElevation, double endElevation, ICurvePrimitiveCR splinePrimitive) :
                T_Super::CreateParams (model, QueryClassId (model.GetDgnDb ()), gridAxis.GetElementId (), startElevation, endElevation)
                {
                m_splinePrimitive = splinePrimitive.Clone();
                }

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in] params           The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
                : T_Super (params)
                {
                m_splinePrimitive = nullptr;
                }
            };

    private:
        BE_PROP_NAME (Spline2d)

    protected:
        explicit GRIDELEMENTS_EXPORT SketchSplineGridSurface (CreateParams const& params);
        friend struct SketchSplineGridSurfaceHandler;

        GRIDELEMENTS_EXPORT  Dgn::DgnDbStatus RecomputeGeometryStream();

        //! Called when an element is about to be inserted into the DgnDb.
        //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
        //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
        GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override;

        //! Called when this element is about to be replace its original element in the DgnDb.
        //! @param [in] original the original state of this element.
        //! Subclasses may override this method to control whether their instances are updated.
        //! @return DgnDbStatus::Success to allow the update, otherwise the update will fail with the returned status.
        //! @note If you override this method, you @em must call T_Super::_OnUpdate, forwarding its status.
        GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnUpdate(Dgn::DgnElementCR original) override;
    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (SketchSplineGridSurface, GRIDELEMENTS_EXPORT)

        //! Creates a SketchSplineGridSurface surface
        //! @param[in]  params           params to create SketchSplineGridSurface
        GRIDELEMENTS_EXPORT static  SketchSplineGridSurfacePtr Create (CreateParams const& params);

        //! Gets Spline2d of this SketchSplineGridSurface
        //! @return base spline of this SketchSplineGridSurface
        GRIDELEMENTS_EXPORT ICurvePrimitivePtr  GetBaseSpline () const;

        //! Sets Line2d of this SketchSplineGridSurface
        //! @param[in]   splinePrimitive        base spline of gridSurface in local coordinates, on zero Z plane
        //! @Note: Only splines on zero Z plane pass as valid geometry
        GRIDELEMENTS_EXPORT void                SetBaseSpline (ICurvePrimitivePtr splinePrimitive);
    };

END_GRIDS_NAMESPACE