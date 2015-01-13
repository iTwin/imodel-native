/*--------------------------------------------------------------------------------------+
|
|     $Source: WebServices/WebApi/WebApi.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WebServicesInternal.h"
#include "WebApi.h"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebApi::WebApi (std::shared_ptr<const ClientConfiguration> configuration) :
m_configuration (configuration)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebApi::~WebApi ()
    {
    }
