/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>

class WSClientIntegrationTestsHost : public RefCounted<BeTest::Host>
    {
    private:
        BeFileName m_programDir;

    private:
        static void InitLogging();
        void InitLibs();
        void SetupTestEnvironment();

    protected:
        WSClientIntegrationTestsHost(const char* programPath);

        virtual void* _InvokeP(char const* function, void* args) override;

        virtual void _GetDocumentsRoot(BeFileName& path) override;
        virtual void _GetDgnPlatformAssetsDirectory(BeFileName& path) override;
        virtual void _GetOutputRoot(BeFileName& path) override;
        virtual void _GetTempDir(BeFileName& path) override;
        virtual void _GetFrameworkSqlangFiles(BeFileName& path) override;

    public:
        static RefCountedPtr<WSClientIntegrationTestsHost> Create(const char* programPath);
    };
