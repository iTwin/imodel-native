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
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridCurve : Dgn::SpatialLocationElement
{
    DEFINE_T_SUPER(Dgn::SpatialLocationElement);
    
protected:
    explicit GRIDELEMENTS_EXPORT GridCurve (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridCurve (CreateParams const& params, ICurvePrimitivePtr curve);
    explicit GRIDELEMENTS_EXPORT GridCurve (CreateParams const& params, CurveVectorPtr curve);

    GRIDELEMENTS_EXPORT void                InitGeometry (ICurvePrimitivePtr curve);
    GRIDELEMENTS_EXPORT void                InitGeometry (CurveVectorPtr curve);

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source) override;
    static  GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridCurve, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a gridcurve
    //! @param[in]  model   model for the gridcurve
    //! @param[in]  curve   curve geometry
    //! @return             GridCurve
    GRIDELEMENTS_EXPORT static GridCurvePtr Create(Dgn::DgnModelCR model, ICurvePrimitivePtr curve);

    //! Creates a gridcurve
    //! @param[in]  model   model for the gridcurve
    //! @param[in]  curve   curve geometry
    //! @return             GridCurve
    GRIDELEMENTS_EXPORT static GridCurvePtr Create(Dgn::DgnModelCR model, CurveVectorPtr curve);
    
    //---------------------------------------------------------------------------------------
    // Getters and setters
    //---------------------------------------------------------------------------------------
    //! gets curve geometry of the gridcurve
    //! @return             GridCurve geometry
    GRIDELEMENTS_EXPORT ICurvePrimitivePtr      GetCurve () const;

    //! gets the intersecting GridSurface which creates this GridCurve (not the baseSurface)
    //! @return             gridsurface, if exists
    GRIDELEMENTS_EXPORT GridSurfacePtr GetIntersectingSurface() const;

    //---------------------------------------------------------------------------------------
    // Geometry modification
    //---------------------------------------------------------------------------------------
    //! sets curve geometry of the gridcurve
    //! @param[in]  curve   model for the gridcurve
    GRIDELEMENTS_EXPORT void    SetCurve (CurveVectorPtr curve);

    //! sets curve geometry of the gridcurve
    //! @param[in]  curve   model for the gridcurve
    GRIDELEMENTS_EXPORT void    SetCurve (ICurvePrimitivePtr curve);    
};

END_GRIDS_NAMESPACE