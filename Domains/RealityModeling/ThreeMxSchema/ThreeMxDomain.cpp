/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxSchemaInternal.h"

USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA

DOMAIN_DEFINE_MEMBERS(ThreeMxDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley       09/2015
//-----------------------------------------------------------------------------------------
ThreeMxDomain::ThreeMxDomain() : DgnDomain(BENTLEY_THREEMX_SCHEMA_NAME, "Bentley 3MX Domain", 1) 
    {
    RegisterHandler(ThreeMxModelHandler::GetHandler());
    }
 
//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley       09/2015
//-----------------------------------------------------------------------------------------
void ThreeMxDomain::_OnSchemaImported(DgnDbR db) const
    {
    }

