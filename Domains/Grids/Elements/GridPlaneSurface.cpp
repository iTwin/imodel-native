
#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridPlanarSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS (PlanGridPlanarSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS (ElevationGridSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS (SketchLineGridSurface)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlanarSurface::GridPlanarSurface
(
CreateParams const& params
) : T_Super(params)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlanarSurface::GridPlanarSurface
(
CreateParams const& params,
CurveVectorPtr  surfaceVector
) : T_Super(params, surfaceVector) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlanarSurface::GridPlanarSurface
(
CreateParams const& params,
ISolidPrimitivePtr surface
) : T_Super(params, surface) 
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
bool GridPlanarSurface::_ValidateGeometry(ISolidPrimitivePtr surface) const
    {
    DgnExtrusionDetail extrDetail;
    if (!surface->TryGetDgnExtrusionDetail(extrDetail))
        return false;

    ICurvePrimitivePtr curve = *(extrDetail.m_baseCurve)->begin();

    if (nullptr == curve->GetLineStringCP() &&
        nullptr == curve->GetPointStringCP() &&
        nullptr == curve->GetLineCP())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlanarSurfacePtr             GridPlanarSurface::Create 
(
Dgn::SpatialLocationModelCR model,
GridAxisCR gridAxis,
CurveVectorPtr  surfaceVector
)
    {
    GridPlanarSurfacePtr surface = new GridPlanarSurface(CreateParamsFromModelAxisClassId (model, gridAxis, QueryClassId(model.GetDgnDb())), surfaceVector);

    if (surface.IsNull() || DgnDbStatus::Success != surface->_Validate())
        return nullptr;
    
    return surface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlanarSurfacePtr             GridPlanarSurface::Create 
(
Dgn::SpatialLocationModelCR model,
GridAxisCR gridAxis,
ISolidPrimitivePtr surface
)
    {
    GridPlanarSurfacePtr gridSurface = new GridPlanarSurface (CreateParamsFromModelAxisClassId (model, gridAxis, QueryClassId(model.GetDgnDb())), surface);

    if (gridSurface.IsNull() || DgnDbStatus::Success != gridSurface->_Validate())
        return nullptr;

    return gridSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPlane3d                        GridPlanarSurface::_GetPlane
(
) const
    {
    GeometryCollection geomData = *ToGeometrySource ();

    if (geomData.begin () == geomData.end ())
        return DPlane3d ();

    ISolidPrimitivePtr solidPrimitive = (*geomData.begin ()).GetGeometryPtr ()->GetAsISolidPrimitive ();
    if (!solidPrimitive.IsValid ())
        {
        CurveVectorPtr curveVector = (*geomData.begin()).GetGeometryPtr()->GetAsCurveVector();
        if (!curveVector.IsValid())
            return DPlane3d();

        curveVector->TransformInPlace((*geomData.begin()).GetGeometryToWorld());

        Transform localToWorld, worldToLocal;
        DRange3d range;
        curveVector->IsPlanar(localToWorld, worldToLocal, range);
        DPlane3d retPlane;
        bsiTransform_getOriginAndVectors(&localToWorld, &retPlane.origin, NULL, NULL, &retPlane.normal);
        return retPlane;
        }

    DgnExtrusionDetail extDetail;
    solidPrimitive->TransformInPlace ((*geomData.begin ()).GetGeometryToWorld ());
    if (!solidPrimitive->TryGetDgnExtrusionDetail (extDetail))
        {
        return DPlane3d ();
        }

    bvector<bvector<DPoint3d>> baseShapePoints;
    extDetail.m_baseCurve->CollectLinearGeometry (baseShapePoints);
    if (baseShapePoints.size () < 1 || baseShapePoints[0].size () < 2)
        {
        return DPlane3d ();
        }

    DPoint3d point3 = DPoint3d::FromSumOf (baseShapePoints[0][0], extDetail.m_extrusionVector);
    return DPlane3d::From3Points (baseShapePoints[0][0], baseShapePoints[0][1], point3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                        GridPlanarSurface::SetCurveVector
(
CurveVectorR newShape
)
    {
    //clean the existing geometry
    GetGeometryStreamR ().Clear ();


    Transform localToWorld, worldToLocal;
    DRange3d range;
    newShape.IsPlanar (localToWorld, worldToLocal, range);
    DPlane3d retPlane;
    bsiTransform_getOriginAndVectors (&localToWorld, &retPlane.origin, NULL, NULL, &retPlane.normal);

    Placement3d newPlacement (retPlane.origin, GetPlacement ().GetAngles ());
    SetPlacement (newPlacement);

    GeometrySource3dP pGeomElem = DgnElement::ToGeometrySource3dP ();

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*pGeomElem);

    builder->Append (newShape, Dgn::GeometryBuilder::CoordSystem::World);
    builder->Finish (*pGeomElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            GridPlanarSurface::GetGeomIdPlane 
(
    int geomId, 
    DPlane3dR planeOut
) const
    {
    if (geomId != 0 && geomId != 1)
        return false;

    planeOut = GetPlane();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            GridPlanarSurface::StretchGeomIdToPlane
(
    int geomId, 
    DPlane3dR targetPlane
)
    {
    BeAssert(!"Not yet implemented");
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridPlanarSurfacePtr GridPlanarSurface::Create
(
Dgn::SpatialLocationModelCR model,
GridAxisCR gridAxis, 
DgnExtrusionDetail extDetail
)
    {
    return GridPlanarSurface::Create(model, gridAxis, ISolidPrimitive::CreateDgnExtrusion(extDetail));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      GridPlanarSurface::_Validate
(
) const
    {
    DgnDbStatus status = T_Super::_Validate ();
    if (status == DgnDbStatus::Success)
        {
        GeometryCollection geomData = *ToGeometrySource ();

        if (geomData.begin () == geomData.end ())
            return DgnDbStatus::ValidationFailed;

        ISolidPrimitivePtr solidPrimitive = (*geomData.begin ()).GetGeometryPtr ()->GetAsISolidPrimitive ();
        if (solidPrimitive.IsValid ())
            return _ValidateGeometry (solidPrimitive) ? DgnDbStatus::Success : DgnDbStatus::ValidationFailed;
        
        CurveVectorPtr curveVector = (*geomData.begin ()).GetGeometryPtr ()->GetAsCurveVector ();
        if (!curveVector.IsValid ())
            return DgnDbStatus::ValidationFailed;

        Transform localToWorld, worldToLocal;
        DRange3d range;
        if (!curveVector->IsPlanar (localToWorld, worldToLocal, range))
            return DgnDbStatus::ValidationFailed;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanGridPlanarSurface::PlanGridPlanarSurface
(
CreateParams const& params,
DgnExtrusionDetailCR extDetail
) : T_Super(params, ISolidPrimitive::CreateDgnExtrusion (extDetail)), IPlanGridSurface(*this, params, params.m_classId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanGridPlanarSurface::PlanGridPlanarSurface
(
CreateParams const& params
) : T_Super(params), IPlanGridSurface(*this, params, params.m_classId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DPlane3d                        PlanGridPlanarSurface::_GetPlane
(
) const
    {
    GeometryCollection geomData = *ToGeometrySource ();

    if (geomData.begin () == geomData.end ())
        return DPlane3d ();

    ISolidPrimitivePtr solidPrimitive = (*geomData.begin ()).GetGeometryPtr ()->GetAsISolidPrimitive ();
    if (!solidPrimitive.IsValid ())
        {
        return DPlane3d ();
        }

    DgnExtrusionDetail extDetail;
    solidPrimitive->TransformInPlace ((*geomData.begin ()).GetGeometryToWorld ());
    if (!solidPrimitive->TryGetDgnExtrusionDetail (extDetail))
        {
        return DPlane3d ();
        }

    bvector<bvector<DPoint3d>> baseShapePoints;
    extDetail.m_baseCurve->CollectLinearGeometry (baseShapePoints);
    if (baseShapePoints.size () != 1 || baseShapePoints[0].size () != 2)
        {
        return DPlane3d ();
        }

    DPoint3d point3 = DPoint3d::FromSumOf (baseShapePoints[0][0], extDetail.m_extrusionVector);
    return DPlane3d::From3Points (baseShapePoints[0][0], baseShapePoints[0][1], point3);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanCartesianGridSurface::PlanCartesianGridSurface
(
CreateParams const& params
) : T_Super(params)
    {
    if (params.m_classId.IsValid ()) // elements created via handler have no classid.
        {
        SetCoordinate (params.m_coordinate);
        SetStartExtent (params.m_startExtent);
        SetEndExtent (params.m_endExtent);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanCartesianGridSurfacePtr             PlanCartesianGridSurface::Create
(
CreateParams const& params
)
    {
    PlanCartesianGridSurfacePtr gridSurface = new PlanCartesianGridSurface (params);

    if (gridSurface.IsNull() || DgnDbStatus::Success != gridSurface->_Validate())
        return nullptr;

    return gridSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanRadialGridSurface::PlanRadialGridSurface
(
CreateParams const& params
) : T_Super(params)
    {
    if (params.m_classId.IsValid ()) // elements created via handler have no classid.
        {
        SetAngle(params.m_angle);
        SetStartRadius(params.m_startRadius);
        SetEndRadius(params.m_endRadius);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanRadialGridSurfacePtr             PlanRadialGridSurface::Create
(
CreateParams const& params
)
    {
    PlanRadialGridSurfacePtr gridSurface = new PlanRadialGridSurface (params);

    if (gridSurface.IsNull() || DgnDbStatus::Success != gridSurface->_Validate())
        return nullptr;

    return gridSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ElevationGridSurface::ElevationGridSurface
(
CreateParams const& params
) : T_Super(params, params.m_surface.IsValid() ? params.m_surface->Clone (Transform::From(0.0, 0.0, params.m_elevation)) : params.m_surface)
    {
    if (params.m_classId.IsValid ()) // elements created via handler have no classid.
        {
        SetElevation (params.m_elevation);
        SetSurface2d (params.m_surface);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ElevationGridSurfacePtr             ElevationGridSurface::Create
(
CreateParams const& params
)
    {
    ElevationGridSurfacePtr surface = new ElevationGridSurface (params);

    if (surface.IsNull() || DgnDbStatus::Success != surface->_Validate())
        return nullptr;
    
    return surface;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus    ElevationGridSurface::_SetPlacement
(
Placement3dCR placement
)
    {
    GridCPtr grid = GetDgnDb ().Elements ().Get<Grid> (GetGridId ());

    Placement3dCR currPlacement = grid->GetPlacement ();

    Transform currTransInv, thatTrans, diffTrans;
    currTransInv.InverseOf(currPlacement.GetTransform ());
    thatTrans = placement.GetTransform ();
    bsiTransform_multiplyTransformTransform (&diffTrans, &currTransInv, &thatTrans);

    DPlane3d diffPlane;
    bsiTransform_getOriginAndVectors (&diffTrans, &diffPlane.origin, NULL, NULL, &diffPlane.normal);

    if (!DoubleOps::AlmostEqualFraction(abs(diffPlane.normal.z), 1.0))
        return Dgn::DgnDbStatus::ValidationFailed;   //if the elevationsurface is rotated from Z axis, fail

    //else recompute the elevation
    SetElevation (diffPlane.origin.z);
    return T_Super::_SetPlacement (placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                            ElevationGridSurface::SetSurface2d
(
CurveVectorPtr surface
)
    {
    if (!surface.IsValid())
        return;
    Transform localToWorld, worldToLocal;
    DRange3d range;
    surface->IsPlanar (localToWorld, worldToLocal, range);
    DPlane3d surfacePlane;
    bsiTransform_getOriginAndVectors (&localToWorld, &surfacePlane.origin, NULL, NULL, &surfacePlane.normal);

    if (!DoubleOps::AlmostEqualFraction (surfacePlane.origin.z, 0.0) ||
        !DoubleOps::AlmostEqualFraction (abs (surfacePlane.normal.z), 0.0)) //must be a zero Z plane
        return;

    IGeometryPtr geometryPtr = IGeometry::Create (surface);

    ECN::ECValue surface2dValue;
    surface2dValue.SetIGeometry (*geometryPtr);

    SetPropertyValue (prop_Surface2d (), surface2dValue);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr                   ElevationGridSurface::GetSurface2d
(
) const
    {
    ECN::ECValue surface2dValue;

    GetPropertyValue (surface2dValue, prop_Surface2d ());

    IGeometryPtr geometryPtr = surface2dValue.GetIGeometry ();
    if (!geometryPtr.IsValid ())
        return nullptr;

    CurveVectorPtr surface = geometryPtr->GetAsCurveVector ();

    return surface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchLineGridSurface::SketchLineGridSurface
(
CreateParams const& params
) : T_Super(params)
    {
    if (params.m_classId.IsValid ()) // elements created via handler have no classid.
        {
        SetBaseLine (params.m_startPoint, params.m_endPoint);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchLineGridSurfacePtr             SketchLineGridSurface::Create
(
CreateParams const& params
)
    {
    SketchLineGridSurfacePtr surface = new SketchLineGridSurface (params);

    if (surface.IsNull() || DgnDbStatus::Success != surface->_Validate())
        return nullptr;
    
    return surface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                            SketchLineGridSurface::SetBaseLine
(
DPoint2d startPoint,
DPoint2d endPoint
)
    {
    DSegment3d segment = DSegment3d::From (startPoint.x, startPoint.y, 0.0,
                                           endPoint.x, endPoint.y, 0.0);
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine (segment);
    IGeometryPtr geometryPtr = IGeometry::Create (line);

    ECN::ECValue line2dValue;
    line2dValue.SetIGeometry (*geometryPtr);

    SetPropertyValue (prop_Line2d (), line2dValue);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   SketchLineGridSurface::GetBaseLine
(
DPoint2dR startPoint,
DPoint2dR endPoint
) const
    {
    ECN::ECValue line2dValue;

    GetPropertyValue (line2dValue, prop_Line2d ());

    IGeometryPtr geometryPtr = line2dValue.GetIGeometry ();
    if (!geometryPtr.IsValid ())
        return BentleyStatus::ERROR;

    ICurvePrimitivePtr curve = geometryPtr->GetAsICurvePrimitive ();
    if (!curve.IsValid ())
        return BentleyStatus::ERROR;
            
    DSegment3dCP pSegment = curve->GetLineCP ();
    if (pSegment == NULL)
        return BentleyStatus::ERROR;

    startPoint.x = pSegment->point[0].x;
    startPoint.y = pSegment->point[0].y;
    endPoint.x = pSegment->point[1].x;
    endPoint.y = pSegment->point[1].y;

    return BentleyStatus::SUCCESS;
    }

END_GRIDS_NAMESPACE

