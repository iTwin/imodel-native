/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

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
        void InitPaths(BeFileNameCR assetsDir, BeFileNameCR outputDir)
            {
            m_platformAssetsDirectory = assetsDir;

            m_temporaryDirectory = outputDir;
            m_temporaryDirectory.AppendToPath(L"AppTemp").AppendSeparator();

            m_documentsDirectory = outputDir;
            m_documentsDirectory.AppendToPath(L"AppDocuments").AppendSeparator();

            m_localStateDirectory = outputDir;
            m_localStateDirectory.AppendToPath(L"AppLocalState").AppendSeparator();
            }

    public:
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
            BeFileName assetsDir = programDir;
            assetsDir.AppendToPath(L"Assets").AppendSeparator();
            InitPaths(assetsDir, outputDir);
            }
    };

END_WSCLIENT_UNITTESTS_NAMESPACE
