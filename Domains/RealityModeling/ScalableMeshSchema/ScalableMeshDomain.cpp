/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshSchemaPCH.h"
USING_NAMESPACE_BENTLEY_DGNPLATFORM
#include <ScalableMeshSchema\ScalableMeshDomain.h>
#include <ScalableMeshSchema\ScalableMeshHandler.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESHSCHEMA

DOMAIN_DEFINE_MEMBERS(ScalableMeshDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Elenie.Godzaridis   02/16
//-----------------------------------------------------------------------------------------
ScalableMeshDomain::ScalableMeshDomain() : DgnDomain("ScalableMesh", "Scalable Mesh Domain", 1)
    {
    RegisterHandler(ScalableMeshModelHandler::GetHandler());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Elenie.Godzaridis   02/16
//-----------------------------------------------------------------------------------------
void ScalableMeshDomain::_OnSchemaImported(DgnDbR db) const
    {}

