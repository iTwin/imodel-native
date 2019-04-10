/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridArc.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE
//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridArc : GridCurve
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridArc, GridCurve);
    DEFINE_T_SUPER(GridCurve);

protected:
    explicit GRIDELEMENTS_EXPORT GridArc (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridArc (CreateParams const& params, ICurvePrimitivePtr curve);
    friend struct GridArcHandler;

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source, CopyFromOptions const& opts) override;

    virtual bool            _ValidateGeometry(ICurvePrimitivePtr curve) const override;
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridArc, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a grid arc
    //! @param[in]  GridCurvesSet   portion for the grid arc
    //! @param[in]  curve   curve geometry
    //! @return             Grid arc
    //! @note GridCurvesSet must be inserted
    GRIDELEMENTS_EXPORT static GridArcPtr Create (GridCurvesSetCR GridCurvesSet, ICurvePrimitivePtr curve);
};

END_GRIDS_NAMESPACE