//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <RealityPlatformTools/SimpleRDSApi.h>
#include "../../RealityPlatformTools/RealityDataServiceInternal.h"
#include "../Common/RealityModFrameworkTestsCommon.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::NiceMock;
using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::StrEq;
using ::testing::HasSubstr;
using ::testing::Matcher;
using ::testing::Mock;

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//! RealityDataServiceFixture
//=====================================================================================
class RDSTransferIntegrationFixture : public testing::Test
	{
	};

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(RDSTransferIntegrationFixture, UploadTest)
	{
    //Creating a repo to upload.
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName outDir = BeFileName(exeDir);
    outDir.AppendToPath(L"RDSTransferTestDirectory");
    WString directory(outDir);
    if (BeFileName::DoesPathExist(directory.c_str()))
        BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    BeFileName::CreateNewDirectory(directory.c_str());

    ASSERT_TRUE(BeFileName::DoesPathExist(directory.c_str()));

    BeFileName dummyRoot = BeFileName(directory.c_str());
    dummyRoot.AppendToPath(L"DummyFolderUp");
    BeFileName::CreateNewDirectory(dummyRoot.c_str());

    ASSERT_TRUE(BeFileName::DoesPathExist(dummyRoot.c_str()));

    BeFileName dummyFile = BeFileName(dummyRoot.c_str());
    dummyFile.AppendToPath(L"DummyRootDocument.json");

    FILE* pFile;
    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 500000, SEEK_SET); //500Kb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummyFolder/
    dummyFile.AppendToPath(L"DummySubFolder");
    BeFileName::CreateNewDirectory(dummyFile.c_str());
    ASSERT_TRUE(BeFileName::DoesPathExist(dummyFile.c_str()));
    dummyFile.AppendToPath(L"smallfile1.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 100000, SEEK_SET); //100Kb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile2.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 100000, SEEK_SET); //100Kb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile3.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 100000, SEEK_SET); //100Kb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile4.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 100000, SEEK_SET); //100Kb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile5.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 100000, SEEK_SET); //100Kb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    //the actual upload begins here
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, "SimpleRDSApiExample (DELETE THIS)");
    properties.Insert(RealityDataField::Classification, RealityDataBase::GetTagFromClassification(RealityDataBase::Classification::TERRAIN));
    properties.Insert(RealityDataField::Type, "S3MX");
    properties.Insert(RealityDataField::Visibility, RealityDataBase::GetTagFromVisibility(RealityDataBase::Visibility::ENTERPRISE));
    properties.Insert(RealityDataField::RootDocument, "DummyRootDocument.json");
        
    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    RDSRequestManager::Setup();

    RealityDataServiceUpload upload = RealityDataServiceUpload(dummyRoot, "", propertyString);
    ASSERT_TRUE(upload.IsValidTransfer());
       
    Utf8String identifier = upload.GetRealityDataId();
        
    ASSERT_TRUE(!identifier.empty());
    upload.OnlyReportErrors(true);
    upload.Perform();

    dummyRoot.PopDir();
    dummyRoot = BeFileName(directory.c_str());
    dummyRoot.AppendToPath(L"DummyFolderDown");

    ASSERT_TRUE(BeFileName::EmptyAndRemoveDirectory(directory.c_str()) == BeFileNameStatus::Success);

    RealityDataDelete del = RealityDataDelete(identifier);
    RawServerResponse rawResponse = RawServerResponse();
    RealityDataService::Request(del, rawResponse);

    ASSERT_EQ(rawResponse.status, 0);
	}