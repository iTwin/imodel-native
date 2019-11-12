/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeSQLite/L10N.h>


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::Terminate(bool onProgramExit)
    {
    DGNPLATFORM_TRACE ("DgnPlatformLib::Host::Terminate");

    if (NULL == DgnPlatformLib::QueryHost())
        {
        BeAssert(false && "Terminate called with no host");
        return;
        }

    ON_HOST_TERMINATE(m_fontAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_lineStyleAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_pointCloudAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_geoCoordAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_repositoryAdmin, onProgramExit);

    // UnRegister Symbol Provider for ECExpressions
    IECSymbolProvider::UnRegisterExternalSymbolPublisher();

    TerminateDgnCore(onProgramExit);

    BeAssert(NULL == DgnPlatformLib::QueryHost());
    }

DgnPlatformLib::Host::PointCloudAdmin&       DgnPlatformLib::Host::_SupplyPointCloudAdmin()       {return *new PointCloudAdmin();}

#if defined (__unix__) // WIP_NONPORT
// ???
// ???
// ???  Why is this necessary? Who is referencing _wassert?
// ???
// ???
void _wassert(wchar_t const*, wchar_t const*, int) {BeAssert(false);}
#endif
