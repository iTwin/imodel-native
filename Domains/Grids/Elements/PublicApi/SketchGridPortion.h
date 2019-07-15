/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SketchGrid : PlanGrid
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_SketchGrid, PlanGrid);

protected:
    explicit GRIDELEMENTS_EXPORT SketchGrid (T_Super::CreateParams const& params);
    friend struct SketchGridHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (SketchGrid, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates an empty sketch grid
    //! @param[in] model                    Model for the sketch grid
    //! @param[in] scopeElementId           Element that contains sketch grid's model
    //! @param[in] name                     Name of the sketch grid. Must be unique.
    //! @param[in] defaultStartElevation    Default start elevation
    //! @param[in] defaultEndElevation      Default end elevation
    //! @return                             Sketch grid
    GRIDELEMENTS_EXPORT static SketchGridPtr Create (Dgn::SpatialModelCR model, Dgn::DgnElementId scopeElementId, Utf8CP name, double defaultStartElevation, double defaultEndElevation);

    //! Creates and inserts an empty (no axes) sketch grid
    //! @param[in] model                    Model for the sketch grid
    //! @param[in] scopeElementId           Element that contains sketch grid's model
    //! @param[in] name                     Name of the sketch grid. Must be unique.
    //! @param[in] defaultStartElevation    Default start elevation
    //! @param[in] defaultEndElevation      Default end elevation
    //! @return                             Sketch grid
    GRIDELEMENTS_EXPORT static SketchGridPtr CreateAndInsert (Dgn::SpatialModelCR model, Dgn::DgnElementId scopeElementId, Utf8CP name, double defaultStartElevation, double defaultEndElevation);
    };

END_GRIDS_NAMESPACE