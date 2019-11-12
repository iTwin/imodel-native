/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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