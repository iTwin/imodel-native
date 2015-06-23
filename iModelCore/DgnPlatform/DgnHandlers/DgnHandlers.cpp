/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnHandlers.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeSQLite/L10N.h>
#include <DgnPlatform/DgnHandlers/DgnECSymbolProvider.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::InitializeDgnHandlers()
    {
    InitializeDgnCore ();

    // Register Symbol Provider for ECExpressions
    IECSymbolProvider::RegisterExternalSymbolPublisher (&DgnECSymbolProvider::ExternalSymbolPublisher);

    BeAssert (NULL == m_fontAdmin);             m_fontAdmin             = &_SupplyFontAdmin();
    BeAssert (NULL == m_lineStyleAdmin);        m_lineStyleAdmin        = &_SupplyLineStyleAdmin();
    BeAssert (NULL == m_rasterAttachmentAdmin); m_rasterAttachmentAdmin = &_SupplyRasterAttachmentAdmin();
    BeAssert (NULL == m_pointCloudAdmin);       m_pointCloudAdmin       = &_SupplyPointCloudAdmin();
    BeAssert (NULL == m_graphicsAdmin);         m_graphicsAdmin         = &_SupplyGraphicsAdmin();
    BeAssert (NULL == m_materialAdmin);         m_materialAdmin         = &_SupplyMaterialAdmin();
    BeAssert (NULL == m_solidsKernelAdmin);     m_solidsKernelAdmin     = &_SupplySolidsKernelAdmin();
    BeAssert (NULL == m_formatterAdmin);        m_formatterAdmin        = &_SupplyFormatterAdmin ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::Terminate (bool onProgramExit)
    {
    DGNPLATFORM_TRACE ("DgnPlatformLib::Host::Terminate");

    if (NULL == DgnPlatformLib::QueryHost())
        {
        BeAssert (false && "Terminate called on a thread that is not associated with a host");
        return;
        }

    TERMINATE_HOST_OBJECT(m_notificationAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_fontAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_lineStyleAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_rasterAttachmentAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_pointCloudAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_graphicsAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_materialAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_solidsKernelAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_geoCoordAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_formatterAdmin, onProgramExit);

    // UnRegister Symbol Provider for ECExpressions
    IECSymbolProvider::UnRegisterExternalSymbolPublisher ();

    TerminateDgnCore (onProgramExit);

    BeAssert (NULL == DgnPlatformLib::QueryHost());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::StaticInitialize()
    {
    BeSystemMutexHolder holdBeSystemMutexInScope;

    static bool s_staticInitialized = false;
    if (s_staticInitialized)
        return;

    bentleyAllocator_enableLowFragmentationCRTHeap();

    SectionClipObjectFactory::Register();

    s_staticInitialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Initialize (Host& host, bool loadResources, bool adoptHost)
    {
    StaticInitialize();

    DgnPlatformLib::Host* wasHost = NULL;   
    if (!adoptHost)
        {
        // *** NEEDS WORK: All of this monkey business with forgetting and adopting the current host is 
        // necessary because we have initializing and adopting mixed up.
        // If you want to create a Host for later use or for use by another thread, you
        // have to call DgnPlaformLib::Initialize to get it set up. But that has the side
        // effect of making the current thread actually adopt that host. Ideally, we should 
        // be able to initialize a host without adopting it. However, we can't be sure that
        // the various initialization functions aren't accessing the host in thread-local 
        // storage during initialization. Since they might, we have to plug it in.
        // To preserve the original host, we swap it out again when we're done.
        wasHost = QueryHost();
        if (wasHost)
            ForgetHost();
        }

    if (NULL != QueryHost())
        {
        BeAssert (false && "Call DgnPlatformLib::Initialize once per thread");
        return;
        }

    host.InitializeDgnHandlers(); 

    if (loadResources)
        host.LoadResources();

    if (!adoptHost)
        {
        ForgetHost();
        if (wasHost)
            AdoptHost(*wasHost);
        }
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
void _wassert(wchar_t const*, wchar_t const*, int) {BeAssert (false);}
#endif
