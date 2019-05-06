/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once


BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! planar grid surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridPlanarSurface : GridSurface
{
    DEFINE_T_SUPER(GridSurface);

private:

protected:
    explicit GRIDELEMENTS_EXPORT GridPlanarSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridPlanarSurface (CreateParams const& params, CurveVectorPtr surfaceVector);
    explicit GRIDELEMENTS_EXPORT GridPlanarSurface (CreateParams const& params, ISolidPrimitivePtr  surface);
    friend struct GridPlanarSurfaceHandler;

    GRIDELEMENTS_EXPORT virtual DPlane3d _GetPlane () const;

    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const override;

    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _Validate () const override;
public:
    //IConstrainable TODO: remove IConstrainable
    GRIDELEMENTS_EXPORT bool GetGeomIdPlane (int geomId, DPlane3dR planeOut) const;
    GRIDELEMENTS_EXPORT bool StretchGeomIdToPlane (int geomId, DPlane3dR targetPlane);

    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridPlanarSurface, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Getters and setters
    //---------------------------------------------------------------------------------------
    //! gets the plane of this gridplanesurface
    //! @return     plane of this gridplanesurface
    GRIDELEMENTS_EXPORT         DPlane3d            GetPlane () const { return _GetPlane (); };

    //---------------------------------------------------------------------------------------
    // Geometry modification
    //---------------------------------------------------------------------------------------
    //! sets curveVector for this gridPlane
    //! @param[in]  newShape        new curvevector shape for the GridPlanarSurface
    GRIDELEMENTS_EXPORT void    SetCurveVector (CurveVectorR newShape);
};

//=======================================================================================
//! plan grid planar surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanGridPlanarSurface : GridPlanarSurface, IPlanGridSurface
{
    DEFINE_T_SUPER(GridPlanarSurface);
public:

    struct CreateParams : T_Super::CreateParams, IPlanGridSurface::CreateParams
        {
        DEFINE_T_SUPER (GridPlanarSurface::CreateParams);

        //! Creates create parameters for orthogonal grid
        //! @param[in] model              model for the PlanGridPlanarSurface
        CreateParams (Dgn::SpatialLocationModelCR model, Dgn::DgnClassId classId, Dgn::DgnElementId gridAxisId, double staElevation, double endElevation) :
            T_Super (model, classId, gridAxisId), IPlanGridSurface::CreateParams(staElevation, endElevation)
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
    explicit GRIDELEMENTS_EXPORT PlanGridPlanarSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT PlanGridPlanarSurface (CreateParams const& params, DgnExtrusionDetailCR  surface);
    friend struct PlanGridPlanarSurfaceHandler;

    //! gets the plane of this gridplanesurface
    //! @return     plane of this gridplanesurface
    GRIDELEMENTS_EXPORT virtual DPlane3d _GetPlane () const override;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (PlanGridPlanarSurface, GRIDELEMENTS_EXPORT)

};

//=======================================================================================
//! plan grid planar surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanCartesianGridSurface : PlanGridPlanarSurface
    {
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_PlanCartesianGridSurface, PlanGridPlanarSurface);
    DEFINE_T_SUPER (PlanGridPlanarSurface);
    public:
        struct CreateParams : T_Super::CreateParams
            {
            DEFINE_T_SUPER (PlanCartesianGridSurface::T_Super::CreateParams);
            double m_coordinate;
            double m_startExtent;
            double m_endExtent;

            //! Creates create parameters for orthogonal grid
            //! @param[in] model              model for the PlanCartesianGridSurface
            CreateParams (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, double coordinate, double staExtent, double endExtent, double staElevation, double endElevation) :
                T_Super::CreateParams (model, QueryClassId (model.GetDgnDb ()), gridAxis.GetElementId(), staElevation, endElevation)
                {
                m_coordinate = coordinate;
                m_startExtent = staExtent;
                m_endExtent = endExtent;
                }

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in]      params   The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
                : T_Super (params)
                {
                m_coordinate = 0.0;
                m_startExtent = 0.0;
                m_endExtent = 0.0;
                }
            };

    private:
        BE_PROP_NAME (Coordinate)
        BE_PROP_NAME (StartExtent)
        BE_PROP_NAME (EndExtent)

       
    protected:
        explicit GRIDELEMENTS_EXPORT PlanCartesianGridSurface (CreateParams const& params);
        friend struct PlanCartesianGridSurfaceHandler;


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
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (PlanCartesianGridSurface, GRIDELEMENTS_EXPORT)

        //! Creates a PlanCartesianGridSurface surface
        //! @param[in]  params           params to create PlanCartesianGridSurface
        GRIDELEMENTS_EXPORT static  PlanCartesianGridSurfacePtr Create (CreateParams const& params);

        //! Creates and inserts a PlanCartesianGridSurface surface
        //! @param[in]  params           params to create PlanCartesianGridSurface
        GRIDELEMENTS_EXPORT static  PlanCartesianGridSurfacePtr CreateAndInsert(CreateParams const& params);

        //! Gets coordinate along axis of this PlanCartesianGridSurface
        //! @return Coordinate of this PlanCartesianGridSurface
        GRIDELEMENTS_EXPORT double      GetCoordinate () const { return GetPropertyValueDouble (prop_Coordinate ()); }

        //! Sets coordinate along axis of this PlanCartesianGridSurface
        //! @param[in]  coordinate   new Coordinate for this PlanCartesianGridSurface
        GRIDELEMENTS_EXPORT void        SetCoordinate (double coordinate) { SetPropertyValue (prop_Coordinate (), coordinate); };

        //! Gets StartExtent of this PlanCartesianGridSurface
        //! @return StartExtent of this PlanCartesianGridSurface
        GRIDELEMENTS_EXPORT double      GetStartExtent () const { return GetPropertyValueDouble (prop_StartExtent ()); }

        //! Sets StartExtent of this PlanCartesianGridSurface
        //! @param[in]  staExtent   new StartExtent for this PlanCartesianGridSurface
        GRIDELEMENTS_EXPORT void        SetStartExtent (double staExtent) { SetPropertyValue (prop_StartExtent (), staExtent); };

        //! Gets EndExtent of this PlanCartesianGridSurface
        //! @return EndExtent of this PlanCartesianGridSurface
        GRIDELEMENTS_EXPORT double      GetEndExtent () const { return GetPropertyValueDouble (prop_EndExtent ()); }

        //! Sets EndExtent of this PlanCartesianGridSurface
        //! @param[in]  endExtent   new EndExtent for this PlanCartesianGridSurface
        GRIDELEMENTS_EXPORT void        SetEndExtent (double endExtent) { SetPropertyValue (prop_EndExtent (), endExtent); };
    };


//=======================================================================================
//! plan grid planar surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanRadialGridSurface : PlanGridPlanarSurface
    {
    DGNELEMENT_DECLARE_MEMBERS(GRIDS_CLASS_PlanRadialGridSurface, PlanGridPlanarSurface);
    DEFINE_T_SUPER(PlanGridPlanarSurface);
    public:
        struct CreateParams : T_Super::CreateParams
            {
            DEFINE_T_SUPER(PlanRadialGridSurface::T_Super::CreateParams);
            double m_angle;
            double m_startRadius;
            double m_endRadius;

            //! Creates create parameters for orthogonal grid
            //! @param[in] model              model for the PlanCartesianGridSurface
            CreateParams(Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, double angle, double startRadius, double endRadius, double staElevation, double endElevation) :
                T_Super::CreateParams(model, QueryClassId(model.GetDgnDb()), gridAxis.GetElementId(), staElevation, endElevation)
                {
                m_angle = angle;
                m_startRadius = startRadius;
                m_endRadius = endRadius;
                }

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in]      params   The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
                : T_Super(params)
                {
                m_angle = 0.0;
                m_startRadius = 0.0;
                m_endRadius = 0.0;
                }
            };

    private:
        BE_PROP_NAME(Angle)
        BE_PROP_NAME(StartRadius)
        BE_PROP_NAME(EndRadius)


    protected:
        explicit GRIDELEMENTS_EXPORT PlanRadialGridSurface(CreateParams const& params);
        friend struct PlanRadialGridSurfaceHandler;

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
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(PlanRadialGridSurface, GRIDELEMENTS_EXPORT)

        //! Creates a PlanRadialGridSurface surface
        //! @param[in]  params           params to create PlanRadialGridSurface
        GRIDELEMENTS_EXPORT static  PlanRadialGridSurfacePtr Create(CreateParams const& params);

        //! Creates a PlanRadialGridSurface surface
        //! @param[in]  params           params to create PlanRadialGridSurface
        GRIDELEMENTS_EXPORT static  PlanRadialGridSurfacePtr CreateAndInsert(CreateParams const& params);

        //! Gets angle for this PlanRadialGridSurface
        //! @return Coordinate of this PlanRadialGridSurface
        GRIDELEMENTS_EXPORT double      GetAngle() const { return GetPropertyValueDouble(prop_Angle()); }

        //! Sets angle for this PlanRadialGridSurface
        //! @param[in]  angle   new angle for this PlanRadialGridSurface
        GRIDELEMENTS_EXPORT void        SetAngle(double angle) { SetPropertyValue(prop_Angle(), angle); };

        //! Gets StartRadius of this PlanRadialGridSurface
        //! @return StartRadius of this PlanRadialGridSurface
        GRIDELEMENTS_EXPORT double      GetStartRadius() const { return GetPropertyValueDouble(prop_StartRadius()); }

        //! Sets StartRadius of this PlanRadialGridSurface
        //! @param[in]  staRadius   new StartRadius for this PlanRadialGridSurface
        GRIDELEMENTS_EXPORT void        SetStartRadius(double staRadius) { SetPropertyValue(prop_StartRadius(), staRadius); };

        //! Gets EndRadius of this PlanRadialGridSurface
        //! @return EndRadius of this PlanRadialGridSurface
        GRIDELEMENTS_EXPORT double      GetEndRadius() const { return GetPropertyValueDouble(prop_EndRadius()); }

        //! Sets EndRadius of this PlanRadialGridSurface
        //! @param[in]  endRadius   new EndRadius for this PlanRadialGridSurface
        GRIDELEMENTS_EXPORT void        SetEndRadius(double endRadius) { SetPropertyValue(prop_EndRadius(), endRadius); };
    };

//=======================================================================================
//! plan grid planar surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevationGridSurface : 
                                GridPlanarSurface, 
                                BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::IBCSSerializable,
                                BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::IBCSJsonActionPerformer
    {
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_ElevationGridSurface, GridPlanarSurface);
    DEFINE_T_SUPER (GridPlanarSurface);
    public:
        struct CreateParams : T_Super::CreateParams
            {
            DEFINE_T_SUPER (ElevationGridSurface::T_Super::CreateParams);
            double m_elevation;
            CurveVectorPtr m_surface;

            //! Creates create parameters for elevation grid
            //! @param[in] model              model for the PlanCartesianGridSurface
            CreateParams (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, CurveVectorP surface, double elevation) :
                T_Super::CreateParams (model, QueryClassId (model.GetDgnDb ()), gridAxis.GetElementId())
                {
                m_elevation = elevation;
                m_surface = surface;
                }

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in]      params   The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
                : T_Super (params)
                {
                m_elevation = 478.0;
                m_surface = NULL;
                }
            };

    private:
        BE_PROP_NAME (Elevation)
        BE_PROP_NAME (Surface2d)

    protected:
        explicit GRIDELEMENTS_EXPORT ElevationGridSurface (CreateParams const& params);
        friend struct ElevationGridSurfaceHandler;

        GRIDELEMENTS_EXPORT  Dgn::DgnDbStatus RecomputeGeometryStream();

        GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnUpdate(Dgn::DgnElementCR original) override;
        GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override;
        
        //IBCSJsonActionPerformer
        //! performs action based on the json message
        //! @param[in]  actionData JSON object that holds serialized action data
        GRIDELEMENTS_EXPORT virtual void _PerformJsonAction(Json::Value const& actionData) override;
        
        //IBCSSerializable:
        //! Serializes element data to a JSON object
        //! @param[in]  elementData JSON object that will hold serialized data
        GRIDELEMENTS_EXPORT void _SerializeProperties(Json::Value& elementData) const override;

        //! Formats serialized element data in a JSON object
        //! @param[in]  elementData JSON object that holds serialized data
        GRIDELEMENTS_EXPORT void _FormatSerializedProperties(Json::Value& elementData) const override;

        GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _Validate() const override;
    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (ElevationGridSurface, GRIDELEMENTS_EXPORT)
        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a ElevationGridSurface surface
        //! @param[in]  params          parameters for creating ElevationGridSurface
        //! Note: Only planar curve vectors pass as valid geometry
        GRIDELEMENTS_EXPORT static  ElevationGridSurfacePtr Create (CreateParams const& params);

        //! Gets Elevation of this ElevationGridSurface
        //! @return Elevation of this ElevationGridSurface
        GRIDELEMENTS_EXPORT double              GetElevation () const { return GetPropertyValueDouble (prop_Elevation ()); }

        //! Sets Elevation of this ElevationGridSurface
        //! @param[in]  Elevation   new Elevation for this ElevationGridSurface
        GRIDELEMENTS_EXPORT void                SetElevation (double elevation) { SetPropertyValue (prop_Elevation (), elevation); };

        //! Gets Surface2d of this ElevationGridSurface
        //! @return surface of this ElevationGridSurface, as curvevector on zero Z plane
        GRIDELEMENTS_EXPORT CurveVectorPtr      GetSurface2d () const;

        //! Sets Surface2d of this ElevationGridSurface
        //! @param[in]   surface        curvevector of gridSurface in local coordinates, on zero Z plane
        GRIDELEMENTS_EXPORT void                SetSurface2d (CurveVectorPtr surface);
    };

