/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridLine.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include <Grids/gridsApi.h>

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridLine)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridLine : GridCurve
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridLine, GridCurve);
    DEFINE_T_SUPER(GridCurve);

protected:
    explicit GRIDELEMENTS_EXPORT GridLine (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridLine (CreateParams const& params, ICurvePrimitivePtr curve);
    friend struct GridLineHandler;

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source) override;

    virtual bool            _ValidateGeometry(ICurvePrimitivePtr curve) const override;
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridLine, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a grid line
    //! @param[in]  gridCurvesPortion   portion for the gridline
    //! @param[in]  curve   curve geometry
    //! @return             Grid line
    //! @note GridCurvesPortion must be inserted
    GRIDELEMENTS_EXPORT static GridLinePtr Create (GridCurvesPortionCR gridCurvesPortion, ICurvePrimitivePtr curve);
};

END_GRIDS_NAMESPACE