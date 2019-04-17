/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <Bentley/BeTest.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestAppPathProvider
    {
    private:
        BeFileName m_documentsDirectory;
        BeFileName m_temporaryDirectory;
        BeFileName m_platformAssetsDirectory;
        BeFileName m_localStateDirectory;

    private:
        void InitPaths(BeFileNameCR programDir, BeFileNameCR outputDir)
            {
            m_platformAssetsDirectory = programDir;

            m_temporaryDirectory = outputDir;
            m_temporaryDirectory.AppendToPath(L"AppTemp").AppendSeparator();

            m_documentsDirectory = outputDir;
            m_documentsDirectory.AppendToPath(L"AppDocuments").AppendSeparator();

            m_localStateDirectory = outputDir;
            m_localStateDirectory.AppendToPath(L"AppLocalState").AppendSeparator();
            }

    protected:
        virtual BeFileNameCR _GetDocumentsDirectory() const
            {
            return m_documentsDirectory;
            }
        virtual BeFileNameCR _GetTemporaryDirectory() const
            {
            return m_temporaryDirectory;
            }
        virtual BeFileNameCR _GetCachesDirectory() const
            {
            return m_temporaryDirectory;
            }
        virtual BeFileNameCR _GetLocalStateDirectory() const
            {
            return m_localStateDirectory;
            }
        virtual BeFileNameCR _GetAssetsRootDirectory() const
            {
            return m_platformAssetsDirectory;
            }

    public:
        TestAppPathProvider()
            {
            BeFileName programDir;
            BeTest::GetHost().GetDgnPlatformAssetsDirectory(programDir);
            BeFileName outputDir;
            BeTest::GetHost().GetOutputRoot(outputDir);

            InitPaths(programDir, outputDir);
            }

        TestAppPathProvider(BeFileNameCR programDir, BeFileNameCR outputDir)
            {
            InitPaths(programDir, outputDir);
            }

        BeFileNameCR GetDocumentsDirectory()
            {
            return m_documentsDirectory;
            }
        BeFileNameCR GetTemporaryDirectory()
            {
            return m_temporaryDirectory;
            }
        BeFileNameCR GetCachesDirectory()
            {
            return m_temporaryDirectory;
            }
        BeFileNameCR GetLocalStateDirectory()
            {
            return m_localStateDirectory;
            }
        BeFileNameCR GetAssetsRootDirectory()
            {
            return m_platformAssetsDirectory;
            }
    };
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
