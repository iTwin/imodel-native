/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeSQLite/L10N.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::InitializeDgnHandlers()
    {
    InitializeDgnCore ();

    BeAssert (NULL == m_fontAdmin);             m_fontAdmin             = &_SupplyFontAdmin();
    BeAssert (NULL == m_lineStyleAdmin);        m_lineStyleAdmin        = &_SupplyLineStyleAdmin();
    BeAssert (NULL == m_rasterAttachmentAdmin); m_rasterAttachmentAdmin = &_SupplyRasterAttachmentAdmin();
    BeAssert (NULL == m_pointCloudAdmin);       m_pointCloudAdmin       = &_SupplyPointCloudAdmin();
    BeAssert (NULL == m_elementHandlerLoader);  m_elementHandlerLoader  = &_SupplyElementHandlerLoader();
    BeAssert (NULL == m_graphicsAdmin);         m_graphicsAdmin         = &_SupplyGraphicsAdmin();
    BeAssert (NULL == m_materialAdmin);         m_materialAdmin         = &_SupplyMaterialAdmin();
    BeAssert (NULL == m_solidsKernelAdmin);     m_solidsKernelAdmin     = &_SupplySolidsKernelAdmin();
    BeAssert (NULL == m_geoCoordAdmin);         m_geoCoordAdmin         = &_SupplyGeoCoordinationAdmin();
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
    TERMINATE_HOST_OBJECT(m_elementHandlerLoader, onProgramExit);
    TERMINATE_HOST_OBJECT(m_graphicsAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_materialAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_solidsKernelAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_geoCoordAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_formatterAdmin, onProgramExit);

    TerminateDgnCore (onProgramExit);

    BeAssert (NULL == DgnPlatformLib::QueryHost());
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

#if defined (BENTLEY_WIN32)
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
//=======================================================================================
// WIP: experimental Graphite / Vancouver interop
// @bsiclass                                    Shaun.Sewall                    03/14
//=======================================================================================
struct DefaultLibraryHost : DgnPlatformLib::Host
{
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override {return *new WindowsKnownLocationsAdmin();}
    virtual void _SupplyProductName (WStringR name) override {name.assign (L"DefaultLibraryHost");}
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() {return BeSQLite::L10N::SqlangFiles(BeFileName());} // no translatable strings
};
#endif

//---------------------------------------------------------------------------------------
// WIP: experimental Graphite / Vancouver interop
// @bsimethod                                   Shaun.Sewall                    03/14
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Initialize ()
    {
#if defined (BENTLEY_WIN32)
    DgnPlatformLib::Initialize (*new DefaultLibraryHost(), true);
#else
    BeAssert (false);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::LoadResources()
    {
    GetDgnFontManager().Initialize();
    GetLineStyleManager().Initialize();
    }

ElementHandlerLoaderR DgnPlatformLib::Host::_SupplyElementHandlerLoader()  {return *new ElementHandlerLoader();}
DgnPlatformLib::Host::RasterAttachmentAdmin& DgnPlatformLib::Host::_SupplyRasterAttachmentAdmin() {return *new RasterAttachmentAdmin();}
DgnPlatformLib::Host::PointCloudAdmin&       DgnPlatformLib::Host::_SupplyPointCloudAdmin()       {return *new PointCloudAdmin();}

#if defined (__unix__) // WIP_NONPORT
// ???
// ???
// ???  Why is this necessary? Who is referencing _wassert?
// ???
// ???
void _wassert(wchar_t const*, wchar_t const*, int) {BeAssert (false);;}
#endif

///////////////////////////////////////////////////////////////////////////////////////////
/////////// static initialization /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
static bool   s_inStaticInitialization;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnPlatformLib::InStaticInitialization ()
    {
    ___DGNPLATFORM_SERIALIZED___;
    return s_inStaticInitialization;
    }

/*---------------------------------------------------------------------------------**//**
* *Private* method called by DgnPlatformLib::StaticInitialize. Never called directly by app.
* @bsimethod                                    Keith.Bentley                   09/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void staticInitializeDgnCore()
    {
    XGraphicsOperations::StaticInitialize ();
    SectionClipObjectFactory::Register();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void staticInitializeDgnHandlers()
    {
    s_inStaticInitialization = true;

    staticInitializeDgnCore();

    Handler::locked_StaticInitializeDomains ();
    Handler::locked_StaticInitializeRegisterHandlers();
    
    s_inStaticInitialization = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::StaticInitialize()
    {
    ___DGNPLATFORM_SERIALIZED___;
    static bool s_staticInitialized = false;
    if (s_staticInitialized)
        return;

    bentleyAllocator_enableLowFragmentationCRTHeap();

    staticInitializeDgnHandlers();
    s_staticInitialized = true;
    }

DEFINE_KEY_METHOD(ICurvePathQuery)
DEFINE_KEY_METHOD(ICurvePathEdit)
DEFINE_KEY_METHOD(IBsplineSurfaceQuery)
DEFINE_KEY_METHOD(IBsplineSurfaceEdit)
DEFINE_KEY_METHOD(ISolidPrimitiveQuery)
DEFINE_KEY_METHOD(ISolidPrimitiveEdit)
DEFINE_KEY_METHOD(IMeshQuery)
DEFINE_KEY_METHOD(IMeshEdit)
DEFINE_KEY_METHOD(IBRepQuery)
DEFINE_KEY_METHOD(ITextEdit)
DEFINE_KEY_METHOD(IMaterialPropertiesExtension)
DEFINE_KEY_METHOD(ITextQuery)
DEFINE_KEY_METHOD(IDeleteManipulator)
