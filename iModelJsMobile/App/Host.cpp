/*--------------------------------------------------------------------------------------+
|
|     $Source: App/Host.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelJsAppInternal.h"

BEGIN_BENTLEY_IMODELJS_APP_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Host::Host()
    {
    BeAssert (s_instance == nullptr);
    s_instance = this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Host::~Host()
    {
    BeAssert (s_instance == this);
    s_instance = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
HostR Host::GetInstance()
    {
    BeAssert (s_instance != nullptr);

    return *s_instance;
    }

HostP Host::s_instance = nullptr;

END_BENTLEY_IMODELJS_APP_NAMESPACE
