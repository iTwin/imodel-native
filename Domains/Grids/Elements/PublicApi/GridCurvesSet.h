/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridCurvesSet.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridCurvesSet : Dgn::SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridCurvesSet, Dgn::SpatialLocationElement);

protected:
    explicit GRIDELEMENTS_EXPORT GridCurvesSet (CreateParams const& params);
    friend struct GridCurvesSetHandler;

    virtual Dgn::DgnDbStatus _OnDelete() const override;

    //! Override this method if your element needs to do additional Inserts into the database (for example,
    //! insert a relationship between the element and something else).
    //! @note If you override this method, you @em must call T_Super::_InsertInDb() first.
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _InsertInDb() override;

    static  GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS(GridCurvesSet, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT static GridCurvesSetPtr Create (Dgn::DgnModelCR model);
};

END_GRIDS_NAMESPACE