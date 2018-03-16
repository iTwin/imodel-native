/*--------------------------------------------------------------------------------------+
|
|  $Source: Dwg/Tests/ImporterBaseFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImporterBaseFixture.h"

#include <DgnPlatform/FunctionalDomain.h>
#include <Raster/RasterDomain.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::SetUp()
    {
    iModelBridgeSacAdapter::InitForBeTest (m_options);

    BentleyApi::BeFileName configFileName;
    BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(configFileName);
    configFileName.AppendToPath(L"ImporterTestsConfig.xml");
    m_options.SetConfigFile(configFileName);
    m_options.SetSkipUnchangedFiles(false);  // file time granularity is 1 second. That's too long for an automated test.

    BentleyApi::BeFileName::CreateNewDirectory(GetOutputDir());

    // create the seed file
    BentleyApi::BeTest::GetHost().GetTempDir(m_seedDgnDbFileName);
    m_seedDgnDbFileName.AppendToPath(L"testSeed.ibim");
    static bool s_isSeedCreated;
    if (!s_isSeedCreated)
        {
        SetUp_CreateNewDgnDb();
        s_isSeedCreated = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::SetUp_CreateNewDgnDb()
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetRootSubjectName("ImporterTestBaseFixture");
    if (m_seedDgnDbFileName.DoesPathExist())
        {
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, m_seedDgnDbFileName.BeDeleteFile());
        }
    DgnDbPtr dgndb = DgnDb::CreateDgnDb(nullptr, m_seedDgnDbFileName, createProjectParams);
    ASSERT_TRUE(dgndb.IsValid());

    auto sfilename = DwgSyncInfo::GetDbFileName(m_seedDgnDbFileName);
    if (sfilename.DoesPathExist())
        {
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, sfilename.BeDeleteFile());
        }
    ASSERT_EQ(BSISUCCESS, DwgSyncInfo::CreateEmptyFile(sfilename));
    ASSERT_TRUE(sfilename.DoesPathExist());
    FunctionalDomain::GetDomain().ImportSchema(*dgndb);
    BentleyApi::Raster::RasterDomain::GetDomain().ImportSchema(*dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::TearDown()
    {
    if (m_wantCleanUp)
        BentleyApi::BeFileName::EmptyAndRemoveDirectory(GetOutputDir());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::DoConvert(BentleyApi::BeFileNameCR outputName, BentleyApi::BeFileNameCR inputFileName)
    {
    m_options.SetInputFileName (inputFileName);
    m_options.SetBridgeRegSubKey (ImporterTests::GetDwgBridgeRegistryKey());
    m_options.SetIsUpdating (false);
#ifndef WIP_GENRATE_THUMBNAILS
    // don't generate thumbnails for now
    m_options.SetWantThumbnails (false);
#endif
    
    DwgImporter*    importer = new DwgImporter(m_options);
    ASSERT_NOT_NULL(importer);
    // try open the inpurt DWG file:
    auto status = importer->OpenDwgFile(inputFileName);
    ASSERT_SUCCESS(status);

    // open seed iModel
    auto db = OpenExistingDgnDb(outputName);
    ASSERT_TRUE(db.IsValid());

    importer->SetDgnDb(*db.get());
    importer->AttachSyncInfo();

    // start a new import job
    ASSERT_EQ(DwgImporter::ImportJobCreateStatus::Success, importer->InitializeJob());

    // ready to import DWG into iModel:
    status = importer->Process ();
    ASSERT_SUCCESS(status);

    m_count = importer->GetEntitiesImported ();
    m_scaleDwgToMeters = importer->GetScaleToMeters ();

    db->SaveChanges();

    delete importer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::DoUpdate(BentleyApi::BeFileNameCR outputName, BentleyApi::BeFileNameCR inputFileName, bool expectFailure)
    {
    m_options.SetInputFileName (inputFileName);
    m_options.SetBridgeRegSubKey (ImporterTests::GetDwgBridgeRegistryKey());
    m_options.SetIsUpdating (true);
#ifndef WIP_GENRATE_THUMBNAILS
    // don't generate thumbnails for now
    m_options.SetWantThumbnails (false);
#endif

    DwgImporter*    importer = new DwgImporter(m_options);
    ASSERT_NOT_NULL(importer);

    // try open the inpurt DWG file:
    auto status = importer->OpenDwgFile(inputFileName);
    ASSERT_SUCCESS(status);

    // open an existing iModel
    auto db = OpenExistingDgnDb(outputName);
    ASSERT_TRUE(db.IsValid());

    importer->SetDgnDb (*db.get());
    importer->AttachSyncInfo ();
    importer->MakeSchemaChanges ();

    // find an existing import job
    ASSERT_EQ(DwgImporter::ImportJobLoadStatus::Success, importer->FindJob());

    // ready to update the iModel from the DWG
    status = importer->Process ();
    ASSERT_SUCCESS(status);

    m_count = importer->GetEntitiesImported ();
    m_scaleDwgToMeters = importer->GetScaleToMeters ();

    db->SaveChanges();

    delete importer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::LineUpFiles(BentleyApi::WCharCP outputDgnDbFileName, BentleyApi::WCharCP inputDwgFileName, bool doConvert)
    {
    ImporterTests::MakeWritableCopyOf(m_dwgFileName, inputDwgFileName);
    m_dgnDbFileName = GetOutputFileName(outputDgnDbFileName);
    DeleteExistingDgnDb(m_dgnDbFileName);
    MakeWritableCopyOf(m_dgnDbFileName, m_seedDgnDbFileName, m_dgnDbFileName.GetFileNameAndExtension().c_str());
    auto syncFile(DwgSyncInfo::GetDbFileName(m_seedDgnDbFileName));
    BentleyApi::BeFileName outSyncFile;
    MakeWritableCopyOf(outSyncFile, syncFile, DwgSyncInfo::GetDbFileName(m_dgnDbFileName).GetFileNameAndExtension().c_str());
    if (doConvert)
        DoConvert(m_dgnDbFileName, m_dwgFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::LineUpFilesForNewDwg(WCharCP outputDgnDbFileName, WCharCP inputDwgFileName)
    {
    // prepare DgnDb for an input DWG file to be created anew:
    m_dwgFileName = GetOutputFileName(inputDwgFileName);
    m_dgnDbFileName = GetOutputFileName(outputDgnDbFileName);
    DeleteExistingDgnDb(m_dgnDbFileName);
    MakeWritableCopyOf(m_dgnDbFileName, m_seedDgnDbFileName, m_dgnDbFileName.GetFileNameAndExtension().c_str());
    auto syncFile(DwgSyncInfo::GetDbFileName(m_seedDgnDbFileName));
    BentleyApi::BeFileName outSyncFile;
    MakeWritableCopyOf(outSyncFile, syncFile, DwgSyncInfo::GetDbFileName(m_dgnDbFileName).GetFileNameAndExtension().c_str());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImporterTestBaseFixture::GetCount() const
    {
    return  m_count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
double ImporterTestBaseFixture::GetScaleDwgToMeters() const
    {
    return  m_scaleDwgToMeters;
    }
