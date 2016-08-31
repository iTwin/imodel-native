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
//! 
//=======================================================================================
struct AlignmentModel : Dgn::SpatialModel
{
DGNMODEL_DECLARE_MEMBERS(RRA_CLASS_AlignmentModel, Dgn::SpatialModel);

protected:
    explicit AlignmentModel(CreateParams const& params) : T_Super(params) {}
}; // AlignmentModel

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE