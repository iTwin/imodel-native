/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridSurface.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <ConstraintSystem/ConstraintSystemApi.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING

#define ELLIPSE_FROM_SINGLE_POINT_RADIUS 0.1 // 10cm

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridSurface)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSurface::GridSurface
(
CreateParams const& params
) : T_Super(params) 
    {
    if (params.m_classId.IsValid()) // elements created via handler have no classid.
        SetAxisId (params.m_gridAxisId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSurface::GridSurface
(
CreateParams const& params,
ISolidPrimitivePtr  surface
) : T_Super(params) 
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP ();

    DgnExtrusionDetail extrDetail;
    surface->TryGetDgnExtrusionDetail (extrDetail);

    ICurvePrimitivePtr curve = *(extrDetail.m_baseCurve)->begin ();
    DPoint3d originPoint;

    bvector<DPoint3d> const* lineString;
    DEllipse3dCP arc;
    MSBsplineCurveCP spline;
    DSegment3dCP line;
    MSInterpolationCurveCP interpolationCurve;

    if (nullptr != (lineString = curve->GetLineStringCP ()) || nullptr != (lineString = curve->GetPointStringCP ()))
        originPoint = (*lineString)[0];
    else if (nullptr != (line = curve->GetLineCP ()))
        originPoint = line->point[0];
    else if (nullptr != (arc = curve->GetArcCP ()))
        originPoint = arc->center;
    else if (nullptr != (spline = curve->GetBsplineCurveCP ()))
        originPoint = spline->GetPole (0);
    else if (nullptr != (interpolationCurve = curve->GetInterpolationCurveCP ()))
        originPoint = interpolationCurve->startTangent;
    else
        BeAssert (!"Unrecognized DrivingSurface Base Curve");


    //Placement3d newPlacement (originPoint, GetPlacement ().GetAngles ());
    Placement3d newPlacement (DPoint3d::FromZero (), GetPlacement ().GetAngles ());
    SetPlacement (newPlacement);

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*geomElem);

    if (builder->Append (*surface, Dgn::GeometryBuilder::CoordSystem::World))
        {
        if (SUCCESS != builder->Finish (*geomElem))
            BeAssert (!"Failed to create DrivingSurface Geometry");
        }

    SetAxisId (params.m_gridAxisId);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSurface::GridSurface
(
CreateParams const& params,
CurveVectorPtr  surfaceVector
) : T_Super(params) 
    {

    if (params.m_classId.IsValid () && surfaceVector.IsValid()) // elements created via handler have no classid.
        {
        Dgn::GeometrySourceP geomElem = ToGeometrySourceP ();

        //Placement3d newPlacement ((*(*surfaceVector->begin ())->GetLineStringCP ())[0], GetPlacement ().GetAngles ());
        Placement3d newPlacement (DPoint3d::FromZero (), GetPlacement ().GetAngles ());
        SetPlacement (newPlacement);

        Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*geomElem);

        if (surfaceVector->size () == 1 &&
            surfaceVector->at (0)->GetCurvePrimitiveType () == ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_PointString)
            {
            ICurvePrimitivePtr primitive = surfaceVector->at (0);
            bvector<DPoint3d> points = *primitive->GetPointStringCP ();
            DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY (points[0], ELLIPSE_FROM_SINGLE_POINT_RADIUS);
            ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
            surfaceVector = CurveVector::Create (arc, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
            }

        if (builder->Append (*surfaceVector, Dgn::GeometryBuilder::CoordSystem::World))
            {
            if (SUCCESS != builder->Finish (*geomElem))
                BeAssert (!"Failed to create DrivingSurface Geometry");
            }

        SetAxisId (params.m_gridAxisId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSurface::CreateParams       GridSurface::CreateParamsFromModelAxisClassId
(
Dgn::SpatialLocationModelCR model,
GridAxisCR axis,
DgnClassId classId
)
    {
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (model.GetDgnDb().GetDictionaryModel (), GRIDS_CATEGORY_CODE_GridSurface);

    CreateParams createParams (model, classId, axis.GetElementId());

    return createParams;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::RotateXY(double theta)
    {
    Placement3d placement = GetPlacement();
    GeometryUtils::RotatePlacementXY(placement, theta);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::RotateXY(DPoint3d point, double theta)
    {
    Placement3d placement = GetPlacement();
    GeometryUtils::RotatePlacementAroundPointXY(placement, point, theta);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::MoveToPoint(DPoint3d target)
    {
    Placement3d placement = GetPlacement();
    placement.SetOrigin(target);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::TranslateXY(DVec3d translation)
    {
    Placement3d placement = GetPlacement();
    GeometryUtils::TranslatePlacementXY(placement, translation);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::Translate(DVec3d translation)
    {
    Placement3d placement = GetPlacement();
    GeometryUtils::TranslatePlacement(placement, translation);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus GridSurface::SetHeight(double height)
    {
    CurveVectorPtr baseCurve = GetSurfaceVector();
    if (baseCurve.IsNull())
        return BentleyStatus::ERROR;

    DVec3d up = DVec3d::From(0, 0, height);
    DgnExtrusionDetail detail = DgnExtrusionDetail(baseCurve, up, false);
    ISolidPrimitivePtr geometry = ISolidPrimitive::CreateDgnExtrusion(detail);
    if (geometry.IsNull())
        return BentleyStatus::ERROR;
    
    return SetGeometry(geometry);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
void GridSurface::SetElevation(double elevation)
    {
    DPoint3d origin = GetPlacement().GetOrigin();
    DVec3d translation = DVec3d::From(0, 0, elevation - origin.z);

    Translate(translation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus GridSurface::SetBaseCurve(CurveVectorPtr base)
    {
    double existingHeight;
    if (BentleyStatus::ERROR == TryGetHeight(existingHeight))
        return BentleyStatus::ERROR;

    DVec3d up = DVec3d::From(0, 0, existingHeight);
    DgnExtrusionDetail detail = DgnExtrusionDetail(base, up, false);
    ISolidPrimitivePtr geometry = ISolidPrimitive::CreateDgnExtrusion(detail);
    if (geometry.IsNull())
        return BentleyStatus::ERROR;

    return SetGeometry(geometry);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
BentleyStatus GridSurface::SetGeometry(ISolidPrimitivePtr surface)
    {
    if (_ValidateGeometry(surface))
        return _SetGeometry(surface);

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
BentleyStatus GridSurface::_SetGeometry(ISolidPrimitivePtr surface)
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP();
    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*geomElem);

    if (builder->Append(*surface, Dgn::GeometryBuilder::CoordSystem::World))
        {
        if (SUCCESS != builder->Finish(*geomElem))
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      GridSurface::_Validate
(
) const
    {
    if (!GetAxisId ().IsValid ()) //grid axis must be set
        return DgnDbStatus::ValidationFailed;

    return DgnDbStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      GridSurface::_OnInsert
(
)
    {
    DgnDbStatus status = T_Super::_OnInsert ();
    if (status == DgnDbStatus::Success)
        {
        return _Validate ();
        }
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      GridSurface::_OnUpdate
(
    DgnElementCR original
)
    {
    DgnDbStatus status = T_Super::_OnUpdate (original);
    if (status == DgnDbStatus::Success)
        {
        return _Validate ();
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  11/17
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridSurface::_OnDelete() const
    {
    bvector<DgnElementId> curves = MakeCreatedCurvesIterator().BuildIdList<DgnElementId>();

    DgnDbR db = GetDgnDb();

    // Delete relationship with axis and axis itself
    for (DgnElementId curveId : curves)
        {
        db.DeleteLinkTableRelationships(GRIDS_SCHEMA(GRIDS_REL_GridSurfaceCreatesGridCurve), BeSQLite::EC::ECInstanceId(GetElementId()), BeSQLite::EC::ECInstanceId(curveId));
        db.Elements().Delete(curveId);
        }

    return T_Super::_OnDelete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnElementId     GridSurface::GetGridId
(
) const
    {
    GridAxisCPtr axis = GetDgnDb ().Elements ().Get<GridAxis> (GetAxisId ());
    return axis->GetGridId ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GridSurface::GetSurfaceVector
(
) const
    {
    GeometryCollection geomData = *ToGeometrySource ();
    ISolidPrimitivePtr solidPrimitivePtr = (*(geomData.begin())).GetGeometryPtr()->GetAsISolidPrimitive();
    if (!solidPrimitivePtr.IsValid ())
        {
        return nullptr;
        }

    DgnExtrusionDetail extrDetail;
    if (!solidPrimitivePtr->TryGetDgnExtrusionDetail(extrDetail))
        {
        return nullptr;
        }

    CurveVectorPtr curve = extrDetail.m_baseCurve;
    curve->TransformInPlace ((*geomData.begin ()).GetGeometryToWorld ());

    return curve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GridSurface::TryGetHeight(double& height) const
    {
    GeometryCollection geomData = *ToGeometrySource();
    ISolidPrimitivePtr solidPrimitivePtr = (*(geomData.begin())).GetGeometryPtr()->GetAsISolidPrimitive();
    if (!solidPrimitivePtr.IsValid())
        {
        return BentleyStatus::ERROR;
        }

    DgnExtrusionDetail extrDetail;
    if (!solidPrimitivePtr->TryGetDgnExtrusionDetail(extrDetail))
        {
        return BentleyStatus::ERROR;
        }

    height = extrDetail.m_extrusionVector.Magnitude();

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GridSurface::TryGetLength(double& length) const
    {
    CurveVectorPtr base = GetSurfaceVector();
    if (base.IsNull())
        return BentleyStatus::ERROR;

    length = base->Length();
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::ElementIterator GridSurface::MakeCreatedCurvesIterator() const
    {
    Dgn::ElementIterator iterator = GetDgnDb().Elements().MakeIterator(GRIDS_SCHEMA(GRIDS_CLASS_GridCurve), "WHERE ECInstanceId IN"
                                                                                   "(SELECT TargetECInstanceId"
                                                                                   " FROM " GRIDS_SCHEMA(GRIDS_REL_GridSurfaceCreatesGridCurve)
                                                                                   " WHERE SourceECInstanceId = ?)");

    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement())
        {
        pStmnt->BindId(1, GetElementId());
        }
    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IPlanGridSurface::IPlanGridSurface 
(
Dgn::DgnElementR thisElem,
CreateParams const& params,
Dgn::DgnClassId classId
) :m_thisElem(thisElem)
    {
    if (classId.IsValid ()) // elements created via handler have no classid.
        {
        SetStartElevation (params.m_startElevation);
        SetEndElevation (params.m_endElevation);
        }
    }

END_GRIDS_NAMESPACE