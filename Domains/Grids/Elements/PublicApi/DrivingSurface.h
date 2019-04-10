/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/DrivingSurface.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

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

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source, CopyFromOptions const& opts) override;
    static  GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (DrivingSurface, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Getters and setters
    //---------------------------------------------------------------------------------------
    //! Returns base curve of this surface
    //! @return a ptr to a curve vector of this surface
    GRIDELEMENTS_EXPORT CurveVectorPtr    GetSurfaceVector () const;

    //! Tries to return height of this surface which is essencially the magnitude of its extrusion vector
    //! @param[out] height height of this surface
    //! @return     BentleyStatus::ERROR if error occured when trying to get surface height
    GRIDELEMENTS_EXPORT BentleyStatus   TryGetHeight(double& height) const;

    //! Tries to return length of this surface which is essencially the length of its base curve
    //! @param[out] length  length of this surface
    //! @return     BentleyStatus::ERROR if error occured when trying to get surface length
    GRIDELEMENTS_EXPORT BentleyStatus   TryGetLength(double& length) const;

    
};

END_GRIDS_NAMESPACE