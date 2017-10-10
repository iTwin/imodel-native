#include "PublicApi/GridPortion.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <ConstraintSystem/ConstraintSystemApi.h>
#include "PublicApi\GridArcSurface.h"
#include "PublicApi\GridPlaneSurface.h"
#include <ConstraintSystem/ConstraintSystemApi.h>
//#include <DimensionHandler.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridPortion)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortion::GridPortion
(
T_Super::CreateParams const& params,
DVec3d          normal
) : T_Super(params) 
    {
    if (!m_categoryId.IsValid () && m_classId.IsValid()) //really odd tests in platform.. attempts to create elements with 0 class id and 0 categoryId. NEEDS WORK: PLATFORM
        {
        Dgn::DgnCategoryId catId = SpatialCategory::QueryCategoryId (params.m_dgndb.GetDictionaryModel (), GRIDS_CATEGORY_CODE_GridSurface);
        if (catId.IsValid ())
            DoSetCategoryId (catId);

        SetPropertyValue (prop_Normal (), normal);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortion::CreateParams           GridPortion::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnElement::CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId);

    return CreateParams(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortionPtr                 GridPortion::Create 
(
Dgn::DgnModelCR model,
DVec3d          normal
)
    {
    return new GridPortion (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), normal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus GridPortion::InsertGridMapElements(GridAxisMap elements)
    {
    for (bpair<Utf8String, GridElementVector> pair : elements)
        for (GridSurfacePtr gridSurface : pair.second)
            if (BentleyStatus::ERROR == BuildingUtils::InsertElement(gridSurface))
                return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridPortion::TranslateToPoint(GridElementVector& grid, DPoint3d point)
    {
    if (grid.empty())
        return;

    DVec3d translation = DVec3d::FromStartEnd(grid.front()->GetPlacement().GetOrigin(), point);

    for (GridSurfacePtr gridElement : grid)
        gridElement->Translate(translation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridPortion::RotateToAngleXY(GridElementVector& grid, double theta)
    {
    if (grid.empty())
        return;

    for (GridSurfacePtr gridElement : grid)
        gridElement->RotateXY(theta);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     09/17
//---------------------------------------------------------------------------------------
DPlane3d GridPortion::GetPlane ()
    {
    DPlane3d plane;
    plane.Zero ();
    plane.normal = DVec3d::From (GetPropertyValueDPoint3d (prop_Normal ()));
    return plane;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
RepositoryStatus GridPortion::RotateToAngleXY (double theta)
    {
    RepositoryStatus status = RepositoryStatus::Success;
    for (Dgn::ElementIteratorEntry pIterEntry : MakeIterator ())
        {
        GridSurfacePtr gridSurface = GetDgnDb ().Elements ().GetForEdit<GridSurface> (pIterEntry.GetElementId ());
        gridSurface->RotateXY (theta);
        if (RepositoryStatus::Success != (status = BuildingLocks_LockElementForOperation (*gridSurface, BeSQLite::DbOpcode::Update, "update GridSurface")))
            return status;
        gridSurface->Update ();
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
RepositoryStatus GridPortion::TranslateToPoint (DPoint3d point)
    {
    RepositoryStatus status = RepositoryStatus::Success;
    Dgn::ElementIterator gridElements = MakeIterator ();

    GridSurfaceCPtr firstGridElem = GetDgnDb ().Elements ().Get<GridSurface> ((*gridElements.begin()).GetElementId ());

    DVec3d translation = DVec3d::FromStartEnd (firstGridElem->GetPlacement ().GetOrigin (), point);

    for (Dgn::ElementIteratorEntry pIterEntry : gridElements)
        {
        GridSurfacePtr gridSurface = GetDgnDb ().Elements ().GetForEdit<GridSurface> (pIterEntry.GetElementId());
        gridSurface->Translate (translation);
        if (RepositoryStatus::Success != (status = BuildingLocks_LockElementForOperation (*gridSurface, BeSQLite::DbOpcode::Update, "update GridSurface")))
            return status;
        gridSurface->Update ();
        }
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/17
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator GridPortion::MakeIterator () const
    {
    Dgn::ElementIterator iterator;
    if (GetSubModelId ().IsValid ())
        {
        iterator = GetDgnDb ().Elements ().MakeIterator (GRIDS_SCHEMA (GRIDS_CLASS_GridSurface), "WHERE Model.Id=?");
        iterator.GetStatement ()->BindId (1, GetSubModelId ());
        }
    return iterator;
    }

END_GRIDS_NAMESPACE