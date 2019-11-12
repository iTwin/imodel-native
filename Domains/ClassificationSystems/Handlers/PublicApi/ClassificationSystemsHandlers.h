/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE


struct ClassificationSystemHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem, ClassificationSystem, ClassificationSystemHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

struct ClassificationHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_Classification, Classification, ClassificationHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

struct ClassificationGroupHandler : Dgn::dgn_ElementHandler::GroupInformation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationGroup, ClassificationGroup, ClassificationGroupHandler, Dgn::dgn_ElementHandler::GroupInformation, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE