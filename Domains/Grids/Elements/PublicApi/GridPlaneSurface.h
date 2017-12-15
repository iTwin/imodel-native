/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridPlaneSurface.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include <ConstraintSystem/ConstraintSystemApi.h>
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! planar grid surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridPlanarSurface : GridSurface, ConstraintModel::IConstrainable
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridPlanarSurface, GridSurface);
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
    GRIDELEMENTS_EXPORT virtual bool GetGeomIdPlane (int geomId, DPlane3dR planeOut) const override;
    GRIDELEMENTS_EXPORT virtual bool StretchGeomIdToPlane (int geomId, DPlane3dR targetPlane) override;

    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridPlanarSurface, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a gridplane surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  surfaceVector   surface geometry
    //! @return                     gridplane surface
    //! Note: Only planar curve vectors pass as valid geometry
    GRIDELEMENTS_EXPORT static  GridPlanarSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, CurveVectorPtr surfaceVector);

    //! Creates a gridplane surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  surface         surface geometry
    //! @return                     gridplane surface
    //! @Note: Only solid primitives from DgnExtrusionDetails with LineString, PointString or Line as base curve pass as valid geometry
    GRIDELEMENTS_EXPORT static  GridPlanarSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, ISolidPrimitivePtr  surface);

    //! Creates a gridplane surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  extDetail       surface geometry
    //! @return                     gridplane surface
    //! Note: Only DgnExtrusionDetails with LineString, PointString or Line as base curve pass as valid geometry
    GRIDELEMENTS_EXPORT static  GridPlanarSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, DgnExtrusionDetail  extDetail);

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
        //! @param[in] model              model for the PlanCartesianGridSurface
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

    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (PlanCartesianGridSurface, GRIDELEMENTS_EXPORT)

        //! Creates a PlanCartesianGridSurface surface
        //! @param[in]  params           params to create PlanCartesianGridSurface
        GRIDELEMENTS_EXPORT static  PlanCartesianGridSurfacePtr Create (CreateParams const& params);

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
struct EXPORT_VTABLE_ATTRIBUTE ElevationGridSurface : GridPlanarSurface
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
            CreateParams (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, CurveVectorR surface, double elevation) :
                T_Super::CreateParams (model, QueryClassId (model.GetDgnDb ()), gridAxis.GetElementId())
                {
                m_elevation = elevation;
                m_surface = &surface;
                }

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in]      params   The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
                : T_Super (params)
                {
                }
            };

    private:
        BE_PROP_NAME (Elevation)
        BE_PROP_NAME (Surface2d)

    protected:
        explicit GRIDELEMENTS_EXPORT ElevationGridSurface (CreateParams const& params);
        friend struct ElevationGridSurfaceHandler;

        GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _SetPlacement (Dgn::Placement3dCR placement) override;
        
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