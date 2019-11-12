/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

struct GridLabelHandler : Dgn::dgn_ElementHandler::InformationRecord
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (GRIDS_CLASS_GridLabel, GridLabel, GridLabelHandler, Dgn::dgn_ElementHandler::InformationRecord, GRIDHANDLERS_EXPORT)
    };

END_GRIDS_NAMESPACE
