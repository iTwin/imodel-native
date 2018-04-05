/*--------------------------------------------------------------------------------------+
|
|     $Source: Handlers/PublicApi/ClassificationSystemsHandlers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

struct ClassificationSystemClassDefinitionGroupHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystemClassDefinitionGroup, ClassificationSystemClassDefinitionGroup, ClassificationSystemClassDefinitionGroupHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

struct CIBSEClassDefinitionHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_CIBSEClassDefinition, CIBSEClassDefinition, CIBSEClassDefinitionHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

struct OmniClassClassDefinitionHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_OmniClassClassDefinition, OmniClassClassDefinition, OmniClassClassDefinitionHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

struct ASHRAE2004ClassDefinitionHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ASHRAE2004ClassDefinition, ASHRAE2004ClassDefinition, ASHRAE2004ClassDefinitionHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

struct ASHRAE2010ClassDefinitionHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ASHRAE2010ClassDefinition, ASHRAE2010ClassDefinition, ASHRAE2010ClassDefinitionHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

struct MasterFormatClassDefinitionHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_MasterFormatClassDefinition, MasterFormatClassDefinition, MasterFormatClassDefinitionHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

struct UniFormatClassDefinitionHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_UniFormatClassDefinition, UniFormatClassDefinition, UniFormatClassDefinitionHandler, Dgn::dgn_ElementHandler::Definition, CLASSIFICATIONSYSTEMSHANDLERS_EXPORT)
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE