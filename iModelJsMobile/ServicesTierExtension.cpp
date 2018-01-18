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
