/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeBimHost.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#define UNICODE
#include <Windows.h>
#endif
#include <iModelBridge/iModelBridgeBimHost.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/Desktop/FileSystem.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING

#undef min
#undef max

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

BeFileName  iModelBridgeKnownLocationsAdmin::s_tempDirectory;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
DgnFontCR iModelBridgeFontAdmin::_ResolveFont(DgnFontCP font)
    {
    // Don't override a functional scenario.
    if ((nullptr != font) && (font->IsResolved()))
        return *font;
    
#ifdef WIP_BRIDGE
    // If a converter was registered, allow it to attempt to look up a workspace font.
    if (nullptr != m_converter)
        {
        DgnFontCP workspaceFont = m_converter->TryResolveFont(font);
        if ((nullptr != workspaceFont) && (workspaceFont->IsResolved()))
            return *workspaceFont;
        }
#endif

    // Otherwise use fallback behavior (e.g. will look up a last resort font for us).
    return T_Super::_ResolveFont(font);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Display::SystemContext* iModelBridgeViewManager::_GetSystemContext() 
    {
#if defined(_WIN32)
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

        m_systemContext = (Display::SystemContext*) ::CreateWindowEx(WS_EX_NOPARENTNOTIFY, MAKEINTATOM(classAtom), L"DgnViewNonInteractiveWindow", 0,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, 0);
        }
#endif

    return m_systemContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR iModelBridgeKnownLocationsAdmin::_GetLocalTempDirectoryBaseName()
    {
    static std::once_flag s_setTempDir;
    std::call_once(s_setTempDir, []()
        {
        Desktop::FileSystem::BeGetTempPath(s_tempDirectory);
        });
    return s_tempDirectory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::L10N::SqlangFiles iModelBridgeBimHost::_SupplySqlangFiles() 
    {
    return BeSQLite::L10N::SqlangFiles(m_fwkSqlangPath);
    }
