/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/IntersectionCurve.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

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

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates intersection curve from given curve primitive
    //! @param[in]  model   model to create intersection curve in
    //! @param[in]  curve   geometry as curve primitive
    //! @returns            a ptr to created intersection curve
    GRIDELEMENTS_EXPORT static IntersectionCurvePtr Create(Dgn::DgnModelCR model, ICurvePrimitivePtr curve);

    //! Creates intersection curve from given curve vector
    //! @param[in]  model   model to create intersection curve in
    //! @param[in]  curve   geometry as curve vector
    //! @returns            a ptr to created intersection curve
    GRIDELEMENTS_EXPORT static IntersectionCurvePtr Create(Dgn::DgnModelCR model, CurveVectorPtr curve);
    
    //---------------------------------------------------------------------------------------
    // Setters and getters
    //---------------------------------------------------------------------------------------
    //! Returns curve's geometry as curve primitive
    //! @return             a ptr to curve's geometry as curve primitive
    GRIDELEMENTS_EXPORT ICurvePrimitivePtr      GetCurve () const;

    //---------------------------------------------------------------------------------------
    // Geometry modification
    //---------------------------------------------------------------------------------------
    //! Sets geometry for intersection curve as curve vector
    //! @param[in]  curve   new geometry as curve vector
    GRIDELEMENTS_EXPORT void    SetCurve (CurveVectorPtr curve);

    //! Sets geometry for intersection curve as curve primitive
    //! @param[in]  curve   new geometry as curve primitive
    GRIDELEMENTS_EXPORT void    SetCurve (ICurvePrimitivePtr curve);
};

END_GRIDS_NAMESPACE