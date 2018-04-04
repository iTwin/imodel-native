/*--------------------------------------------------------------------------------------+
|
|     $Source: Handlers/PublicApi/ClassificationSystemsHandlers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE


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

END_CLASSIFICATIONSYSTEMS_NAMESPACE