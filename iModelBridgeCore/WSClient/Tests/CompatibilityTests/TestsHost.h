/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/BeTest.h>

class TestsHost : public RefCounted<BeTest::Host>
    {
    private:
        BeFileName m_programDir;
        BeFileName m_outputDir;

    private:
        void SetupTestEnvironment();
        void InitLibraries();
        void InitLogging(int logLevel);

    protected:
        TestsHost(BeFileNameCR programPath, BeFileNameCR workDir, int logLevel);

        virtual void* _InvokeP(char const* function, void* args) override;

        virtual void _GetDocumentsRoot(BeFileName& path) override;
        virtual void _GetDgnPlatformAssetsDirectory(BeFileName& path) override;
        virtual void _GetOutputRoot(BeFileName& path) override;
        virtual void _GetTempDir(BeFileName& path) override;

        virtual void _GetFrameworkSqlangFiles (BeFileName& path) override;

    public:
        static RefCountedPtr<TestsHost> Create(BeFileNameCR programPath, BeFileNameCR workDir, int logLevel);
        static Utf8String& GetErrorLog();
    };
