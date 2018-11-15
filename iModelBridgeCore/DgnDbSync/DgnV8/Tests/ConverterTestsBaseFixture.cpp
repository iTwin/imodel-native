/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/ConverterTestsBaseFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"

#include <DgnPlatform/WebMercator.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <Raster/RasterDomain.h>
#include <ThreeMx/ThreeMxApi.h>
#include <PointCloud/PointCloudDomain.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>

ConverterTestsHost ConverterTestBaseFixture::m_host;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::InitializeTheConverter()
    {
    static bool s_isConverterInitialized;
    if (!s_isConverterInitialized) // I can't currently terminate the V8 host, so I can't call Converter::Initialize more than once
        {
        s_isConverterInitialized = true;

        iModelBridgeSacAdapter::InitForBeTest(m_params);

        BentleyApi::BeFileName v8DllsRelativeDir(L"DgnV8"); // it's relative to dllDirectory

        Converter::Initialize(m_params.GetLibraryDir(), m_params.GetAssetsDir(), v8DllsRelativeDir, nullptr, false, 0, nullptr, nullptr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::SetUp_CreateNewDgnDb()
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetRootSubjectName("ConverterTestBaseFixture");
    if (m_seedDgnDbFileName.DoesPathExist())
        {
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, m_seedDgnDbFileName.BeDeleteFile());
        }
    DgnDbPtr dgndb = DgnDb::CreateDgnDb(nullptr, m_seedDgnDbFileName, createProjectParams);
    ASSERT_TRUE(dgndb.IsValid());

    auto sfilename = SyncInfo::GetDbFileName(m_seedDgnDbFileName);
    if (sfilename.DoesPathExist())
        {
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, sfilename.BeDeleteFile());
        }
    ASSERT_EQ(BSISUCCESS, SyncInfo::CreateEmptyFile(sfilename));
    ASSERT_TRUE(sfilename.DoesPathExist());
    FunctionalDomain::GetDomain().ImportSchema(*dgndb);
    BentleyApi::Raster::RasterDomain::GetDomain().ImportSchema(*dgndb);
    BentleyApi::PointCloud::PointCloudDomain::GetDomain().ImportSchema(*dgndb);
    BentleyApi::ThreeMx::ThreeMxDomain::GetDomain().ImportSchema(*dgndb);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::SetUp()
    {
    InitializeTheConverter(); // this initializes the converter itself and the DgnV8 host

    iModelBridgeSacAdapter::InitForBeTest(m_params);

    BentleyApi::BeFileName configFileName;
    BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(configFileName);
    configFileName.AppendToPath(L"ImportConfig.xml");
    m_params.SetConfigFile(configFileName);
    m_params.SetSkipUnchangedFiles(false);  // file time granularity is 1 second. That's too long for an automated test.
    m_params.SetWantThumbnails(false); // It takes too long, and most tests do not look at them
    m_count = 0;
    m_opts.m_useTiledConverter = false;
    BentleyApi::BeFileName::CreateNewDirectory(GetOutputDir());

    BentleyApi::BeTest::GetHost().GetTempDir(m_seedDgnDbFileName);
    m_seedDgnDbFileName.AppendToPath(L"testSeed.bim");
    static bool s_isSeedCreated;
    if (!s_isSeedCreated)
        {
        SetUp_CreateNewDgnDb();
        s_isSeedCreated = true;
        }
    }

//-----------------------------------------------------------------------------------------
// Method called only once for the the test case.
// Called before the first test in this test case.
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
void ConverterTestBaseFixture::SetUpTestCase()
    {
    DgnViewLib::Initialize(m_host, true); // this initializes the DgnDb libraries
    }

//-----------------------------------------------------------------------------------------
// Method called only once for the the test case.
// Called after the last test in this test case.
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
void ConverterTestBaseFixture::TearDownTestCase()
    {
    m_host.Terminate(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::TearDown()
    {
    if (m_wantCleanUp)
        BentleyApi::BeFileName::EmptyAndRemoveDirectory(GetOutputDir());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::SetGcsDef()
    {
    iModelBridge::GCSDefinition gcsDef;
    gcsDef.m_isValid = true;
    gcsDef.m_originUors.Zero();
    gcsDef.m_azimuthAngle = 0.0;
    gcsDef.m_geoPoint.latitude = 40.06569747222222; // coordinates of Bentley in Exton, PA
    gcsDef.m_geoPoint.longitude = -75.68858161111112;
    gcsDef.m_geoPoint.elevation = 0;                // what is the elevation here?
    m_params.SetOutputGcsDefinition(gcsDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName ConverterTestBaseFixture::GetInputFileName(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"TestData");
    filepath.AppendToPath(filename);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName ConverterTestBaseFixture::GetOutputDir()
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetOutputRoot(filepath);
    filepath.AppendToPath(L"Output");
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName ConverterTestBaseFixture::GetOutputFileName(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath = GetOutputDir();
    filepath.AppendToPath(filename);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::MakeWritableCopyOf(BentleyApi::BeFileName& outFile, BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName inFile = GetInputFileName(filename);
    MakeWritableCopyOf(outFile, inFile, filename);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
void ConverterTestBaseFixture::MakeWritableCopyOf(BentleyApi::BeFileName& outFile, BentleyApi::BeFileNameCR inputFileName, BentleyApi::WCharCP filename)
    {
    outFile = GetOutputFileName(filename);
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(inputFileName, outFile)) << "Unable to copy file \nSource: [" << Utf8String(inputFileName.c_str()).c_str() << "]\nDestination: [" << Utf8String(outFile.c_str()).c_str() << "]";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::DeleteExistingDgnDb(BentleyApi::BeFileNameCR dgnDbFileName)
    {
    BentleyApi::BeFileName dgnDbFileNameAndRelatedFiles(dgnDbFileName);
    dgnDbFileNameAndRelatedFiles.append(L".*");
    BentleyApi::BeFileListIterator iter(dgnDbFileNameAndRelatedFiles, false);
    BentleyApi::BeFileName file;
    while (BentleyApi::SUCCESS == iter.GetNextFileName(file))
        {
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeDeleteFile(file));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr ConverterTestBaseFixture::OpenExistingDgnDb(BentleyApi::BeFileNameCR projectName, DgnDb::OpenMode mode)
    {
    DbResult fileStatus;

    DgnDb::OpenParams openParams(mode);

    DgnDbPtr ret = DgnDb::OpenDgnDb(&fileStatus, projectName, openParams);
    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int ConverterTestBaseFixture::CountGeometricModels(DgnDbR db)
    {
    int numGeometricModels = 0;

    for (ModelIteratorEntryCR modelEntry : db.Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel)))
        numGeometricModels++;

    return numGeometricModels;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Barry.Bentley                   02/17
//--------------+---------------+---------------+---------------+---------------+--------
static bool HasStreetsModel (DgnDbR db)
    {
    DgnModels& models = db.Models();

    for (ModelIteratorEntryCR entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel)))
        {
        BentleyApi::Dgn::DgnModelPtr model = models.GetModel(entry.GetModelId());
        if (!model.IsValid())
            continue;

        if (model->GetName().Equals ("Bing Streets"))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Barry.Bentley                   02/17
//--------------+---------------+---------------+---------------+---------------+--------
void AddExternalDataModels (DgnDbR db)
    {
    using namespace WebMercator;

    // *** WIP_EXTERNAL_DATA_MODELS -- somehow, find out which external data models to create
    if (nullptr == db.GeoLocation().GetDgnGCS())
        return;
    if (HasStreetsModel (db))
        return;

    RepositoryLinkPtr streetsLink = RepositoryLink::Create(*db.GetRealityDataSourcesModel(), nullptr, "Bing Streets");
    if (streetsLink.IsValid() && streetsLink->Insert().IsValid())
        {
        // set up the Bing Street map properties Json.
        BentleyApi::Json::Value jsonParameters;
        jsonParameters[WebMercatorModel::json_providerName()] = BingImageryProvider::prop_BingProvider();
        jsonParameters[WebMercatorModel::json_groundBias()] = -1.0;
        jsonParameters[WebMercatorModel::json_transparency()] = 0.0;
        BentleyApi::Json::Value& bingStreetsJson = jsonParameters[WebMercatorModel::json_providerData()];

        bingStreetsJson[WebMercatorModel::json_mapType()] = (int)MapType::Street;
        WebMercatorModel::CreateParams createParams (db, streetsLink->GetElementId(), jsonParameters);

        WebMercatorModelPtr model = new WebMercatorModel (createParams);
        DgnDbStatus insertStatus = model->Insert();
        BeAssert (DgnDbStatus::Success == insertStatus);
        }
    else
        {
        BeAssert (false);
        }


#if defined (INSERT_MAPBOX_ALSO)
    RepositoryLinkPtr mapBoxStreetsLink = RepositoryLink::Create(*db.GetRealityDataSourcesModel(), nullptr, "Mapbox Streets");
    if (mapBoxStreetsLink.IsValid() && mapBoxStreetsLink->Insert().IsValid())
        {
        // set up the MapBox Streets properties Json.
        BentleyApi::Json::Value jsonParameters;
        jsonParameters[WebMercatorModel::json_providerName()] = MapBoxImageryProvider::prop_MapBoxProvider();
        jsonParameters[WebMercatorModel::json_groundBias()] = -1.0;
        jsonParameters[WebMercatorModel::json_transparency()] = 0.0;
        BentleyApi::Json::Value& mapBoxStreetsJson = jsonParameters[WebMercatorModel::json_providerData()];

        mapBoxStreetsJson[WebMercatorModel::json_mapType()] = (int)MapType::Street;
        WebMercatorModel::CreateParams createParams (db, mapBoxStreetsLink->GetElementId(), jsonParameters);

        WebMercatorModelPtr model = new WebMercatorModel (createParams);
        DgnDbStatus insertStatus = model->Insert();
        BeAssert (DgnDbStatus::Success == insertStatus);
        }
    else
        {
        BeAssert (false);
        }
#endif 

    db.SaveChanges();
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input)
    {
    if (!m_noGcs)
        SetGcsDef();

    // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
    RootModelConverter::RootModelSpatialParams params(m_params);
    params.SetKeepHostAlive(true);
    params.SetInputFileName(input);

    auto db = OpenExistingDgnDb(output);
    ASSERT_TRUE(db.IsValid());

    if (!m_opts.m_useTiledConverter)
        {
        TestRootModelCreator creator(params, this);
        params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());
        creator.SetWantDebugCodes(true);
        creator.SetDgnDb(*db);
        creator.SetIsUpdating(false);
        creator.AttachSyncInfo();
        ASSERT_EQ(BentleyApi::SUCCESS, creator.InitRootModel());
        if (nullptr != m_verifier)
            creator.AddSchemaImportVerifier(*m_verifier);
        creator.MakeSchemaChanges();
        ASSERT_FALSE(creator.WasAborted());
        ASSERT_EQ(TestRootModelCreator::ImportJobCreateStatus::Success, creator.InitializeJob());
        creator.Process();
        DgnDbR db = creator.GetDgnDb();
        AddExternalDataModels (db);
        ASSERT_FALSE(creator.WasAborted());
        m_count = creator.GetElementsConverted();
        }
    else
        {
        TiledFileConverter creator(params);
        params.SetBridgeRegSubKey(TiledFileConverter::GetRegistrySubKey());
        creator.SetWantDebugCodes(true);
        creator.SetDgnDb(*db);
        creator.SetIsUpdating(false);
        creator.AttachSyncInfo();
        ASSERT_EQ(BentleyApi::SUCCESS, creator.InitRootModel());
        creator.MakeSchemaChanges();
        ASSERT_FALSE(creator.WasAborted());
        ASSERT_EQ(TiledFileConverter::ImportJobCreateStatus::Success, creator.InitializeJob());
        creator.ConvertRootModel();
        for (BentleyApi::BeFileName const& tileName : m_opts.m_tiles)
            creator.ConvertTile(tileName);
        creator.FinishedConversion();
        ASSERT_FALSE(creator.WasAborted());
        m_count = creator.GetElementsConverted();
        }

    db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::DoUpdate(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input, bool expectFailure, bool expectUpdate)
    {
    // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
    RootModelConverter::RootModelSpatialParams params(m_params);
    params.SetKeepHostAlive(true);
    params.SetInputFileName(input);
    auto db = OpenExistingDgnDb(output);
    ASSERT_TRUE(db.IsValid());
    bool hadAnyChanges = false;
    time_t mtime;
    BentleyApi::BeFileName::GetFileTime(nullptr, nullptr, &mtime, output);

    if (!m_opts.m_useTiledConverter)
        {
        RootModelConverter updater(params);
        params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());
        updater.SetWantDebugCodes(true);
        updater.SetDgnDb(*db);
        updater.SetIsUpdating(true);
        updater.AttachSyncInfo();
        ASSERT_EQ(BentleyApi::SUCCESS, updater.InitRootModel());
        updater.MakeSchemaChanges();
        ASSERT_EQ(expectFailure, updater.WasAborted());
        if (!updater.WasAborted())
            {
            ASSERT_EQ(RootModelConverter::ImportJobLoadStatus::Success, updater.FindJob());
            updater.Process();
            ASSERT_EQ(expectFailure, updater.WasAborted());
            m_count = updater.GetElementsConverted();
            hadAnyChanges = updater.HadAnyChanges();
            }
        }
    else
        {
        TiledFileConverter updater(params);
        params.SetBridgeRegSubKey(TiledFileConverter::GetRegistrySubKey());
        updater.SetWantDebugCodes(true);
        updater.SetDgnDb(*db);
        updater.SetIsUpdating(true);
        updater.AttachSyncInfo();
        ASSERT_EQ(BentleyApi::SUCCESS, updater.InitRootModel());
        updater.MakeSchemaChanges();
        ASSERT_EQ(expectFailure, updater.WasAborted());
        if (!updater.WasAborted())
            {
            ASSERT_EQ(RootModelConverter::ImportJobLoadStatus::Success, updater.FindJob());
            updater.ConvertRootModel();
            for (BentleyApi::BeFileName const& tileName : m_opts.m_tiles)
                updater.ConvertTile(tileName);
            updater.FinishedConversion();
            ASSERT_EQ(expectFailure, updater.WasAborted());
            m_count = updater.GetElementsConverted();
            hadAnyChanges = updater.HadAnyChanges();
            }
        }
    db->SaveChanges();
    if (!expectFailure)
        ASSERT_EQ(hadAnyChanges, expectUpdate);
    if (!hadAnyChanges)
        output.SetFileTime(nullptr, &mtime);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::LineUpFiles(BentleyApi::WCharCP outputDgnDbFileName, BentleyApi::WCharCP inputV8FileName, bool doConvert)
    {
    MakeWritableCopyOf(m_v8FileName, inputV8FileName);
    m_dgnDbFileName = GetOutputFileName(outputDgnDbFileName);
    DeleteExistingDgnDb(m_dgnDbFileName);
    MakeWritableCopyOf(m_dgnDbFileName, m_seedDgnDbFileName, m_dgnDbFileName.GetFileNameAndExtension().c_str());
    auto syncFile(SyncInfo::GetDbFileName(m_seedDgnDbFileName));
    BentleyApi::BeFileName outSyncFile;
    MakeWritableCopyOf(outSyncFile, syncFile, SyncInfo::GetDbFileName(m_dgnDbFileName).GetFileNameAndExtension().c_str());
    if (doConvert)
        DoConvert(m_dgnDbFileName, m_v8FileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::MakeCopyOfFile(BentleyApi::BeFileNameR refV8File, BentleyApi::WCharCP suffix)
    {
    refV8File = BentleyApi::BeFileName(BentleyApi::BeFileName::FileNameParts::DevAndDir, m_v8FileName.c_str());
    refV8File.AppendToPath(m_v8FileName.GetFileNameWithoutExtension().c_str());
    refV8File.append(suffix);
    refV8File.append(L".");
    refV8File.append(m_v8FileName.GetExtension().c_str());
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(m_v8FileName.c_str(), refV8File.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::CreateAndAddV8Attachment(BentleyApi::BeFileNameR refV8File, int32_t num, bool useOffsetForElement)
    {
    MakeCopyOfFile(refV8File, BentleyApi::WPrintfString(L"-Ref%d", num).c_str());

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    int offset = useOffsetForElement ? num : 0;
    v8editor.AddAttachment(refV8File, nullptr, Bentley::DPoint3d::FromXYZ((double) offset * 1000, (double) offset * 1000, 0));
    v8editor.Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::AddV8Level(BentleyApi::Utf8CP levelname, BentleyApi::BeFileNameCR v8FileNameIn)
    {
    BentleyApi::BeFileName v8FileName = !v8FileNameIn.empty() ? v8FileNameIn : m_v8FileName;
    V8FileEditor v8editor;
    v8editor.Open(v8FileName);
    DgnV8Api::FileLevelCache& lc = *dynamic_cast<DgnV8Api::FileLevelCache*>(&v8editor.m_file->GetLevelCacheR());
    auto level = lc.CreateLevel(Bentley::WString(levelname, Bentley::BentleyCharEncoding::Utf8).c_str(), LEVEL_NULL_CODE, DGNV8_LEVEL_NULL_ID);
    ASSERT_TRUE(level.IsValid());
    ASSERT_EQ(DgnV8Api::LevelCacheErrorCode::None, lc.Write());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::SetV8LevelColor(BentleyApi::Utf8CP levelname, uint32_t v8ColorId, BentleyApi::BeFileNameCR v8FileNameIn)
    {
    BentleyApi::BeFileName v8FileName = !v8FileNameIn.empty() ? v8FileNameIn : m_v8FileName;
    V8FileEditor v8editor;
    v8editor.Open(v8FileName);
    DgnV8Api::FileLevelCache& lc = *dynamic_cast<DgnV8Api::FileLevelCache*>(&v8editor.m_file->GetLevelCacheR());
    auto level = lc.GetLevelByName(Bentley::WString(levelname, Bentley::BentleyCharEncoding::Utf8).c_str());
    ASSERT_TRUE(level.IsValid());
    DgnV8Api::EditLevelHandle eh(level);
    eh.SetByLevelColor(DgnV8Api::LevelDefinitionColor(v8ColorId, v8editor.m_file.get()));
    ASSERT_EQ(DgnV8Api::LevelCacheErrorCode::None, lc.Write());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkEqualAngles(BentleyApi::YawPitchRollAnglesCR lhs, BentleyApi::YawPitchRollAnglesCR rhs)
    {
    ASSERT_TRUE(BentleyApi::Angle::NearlyEqual(lhs.GetYaw().Radians(), rhs.GetYaw().Radians()));
    ASSERT_TRUE(BentleyApi::Angle::NearlyEqual(lhs.GetPitch().Radians(), rhs.GetPitch().Radians()));
    ASSERT_TRUE(BentleyApi::Angle::NearlyEqual(lhs.GetRoll().Radians(), rhs.GetRoll().Radians()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::TestElementChanges(BentleyApi::BeFileNameCR rootV8FileName, BentleyApi::BeFileNameCR editV8FileName, size_t nModelsExpected)
    {
    DgnV8Api::ElementId editV8ElementId;
    DgnV8Api::ModelId editV8ModelId;
    if (true)
        {
        //  Add an element to the V8 file
        V8FileEditor v8editor;
        v8editor.Open(editV8FileName);
        v8editor.AddLine(&editV8ElementId);
        editV8ModelId = v8editor.m_defaultModel->GetModelId();
        }

    //  Run the Updater
    DoUpdate(m_dgnDbFileName, rootV8FileName);
    ASSERT_EQ(1, m_count) << L"Update should have found the one element that I added.";

    //  Count the models
    if (true)
        {
        SyncInfoReader syncInfo;
        syncInfo.AttachToDgnDb(m_dgnDbFileName);
        SyncInfo::ModelIterator models(*syncInfo.m_dgndb, nullptr);
        int count = 0;
        for (SyncInfo::ModelIterator::Entry entry = models.begin(); entry != models.end(); ++entry)
            ++count;
        ASSERT_EQ(nModelsExpected, count);
        }

    DgnElementId dgnDbElementId;
    BentleyApi::Placement3d wasPlacement;
    SyncInfo::V8FileSyncInfoId editV8FileSyncInfoId;
    SyncInfo::V8ModelSyncInfoId editV8ModelSyncInfoId;
    if (true)
        {
        //  Verify that Updater found the new element
        SyncInfoReader syncInfo;
        syncInfo.AttachToDgnDb(m_dgnDbFileName);

        syncInfo.MustFindFileByName(editV8FileSyncInfoId, editV8FileName);

        syncInfo.MustFindModelByV8ModelId(editV8ModelSyncInfoId, editV8FileSyncInfoId, editV8ModelId);

        syncInfo.MustFindElementByV8ElementId(dgnDbElementId, editV8ModelSyncInfoId, editV8ElementId);

        DgnElementCPtr dgnDbElement = syncInfo.m_dgndb->Elements().GetElement(dgnDbElementId);
        ASSERT_TRUE(dgnDbElement.IsValid());

        GeometrySource3dCP geomElement = dgnDbElement->ToGeometrySource3d();
        ASSERT_NE(nullptr, geomElement);
        wasPlacement = geomElement->GetPlacement();

        ASSERT_TRUE(wasPlacement.GetOrigin().AlmostEqual(BentleyApi::DPoint3d::From(0, 0, 0))) << L"V8 line should start at 0,0,0";
        EXPECT_DOUBLE_EQ(wasPlacement.GetElementBox().GetRight(), 1.0) << L"V8 line should be 1 meter long";
        }

    //  Run the Updater again => should be a NOP
    DoUpdate(m_dgnDbFileName, rootV8FileName, false, false);
    ASSERT_EQ(0, m_count) << L"Update should not have found any more changes.";

    if (true)
        {
        // Modify the V8 element
        V8FileEditor v8editor;
        v8editor.Open(editV8FileName);
        DgnV8Api::EditElementHandle v8Eh(editV8ElementId, v8editor.m_defaultModel);
        ASSERT_TRUE(v8Eh.IsValid());
        Bentley::Transform xlat;
        xlat.InitIdentity();
        xlat.SetTranslation(Bentley::DPoint3d::From(10, 10, 10));
        ASSERT_EQ(BentleyApi::SUCCESS, v8Eh.GetDisplayHandler()->ApplyTransform(v8Eh, DgnV8Api::TransformInfo(xlat)));
        ASSERT_EQ(BentleyApi::SUCCESS, v8Eh.ReplaceInModel(v8Eh.GetElementRef()));
        }

    //  Update
    DoUpdate(m_dgnDbFileName, rootV8FileName);
    ASSERT_EQ(1, m_count) << L"Update should have found only the one change.";

    if (true)
        {
        //  Verify that the DgnDb element was updated as expected
        SyncInfoReader syncInfo;
        syncInfo.AttachToDgnDb(m_dgnDbFileName);

        DgnElementId dgnDbElementAfter;
        syncInfo.MustFindElementByV8ElementId(dgnDbElementAfter, editV8ModelSyncInfoId, editV8ElementId);
        ASSERT_EQ(dgnDbElementId, dgnDbElementAfter) << L"modified V8 element should still be mapped to the same DgnDb element";

        DgnElementCPtr dgnDbElement = syncInfo.m_dgndb->Elements().GetElement(dgnDbElementId);
        GeometrySource3dCP geomElement = dgnDbElement->ToGeometrySource3d();
        ASSERT_NE(nullptr, geomElement);
        BentleyApi::Placement3d placementAfter = geomElement->GetPlacement();
        ASSERT_FALSE(placementAfter.GetOrigin().IsEqual(wasPlacement.GetOrigin())) << L"Expected placement origin to change";
        checkEqualAngles(placementAfter.GetAngles(), wasPlacement.GetAngles());
        ASSERT_TRUE(wasPlacement.GetElementBox().IsEqual(placementAfter.GetElementBox(), 1.0e-10)) << L"Did not expect geometry to change";
        }

    //  Run the Updater again => should be a NOP
    DoUpdate(m_dgnDbFileName, rootV8FileName, false, false);
    ASSERT_EQ(0, m_count) << L"Update should not have found any more changes.";

    if (true)
        {
        // Delete the V8 element
        V8FileEditor v8editor;
        v8editor.Open(editV8FileName);
        DgnV8Api::EditElementHandle v8Eh(editV8ElementId, v8editor.m_defaultModel);
        ASSERT_TRUE(v8Eh.IsValid());
        ASSERT_EQ(BentleyApi::SUCCESS, v8Eh.DeleteFromModel());
        }

    //  Update
    DoUpdate(m_dgnDbFileName, rootV8FileName);
    ASSERT_EQ(1, m_count) << L"Update should have found the one deletion.";

    if (true)
        {
        //  Verify that the DgnDb element was deleted from both the db and syncinfo
        SyncInfoReader syncInfo;
        syncInfo.AttachToDgnDb(m_dgnDbFileName);

        DgnElementId dgnDbElementAfter;
        syncInfo.MustFindElementByV8ElementId(dgnDbElementAfter, editV8ModelSyncInfoId, editV8ElementId, /*>>*/0/*<<*/);
        ASSERT_TRUE(!dgnDbElementAfter.IsValid()) << L"V8 element was deleted => we should not find a mapping to a DgnDb element";

        DgnElementCPtr dgnDbElement = syncInfo.m_dgndb->Elements().GetElement(dgnDbElementId);
        ASSERT_TRUE(!dgnDbElement.IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ConverterTestBaseFixture::FindV8ElementInDgnDb(DgnDbR db, DgnV8Api::ElementId eV8Id, uint8_t dgnIndex)
    {
    SubjectCPtr jobSubject = GetFirstJobSubject(db);
    if (!jobSubject.IsValid())
        return nullptr;

    DgnCode code(db.CodeSpecs().QueryCodeSpecId("DgnV8"), jobSubject->GetElementId(), BentleyApi::Utf8PrintfString("DgnV8-%d-%ld", dgnIndex, eV8Id));
    return db.Elements().GetElement(db.Elements().QueryElementIdByCode(code));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr ConverterTestBaseFixture::GetFirstJobSubject(DgnDbR db)
    {
    auto childids = db.Elements().GetRootSubject()->QueryChildren();
    for (auto childid : childids)
        {
        auto subj = db.Elements().Get<Subject>(childid);
        if (subj.IsValid() && JobSubjectUtils::IsJobSubject(*subj))
            return subj;
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
DefinitionModelPtr ConverterTestBaseFixture::GetJobDefinitionModel(DgnDbR db)
    {
    auto jobsubj = GetFirstJobSubject(db);
    if (!jobsubj.IsValid())
        return nullptr;

    Utf8PrintfString partitionName("Definition Model For %s", jobsubj->GetDisplayLabel().c_str());
    DgnCode partitionCode = DefinitionPartition::CreateCode(*jobsubj, partitionName.c_str());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    DgnModelId defModelId = DgnModelId(partitionId.GetValueUnchecked());
    if (defModelId.IsValid())
        return db.Models().Get<DefinitionModel>(defModelId);

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr ConverterTestBaseFixture::GetJobHierarchySubject(DgnDbR db)
    {
    auto jobsubj = GetFirstJobSubject(db);
    if (!jobsubj.IsValid())
        return nullptr;
    auto childids = jobsubj->QueryChildren();
    for (auto childid : childids)
        {
        auto subj = db.Elements().Get<Subject>(childid);
        if (subj.IsValid() && subj->GetSubjectJsonProperties(Subject::json_Model()).GetMember("Type") == "Hierarchy")
            return subj;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr ConverterTestBaseFixture::GetReferencesChildSubjectOf(SubjectCR parent)
    {
    auto childids = parent.QueryChildren();
    for (auto childid : childids)
        {
        auto subj = parent.GetDgnDb().Elements().Get<Subject>(childid);
        if (subj.IsValid() && subj->GetSubjectJsonProperties(Subject::json_Model()).GetMember("Type") == "References")
            return subj;
        }
    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId ConverterTestBaseFixture::getBisClassId(DgnDbR db, Utf8CP className)
    {
    return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, className));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ConverterTestBaseFixture::findFirstElementByClass(DgnDbR db, DgnClassId classId)
    {
    auto stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId from " BIS_SCHEMA(BIS_CLASS_Element) " WHERE ECClassId=?");
    stmt->BindId(1, classId);
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnElementId();

    return stmt->GetValueId<DgnElementId>(0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::countModels(DgnDbR db, int ndrawings_expected, int nspatial_expected)
    {
    int ndrawings=0;
    int nspatial=0;
    for (auto const& modelEntry : db.Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel)))
        {
        BentleyApi::Dgn::DgnModelPtr model = db.Models().GetModel(modelEntry.GetModelId());
        if (model->Is2dModel())
            ++ndrawings;
        else
            ++nspatial;
        }

    if (ndrawings_expected >= 0)
        {
        ASSERT_EQ(ndrawings_expected, ndrawings);
        }

    if (nspatial_expected >= 0)
        {
        ASSERT_EQ(nspatial_expected, nspatial);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::countElements(DgnModel& model, int expected)
    {
    auto stmt = model.GetDgnDb().GetPreparedECSqlStatement("SELECT COUNT(*) from " BIS_SCHEMA(BIS_CLASS_Element) " WHERE Model.Id=?");
    stmt->BindId(1, model.GetModelId());
    stmt->Step();
    EXPECT_EQ(expected, stmt->GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTestBaseFixture::countElementsInModelByClass(DgnModel& model, DgnClassId classId, int expected)
    {
    auto stmt = model.GetDgnDb().GetPreparedECSqlStatement("SELECT COUNT(*) from " BIS_SCHEMA(BIS_CLASS_Element) " WHERE Model.Id=? AND ECClassId=?");
    stmt->BindId(1, model.GetModelId());
    stmt->BindId(2, classId);
    stmt->Step();
    EXPECT_EQ(expected, stmt->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2014
//---------------------------------------------------------------------------------------
int ConverterTestBaseFixture::SelectCountFromECClass(DgnDbR db, Utf8CP className)
    {
    if (!className || !*className)
        return -1;

    Utf8PrintfString sql("SELECT COUNT(*) FROM %s", className);

    EC::ECSqlStatement statement;
    EC::ECSqlStatus status = statement.Prepare(db, sql.c_str());
    if (EC::ECSqlStatus::Success != status)
        return -1;

    if (BE_SQLITE_ROW != statement.Step())
        return -1;

    return statement.GetValueInt(0);
    }
