#include "PublicApi/GridCurve.h"
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
) : T_Super(params, curve) 
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurve::GridCurve
(
CreateParams const& params,
CurveVectorPtr  curve
) : T_Super(params, curve) 
    {
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurvePtr                 GridCurve::Create 
(
Dgn::DgnModelCR model,
ICurvePrimitivePtr  curve
)
    {
    return new GridCurve (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurvePtr                 GridCurve::Create 
(
Dgn::DgnModelCR model,
CurveVectorPtr  curve
)
    {
    return new GridCurve (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridCurve::_CopyFrom(Dgn::DgnElementCR source)
    {
    T_Super::_CopyFrom(source);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas              03/2017
//---------------+---------------+---------------+---------------+---------------+------
GridSurfacePtr GridCurve::GetIntersectingSurface () const
    {
    Utf8String statemntString = Utf8PrintfString ("SELECT SourceECInstanceId FROM " GRIDS_SCHEMA (GRIDS_REL_GridSurfaceCreatesGridCurve) " WHERE TargetECInstanceId=? AND IsBaseSurface=false");
    BeSQLite::EC::ECSqlStatement statement;
    statement.Prepare (GetDgnDb (), statemntString.c_str ());

    statement.BindId (1, GetElementId ());

    if (BeSQLite::DbResult::BE_SQLITE_ROW != statement.Step ())
        {
        return nullptr;
        }

    return GetDgnDb ().Elements ().GetForEdit <GridSurface> (statement.GetValueId<Dgn::DgnElementId> (0));
    }

END_GRIDS_NAMESPACE
