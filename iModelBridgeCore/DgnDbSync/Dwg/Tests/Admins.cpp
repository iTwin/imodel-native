/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "Tests.h"
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ImporterViewManager : ViewManager
{
private:
    Display::SystemContext* m_systemContext = nullptr;

protected:
    virtual Display::SystemContext* _GetSystemContext() override
        {
#if defined(_WIN32) && defined(USE_WINDOWS_FOR_EGL)
        if (nullptr == m_systemContext)
            {
            static ATOM classAtom = 0;
            if (0 == classAtom)
                {
                // This code is needed when running in non-graphics mode (i.e. Print Organizer worker process).
                WNDCLASS wndClass;
                memset(&wndClass, 0, sizeof (wndClass));
                wndClass.lpfnWndProc   = DefWindowProc;
                wndClass.lpszClassName = L"DgnViewNonInteractiveWindow";

                classAtom = RegisterClass(&wndClass);
                if (0 == classAtom)
                    {
                    BeAssert(false);
                    return nullptr;
                    }
                }

            m_systemContext = (Display::SystemContext*) ::CreateWindowEx (
                                WS_EX_NOPARENTNOTIFY,
                                MAKEINTATOM(classAtom),
                                L"DgnViewNonInteractiveWindow",
                                0,
                                CW_USEDEFAULT, 
                                CW_USEDEFAULT, 
                                CW_USEDEFAULT, 
                                CW_USEDEFAULT, 
                                nullptr,
                                nullptr,
                                nullptr,
                                0
                                );
            }
        BeAssert (nullptr != m_systemContext && "Thumbnail generation requires Display::SystemContext!");
#endif
        return m_systemContext;
        }
    virtual bool _DoesHostHaveFocus() override {return true;}
    // In the common case, we're running on a server, probably inside a VM...no guaranteed access to a usable GPU.
    bool _ForceSoftwareRendering() override {return true;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles ImporterTestsHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/DwgImporterTests_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewManager& ImporterTestsHost::_SupplyViewManager()
    {
    return *new ImporterViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::IKnownLocationsAdmin& ImporterTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }
