/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/iModelHubHost.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <DgnPlatform/DgnPlatformLib.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
struct iModelHubHostImpl;
struct iModelHubHost
    {
    private:
        iModelHubHostImpl* m_pimpl;
        BeFileName m_customOutputDir;
        iModelHubHost();
        ~iModelHubHost();
    public:
        static iModelHubHost& Instance();

        void CleanOutputDirectory();
        BeFileName GetDocumentsDirectory();
        BeFileName GetOutputDirectory();
        BeFileName GetDgnPlatformAssetsDirectory();
        BeFileName GetTempDirectory();
        BeFileName BuildDbFileName(Utf8StringCR baseName);
        Dgn::DgnDbPtr CreateTestDb(Utf8StringCR baseName);

        void SetCustomOutputDir(BeFileName outputDir);
        void SetRepositoryAdmin(Dgn::DgnPlatformLib::Host::RepositoryAdmin* admin);
    };
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
