/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/IntegrationTestsHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsHelper.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/TxnManager.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_DGN

Utf8String GenerateErrorMessage(Error const& e)
    {
    Utf8String errorMessage;
    errorMessage.Sprintf("\nError id: %d\nMessage: %s\nDescription: %s\n", (int) e.GetId(), e.GetMessage(), e.GetDescription());
    return errorMessage;
    }

IntegrationTestSettings& IntegrationTestSettings::Instance()
    {
    static IntegrationTestSettings* s_instance = nullptr;
    if (nullptr == s_instance)
        {
        s_instance = new IntegrationTestSettings();
        BeFileName fileName = DgnPlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        fileName = fileName.AppendToPath(L"IntegrationTests.json");
        s_instance->ReadSettings(fileName);
        }

    return *s_instance;
    }

void IntegrationTestSettings::ReadSettings(BeFileNameCR settingsFile)
    {
    BeFile file;
    if (BeFileStatus::Success != file.Open(settingsFile.c_str(), BeFileAccess::Read))
        return;

    ByteStream byteStream;
    if (BeFileStatus::Success != file.ReadEntireFile(byteStream))
        return;

    Utf8String contents((Utf8CP) byteStream.GetData(), byteStream.GetSize());
    file.Close();

    Json::Reader reader;
    Json::Value settings;
    if (!reader.Parse(contents, settings))
        return;

    auto users = settings["Users"];
    for (auto user : users)
        {
        if (user["IsAdmin"].asBool())
            {
            m_adminCredentials = Credentials(user["Username"].asString(), user["Password"].asString());
            continue;
            }

        m_nonAdminCredentials = Credentials(user["Username"].asString(), user["Password"].asString());
        }

    m_host = settings["Host"].asString();
    m_projectNr = settings["ProjectNr"].asString();
    Utf8String environment = settings["Environment"].asString();
    if ("DEV" == environment)
        m_environment = UrlProvider::Environment::Dev;
    else if ("QA" == environment)
        m_environment = UrlProvider::Environment::Qa;
    else if ("PROD" == environment)
        m_environment = UrlProvider::Environment::Release;
    }

CredentialsCR IntegrationTestSettings::GetValidAdminCredentials() const
    {
    return m_adminCredentials;
    }

CredentialsCR IntegrationTestSettings::GetValidNonAdminCredentials() const
    {
    return m_nonAdminCredentials;
    }

Credentials IntegrationTestSettings::GetWrongUsername() const
    {
    return Credentials(m_adminCredentials.GetUsername() + "wrong", m_adminCredentials.GetPassword());
    }

Credentials IntegrationTestSettings::GetWrongPassword() const
    {
    return Credentials(m_adminCredentials.GetUsername(), m_adminCredentials.GetPassword() + "wrong");
    }

Utf8String IntegrationTestSettings::GetValidHost() const
    {
    return m_host;
    }

Utf8String IntegrationTestSettings::GetInvalidHost() const
    {
    return m_host + "/invalid";
    }

Utf8String IntegrationTestSettings::GetProjectNr() const
    {
    return m_projectNr;
    }

ClientInfoPtr IntegrationTestSettings::GetClientInfo() const
    {
    //Use Navigator Desktop product id for now
    return std::shared_ptr<WebServices::ClientInfo>(new WebServices::ClientInfo("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", "1654"));
    }

UrlProvider::Environment IntegrationTestSettings::GetEnvironment() const
    {
    return m_environment;
    }

DgnCategoryId CreateCategory(Utf8CP name, DgnDbR db)
    {
    SpatialCategory category(db.GetDictionaryModel(), name, DgnCategory::Rank::User, "");

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::White());

    auto persistentCategory = category.Insert(appearance);
    BeAssert(persistentCategory.IsValid());

    return persistentCategory->GetCategoryId();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
RepositoryStatus AcquireLock(DgnModelCR model, LockLevel level)
    {
    LockRequest req;
    req.Insert(model, level);
    return model.GetDgnDb().BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None).Result();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                 08/2016
//---------------------------------------------------------------------------------------
RepositoryStatus AcquireLock(DgnDbR db, DgnModelCR model, LockLevel level)
    {
    LockRequest req;
    req.Insert(model, level);
    return db.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None).Result();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                 11/2016
//---------------------------------------------------------------------------------------
PhysicalPartitionPtr CreateModeledElement (Utf8CP name, DgnDbR db)
    {
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), name);
    EXPECT_TRUE(partition.IsValid());
    auto result = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::Success, result);
    return partition;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                 11/2016
//---------------------------------------------------------------------------------------
DgnElementCPtr CreateAndInsertModeledElement (Utf8CP name, DgnDbR db)
    {
    auto modeledElement = CreateModeledElement (name, db);
    auto persistentModeledElement = modeledElement->Insert();
    EXPECT_TRUE(persistentModeledElement.IsValid());
    return persistentModeledElement;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                 11/2016
//---------------------------------------------------------------------------------------
DgnModelPtr CreateModel (PhysicalPartitionCR partition, DgnDbR db)
    {
    PhysicalModelPtr model = PhysicalModel::Create(partition);
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForModelInsert(req, *model, IBriefcaseManager::PrepareAction::Acquire));

    auto status = model->Insert();
    EXPECT_EQ(DgnDbStatus::Success, status);
    return DgnDbStatus::Success == status ? model : nullptr;
    }

DgnModelPtr CreateModel (Utf8CP name, DgnDbR db)
    {
    PhysicalPartitionPtr partition = CreateModeledElement (name, db);
    EXPECT_TRUE(partition->Insert().IsValid());

    return CreateModel (*partition, db);
    }

DgnElementCPtr CreateElement(DgnModelR model, bool acquireLocks)
    {
    auto elem = model.Is3d() ? Create3dElement(model) : Create2dElement(model);
    if (acquireLocks)
        {
        IBriefcaseManager::Request req;
        EXPECT_EQ(RepositoryStatus::Success, model.GetDgnDb().BriefcaseManager().PrepareForElementInsert(req, *elem, IBriefcaseManager::PrepareAction::Acquire));
        }

    auto persistentElem = elem->Insert();
    return persistentElem;
    }

DgnElementPtr Create3dElement(DgnModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnCategoryId catId = (*SpatialCategory::MakeIterator(db).begin()).GetId<DgnCategoryId>();
    return GenericPhysicalObject::Create(*model.ToPhysicalModelP(), catId);
    }

DgnElementPtr Create2dElement(DgnModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Annotation2d::GetHandler());
    DgnCategoryId catId = (*SpatialCategory::MakeIterator(db).begin()).GetId<DgnCategoryId>();
    return AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, model.GetModelId(), classId, catId));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnDbStatus InsertStyle(AnnotationTextStylePtr style, DgnDbR db, bool expectSuccess)
    {
    IBriefcaseManager::Request req;

    RepositoryStatus statusR = db.BriefcaseManager().PrepareForElementInsert(req, *style, IBriefcaseManager::PrepareAction::Acquire);
    EXPECT_EQ(expectSuccess, RepositoryStatus::Success == statusR);

    DgnDbStatus status;
    style->DgnElement::Insert(&status);
    return status;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnDbStatus InsertStyle(Utf8CP name, DgnDbR db, bool expectSuccess)
    {
    AnnotationTextStylePtr style;
    style = AnnotationTextStyle::Create(db);
    style->SetName(name);

    return InsertStyle(style, db, expectSuccess);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
int GetCodesCount(DgnDbR db)
    {
    // LRP InitializeFileJob initialize imodel and create codes in iModel Hub Services. If this place fails, check if LRP are running fine in the server
    int count = 0;

    auto iterator = DgnCode::MakeIterator(db);
    for (auto it = iterator.begin(); it != iterator.end(); ++it)
        count++;

    return count;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
void ExpectUnavailableCodesCount(Briefcase& briefcase, int expectedCount)
    {
    auto result = briefcase.GetiModelConnection().QueryUnavailableCodesLocks(briefcase.GetBriefcaseId(), briefcase.GetLastChangeSetPulled())->GetResult();
    EXPECT_SUCCESS(result);
    auto actualCount = result.GetValue().GetCodes().size();
    EXPECT_EQ(expectedCount, actualCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
void ExpectUnavailableLocksCount(Briefcase& briefcase, int expectedCount)
    {
    auto result = briefcase.GetiModelConnection().QueryUnavailableCodesLocks(briefcase.GetBriefcaseId(), briefcase.GetLastChangeSetPulled())->GetResult();
    EXPECT_SUCCESS(result);
    auto actualCount = result.GetValue().GetLocks().size();
    EXPECT_EQ(expectedCount, actualCount);
    }
