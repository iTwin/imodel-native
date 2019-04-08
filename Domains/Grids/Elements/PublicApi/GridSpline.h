/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridSpline.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridSpline : GridCurve
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridSpline, GridCurve);
    DEFINE_T_SUPER(GridCurve);

protected:
    explicit GRIDELEMENTS_EXPORT GridSpline (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridSpline (CreateParams const& params, ICurvePrimitivePtr curve);
    friend struct GridSplineHandler;

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source, CopyFromOptions const& opts) override;

    virtual bool            _ValidateGeometry(ICurvePrimitivePtr curve) const override;
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridSpline, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a grid spline
    //! @param[in]  GridCurvesSet   portion for the grid spline
    //! @param[in]  curve   curve geometry
    //! @return             Grid spline
    //! @note GridCurvesSet must be inserted
    GRIDELEMENTS_EXPORT static GridSplinePtr Create (GridCurvesSetCR GridCurvesSet, ICurvePrimitivePtr curve);
};

END_GRIDS_NAMESPACE