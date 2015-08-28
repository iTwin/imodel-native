/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/GeomJsApi/GeomJsApi.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/GeomJsApi.h>
#include <DgnPlatform/GeomJsApiProjection.h>

extern Utf8CP geomJsApi_GetBootstrappingSource();

USING_NAMESPACE_BENTLEY_DGNPLATFORM
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
DVec3d GetDVec3d (JsDVector3dP vector)
{
    return vector->Get();
}
END_BENTLEY_DGNPLATFORM_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
GeomJsApi::GeomJsApi(BeJsContext& jsContext) : m_context(jsContext)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
GeomJsApi::~GeomJsApi()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
void GeomJsApi::_ImportScriptLibrary(BeJsContextR jsContext, Utf8CP)
    {
    jsContext.RegisterProjection<GeomJsApiProjection>(geomJsApi_GetBootstrappingSource(), "file:///GeomJsApi.js");
    }