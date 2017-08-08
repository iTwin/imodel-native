/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridCurve.h $
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
#include "IntersectionCurve.h"
#include "GridSurface.h"

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridCurve)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridCurve : IntersectionCurve
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridCurve, IntersectionCurve);
    DEFINE_T_SUPER(IntersectionCurve);
    
protected:
    explicit GRIDELEMENTS_EXPORT GridCurve (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridCurve (CreateParams const& params, ICurvePrimitivePtr curve);
    explicit GRIDELEMENTS_EXPORT GridCurve (CreateParams const& params, CurveVectorPtr curve);
    friend struct GridCurveHandler;

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source) override;
    static  GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridCurve, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT GridSurfacePtr GetIntersectingSurface () const;

    GRIDELEMENTS_EXPORT static GridCurvePtr Create (Dgn::DgnModelCR model, ICurvePrimitivePtr curve);
    GRIDELEMENTS_EXPORT static GridCurvePtr Create (Dgn::DgnModelCR model, CurveVectorPtr curve);
};

END_GRIDS_NAMESPACE