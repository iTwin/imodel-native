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

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (IntersectionCurve)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectionCurve::IntersectionCurve
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectionCurve::IntersectionCurve
(
CreateParams const& params,
ICurvePrimitivePtr  curve
) : T_Super(params) 
    {
    InitGeometry (curve);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectionCurve::IntersectionCurve
(
CreateParams const& params,
CurveVectorPtr  curve
) : T_Super(params) 
    {
    InitGeometry (curve);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            IntersectionCurve::InitGeometry
(
CurveVectorPtr  curve
)
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP ();

    DPoint3d originPoint;
    (*curve)[0]->GetStartPoint (originPoint);

    Placement3d newPlacement (originPoint, GetPlacement ().GetAngles ());
    SetPlacement (newPlacement);

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*geomElem);

    if (builder->Append (*curve, Dgn::GeometryBuilder::CoordSystem::World))
        {
        if (SUCCESS != builder->Finish (*geomElem))
            BeAssert (!"Failed to create IntersectionCurve Geometry");
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            IntersectionCurve::InitGeometry
(
ICurvePrimitivePtr  curve
)
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP ();

    DPoint3d originPoint;
    curve->GetStartPoint (originPoint);

    Placement3d newPlacement (originPoint, GetPlacement ().GetAngles ());
    SetPlacement (newPlacement);

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*geomElem);

    if (builder->Append (*curve, Dgn::GeometryBuilder::CoordSystem::World))
        {
        if (SUCCESS != builder->Finish (*geomElem))
            BeAssert (!"Failed to create IntersectionCurve Geometry");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            IntersectionCurve::SetCurve
(
ICurvePrimitivePtr  curve
)
    {
    //clean the existing geometry
    GetGeometryStreamR ().Clear ();
    InitGeometry (curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            IntersectionCurve::SetCurve
(
CurveVectorPtr  curve
)
    {
    //clean the existing geometry
    GetGeometryStreamR ().Clear ();
    InitGeometry (curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::GeometricElement3d::CreateParams           IntersectionCurve::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (model.GetDgnDb ().GetDictionaryModel (), GRIDS_CATEGORY_CODE_IntersectionCurve);

    CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId, categoryId);

    return createParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectionCurvePtr                 IntersectionCurve::Create 
(
Dgn::DgnModelCR model,
ICurvePrimitivePtr  curve
)
    {
    return new IntersectionCurve (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), curve);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectionCurvePtr                 IntersectionCurve::Create 
(
Dgn::DgnModelCR model,
CurveVectorPtr  curve
)
    {
    return new IntersectionCurve (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr                  IntersectionCurve::GetCurve
(
) const
    {
    GeometryCollection geomData = *ToGeometrySource ();
    ICurvePrimitivePtr curve = nullptr;
    GeometricPrimitivePtr geometricPrimitivePtr = (*(geomData.begin ())).GetGeometryPtr ();
    if (geometricPrimitivePtr.IsValid())
        {
        curve = geometricPrimitivePtr->GetAsICurvePrimitive ();
        curve->TransformInPlace ((*geomData.begin ()).GetGeometryToWorld ());
        }

    return curve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            IntersectionCurve::_CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(source, opts);
    }

END_GRIDS_NAMESPACE