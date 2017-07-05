/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/IntegrationTestsHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnDb.h>
#include <BeHttp/Credentials.h>
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <WebServices/iModelHub/Client/Briefcase.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Client/ClientInfo.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGN

#define EXPECT_SUCCESS(x) EXPECT_TRUE(x.IsSuccess()) << GenerateErrorMessage(x.GetError())

struct IntegrationTestSettings
    {
private:
    Http::Credentials m_nonAdminCredentials;
    Http::Credentials m_adminCredentials;

    Utf8String m_host;
    Utf8String m_projectNr;
    UrlProvider::Environment m_environment;
    bool m_ims;
    void ReadSettings(BeFileNameCR settingsFile);
public:
    static IntegrationTestSettings& Instance();
    Http::CredentialsCR GetValidAdminCredentials() const;
    Http::CredentialsCR GetValidNonAdminCredentials() const;
    Http::Credentials GetWrongUsername() const;
    Http::Credentials GetWrongPassword() const;
    Utf8String GetValidHost() const;
    Utf8String GetInvalidHost() const;
    Utf8String GetProjectNr() const;
    UrlProvider::Environment GetEnvironment() const;
    ClientInfoPtr GetClientInfo() const;
    bool IsIms() const;
    };

Utf8String GenerateErrorMessage(Error const& e);
DgnCategoryId CreateCategory(Utf8CP name, DgnDbR db);
RepositoryStatus AcquireLock(DgnModelCR model, LockLevel level = LockLevel::Exclusive);
RepositoryStatus AcquireLock(DgnDbR db, DgnModelCR model, LockLevel level = LockLevel::Exclusive);

PhysicalPartitionPtr CreateModeledElement (Utf8CP name, DgnDbR db);
DgnElementCPtr CreateAndInsertModeledElement (Utf8CP name, DgnDbR db);
DgnModelPtr CreateModel (PhysicalPartitionCR partition, DgnDbR db);
DgnModelPtr CreateModel (Utf8CP name, DgnDbR db);
DgnElementCPtr CreateElement (DgnModelR model, bool acquireLocks = true);
DgnElementPtr Create3dElement (DgnModelR model);
DgnElementPtr Create2dElement (DgnModelR model);
DgnDbStatus InsertStyle(AnnotationTextStylePtr style, DgnDbR db, bool expectSuccess = true);
DgnDbStatus InsertStyle(Utf8CP name, DgnDbR db, bool expectSuccess = true);
int GetCodesCount(DgnDbR db);
void ExpectUnavailableCodesCount(Briefcase& briefcase, int expectedCount);
void ExpectUnavailableLocksCount(Briefcase& briefcase, int expectedCount);
