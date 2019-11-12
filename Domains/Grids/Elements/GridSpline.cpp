/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Grids/Elements/GridElementsAPI.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridSpline)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSpline::GridSpline
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSpline::GridSpline
(
CreateParams const& params,
ICurvePrimitivePtr  curve
) : T_Super(params, curve) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSplinePtr                 GridSpline::Create 
(
GridCurvesSetCR portion,
ICurvePrimitivePtr  curve
)
    {
    return new GridSpline (CreateParamsFromModel(*portion.GetSubModel(), QueryClassId(portion.GetDgnDb())), curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridSpline::_CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(source, opts);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              12/2017
//--------------+---------------+---------------+---------------+---------------+--------
bool GridSpline::_ValidateGeometry(ICurvePrimitivePtr curve) const
    {
    return nullptr != curve->GetBsplineCurveCP() ||
        nullptr != curve->GetInterpolationCurveCP();
    }

END_GRIDS_NAMESPACE