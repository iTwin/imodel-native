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

#define ELLIPSE_FROM_SINGLE_POINT_RADIUS 0.1 // 10cm

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (DrivingSurface)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DrivingSurface::DrivingSurface
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DrivingSurface::DrivingSurface
(
CreateParams const& params,
CurveVectorPtr  surfaceVector
) : T_Super(params) 
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP ();

    //Placement3d newPlacement ((*(*surfaceVector->begin ())->GetLineStringCP ())[0], GetPlacement ().GetAngles ());
    Placement3d newPlacement(DPoint3d::FromZero(), GetPlacement().GetAngles());
    SetPlacement (newPlacement);

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*geomElem);

    if (surfaceVector->size() == 1 &&
        surfaceVector->at(0)->GetCurvePrimitiveType() == ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_PointString)
        {
        ICurvePrimitivePtr primitive = surfaceVector->at(0);
        bvector<DPoint3d> points = *primitive->GetPointStringCP();
        DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY(points[0], ELLIPSE_FROM_SINGLE_POINT_RADIUS);
        ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc(ellipse);
        surfaceVector = CurveVector::Create(arc, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
        }

    if (builder->Append (*surfaceVector, Dgn::GeometryBuilder::CoordSystem::World))
        {
        if (SUCCESS != builder->Finish (*geomElem))
            BeAssert (!"Failed to create DrivingSurface Geometry");
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DrivingSurface::DrivingSurface
(
CreateParams const& params,
ISolidPrimitivePtr  surface
) : T_Super(params) 
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP ();

    DgnExtrusionDetail extrDetail;
    surface->TryGetDgnExtrusionDetail (extrDetail);

    ICurvePrimitivePtr curve = *(extrDetail.m_baseCurve)->begin();
    DPoint3d originPoint;

    bvector<DPoint3d> const* lineString;
    DEllipse3dCP arc;
    MSBsplineCurveCP spline;
    DSegment3dCP line;
    MSInterpolationCurveCP interpolationCurve;

    if (nullptr != (lineString = curve->GetLineStringCP()) || nullptr != (lineString = curve->GetPointStringCP()))
        originPoint = (*lineString)[0];
    else if (nullptr != (line = curve->GetLineCP()))
        originPoint = line->point[0];
    else if (nullptr != (arc = curve->GetArcCP()))
        originPoint = arc->center;
    else if (nullptr != (spline = curve->GetBsplineCurveCP()))
        originPoint = spline->GetPole(0);
    else if (nullptr != (interpolationCurve = curve->GetInterpolationCurveCP()))
        originPoint = interpolationCurve->startTangent;
    else
        BeAssert(!"Unrecognized DrivingSurface Base Curve");


    //Placement3d newPlacement (originPoint, GetPlacement ().GetAngles ());
    Placement3d newPlacement(DPoint3d::FromZero(), GetPlacement().GetAngles());
    SetPlacement (newPlacement);

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*geomElem);

    if (builder->Append (*surface, Dgn::GeometryBuilder::CoordSystem::World))
        {
        if (SUCCESS != builder->Finish (*geomElem))
            BeAssert (!"Failed to create DrivingSurface Geometry");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::GeometricElement3d::CreateParams           DrivingSurface::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (model.GetDgnDb().GetDictionaryModel (), GRIDS_CATEGORY_CODE_DrivingSurface);

    CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId, categoryId);

    return createParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr DrivingSurface::GetSurfaceVector
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
BentleyStatus   DrivingSurface::TryGetHeight(double& height) const
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
BentleyStatus   DrivingSurface::TryGetLength(double& length) const
    {
    CurveVectorPtr base = GetSurfaceVector();
    if (base.IsNull())
        return BentleyStatus::ERROR;

    length = base->Length();
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            DrivingSurface::_CopyFrom(Dgn::DgnElementCR source)
    {
    T_Super::_CopyFrom(source);
    }

END_GRIDS_NAMESPACE