/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/SurfaceSet.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SurfaceSet : Dgn::SpatialLocationPortion
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_SurfaceSet, Dgn::SpatialLocationPortion);

protected:
    explicit GRIDELEMENTS_EXPORT SurfaceSet (CreateParams const& params);
    friend struct SurfaceSetHandler;

    static  GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (SurfaceSet, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates surface set in given model
    //! @param[in] model    model to create surface set in
    //! @return             a ptr to created surface set
    GRIDELEMENTS_EXPORT static SurfaceSetPtr Create (Dgn::DgnModelCR model);

};

END_GRIDS_NAMESPACE