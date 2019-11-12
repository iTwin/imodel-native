/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

struct ClassificationTableHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationTable, ClassificationTable, ClassificationTableHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE