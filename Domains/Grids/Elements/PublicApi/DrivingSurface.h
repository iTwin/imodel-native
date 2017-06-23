/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/DrivingSurface.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include <GridsMacros.h>

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (DrivingSurface)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrivingSurface : Dgn::SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_DrivingSurface, Dgn::SpatialLocationElement);
    DEFINE_T_SUPER(Dgn::SpatialLocationElement);

protected:
    explicit GRIDELEMENTS_EXPORT DrivingSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT DrivingSurface (CreateParams const& params, CurveVectorPtr surfaceVector);
    explicit GRIDELEMENTS_EXPORT DrivingSurface (CreateParams const& params, ISolidPrimitivePtr surface);
    friend struct DrivingSurfaceHandler;

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source) override;
    static  GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (DrivingSurface, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT CurveVectorPtr    GetSurfaceVector () const;

    GRIDELEMENTS_EXPORT static DrivingSurfacePtr Create (Dgn::DgnModelCR model, CurveVectorPtr surfaceVector);
    GRIDELEMENTS_EXPORT static DrivingSurfacePtr Create (Dgn::DgnModelCR model, ISolidPrimitivePtr surface);
};

END_GRIDS_NAMESPACE