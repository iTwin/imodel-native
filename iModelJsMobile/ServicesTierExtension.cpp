/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierExtension.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Extension::Install (InstallCallback_T const& callback)
    {
    Host::DispatchExtensionCallback (callback);
    }

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

extern "C" void imodeljs_extension_register(void* modPtr)
    {
    napi_module* mod = (napi_module*)modPtr;

    // *** TBD: create and add an "NapiExtension" object that invokes the napi module initialization function as expected
    }