//=======================================================================================
//! plan grid planar surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SketchLineGridSurface : PlanGridPlanarSurface
    {
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_SketchLineGridSurface, PlanGridPlanarSurface);
    DEFINE_T_SUPER (PlanGridPlanarSurface);
    public:
        struct CreateParams : T_Super::CreateParams
            {
            DEFINE_T_SUPER (SketchLineGridSurface::T_Super::CreateParams);
            DPoint2d m_startPoint;
            DPoint2d m_endPoint;

            //! Creates create parameters for orthogonal grid
            //! @param[in] model              model for the PlanCartesianGridSurface
            CreateParams (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, double staElevation, double endElevation, DPoint2d staPoint, DPoint2d endPoint) :
                T_Super::CreateParams (model, QueryClassId (model.GetDgnDb ()), gridAxis.GetElementId (), staElevation, endElevation)
                {
                m_startPoint = staPoint;
                m_endPoint = endPoint;
                }

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in]      params   The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
                : T_Super (params)
                {
                m_startPoint = m_endPoint = DPoint2d::FromZero ();
                }
            };

    private:
        BE_PROP_NAME (Line2d)


    protected:
        explicit GRIDELEMENTS_EXPORT SketchLineGridSurface (CreateParams const& params);
        friend struct SketchLineGridSurfaceHandler;

        GRIDELEMENTS_EXPORT  Dgn::DgnDbStatus RecomputeGeometryStream();

        GRIDELEMENTS_EXPORT BentleyStatus     ApplyTransform(Transform trans);

        GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnUpdate(Dgn::DgnElementCR original) override;
        GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override;

        //! gets the plane of this gridplanesurface
        //! @return     plane of this gridplanesurface
        GRIDELEMENTS_EXPORT virtual DPlane3d _GetPlane() const override;

        //! Rotates grid by given angle in radians
        //! @param[in] theta            angle in radians
        GRIDELEMENTS_EXPORT virtual BentleyStatus _RotateXY(double theta) override;

        //! Rotates grid around point by given angle in radians
        //! @param[in] point            point to rotate around
        //! @param[in] theta            angle in radians
        GRIDELEMENTS_EXPORT virtual BentleyStatus _RotateXY(DPoint3d point, double theta) override;

        //! Translates gridsurface by given vector
        //! @param[in] translation   vector to translate by
        GRIDELEMENTS_EXPORT virtual BentleyStatus _TranslateXY(DVec2d translation) override;

        //! Translates gridsurface by given vector
        //! @param[in] translation   vector to translate by
        GRIDELEMENTS_EXPORT virtual BentleyStatus _Translate(DVec3d translation) override;

    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (SketchLineGridSurface, GRIDELEMENTS_EXPORT)

        //! Creates a SketchLineGridSurface surface
        //! @param[in]  params           params to create SketchLineGridSurface
        GRIDELEMENTS_EXPORT static  SketchLineGridSurfacePtr Create (CreateParams const& params);

        //! Gets Line2d of this SketchLineGridSurface
        //! @param[out]  startPoint     startPoint of gridSurface in local coordinates
        //! @param[out]  endPoint       endPoint of gridSurface in local coordinates
        //! @return baseline of this SketchLineGridSurface
        GRIDELEMENTS_EXPORT BentleyStatus       GetBaseLine (DPoint2dR startPoint, DPoint2dR endPoint) const;

        //! Sets Line2d of this SketchLineGridSurface
        //! @param[in]   startPoint     startPoint of gridSurface in local coordinates
        //! @param[in]   endPoint       endPoint of gridSurface in local coordinates
        GRIDELEMENTS_EXPORT void                SetBaseLine (DPoint2d startPoint, DPoint2d endPoint);
    };

END_GRIDS_NAMESPACE