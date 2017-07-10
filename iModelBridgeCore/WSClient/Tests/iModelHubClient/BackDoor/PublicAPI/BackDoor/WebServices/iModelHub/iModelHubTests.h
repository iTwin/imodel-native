/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/iModelHubClient/BackDoor/PublicAPI/BackDoor/WebServices/iModelHub/iModelHubTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnDb.h>
#include <Bentley/BeTest.h>

#define BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE  BEGIN_BENTLEY_NAMESPACE namespace iModel { namespace Hub { namespace Tests {
#define END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE    } } } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS  using namespace BentleyApi::iModel::Hub::Tests;

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE

struct ScopediModelHubHostImpl;

//=======================================================================================
//! A DgnPlatformLib host that can be used with "Published" tests
//=======================================================================================
struct ScopediModelHubHost
{
private:
    ScopediModelHubHostImpl* m_pimpl;
public:
    ScopediModelHubHost();
    ~ScopediModelHubHost();

    void CleanOutputDirectory();
    BeFileName GetDocumentsDirectory();
    BeFileName GetOutputDirectory();
    BeFileName GetDgnPlatformAssetsDirectory();
    BeFileName GetTempDirectory();
    BeFileName BuildDbFileName(Utf8StringCR baseName);
    DgnDbPtr CreateTestDb(Utf8StringCR baseName);

    void SetRepositoryAdmin(DgnPlatformLib::Host::RepositoryAdmin* admin);
};
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
