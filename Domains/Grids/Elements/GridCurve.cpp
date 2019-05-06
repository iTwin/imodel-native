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

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridCurve)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurve::GridCurve
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurve::GridCurve
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
GridCurve::GridCurve
(
CreateParams const& params,
CurveVectorPtr  curve
) : T_Super(params) 
    {
    InitGeometry (curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::GeometricElement3d::CreateParams           GridCurve::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (model.GetDgnDb ().GetDictionaryModel (), GRIDS_CATEGORY_CODE_GridCurve);

    CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId, categoryId);

    return createParams;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridCurve::CheckDependancyToModel
(
) const 
{ 
    DgnModelPtr model = GetModel();
    GridCurvesSetCPtr portion = GetDgnDb().Elements().Get<GridCurvesSet>(model->GetModeledElementId());
    if (portion.IsNull()) //pointer must not be null
        return DgnDbStatus::ValidationFailed;

    return DgnDbStatus::Success;
}


//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridCurve::_OnInsert()
    {
    DgnDbStatus status = CheckDependancyToModel();
    if (status != DgnDbStatus::Success)
        return status;
    if (!_ValidateGeometry(GetCurve()))
        return DgnDbStatus::ValidationFailed;
    return T_Super::_OnInsert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridCurve::_OnUpdate(Dgn::DgnElementCR original)
    {
    DgnDbStatus status = CheckDependancyToModel();
    if (status != DgnDbStatus::Success)
        return status;
    if (!_ValidateGeometry(GetCurve()))
        return DgnDbStatus::ValidationFailed;

    return T_Super::_OnUpdate(original);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
GridLabelCPtr GridCurve::GetNonElevationSurfaceGridLabel () const
    {
    GridSurfaceCPtr labelOwner = GetFirstNonElevationIntersectingGridSurface ();
    if (labelOwner.IsNull ())
        return nullptr;

    return labelOwner->GetGridLabel ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
GridSurfaceCPtr GridCurve::GetFirstNonElevationIntersectingGridSurface () const
    {
    bvector<DgnElementId> intersectingSurfaces = GetIntersectingSurfaceIds ();
    for (DgnElementId surfaceId : intersectingSurfaces)
        {
        GridSurfaceCPtr surface = GetDgnDb ().Elements ().Get<GridSurface> (surfaceId);
        if (nullptr == dynamic_cast<ElevationGridSurfaceCP>(surface.get ()))
            return surface;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridCurve::_CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(source, opts);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas              03/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<Dgn::DgnElementId> GridCurve::GetIntersectingSurfaceIds() const
    {
    return GridCurveBundle::MakeDrivingSurfaceIterator(*this).BuildIdList<Dgn::DgnElementId>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridCurve::InitGeometry
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
void            GridCurve::InitGeometry
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
void            GridCurve::SetCurve
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
void            GridCurve::SetCurve
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
ICurvePrimitivePtr                  GridCurve::GetCurve
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
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridCurve::GeneralGridCurve
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridCurve::GeneralGridCurve
(
CreateParams const& params,
ICurvePrimitivePtr  curve
) : T_Super(params, curve) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridCurvePtr                 GeneralGridCurve::Create 
(
Dgn::DgnModelCR model,
ICurvePrimitivePtr  curve
)
    {
    return new GeneralGridCurve (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), curve);
    }

END_GRIDS_NAMESPACE
