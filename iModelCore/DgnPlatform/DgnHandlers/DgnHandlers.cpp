/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeSQLite/L10N.h>
#include <DgnPlatform/DgnECSymbolProvider.h>
#include <DgnPlatform/DgnECTypes.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::InitializeDgnHandlers()
    {
    InitializeDgnCore();

    // Register Symbol Provider for ECExpressions
    IECSymbolProvider::RegisterExternalSymbolPublisher(&DgnECSymbolProvider::ExternalSymbolPublisher);

    BeAssert(NULL == m_fontAdmin);             m_fontAdmin             = &_SupplyFontAdmin();
    BeAssert(NULL == m_lineStyleAdmin);        m_lineStyleAdmin        = &_SupplyLineStyleAdmin();
    BeAssert(NULL == m_rasterAttachmentAdmin); m_rasterAttachmentAdmin = &_SupplyRasterAttachmentAdmin();
    BeAssert(NULL == m_pointCloudAdmin);       m_pointCloudAdmin       = &_SupplyPointCloudAdmin();
    BeAssert(NULL == m_formatterAdmin);        m_formatterAdmin        = &_SupplyFormatterAdmin();
    BeAssert(NULL == m_repositoryAdmin);       m_repositoryAdmin       = &_SupplyRepositoryAdmin();
    BeAssert(NULL == m_tileAdmin);             m_tileAdmin             = &_SupplyTileAdmin();
    BeAssert(NULL == m_sessionSettingsAdmin);  m_sessionSettingsAdmin  = &_SupplySessionSettingsAdmin();

    m_fontAdmin->Initialize();
    }

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

    ON_HOST_TERMINATE(m_notificationAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_fontAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_lineStyleAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_rasterAttachmentAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_pointCloudAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_geoCoordAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_formatterAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_scriptingAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_repositoryAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_tileAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_sessionSettingsAdmin, onProgramExit);

    // UnRegister Symbol Provider for ECExpressions
    IECSymbolProvider::UnRegisterExternalSymbolPublisher();

    TerminateDgnCore(onProgramExit);

    BeAssert(NULL == DgnPlatformLib::QueryHost());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::LoadResources()
    {
    }

DgnPlatformLib::Host::RasterAttachmentAdmin& DgnPlatformLib::Host::_SupplyRasterAttachmentAdmin() {return *new RasterAttachmentAdmin();}
DgnPlatformLib::Host::PointCloudAdmin&       DgnPlatformLib::Host::_SupplyPointCloudAdmin()       {return *new PointCloudAdmin();}

#if defined (__unix__) // WIP_NONPORT
// ???
// ???
// ???  Why is this necessary? Who is referencing _wassert?
// ???
// ???
void _wassert(wchar_t const*, wchar_t const*, int) {BeAssert(false);}
#endif
