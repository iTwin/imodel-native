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
#include <ScalableMeshSchema\ScalableMeshSchemaApi.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

DOMAIN_DEFINE_MEMBERS(ScalableMeshDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Elenie.Godzaridis   02/16
//-----------------------------------------------------------------------------------------
ScalableMeshDomain::ScalableMeshDomain() : DgnDomain(BENTLEY_SCALABLEMESH_SCHEMA_NAME, "Scalable Mesh Domain", 1)
    {
    RegisterHandler(ScalableMeshModelHandler::GetHandler());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Elenie.Godzaridis   02/16
//-----------------------------------------------------------------------------------------
void ScalableMeshDomain::_OnSchemaImported(DgnDbR db) const
    {
    }

