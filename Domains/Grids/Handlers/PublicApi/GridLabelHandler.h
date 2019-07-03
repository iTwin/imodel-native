/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

struct GridLabelHandler : Dgn::dgn_ElementHandler::InformationRecord
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridLabel, GridLabel, GridLabelHandler, Dgn::dgn_ElementHandler::InformationRecord, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE
