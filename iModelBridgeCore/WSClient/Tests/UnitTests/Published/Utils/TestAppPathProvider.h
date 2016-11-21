/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/TestAppPathProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"
#include <DgnClientFx/DgnClientFxCommon.h>

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestAppPathProvider : DgnClientFx::IApplicationPathsProvider
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
        virtual BeFileNameCR _GetDocumentsDirectory() const  override
            {
            return m_documentsDirectory;
            }
        virtual BeFileNameCR _GetTemporaryDirectory() const override
            {
            return m_temporaryDirectory;
            }
        virtual BeFileNameCR _GetCachesDirectory() const override
            {
            return m_temporaryDirectory;
            }
        virtual BeFileNameCR _GetLocalStateDirectory() const override
            {
            return m_localStateDirectory;
            }
        virtual BeFileNameCR _GetAssetsRootDirectory() const override
            {
            return m_platformAssetsDirectory;
            }
        virtual BeFileNameCR _GetMarkupSeedFilePath() const override
            {
            static BeFileName s_blank;
            return s_blank;
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
    };

END_WSCLIENT_UNITTESTS_NAMESPACE