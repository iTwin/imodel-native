/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_RASTER_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct GcsUtils
    {

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  5/2015
    //----------------------------------------------------------------------------------------
    static bool IsHardError(ReprojectStatus status);

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  9/2016
    //----------------------------------------------------------------------------------------
    static StatusInt Reproject(DPoint3dR dest, DgnGCSCR destGcs, DPoint3dCR srcCartesian, GeoCoordinates::BaseGCSCR sourceGcs);
    };

END_BENTLEY_RASTER_NAMESPACE
