/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/IntersectionCurve.h $
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

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (IntersectionCurve)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IntersectionCurve : Dgn::SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_IntersectionCurve, Dgn::SpatialLocationElement);
    DEFINE_T_SUPER(Dgn::SpatialLocationElement);

protected:
    explicit GRIDELEMENTS_EXPORT IntersectionCurve (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT IntersectionCurve (CreateParams const& params, ICurvePrimitivePtr curve);
    explicit GRIDELEMENTS_EXPORT IntersectionCurve (CreateParams const& params, CurveVectorPtr curve);
    friend struct IntersectionCurveHandler;

            GRIDELEMENTS_EXPORT void                InitGeometry (ICurvePrimitivePtr curve);
            GRIDELEMENTS_EXPORT void                InitGeometry (CurveVectorPtr curve);

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source) override;
    static  GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (IntersectionCurve, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT ICurvePrimitivePtr      GetCurve () const;

    GRIDELEMENTS_EXPORT void    SetCurve (CurveVectorPtr curve);
    GRIDELEMENTS_EXPORT void    SetCurve (ICurvePrimitivePtr curve);

    GRIDELEMENTS_EXPORT static IntersectionCurvePtr Create (Dgn::DgnModelCR model, ICurvePrimitivePtr curve);
    GRIDELEMENTS_EXPORT static IntersectionCurvePtr Create (Dgn::DgnModelCR model, CurveVectorPtr curve);
};

END_GRIDS_NAMESPACE