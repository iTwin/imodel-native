/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshSchemaPCH.h"
#include <ScalableMeshSchema\ScalableMeshHandler.h>
#include <ScalableMeshSchema\ScalableMeshSchemaApi.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

DOMAIN_DEFINE_MEMBERS(ScalableMeshDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley       09/2015
//-----------------------------------------------------------------------------------------
ScalableMeshDomain::ScalableMeshDomain() : DgnDomain(BENTLEY_SCALABLEMESH_SCHEMA_NAME, "Bentley ScalableMesh Domain", 1) 
    {
    RegisterHandler(ScalableMeshModelHandler::GetHandler());
    }
 
//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley       09/2015
//-----------------------------------------------------------------------------------------
void ScalableMeshDomain::_OnSchemaImported(DgnDbR db) const
    {
    }

