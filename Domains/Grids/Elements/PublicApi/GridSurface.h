/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridSurface.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include <Grids/Domain/GridsMacros.h>
#include "DrivingSurface.h"

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridSurface)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridSurface : DrivingSurface
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridSurface, DrivingSurface);

protected:
    explicit GRIDELEMENTS_EXPORT GridSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridSurface (CreateParams const& params, CurveVectorPtr surfaceVector);
    explicit GRIDELEMENTS_EXPORT GridSurface (CreateParams const& params, ISolidPrimitivePtr surface);
    friend struct GridSurfaceHandler;

    static GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::SpatialLocationModelCR model, Dgn::DgnClassId classId);
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridSurface, GRIDELEMENTS_EXPORT)

    //! Rotates grid by given angle in radians
    //! @param[in] theta            angle in radians
    GRIDELEMENTS_EXPORT void RotateXY(double theta);

    //! Translates grid to given point
    //! @param[in] target   point to move
    GRIDELEMENTS_EXPORT void MoveToPoint(DPoint3d target);

    //! Translates grid to given point
    //! @param[in] translation   vector to translate by
    GRIDELEMENTS_EXPORT void TranslateXY(DVec3d translation);

    //! Translates grid to given point
    //! @param[in] translation   vector to translate by
    GRIDELEMENTS_EXPORT void Translate(DVec3d translation);

    GRIDELEMENTS_EXPORT static GridSurfacePtr Create(Dgn::SpatialLocationModelCR model, CurveVectorPtr surfaceVector);
    GRIDELEMENTS_EXPORT static GridSurfacePtr Create(Dgn::SpatialLocationModelCR model, ISolidPrimitivePtr surface);

    GRIDELEMENTS_EXPORT static GridSurfacePtr Create(Dgn::SpatialLocationModelCR model, DgnExtrusionDetail extDetail);
};

END_GRIDS_NAMESPACE