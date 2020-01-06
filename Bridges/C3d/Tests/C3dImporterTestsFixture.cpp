/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "C3dImporterTestsFixture.h"

#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <Raster/RasterDomain.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTestsFixture::SetUp()
    {
    iModelBridgeSacAdapter::InitForBeTest (m_options);

    BeFileName configFileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(configFileName);
    configFileName.AppendToPath(L"ImporterTestsConfig.xml");
    m_options.SetConfigFile(configFileName);
    m_options.SetSkipUnchangedFiles(false);  // file time granularity is 1 second. That's too long for an automated test.

    BeFileName::CreateNewDirectory(GetOutputDir());

    // create the seed file
    BeTest::GetHost().GetTempDir(m_seedDgnDbFileName);
    m_seedDgnDbFileName.AppendToPath(L"testSeed.bim");
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
void C3dImporterTestsFixture::SetUp_CreateNewDgnDb()
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetRootSubjectName("C3dImporterTestsFixture");
    if (m_seedDgnDbFileName.DoesPathExist())
        {
        ASSERT_EQ(BeFileNameStatus::Success, m_seedDgnDbFileName.BeDeleteFile());
        }
    DgnDbPtr dgndb = DgnDb::CreateDgnDb(nullptr, m_seedDgnDbFileName, createProjectParams);
    ASSERT_TRUE(dgndb.IsValid());

    FunctionalDomain::GetDomain().ImportSchema(*dgndb);
    Raster::RasterDomain::GetDomain().ImportSchema(*dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTestsFixture::TearDown()
    {
    if (m_wantCleanUp)
        BeFileName::EmptyAndRemoveDirectory(GetOutputDir());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTestsFixture::DoConvert(BeFileNameCR outputName, BeFileNameCR inputFileName)
    {
    InitializeImporterOptions (inputFileName, false);
    
    C3dImporter*    importer = new C3dImporter(m_options);
    ASSERT_NOT_NULL(importer);

    DoConvert (importer, outputName, inputFileName);
    delete importer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    C3dImporterTestsFixture::DoConvert (C3dImporter* importer, BeFileNameCR outputName, BeFileNameCR inputFileName)
    {
    ASSERT_NOT_NULL(importer);

    // try open the inpurt DWG file:
    auto status = importer->OpenDwgFile(inputFileName);
    ASSERT_SUCCESS(status);

    // open seed iModel
    auto db = OpenExistingDgnDb(outputName);
    ASSERT_TRUE(db.IsValid());

    importer->SetDgnDb(*db.get());
    importer->MakeSchemaChanges ();

    // start a new import job
    ASSERT_EQ(C3dImporter::ImportJobCreateStatus::Success, importer->InitializeJob());

    importer->MakeDefinitionChanges (importer->GetImportJob().GetSubject());

    // ready to import DWG into iModel:
    status = importer->Process ();
    ASSERT_SUCCESS(status);

    m_count = importer->GetEntitiesImported ();
    m_scaleDwgToMeters = importer->GetScaleToMeters ();

    db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTestsFixture::DoUpdate(BeFileNameCR outputName, BeFileNameCR inputFileName, bool expectFailure)
    {
    InitializeImporterOptions (inputFileName, true);

    C3dImporter*    importer = new C3dImporter(m_options);
    ASSERT_NOT_NULL(importer);

    DoUpdate (importer, outputName, inputFileName);
    delete importer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTestsFixture::DoUpdate (C3dImporter* importer, BeFileNameCR outputName, BeFileNameCR inputFileName)
    {
    ASSERT_NOT_NULL(importer);

    // try open the inpurt DWG file:
    auto status = importer->OpenDwgFile(inputFileName);
    ASSERT_SUCCESS(status);

    // open an existing iModel
    auto db = OpenExistingDgnDb(outputName);
    ASSERT_TRUE(db.IsValid());

    importer->SetDgnDb (*db.get());
    importer->MakeSchemaChanges ();

    // find an existing import job
    ASSERT_EQ (C3dImporter::ImportJobLoadStatus::Success, importer->FindJob());

    // ready to update the iModel from the DWG
    status = importer->Process ();
    ASSERT_SUCCESS(status);

    m_count = importer->GetEntitiesImported ();
    m_scaleDwgToMeters = importer->GetScaleToMeters ();

    db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTestsFixture::LineUpFiles(WCharCP outputDgnDbFileName, WCharCP inputDwgFileName, bool doConvert)
    {
    C3dImporterTests::MakeWritableCopyOf(m_dwgFileName, inputDwgFileName);
    m_dgnDbFileName = GetOutputFileName(outputDgnDbFileName);
    DeleteExistingDgnDb(m_dgnDbFileName);
    MakeWritableCopyOf(m_dgnDbFileName, m_seedDgnDbFileName, m_dgnDbFileName.GetFileNameAndExtension().c_str());
    if (doConvert)
        DoConvert(m_dgnDbFileName, m_dwgFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTestsFixture::LineUpFilesForNewDwg(WCharCP outputDgnDbFileName, WCharCP inputDwgFileName)
    {
    // prepare DgnDb for an input DWG file to be created anew:
    m_dwgFileName = GetOutputFileName(inputDwgFileName);
    m_dgnDbFileName = GetOutputFileName(outputDgnDbFileName);
    DeleteExistingDgnDb(m_dgnDbFileName);
    MakeWritableCopyOf(m_dgnDbFileName, m_seedDgnDbFileName, m_dgnDbFileName.GetFileNameAndExtension().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTestsFixture::InitializeImporterOptions (BeFileNameCR dwgFilename, bool isUpdating)
    {
    m_options.SetInputFileName (dwgFilename);
    m_options.SetBridgeRegSubKey (L"C3dBridge");
    m_options.SetIsUpdating (isUpdating);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t C3dImporterTestsFixture::GetCount() const
    {
    return  m_count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
double C3dImporterTestsFixture::GetScaleDwgToMeters() const
    {
    return  m_scaleDwgToMeters;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles C3dImporterTestsHost::_SupplySqlangFiles()
    {
    BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/C3dImporterTests_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host::IKnownLocationsAdmin& C3dImporterTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }
