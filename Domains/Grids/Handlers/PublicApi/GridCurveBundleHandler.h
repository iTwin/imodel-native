/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Handlers/PublicApi/GridCurveBundleHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               06/2018
//=======================================================================================
struct GridCurveBundleHandler : Dgn::dgn_ElementHandler::DriverBundle
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(GRIDS_CLASS_GridCurveBundle, GridCurveBundle, GridCurveBundleHandler, Dgn::dgn_ElementHandler::DriverBundle, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE