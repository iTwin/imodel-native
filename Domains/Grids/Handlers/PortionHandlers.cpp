/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PortionHandlers.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/PortionHandlers.h"
#include "PublicApi/GridSurfaceCreatesGridCurveHandler.h"

USING_NAMESPACE_GRIDS

HANDLER_DEFINE_MEMBERS(SurfaceSetHandler)
HANDLER_DEFINE_MEMBERS (GridPortionHandler)
HANDLER_DEFINE_MEMBERS (RadialGridPortionHandler)
HANDLER_DEFINE_MEMBERS (OrthogonalGridPortionHandler)
HANDLER_DEFINE_MEMBERS (SketchGridPortionHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GridPortionHandler::IntersectGridSurface (GridPortionCPtr thisPortion, GridSurfaceCPtr surface, Dgn::DgnModelCR targetModel)
    {
    Dgn::DgnModelPtr model = thisPortion->GetSubModel ();
    if (!model.IsValid ())
        {
        return ERROR;
        }

    Dgn::DgnDbR db = model->GetDgnDb ();

    for (Dgn::ElementIteratorEntry elementEntry : model->MakeIterator ())
        {
        GridSurfaceCPtr innerSurface = db.Elements ().Get<GridSurface> (elementEntry.GetElementId ());
        if (innerSurface.IsValid())
            {
            GridSurfaceCreatesGridCurveHandler::Insert (db, innerSurface, surface, targetModel);
            }
        }

    return SUCCESS;
    }

