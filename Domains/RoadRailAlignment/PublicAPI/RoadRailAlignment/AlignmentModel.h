/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentModel.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignmentApi.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Model to contain and manage Alignment elements
//=======================================================================================
struct AlignmentModel : Dgn::SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS(BRRA_CLASS_AlignmentModel, Dgn::SpatialModel);
    friend struct AlignmentModelHandler;

protected:
    explicit AlignmentModel(CreateParams const& params) : T_Super(params) {}

public:
    static AlignmentModelPtr Create(CreateParams const& params) { return new AlignmentModel(params); }
    static AlignmentModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< AlignmentModel >(id); }
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetECClassId(BRRA_SCHEMA_NAME, BRRA_CLASS_AlignmentModel)); }
}; // AlignmentModel

//=======================================================================================
//! The ModelHandler for AlignmentModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentModelHandler : Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentModel, AlignmentModel, AlignmentModelHandler, Dgn::dgn_ModelHandler::Spatial, ROADRAILALIGNMENT_EXPORT)
}; // AlignmentModelHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE