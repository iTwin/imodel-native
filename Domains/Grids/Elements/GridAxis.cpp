/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridAxis.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GridAxis.h"
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING
USING_NAMESPACE_BUILDING_SHARED

DEFINE_GRIDS_ELEMENT_BASE_METHODS(GridAxis)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(OrthogonalAxisX)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(OrthogonalAxisY)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(CircularAxis)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(RadialAxis)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(GeneralGridAxis)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridAxis::GridAxis
(
    CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
GridAxis::CreateParams::CreateParams(GridCR grid, Dgn::DgnClassId classId) :
    T_Super::CreateParams(grid.GetDgnDb(), grid.GetSubModelId(), classId), m_gridId(grid.GetElementId())
    {

    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator GridAxis::MakeIterator () const
    {
    Dgn::ElementIterator iterator = GetDgnDb ().Elements ().MakeIterator (GRIDS_SCHEMA (GRIDS_CLASS_GridSurface), "WHERE Axis=?", "ORDER BY ECInstanceId ASC");
    ECN::ECClassId relClassId = GetDgnDb ().Schemas ().GetClassId (GRIDS_SCHEMA_NAME, GRIDS_REL_GridAxisContainsGridSurfaces);
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement ())
        {
        pStmnt->BindNavigationValue (1, GetElementId (), relClassId);
        }
    return iterator;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridAxis::_Validate
(
) const
    {
    DgnElementId id = GetGridId();

    if (!id.IsValid()) //grid must be set
        return Dgn::DgnDbStatus::ValidationFailed;

    GridCPtr grid = Grid::Get(GetDgnDb(), id);

    if (grid.IsNull())
        return Dgn::DgnDbStatus::ValidationFailed;

    return Dgn::DgnDbStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridAxis::_OnInsert
(
)
    {
    Dgn::DgnDbStatus status = T_Super::_OnInsert ();
    if (status == Dgn::DgnDbStatus::Success)
        {
        return _Validate ();
        }
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridAxis::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = T_Super::_OnUpdate (original);
    if (status == Dgn::DgnDbStatus::Success)
        {
        return _Validate ();
        }
    return status;
    }

template <typename A, typename G>
RefCountedPtr<A> GridAxis::CreateAndInsert(const G& grid)
    {
    grid.GetSurfacesModel(); // Create submodel if not created already

    RefCountedPtr<A> thisAxis = A::Create(grid);

    if (thisAxis.IsNull())
        return nullptr;

    if (!thisAxis->Insert().IsValid())
        return nullptr;

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisX::OrthogonalAxisX
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisXPtr                 OrthogonalAxisX::CreateAndInsert
(
OrthogonalGridCR grid
)
    {
    return T_Super::CreateAndInsert<OrthogonalAxisX, OrthogonalGrid>(grid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisXPtr                 OrthogonalAxisX::Create
(
OrthogonalGridCR grid
)
    {
    if(grid.GetSubModel().IsNull())
        return nullptr;

    return new OrthogonalAxisX (CreateParams(grid));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisY::OrthogonalAxisY
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisYPtr                 OrthogonalAxisY::CreateAndInsert
(
OrthogonalGridCR grid
)
    {
    return T_Super::CreateAndInsert<OrthogonalAxisY, OrthogonalGrid>(grid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisYPtr                 OrthogonalAxisY::Create
(
OrthogonalGridCR grid
)
    {
    if (grid.GetSubModel().IsNull())
        return nullptr;

    return new OrthogonalAxisY (CreateParams(grid));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CircularAxis::CircularAxis
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CircularAxisPtr                 CircularAxis::CreateAndInsert
(
RadialGridCR grid
)
    {
    return T_Super::CreateAndInsert<CircularAxis, RadialGrid>(grid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CircularAxisPtr                 CircularAxis::Create
(
RadialGridCR grid
)
    {
    if (grid.GetSubModel().IsNull())
        return nullptr;

    return new CircularAxis (CreateParams(grid));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialAxis::RadialAxis
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialAxisPtr                 RadialAxis::CreateAndInsert
(
RadialGridCR grid
)
    {
    return T_Super::CreateAndInsert<RadialAxis, RadialGrid>(grid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialAxisPtr                 RadialAxis::Create
(
RadialGridCR grid
)
    {
    if (grid.GetSubModel().IsNull())
        return nullptr;

    return new RadialAxis (CreateParams(grid));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridAxis::GeneralGridAxis
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridAxisPtr                 GeneralGridAxis::CreateAndInsert
(
GridCR grid
)
    {
    return T_Super::CreateAndInsert<GeneralGridAxis, Grid>(grid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridAxisPtr                 GeneralGridAxis::Create
(
GridCR grid
)
    {
    if (grid.GetSubModel().IsNull())
        return nullptr;

    return new GeneralGridAxis(CreateParams(grid));
    }
END_GRIDS_NAMESPACE