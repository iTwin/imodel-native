/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridArcSurface : GridSurface
{
    DEFINE_T_SUPER(GridSurface);

protected:
    explicit GRIDELEMENTS_EXPORT GridArcSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridArcSurface (CreateParams const& params, ISolidPrimitivePtr surface);
    friend struct GridArcSurfaceHandler;

    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const override;
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _Validate () const override;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridArcSurface, GRIDELEMENTS_EXPORT)
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

            //! Creates create parameters for a plan grid arc surface
            //! @param[in] model            Model of the grid that will contain this surface
            //! @param[in] classId          ClassId of the grid that will contain this surface
            //! @param[in] gridAxisId       Element id of the axis that this surface is being created from
            //! @param[in] startElevation   Start (bottom) elevation for the surface
            //! @param[in] endElevation     End (top) elevation for the surface
            CreateParams (Dgn::SpatialLocationModelCR model, Dgn::DgnClassId classId, Dgn::DgnElementId gridAxisId, double startElevation, double endElevation) :
                T_Super (model, classId, gridAxisId), IPlanGridSurface::CreateParams (startElevation, endElevation)
                {}

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in] params           The base element parameters
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
struct EXPORT_VTABLE_ATTRIBUTE PlanCircumferentialGridSurface : PlanGridArcSurface
    {
    DGNELEMENT_DECLARE_MEMBERS(GRIDS_CLASS_PlanCircumferentialGridSurface, PlanGridArcSurface);
    DEFINE_T_SUPER(PlanGridArcSurface);
    public:
        struct CreateParams : T_Super::CreateParams
            {
            DEFINE_T_SUPER(PlanCircumferentialGridSurface::T_Super::CreateParams);
            double m_radius;
            double m_startAngle;
            double m_endAngle;

            //! Creates create parameters for a plan circumferential grid surface
            //! @param[in] model            Model of the grid that will contain this surface
            //! @param[in] gridAxis         Axis that this surface is being created from
            //! @param[in] radius           Starting point on radius for the circumferential grid surface
            //! @param[in] startAngle       Starting angle for the circumferential grid surface
            //! @param[in] endAngle         End angle for the circumferential grid surface
            //! @param[in] startElevation   Start (bottom) elevation for the surface
            //! @param[in] endElevation     End (top) elevation for the surface
            CreateParams(Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, double radius, double startAngle, double endAngle, double startElevation, double endElevation) :
                T_Super::CreateParams(model, QueryClassId(model.GetDgnDb()), gridAxis.GetElementId(), startElevation, endElevation)
                {
                m_radius = radius;
                m_startAngle = startAngle;
                m_endAngle = endAngle;
                }

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in] params           The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
                : T_Super(params)
                {
                m_radius = 0.0;
                m_startAngle = 0.0;
                m_endAngle = 0.0;
                }
            };

    private:
        BE_PROP_NAME(Radius)
        BE_PROP_NAME(StartAngle)
        BE_PROP_NAME(EndAngle)


    protected:
        explicit GRIDELEMENTS_EXPORT PlanCircumferentialGridSurface(CreateParams const& params);
        friend struct PlanCircumferentialGridSurfaceHandler;

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
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(PlanCircumferentialGridSurface, GRIDELEMENTS_EXPORT)

        //! Creates a PlanCircumferentialGridSurface surface
        //! @param[in]  params           params to create PlanCircumferentialGridSurface
        GRIDELEMENTS_EXPORT static  PlanCircumferentialGridSurfacePtr Create(CreateParams const& params);

        //! Creates a PlanCircumferentialGridSurface surface
        //! @param[in]  params           params to create PlanCircumferentialGridSurface
        GRIDELEMENTS_EXPORT static  PlanCircumferentialGridSurfacePtr CreateAndInsert(CreateParams const& params);

        //! Gets radius for this PlanCircumferentialGridSurface
        //! @return radius of this PlanCircumferentialGridSurface
        GRIDELEMENTS_EXPORT double      GetRadius() const { return GetPropertyValueDouble(prop_Radius()); }

        //! Sets radius for this PlanCircumferentialGridSurface
        //! @param[in]  radius   new angle for this PlanCircumferentialGridSurface
        GRIDELEMENTS_EXPORT void        SetRadius(double radius) { SetPropertyValue(prop_Radius(), radius); };

        //! Gets StartAngle of this PlanCircumferentialGridSurface
        //! @return StartAngle of this PlanCircumferentialGridSurface
        GRIDELEMENTS_EXPORT double      GetStartAngle() const { return GetPropertyValueDouble(prop_StartAngle()); }

        //! Sets StartAngle of this PlanCircumferentialGridSurface
        //! @param[in]  staAngle   new StartAngle for this PlanCircumferentialGridSurface
        GRIDELEMENTS_EXPORT void        SetStartAngle(double staAngle) { SetPropertyValue(prop_StartAngle(), staAngle); };

        //! Gets EndAngle of this PlanCircumferentialGridSurface
        //! @return EndAngle of this PlanCircumferentialGridSurface
        GRIDELEMENTS_EXPORT double      GetEndAngle() const { return GetPropertyValueDouble(prop_EndAngle()); }

        //! Sets EndAngle of this PlanCircumferentialGridSurface
        //! @param[in]  endAngle   new EndAngle for this PlanCircumferentialGridSurface
        GRIDELEMENTS_EXPORT void        SetEndAngle(double endAngle) { SetPropertyValue(prop_EndAngle(), endAngle); };
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

            //! Creates create parameters for a sketch arc grid surface
            //! @param[in] model            Model of the grid that will contain this surface
            //! @param[in] gridAxis         Axis that this surface is being created from
            //! @param[in] startElevation   Start (bottom) elevation for the surface
            //! @param[in] endElevation     End (top) elevation for the surface
            //! @param[in] arc              Arc shape of the surface
            CreateParams (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, double startElevation, double endElevation, DEllipse3d arc) :
                T_Super::CreateParams (model, QueryClassId (model.GetDgnDb ()), gridAxis.GetElementId (), startElevation, endElevation)
                {
                m_arc = arc;
                }

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in] params           The base element parameters
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