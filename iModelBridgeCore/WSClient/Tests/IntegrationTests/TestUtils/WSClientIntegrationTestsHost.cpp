/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "WSClientIntegrationTestsHost.h"

#include <TestAppPathProvider.h>

WSClientIntegrationTestsHost::WSClientIntegrationTestsHost(const char* programPath)
    {
    m_programDir = BeFileName(BeFileName::DevAndDir, BeFileName(programPath));
    m_programDir.BeGetFullPathName();

    SetupTestEnvironment();
    }

RefCountedPtr<WSClientIntegrationTestsHost> WSClientIntegrationTestsHost::Create(const char* programPath)
    {
    return new WSClientIntegrationTestsHost(programPath);
    }

void WSClientIntegrationTestsHost::SetupTestEnvironment()
    {
    BeFileName tempDir;
    GetTempDir(tempDir);

    BeFileName::EmptyAndRemoveDirectory(tempDir);
    BeFileName::CreateNewDirectory(tempDir);

    BeFileName outputDir;
    GetOutputRoot(outputDir);
    BeFileName::CreateNewDirectory(outputDir);
    }

void* WSClientIntegrationTestsHost::_InvokeP(char const* function, void* args)
    {
    return nullptr;
    }

void WSClientIntegrationTestsHost::_GetDocumentsRoot(BeFileName& path)
    {
    path = m_programDir;
    }

void WSClientIntegrationTestsHost::_GetDgnPlatformAssetsDirectory(BeFileName& path)
    {
    path = m_programDir;
    path.AppendToPath(L"assets");
    }

void WSClientIntegrationTestsHost::_GetOutputRoot(BeFileName& path)
    {
    path = m_programDir;
    path.AppendToPath(L"Output");
    path.AppendSeparator();
    }

void WSClientIntegrationTestsHost::_GetTempDir(BeFileName& path)
    {
    path = m_programDir;
    path.AppendToPath(L"TempTestsWorkDir");
    path.AppendSeparator();
    }

void WSClientIntegrationTestsHost::_GetFrameworkSqlangFiles(BeFileName& path)
    {
    _GetDgnPlatformAssetsDirectory(path);
    path.AppendToPath(L"sqlang");
    path.AppendToPath(L"WSClient_en.sqlang.db3");
    }
