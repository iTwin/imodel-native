/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Grids/Elements/GridElementsAPI.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

DEFINE_GRIDS_ELEMENT_BASE_METHODS(GridPlanarSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(PlanGridPlanarSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(PlanRadialGridSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(PlanCartesianGridSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(ElevationGridSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(SketchLineGridSurface)


#define BCSSERIALIZABLE_ELEVSURFACE_OwnerId                          "OwnerId"
#define BCSSERIALIZABLE_ELEVSURFACE_Elevation                        "Elevation"
#define BCSSERIALIZABLE_ELEVSURFACE_Area                             "Area"
#define BCSSERIALIZABLE_ELEVSURFACE_AxisId                           "AxisId"
#define BCSSERIALIZABLE_ELEVSURFACE_GridId                           "GridId"

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
        retPlane.origin = localToWorld.Origin();
        retPlane.normal = localToWorld.ColumnZ();
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
    retPlane.origin = localToWorld.Origin();
    retPlane.normal = localToWorld.ColumnZ();

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

    return gridSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanCartesianGridSurfacePtr             PlanCartesianGridSurface::CreateAndInsert
(
CreateParams const& params
)
    {
    PlanCartesianGridSurfacePtr gridSurface = PlanCartesianGridSurface::Create(params);

    if (!gridSurface->Insert().IsValid())
        return nullptr;

    return gridSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                PlanCartesianGridSurface::RecomputeGeometryStream
(
)
    {
    GridAxisCPtr gridAxis = GetDgnDb().Elements().Get<GridAxis>(GetAxisId());
    GridCPtr grid = GetDgnDb().Elements().Get<Grid>(GetGridId());

    double staElev = GetStartElevation();
    double endElev = GetEndElevation();
    double height = endElev - staElev;

    double staExt = GetStartExtent();
    double endExt = GetEndExtent();
    double length = endExt - staExt;

    double coordinate = GetCoordinate();
    bool isYAxis = gridAxis->IsOrthogonalAxisY();

    if (!isYAxis && !gridAxis->IsOrthogonalAxisX()) //should be either on X or Y axis
        return Dgn::DgnDbStatus::ValidationFailed;

    if (DoubleOps::AlmostEqualFraction(height, 0.0))    //should have a height
        return Dgn::DgnDbStatus::ValidationFailed;

    if (DoubleOps::AlmostEqualFraction(length, 0.0))    //should have a length
        return Dgn::DgnDbStatus::ValidationFailed;


    //first, create the surface in local coordinates..
    DPoint3d startPt = DPoint3d::From(staExt, 0.0, staElev);
    DPoint3d endPt = DPoint3d::From(endExt, 0.0, staElev);

    //start from the grid transform
    // unused - Placement3dCR currGridPlacement = grid->GetPlacement();
    SetPlacement(Placement3d()); //set the start local coordinates
    Transform rotation = Transform::FromIdentity();
    Transform coordTrans = Transform::From(0.0, coordinate, 0.0);

    //figure out the rotation
    if (isYAxis)
        {
        rotation = Transform::FromPrincipleAxisRotations(Transform::FromIdentity(), 0.0, 0.0, -msGeomConst_piOver2);
        //also negate the sta/end params
        startPt.x = -startPt.x;
        endPt.x = -endPt.x;
        }

    //now set the geometry
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine(DSegment3d::From(startPt, endPt));
    DgnExtrusionDetail detail = DgnExtrusionDetail(CurveVector::Create(line), DVec3d::From(0, 0, height), false);
    ISolidPrimitivePtr geometryInLocalCoords = ISolidPrimitive::CreateDgnExtrusion(detail);

    if (BentleyStatus::SUCCESS != SetGeometry(geometryInLocalCoords))
        Dgn::DgnDbStatus::WriteError;

    //translate to member position
    Placement3d thisPlacement(GetPlacement());
    thisPlacement.TryApplyTransform(coordTrans);
    thisPlacement.TryApplyTransform(rotation);
    Transform gridTrans = grid->GetPlacementTransform();
    thisPlacement.TryApplyTransform(gridTrans);

    SetPlacement(thisPlacement);

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                PlanCartesianGridSurface::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnUpdate(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                PlanCartesianGridSurface::_OnInsert
(
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnInsert();
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

    return gridSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PlanRadialGridSurfacePtr             PlanRadialGridSurface::CreateAndInsert
(
CreateParams const& params
)
    {
    PlanRadialGridSurfacePtr gridSurface = PlanRadialGridSurface::Create(params);

    if (!gridSurface->Insert().IsValid())
        return nullptr;

    return gridSurface;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                PlanRadialGridSurface::RecomputeGeometryStream
(
)
    {
    GridAxisCPtr gridAxis = GetDgnDb().Elements().Get<GridAxis>(GetAxisId());
    GridCPtr grid = GetDgnDb().Elements().Get<Grid>(GetGridId());

    double staElev = GetStartElevation();
    double endElev = GetEndElevation();
    double height = endElev - staElev;

    double angle = GetAngle();
    double startRadius = GetStartRadius();
    double endRadius = GetEndRadius();
    double length = endRadius - startRadius;

    bool isradialAxis = gridAxis->IsRadialAxis();

    if (!isradialAxis) //must be on RadialAxis
        return Dgn::DgnDbStatus::ValidationFailed;

    if (DoubleOps::AlmostEqualFraction(height, 0.0))    //should have a height
        return Dgn::DgnDbStatus::ValidationFailed;

    if (DoubleOps::AlmostEqualFraction(length, 0.0))    //should have a length
        return Dgn::DgnDbStatus::ValidationFailed;


    //first, create the surface in local coordinates..
    DPoint3d startPt = DPoint3d::From(startRadius, 0.0, staElev);
    DPoint3d endPt = DPoint3d::From(endRadius, 0.0, staElev);

    //start from the grid transform
    // unused - Placement3dCR currGridPlacement = grid->GetPlacement();
    SetPlacement(Placement3d()); //set the start local coordinates
    Transform rotation = Transform::FromPrincipleAxisRotations(Transform::FromIdentity(), 0.0, 0.0, angle);

    //now set the geometry
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine(DSegment3d::From(startPt, endPt));
    DgnExtrusionDetail detail = DgnExtrusionDetail(CurveVector::Create(line), DVec3d::From(0, 0, height), false);
    ISolidPrimitivePtr geometryInLocalCoords = ISolidPrimitive::CreateDgnExtrusion(detail);

    if (BentleyStatus::SUCCESS != SetGeometry(geometryInLocalCoords))
        Dgn::DgnDbStatus::WriteError;

    //translate to member position
    Placement3d thisPlacement(GetPlacement());
    thisPlacement.TryApplyTransform(rotation);
    Transform gridTrans = grid->GetPlacementTransform();
    thisPlacement.TryApplyTransform(gridTrans);

    SetPlacement(thisPlacement);

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                PlanRadialGridSurface::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnUpdate(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                PlanRadialGridSurface::_OnInsert
(
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ElevationGridSurface::ElevationGridSurface
(
CreateParams const& params
) : T_Super(params, params.m_surface.IsValid() ? params.m_surface->Clone (Transform::From(0.0, 0.0, params.m_elevation)) : nullptr)
    {
    if (!params.m_isLoadingElement) // should not set properties on elements created via handler
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  05/2018
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      ElevationGridSurface::_Validate
(
) const
    {
    if (GetSurface2d().IsNull())
        return T_Super::T_Super::_Validate();   //it's fine for ElevationGridSurface to have null surface2d

    return T_Super::_Validate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                ElevationGridSurface::RecomputeGeometryStream
(
)
    {
    CurveVectorPtr shape = GetSurface2d();
    Transform translation = Transform::From(0.0, 0.0, GetElevation());

    GridCPtr grid = GetDgnDb().Elements().Get<Grid>(GetGridId());

    if (grid.IsNull())
        return Dgn::DgnDbStatus::ValidationFailed;  //has no grid??

    Placement3dCR currGridPlacement = grid->GetPlacement();

    Placement3d thisPlacement(currGridPlacement);

    if (!thisPlacement.TryApplyTransform(translation))
        return Dgn::DgnDbStatus::WriteError;

    SetPlacement(thisPlacement);

    Dgn::GeometrySourceP geomElem = ToGeometrySourceP();
    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*geomElem);

    if (shape.IsValid())
        {
        if (!builder->Append(*shape, Dgn::GeometryBuilder::CoordSystem::Local))
            return Dgn::DgnDbStatus::WriteError;

        if (SUCCESS != builder->Finish(*geomElem))
            return Dgn::DgnDbStatus::WriteError;
        }
    else
        {
        GetGeometryStreamR().Clear();
        }

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                ElevationGridSurface::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();

    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnUpdate(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                ElevationGridSurface::_OnInsert
(
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();

    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                            ElevationGridSurface::SetSurface2d
(
CurveVectorPtr surface
)
    {
    if (surface.IsValid())
        {
        Transform localToWorld, worldToLocal;
        DRange3d range;
        surface->IsPlanar(localToWorld, worldToLocal, range);
        DPlane3d surfacePlane;
        surfacePlane.origin = localToWorld.Origin();
        surfacePlane.normal = localToWorld.ColumnZ();

        if (!DoubleOps::AlmostEqualFraction(surfacePlane.origin.z, 0.0) ||
            !DoubleOps::AlmostEqualFraction(abs(surfacePlane.normal.z), 1.0)) //must be a zero Z plane
            return;

        IGeometryPtr geometryPtr = IGeometry::Create(surface);
        ECN::ECValue surface2dValue;
        surface2dValue.SetIGeometry(*geometryPtr);
        SetPropertyValue(prop_Surface2d(), surface2dValue);
        }
    else
        {
        ECN::ECValue surface2dValue(ECN::PrimitiveType::PRIMITIVETYPE_IGeometry);
        surface2dValue.SetToNull();
        SetPropertyValue(prop_Surface2d(), surface2dValue);
        }
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
* @bsimethod                                    Jonas.Valiunas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ElevationGridSurface::_SerializeProperties (Json::Value& elementData) const
    {
    elementData[BCSSERIALIZABLE_ELEMENT_ClassName] = GetElementClass()->GetName();
    elementData[BCSSERIALIZABLE_ELEMENT_SchemaName] = GetElementClass()->GetSchema().GetName();
    elementData[BCSSERIALIZABLE_ELEMENT_ElementId] = GetElementId().ToHexStr();
    elementData[BCSSERIALIZABLE_ELEMENT_Name] = GetUserLabel();
    elementData[BCSSERIALIZABLE_ELEMENT_CodeValue] = GetCode().GetValueUtf8();

    GridCPtr grid = GetDgnDb().Elements().Get<Grid>(GetGridId());
    elementData[BCSSERIALIZABLE_ELEVSURFACE_OwnerId] = grid->GetCode().GetScopeElementId(GetDgnDb()).ToHexStr();
    elementData[BCSSERIALIZABLE_ELEVSURFACE_Elevation] = GetElevation();
    elementData[BCSSERIALIZABLE_ELEVSURFACE_AxisId] = GetAxisId().ToHexStr();
    elementData[BCSSERIALIZABLE_ELEVSURFACE_GridId] = GetGridId().ToHexStr();


    DPoint3d centroid;
    DVec3d   normal;
    double   area = 0.0;
    CurveVectorPtr surf2d = GetSurface2d();
    if (surf2d.IsValid())
        surf2d->CentroidNormalArea(centroid, normal, area);

    elementData[BCSSERIALIZABLE_ELEVSURFACE_Area] = area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ElevationGridSurface::_PerformJsonAction (Json::Value const& actionData)
    {
    Utf8String action = actionData[BCSJSONACTIONPERFORMER_ACTION].asString();
    if (action.Equals(BCSJSONACTIONPERFORMER_ACTION_UPDATE))
        {
        Json::Value elementData = actionData[BCSJSONACTIONPERFORMER_ACTIONPARAMS];
        BeAssert(GetElementId().GetValueUnchecked() == elementData[BCSSERIALIZABLE_ELEMENT_ElementId].asUInt64());

        SetElevation(elementData[BCSSERIALIZABLE_ELEVSURFACE_Elevation].asDouble());
        SetUserLabel(elementData[BCSSERIALIZABLE_ELEMENT_Name].asString().c_str());
        if (!elementData[BCSSERIALIZABLE_ELEMENT_CodeValue].asString().empty())
            {
            DgnCode newCode = DgnCode::From(GetCode().GetCodeSpecId(), GetCode().GetScopeString(), elementData[BCSSERIALIZABLE_ELEMENT_CodeValue].asString());
            SetCode(newCode);
            }

        Update();
        }
    else if (action.Equals(BCSJSONACTIONPERFORMER_ACTION_DELETE))
        {
        Delete();
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ElevationGridSurface::_FormatSerializedProperties (Json::Value& elementData) const
    {
    //do nothing
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
    
    return surface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DPlane3d                        SketchLineGridSurface::_GetPlane
(
) const
    {
    GeometryCollection geomData = *ToGeometrySource ();

    DPoint2d staPt, endPt;
    DPoint3d startPoint, endPoint;
    DVec3d zVec = DVec3d::From(0.0, 0.0, 1.0);
    endPoint.z = startPoint.z = 0.0;
    if (BentleyStatus::SUCCESS != GetBaseLine(staPt, endPt))
        return DPlane3d();

    startPoint.x = staPt.x;
    startPoint.y = staPt.y;
    endPoint.x = endPt.x;
    endPoint.y = endPt.y;

    DPoint3d zPoint = DPoint3d::FromSumOf(startPoint, zVec);

    return DPlane3d::From3Points (startPoint, endPoint, zPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                SketchLineGridSurface::RecomputeGeometryStream
(
)
    {
    double height = GetEndElevation() - GetStartElevation();

    if (DoubleOps::AlmostEqualFraction(height, 0.0))    //should have a height
        return Dgn::DgnDbStatus::ValidationFailed;

    DPoint2d staPt, endPt;
    DPoint3d startPoint, endPoint;
    endPoint.z = startPoint.z = 0.0;
    if (BentleyStatus::SUCCESS != GetBaseLine(staPt, endPt))
        return Dgn::DgnDbStatus::ValidationFailed;

    double length = staPt.Distance(endPt);
    if (DoubleOps::AlmostEqualFraction(length, 0.0))    //should have a length
        return Dgn::DgnDbStatus::ValidationFailed;

    startPoint.x = staPt.x;
    startPoint.y = staPt.y;
    endPoint.x = endPt.x;
    endPoint.y = endPt.y;

    CurveVectorPtr newBase = CurveVector::CreateLinear({ startPoint, endPoint }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);

    Transform translation = Transform::From(0.0, 0.0, GetStartElevation());

    DVec3d up = DVec3d::From(0, 0, height);
    DgnExtrusionDetail detail = DgnExtrusionDetail(newBase->Clone(translation), up, false);
    ISolidPrimitivePtr geometry = ISolidPrimitive::CreateDgnExtrusion(detail);

    GridCPtr grid = GetDgnDb().Elements().Get<Grid>(GetGridId());
    SetPlacement(Placement3d()); //set the start local coordinates
    SetGeometry(geometry);
    Transform gridTrans = grid->GetPlacementTransform();
    Placement3d thisPlacement(GetPlacement());
    thisPlacement.TryApplyTransform(gridTrans);

    SetPlacement(thisPlacement);
    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                SketchLineGridSurface::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnUpdate(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                SketchLineGridSurface::_OnInsert
(
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnInsert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     01/18
//---------------------------------------------------------------------------------------
BentleyStatus SketchLineGridSurface::ApplyTransform(Transform trans)
    {
    DPoint2d staPt, endPt;
    DPoint3d startPoint, endPoint;
    DVec3d zVec = DVec3d::From(0.0, 0.0, 1.0);
    endPoint.z = startPoint.z = 0.0;
    if (BentleyStatus::SUCCESS != GetBaseLine(staPt, endPt))
        return BentleyStatus::ERROR;

    startPoint.x = staPt.x;
    startPoint.y = staPt.y;
    endPoint.x = endPt.x;
    endPoint.y = endPt.y;

    startPoint = DPoint3d::FromProduct(trans, startPoint);
    endPoint = DPoint3d::FromProduct(trans, endPoint);

    staPt.x = startPoint.x;
    staPt.y = startPoint.y;
    endPt.x = endPoint.x;
    endPt.y = endPoint.y;

    SetBaseLine(staPt, endPt);
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     01/18
//---------------------------------------------------------------------------------------
BentleyStatus SketchLineGridSurface::_RotateXY(double theta)
    {
    //Create rotation matrix
    RotMatrix rotationMatrix = RotMatrix::FromAxisAndRotationAngle(2, theta);

    Transform localTransformation = Transform::FromIdentity();
    localTransformation.SetMatrix(rotationMatrix);
    //transform
    return ApplyTransform(localTransformation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     01/18
//---------------------------------------------------------------------------------------
BentleyStatus SketchLineGridSurface::_RotateXY(DPoint3d point, double theta)
    {
    //Create rotation matrix
    RotMatrix rotationMatrix = RotMatrix::FromAxisAndRotationAngle(2, theta);
    Transform localTransformation = Transform::FromMatrixAndFixedPoint(rotationMatrix, point);

    return ApplyTransform(localTransformation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     01/18
//---------------------------------------------------------------------------------------
BentleyStatus SketchLineGridSurface::_TranslateXY(DVec2d translation)
    {
    Placement3d placement = GetPlacement();
    DVec3d vec3d = DVec3d::From(translation.x, translation.y, 0.0);
    return ApplyTransform(Transform::From(vec3d));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     01/18
//---------------------------------------------------------------------------------------
BentleyStatus SketchLineGridSurface::_Translate(DVec3d translation)
    {
    double zChange = translation.z;
    if (!DoubleOps::AlmostEqualFraction(zChange, 0.0))
        {
        SetStartElevation(GetStartElevation() + zChange);
        SetEndElevation(GetEndElevation() + zChange);
        }
    translation.z = 0.0;
    return ApplyTransform(Transform::From(translation));
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

