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

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridLine)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridLine::GridLine
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridLine::GridLine
(
CreateParams const& params,
ICurvePrimitivePtr  curve
) : T_Super(params, curve) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridLinePtr                 GridLine::Create 
(
GridCurvesSetCR portion,
ICurvePrimitivePtr  curve
)
    {
    return new GridLine(CreateParamsFromModel(*portion.GetSubModel(), QueryClassId(portion.GetDgnDb())), curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridLine::_CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(source, opts);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Haroldas.Vitunskas              12/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
bool GridLine::_ValidateGeometry(ICurvePrimitivePtr curve) const
    {
    return nullptr != curve->GetLineStringCP() ||
        nullptr != curve->GetPointStringCP() ||
        nullptr != curve->GetLineCP();
    }

END_GRIDS_NAMESPACE