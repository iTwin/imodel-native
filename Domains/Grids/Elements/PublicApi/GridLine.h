/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

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

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source, CopyFromOptions const& opts) override;

    virtual bool            _ValidateGeometry(ICurvePrimitivePtr curve) const override;
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridLine, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a grid line
    //! @param[in]  GridCurvesSet   portion for the gridline
    //! @param[in]  curve   curve geometry
    //! @return             Grid line
    //! @note GridCurvesSet must be inserted
    GRIDELEMENTS_EXPORT static GridLinePtr Create (GridCurvesSetCR GridCurvesSet, ICurvePrimitivePtr curve);
};

END_GRIDS_NAMESPACE