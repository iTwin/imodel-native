/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>
#include "ConverterTestsBaseFixture.h"
#include <iModelBridge/iModelBridgeSacAdapter.h>
#include <Bentley/BeTimeUtilities.h>
#include <iModelBridge/iModelBridge.h>

struct CrashControlMonitor : DgnV8::Converter::Monitor
    {
    enum class ModelAction {Nothing, Save, Crash};
    BentleyApi::bmap<BentleyApi::WString, ModelAction> m_modelsAndActions;

    void _OnModelConverted(ResolvedModelMapping const& v8mm, uint32_t changeCount) override
        {
        auto const& iModelAndAction = m_modelsAndActions.find(v8mm.GetV8Model().GetModelNameCP());
        if (iModelAndAction == m_modelsAndActions.end())
            return;
        if (iModelAndAction->second == ModelAction::Save)
            {
            v8mm.GetDgnModel().GetDgnDb().SaveChanges();
            }
        else if (iModelAndAction->second == ModelAction::Crash)
            {
            *((int*)nullptr) = 0;
            }
        }
    };

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct ConverterTests : public ConverterTestBaseFixture
    {
    void CheckNoDupXsas(DgnDbR, Utf8CP kind);
    void ConvertWithCrash(DgnDbR, CrashControlMonitor&);
    bool ConvertWithCrashCaught(DgnDbR, CrashControlMonitor&);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, ElementCRUD)
    {
    LineUpFiles(L"ElementCRUD.bim", L"Test3d.dgn", true); // creates Test1.idgndb from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    TestElementChanges(m_v8FileName, m_v8FileName, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, ElementCRUDInAttachment)
    {
    LineUpFiles(L"ElementCRUDInAttachment.bim", L"Test3d.dgn", true); // creates TestAddRef.idgndb from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";

    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File);

    TestElementChanges(m_v8FileName, refV8File, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, ElementCRUDTiled)
    {
    m_opts.m_useTiledConverter = true;
    LineUpFiles(L"ElementCRUDTiled.bim", L"Test3d.dgn", true); // creates Test1.idgndb from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    TestElementChanges(m_v8FileName, m_v8FileName, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, ElementCRUDInTile)
    {
    m_opts.m_useTiledConverter = true;
    LineUpFiles(L"ElementCRUDInTile.bim", L"Test3d.dgn", true); // creates TestAddRef.idgndb from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";

    BentleyApi::BeFileName tileFile;
    MakeCopyOfFile(tileFile, L"-Tile1");

    m_opts.m_tiles.push_back(tileFile);
    m_opts.m_expectModelToBeMappedIn = false; // *** WIP_SYNCINFO -- We defined the syncinfo "Model" table such that there can be only 1 V8 model mapped to a given DgnDb model, so we can't map all tiles to the one target model

    TestElementChanges(m_v8FileName, tileFile, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, DuplicateTile)
    {
    LineUpFiles(L"tiledMultip.bim", L"Test3d.dgn", false);


    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId elementId;
    v8editor.AddLine(&elementId);
    v8editor.Save();

    // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
    RootModelConverter::RootModelSpatialParams params(m_params);

    params.SetInputFileName(m_v8FileName);
    params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());

    TiledFileConverter creator(params);

    auto db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db.IsValid());
    creator.SetDgnDb(*db);
    creator.SetIsUpdating(false);
    creator.AttachSyncInfo();
    ASSERT_EQ(BentleyApi::SUCCESS, creator.InitRootModel());
    creator.MakeSchemaChanges();
    ASSERT_FALSE(creator.WasAborted());
    creator.InitializeJob();
    creator.ConvertRootModel();
    creator.ConvertTile(m_v8FileName);
    creator.FinishedConversion();
    ASSERT_TRUE(!creator.WasAborted());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, ColorMap)
    {
    LineUpFiles(L"ProjectProperties.bim", L"Test3d.dgn", false);

    V8FileEditor editor;
    editor.Open(m_v8FileName);
    Bentley::DgnColorMapP colorMap = DgnV8Api::DgnColorMap::GetForFile(editor.m_file.get());
    EXPECT_TRUE(nullptr != colorMap);
    DoConvert(m_dgnDbFileName, m_v8FileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, EmbedDir)
    {
#define EMBEDDEDFILE  "chkmrk.bmp"
#define EMBEDDEDFILEW L"chkmrk.bmp"

    LineUpFiles(L"EmbeddDir.bim", L"Test3d.dgn", false);
    BentleyApi::BeFileName imageFile;
    MakeWritableCopyOf(imageFile, EMBEDDEDFILEW);
    m_wantCleanUp = false;

    BentleyApi::BeFileName embedDir = GetOutputDir();
    embedDir.AppendToPath(L"DirToEmbed");
    BentleyApi::BeFileName::CreateNewDirectory(embedDir.c_str());
    BentleyApi::BeFileName embedFile = embedDir;
    embedFile.AppendToPath(EMBEDDEDFILEW);
    BeFileName::BeMoveFile(imageFile.c_str(), embedFile.c_str());
    uint64_t expectedFileSize;
    ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::GetFileSize(expectedFileSize, embedFile.c_str()));

    m_params.SetEmbedDir(embedDir);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    DbEmbeddedFileTable::Iterator iterator = db->EmbeddedFiles().MakeIterator();
    ASSERT_EQ(1, iterator.QueryCount());
    DbEmbeddedFileTable::Iterator::Entry entry = *iterator.begin();
    EXPECT_STREQ(EMBEDDEDFILE, entry.GetDescriptionUtf8());
    EXPECT_TRUE(expectedFileSize == entry.GetFileSize());

    // Verify file can be exported
    if (imageFile.DoesPathExist())
        imageFile.BeDeleteFile();
    Utf8String localFileName(imageFile.c_str());
    ASSERT_TRUE(BE_SQLITE_OK == db->EmbeddedFiles().Export(localFileName.c_str(), Utf8String(embedFile.c_str()).c_str()));
    uint64_t exportedFileSize;
    ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::GetFileSize(exportedFileSize, imageFile.c_str()));
    EXPECT_TRUE(expectedFileSize == exportedFileSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void createFiles(BentleyApi::BeFileNameCR root)
    {
    if (!root.DoesPathExist())
        BentleyApi::BeFileName::CreateNewDirectory(root.c_str());

    BentleyApi::BeFileName child1;
    child1 = root;
    child1.AppendToPath(L"Child1");

    if (!child1.DoesPathExist())
        BentleyApi::BeFileName::CreateNewDirectory(child1.c_str());

    BeFile file0, file1;
    BentleyApi::BeFileName fileName0, fileName1;

    fileName0 = root;
    fileName0.AppendToPath(L"File0.txt");
    EXPECT_EQ(BeFileStatus::Success, file0.Create(fileName0, true));
    fileName1 = child1;
    fileName1.AppendToPath(L"File1.log");
    EXPECT_EQ(BeFileStatus::Success, file1.Create(fileName1, true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, EmbeddDir_SubDir)
    {
    LineUpFiles(L"EmbeddDir_SubDir.bim", L"Test3d.dgn", false);
    BentleyApi::BeFileName imageFile;
    MakeWritableCopyOf(imageFile, L"chkmrk.bmp");

    BentleyApi::BeFileName embedDir = GetOutputDir();
    embedDir.AppendToPath(L"DirToEmbed");
    BentleyApi::BeFileName::CreateNewDirectory(embedDir.c_str());
    createFiles(embedDir);
    m_params.SetEmbedDir(embedDir);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    EXPECT_EQ(1, db->EmbeddedFiles().MakeIterator().QueryCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, EmbeddDir_empty)
    {
    LineUpFiles(L"EmbeddDir_empty.bim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    BentleyApi::BeFileName embedDir = GetOutputDir();
    embedDir.AppendToPath(L"DirToEmbed");
    BentleyApi::BeFileName::CreateNewDirectory(embedDir.c_str());

    m_params.SetEmbedDir(embedDir);

    DoConvert(m_dgnDbFileName, m_v8FileName);
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    EXPECT_EQ(0, db->EmbeddedFiles().MakeIterator().QueryCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, ExpirationDate)
    {
    LineUpFiles(L"expirationdate.bim", L"filewithExpiryNotExpired.i.dgn", false);

    double actualExpiry = 2553361200000.0000; /* i.e. 30/11/2050 */ // actual value is in unixmilis
    DateTime expirationDate;
    DateTime::FromUnixMilliseconds(expirationDate, actualExpiry);

    if (true)
        {
        // verify expiration date exists in Dgn file.
        V8FileEditor editor;
        editor.Open(m_v8FileName);
        DgnFileP dgnv8File = editor.m_file.get();
        EXPECT_TRUE(dgnv8File->IsLoaded());

        Bentley::DgnPlatform::DgnFileLicense license;
        dgnFileObj_getCurrentLicense(&license, dgnv8File);
        EXPECT_EQ(actualExpiry, license.expiry) << L"Expiry date doesnot match";
        }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    ASSERT_TRUE(dgnProj->IsDbOpen());

    BentleyApi::BeSQLite::Statement statement;
    ASSERT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK, statement.Prepare(*dgnProj, "Select strData FROM be_Prop WHERE Name='ExpirationDate'"));
    ASSERT_EQ(statement.Step(), BentleyApi::BeSQLite::DbResult::BE_SQLITE_ROW);
    EXPECT_STREQ(statement.GetValueText(0), expirationDate.ToUtf8String().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
//Covers GetAzimuth(), GetLatitude() and GetLongitude() in DgnDb
TEST_F(ConverterTests, GetCoordinateSystemProperties)
    {
    m_noGcs = true;
    LineUpFiles(L"GeoCoordinateSystem.bim", L"GeoCoordinateSystem.i.dgn", true);

    DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    ASSERT_TRUE(dgnProj->IsDbOpen());

    BentleyApi::Dgn::DgnGCS* dgnGCS = dgnProj->GeoLocation().GetDgnGCS();
    ASSERT_NE(nullptr, dgnGCS);

    double azimuth = (dgnGCS != nullptr) ? dgnGCS->GetAzimuth() : 0.0;
    double azimuthExpected = 178.29128626108181;
    double eps = 0.0001;
    EXPECT_TRUE(fabs(azimuthExpected - azimuth) < eps) << "Expected different azimuth ";

    BentleyApi::GeoPoint gp;
    dgnProj->GeoLocation().LatLongFromXyz(gp, BentleyApi::DPoint3d::FromZero());

    double const latitudeExpected = 42.3412;
    EXPECT_TRUE(fabs(latitudeExpected - gp.latitude) < eps) << "Expected different latitude ";

    double const longitudeExpected = -71.0805;
    EXPECT_TRUE(fabs(longitudeExpected - gp.longitude) < eps) << "Expected different longitude ";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConverterTests, GCSNoChange)
    {
    LineUpFiles(L"GeoCoordinateSystem.bim", L"GeoCoordinateSystem.i.dgn", true);
    DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    auto originalExtents = dgnProj->GeoLocation().GetProjectExtents();

    dgnProj->CloseDb();
    DoUpdate(m_dgnDbFileName, m_v8FileName, false, false);

    dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    auto updatedExtents = dgnProj->GeoLocation().GetProjectExtents();

    ASSERT_TRUE(updatedExtents.IsEqual(originalExtents));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConverterTests, NoGCSNoChange)
    {
    LineUpFiles(L"NoGCS.bim", L"Test3d.dgn", true);
    DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    auto originalExtents = dgnProj->GeoLocation().GetProjectExtents();

    dgnProj->CloseDb();
    DoUpdate(m_dgnDbFileName, m_v8FileName, false, false);

    dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    auto updatedExtents = dgnProj->GeoLocation().GetProjectExtents();

    ASSERT_TRUE(updatedExtents.IsEqual(originalExtents));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
// Importing DGN file that does not have a GCS. In that case, specify �inputgcs=lat,lng,angle in order to position the input DGN data in the BIM.
TEST_F(ConverterTests, InputGCS)
    {
    LineUpFiles(L"inputGCS.bim", L"Test3d.dgn", false);
    m_wantCleanUp = false;
    m_noGcs = true;

    iModelBridge::GCSDefinition gcsDef;
    gcsDef.m_isValid = true;
    gcsDef.m_originUors.Zero();
    gcsDef.m_azimuthAngle = 0.0;
    gcsDef.m_geoPoint.latitude = 40.06569747222222; // coordinates of Bentley in Exton, PA
    gcsDef.m_geoPoint.longitude = -75.68858161111112;
    gcsDef.m_geoPoint.elevation = 0;
    m_params.SetInputGcsDefinition(gcsDef);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    ASSERT_TRUE(dgnProj->IsDbOpen());

    BentleyApi::Dgn::DgnGCSP gcs = dgnProj->GeoLocation().GetDgnGCS();
    ASSERT_NE(nullptr, gcs);

    BentleyApi::GeoPoint gpt;
    gcs->LatLongFromCartesian(gpt, BentleyApi::DPoint3d::FromZero());
    ASSERT_NEAR(gpt.latitude, 40.0, 1.0);
    ASSERT_NEAR(gpt.longitude, -75.0, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
// input DGN File that does have a GCS, but it�s not the one we want for the BIM. In that case, specify --outputgcs=csname, and the converter will
// a) create the BIM with the specified GCS.
// b) reproject the input DGN into it.
TEST_F(ConverterTests, OutputGCS)
    {
    m_noGcs = true;
    LineUpFiles(L"outputGCS.bim", L"GeoCoordinateSystem.i.dgn", false);

    iModelBridge::GCSDefinition gcsDef;
    gcsDef.m_isValid = true;
    gcsDef.m_originUors.Zero();
    gcsDef.m_azimuthAngle = 0.0;
    gcsDef.m_geoPoint.latitude = 40.06569747222222; // coordinates of Bentley in Exton, PA
    gcsDef.m_geoPoint.longitude = -75.68858161111112;
    gcsDef.m_geoPoint.elevation = 0;
    m_params.SetOutputGcsDefinition(gcsDef);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    ASSERT_TRUE(dgnProj->IsDbOpen());

    BentleyApi::Dgn::DgnGCSP gcs = dgnProj->GeoLocation().GetDgnGCS();
    ASSERT_NE(nullptr, gcs);

    BentleyApi::GeoPoint gpt;
    gcs->LatLongFromCartesian(gpt, BentleyApi::DPoint3d::FromZero());
    ASSERT_NEAR(gpt.latitude, 40.0, 1.0);
    ASSERT_NEAR(gpt.longitude, -75.0, 1.0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
static RefCountedPtr<GeometricModel3d> FindModel(DgnDbCR db, Utf8StringCR modelName)
    {
    DgnModels& models = db.Models();

    for (ModelIteratorEntryCR entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel)))
        {
        BentleyApi::Dgn::DgnModelPtr model = models.GetModel(entry.GetModelId());
        if (!model.IsValid())
            continue;

        if (modelName.Equals(model->GetName().c_str()))
            return dynamic_cast<GeometricModel3dP>(model.get());
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateModelRange (DgnDbR db, Utf8Char* modelName, double x0, double y0, double z0, double x1, double y1, double z1)
    {
    BentleyApi::DRange3d expectedRange = BentleyApi::DRange3d::From(x0, y0, z0, x1, y1, z1);

    RefCountedPtr<GeometricModel3d> model = FindModel (db, modelName);
    ASSERT_TRUE (model.IsValid());
    BentleyApi::AxisAlignedBox3d modelRange = model->QueryElementsRange();

    // Compare ranges. Ignore z values because QueryModelRange adds some depth (z = 1.0 or -1.0), which is irrelevant for us.
    bool areEqual = true;
    if ((fabs(modelRange.low.x - expectedRange.low.x) > .01) ||
        (fabs(modelRange.low.y - expectedRange.low.y) > .01) ||
        (fabs(modelRange.high.x - expectedRange.high.x) > .01) ||
        (fabs(modelRange.high.y - expectedRange.high.y) > .01))
        areEqual = false;

    if (!areEqual)
        {
        Utf8String msg;
        msg.Sprintf("Model range is wrong. Model: %s\nExpected: {%lf %lf}{%lf %lf}\nActual:   {%lf %lf}{%lf %lf}",
                    modelName, x0, y0, x1, y1,
                    modelRange.low.x, modelRange.low.y, modelRange.high.x, modelRange.high.y);
        FAIL() << msg;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2019
//---------------+---------------+---------------+---------------+---------------+-------
static void ValidateProjectExtents(double x0, double y0, double z0, double x1, double y1, double z1, BentleyApi::AxisAlignedBox3d extents)
    {
    bool areEqual = true;
    if ((fabs(extents.low.x - x0) > 0.01) ||
        (fabs(extents.low.y - y0) > 0.01) ||
        (fabs(extents.low.z - z0) > 0.01) ||
        (fabs(extents.high.x - x1) > 0.01) ||
        (fabs(extents.high.y - y1) > 0.01) ||
        (fabs(extents.high.z - z1) > 0.01))
        areEqual = false;

    if (!areEqual)
        {
        Utf8String msg;
        msg.Sprintf("Project Extents are wrong. Expected: {%lf %lf %lf} {%lf %lf %lf}\nActual:   {%lf %lf %lf}{%lf %lf %lf}",
                    x0, y0, z0, x1, y1, z1,
                    extents.low.x, extents.low.y, extents.low.z, extents.high.x, extents.high.y, extents.high.z);
        FAIL() << msg;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
// We start with a DGN file that has the PA State Plane North GCS with a Helmert transform and a global origin.
// We pull in five different DGN files, each of which has the same Lat/Long data.
// The first three are PA State Plane North, but with different combinations of global origins and Helmert Transforms
//  (the objective for these first three is to make sure that we are correctly calculating the linear transform to use in these cases).
// The last two have a different GCS (including a different Dataum). They are reprojected, and because of the datum difference, the range is slightly different.
TEST_F(ConverterTests, GCSMultiFilesGOAndHelmert)
    {
    m_noGcs       = true;

    // Each of the first four files is in Pennsylvania State Plane north with NAD83 Datum. (PA83-N)
    // They each have a block with corners at -75,40 degress to -74,39 degrees.
    // This first one has a helmert transform and a non-centered Global Origin. Model Name WithGOAndHelmert
    LineUpFiles(L"outputGCS.bim", L"PennsylvaniaStatePlaneNorthWithGOAndHelmert.dgn", false);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    // This one has no helmert transform and a centered Global Origin. Model name CenterGO
    MakeWritableCopyOf (m_v8FileName, L"PennsylvaniaStatePlaneNorthCenterGO.dgn");
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // This first one no helmert transform and a non-centered Global Origin. Model name WithGO
    MakeWritableCopyOf (m_v8FileName, L"PennsylvaniaStatePlaneNorthWithGO.dgn");
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // This first one has a helmert transform and a centered Global Origin. Model name CenterGOWithHelmert
    MakeWritableCopyOf (m_v8FileName, L"PennsylvaniaStatePlaneNorthCenterGOWithHelmert.dgn");
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // These two files are in the Pennsylvania State Plane south with the NAD27 Datum (PA-2).
    // They each have a block with corners at -75,40 degress to -74,39 degrees, but in the NAD27 Datum.
    // The first one has a centered Global origin. Model name SouthCenterGO.
    MakeWritableCopyOf (m_v8FileName, L"PennsylvaniaStatePlaneSouthCenterGO.dgn");
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // The first one has a non-centered Global origin. Model name SouthWithGO.
    MakeWritableCopyOf (m_v8FileName, L"PennsylvaniaStatePlaneSouthWithGO.dgn");
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // NOTE: Because of the datum difference (NAD83 to NAD27), the models in the last two files DO NOT line up exactly with those in the first set of four. That is expected.

    // Compare model ranges
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE (db->IsDbOpen());
    ValidateModelRange (*db.get(), "WithGOAndHelmert", 174588.644, -357775.920, -0.0005, 289574.018, -230337.770, 0.0005);
    ValidateModelRange (*db.get(), "CenterGO", 174588.644, -357775.920, -0.0005, 289574.018, -230337.770, 0.0005);
    ValidateModelRange (*db.get(), "WithGO", 174588.644, -357775.920, -0.0005, 289574.018, -230337.770, 0.0005);
    ValidateModelRange (*db.get(), "CenterGOWithHelmert", 174588.644, -357775.920, -0.0005, 289574.018, -230337.770, 0.0005);

    ValidateModelRange (*db.get(), "SouthCenterGO", 174669.782, -357771.034, -0.0005, 289552.579, -230329.427, 0.0005);
    ValidateModelRange (*db.get(), "SouthWithGO", 174669.782, -357771.034, -0.0005, 289552.579, -230329.427, 0.0005);
    ValidateProjectExtents(174588.644, -357775.920, -0.0005, 289574.018, -230329.427, 0.0005, db->GeoLocation().GetProjectExtents());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
// The first dgn file (ExtonBlock) is in Pennsylvania State Plane South with a Helmert transform, and it has a non-centered global origin.
// The other three models are pulled in from BentleyCampus.dgn. Each has an Azimuthal Equal Area GCS made from placemarks.
// In this tests, they are each reprojected to ExtonBlock's GCS.
TEST_F(ConverterTests, GCSMultiFilesReprojectImport)
    {
    m_noGcs       = true;

    // Note: The root model is converted to a BIM Spatial model because it has a GCS, which the converter treats as evidence that it is not a drawing model, even though it is 2D.

    // ExtonBlock.dgn is a design file that has a block with the Pennsylvania State Plane South GCS, with a block from -75.7, 40.1 to -75.6, 40.0 degrees.
    // We will first convert it.
    LineUpFiles(L"outputGCS.bim", L"ExtonBlock.dgn", false);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    RootModelConverter::RootModelChoice rmc;

    // Now we will pull in the Bentley Campus models, one at a time. The first time we pull them in, it will be the default method (reprojection).
    MakeWritableCopyOf (m_v8FileName, L"BentleyCampus.dgn");
    // Import "Building One"
    rmc.SetModelName ("Building 1");
    m_params.SetRootModelChoice(rmc);
    m_params.SetGcsCalculationMethod (iModelBridge::GCSCalculationMethod::UseReprojection);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    // Import "TPB Building"
    rmc.SetModelName ("TPB Building");
    m_params.SetRootModelChoice(rmc);
    m_params.SetGcsCalculationMethod (iModelBridge::GCSCalculationMethod::UseReprojection);
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // Import "2D Site Data"
    rmc.SetModelName ("2D Site Data");
    m_params.SetRootModelChoice(rmc);
    m_params.SetGcsCalculationMethod (iModelBridge::GCSCalculationMethod::UseReprojection);
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // Compare model ranges
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE (db->IsDbOpen());

    ValidateModelRange (*db.get(), "2D Site Data", 775473.879, 82772.289, -0.0005, 776530.835, 83720.500, 0.0005);
    ValidateModelRange(*db.get(), "Building 1", 775953.178862, 83335.970702, -0.0005, 776088.567471, 83467.103977, 0.0005);
    ValidateModelRange (*db.get(), "TPB Building", 775885.795, 83307.686, -0.0005, 775960.121, 83398.667, 0.0005);
    ValidateProjectExtents(774782.254, 76257.877, -5.839, 783576.865, 87155.225, 20.159, db->GeoLocation().GetProjectExtents());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
// The first dgn file (ExtonBlock) is in Pennsylvania State Plane South with a Helmert transform, and it has a non-centered global origin.
// The other three models are pulled in from BentleyCampus.dgn. Each has an Azimuthal Equal Area GCS made from placemarks.
// In this tests, they are GCS Transformed to ExtonBlock's GCS, without scaling. The ranges end up slightly different.
TEST_F(ConverterTests, GCSMultiFilesGCSTransformNoScale)
    {
    m_noGcs       = true;
    m_wantCleanUp = false;

    // Note: The root model is converted to a BIM Spatial model because it has a GCS, which the converter treats as evidence that it is not a drawing model, even though it is 2D.

    // ExtonBlock.dgn is a design file that has a block with the Pennsylvania State Plane South GCS, with a block from -75.7, 40.1 to -75.6, 40.0 degrees.
    // We will first convert it.
    LineUpFiles(L"outputGCS.bim", L"ExtonBlock.dgn", false);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    RootModelConverter::RootModelChoice rmc;

    // Now we will pull in the Bentley Campus models, one at a time. The first time we pull them in, it will be the default method (reprojection).
    MakeWritableCopyOf (m_v8FileName, L"BentleyCampus.dgn");
    // Import "Building One"
    rmc.SetModelName ("Building 1");
    m_params.SetRootModelChoice(rmc);
    m_params.SetGcsCalculationMethod (iModelBridge::GCSCalculationMethod::UseGcsTransform);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    // Import "TPB Building"
    rmc.SetModelName ("TPB Building");
    m_params.SetRootModelChoice(rmc);
    m_params.SetGcsCalculationMethod (iModelBridge::GCSCalculationMethod::UseGcsTransform);
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // Import "2D Site Data"
    rmc.SetModelName ("2D Site Data");
    m_params.SetRootModelChoice(rmc);
    m_params.SetGcsCalculationMethod (iModelBridge::GCSCalculationMethod::UseGcsTransform);
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // Compare model ranges
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE (db->IsDbOpen());

    ValidateModelRange (*db.get(), "2D Site Data", 775473.879, 82772.278, -0.0005, 776530.847, 83720.506, 0.0005);
    ValidateModelRange (*db.get(), "Building 1", 775953.177, 83335.970, -0.0005, 776088.568, 83467.105, 0.0005);
    ValidateModelRange (*db.get(), "TPB Building", 775885.794, 83307.685, -0.0005, 775960.122, 83398.668, 0.0005);
    ValidateProjectExtents(774782.254, 76257.877, -5.839, 783576.865, 87155.225, 20.159, db->GeoLocation().GetProjectExtents());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
// The first dgn file (ExtonBlock) is in Pennsylvania State Plane South with a Helmert transform, and it has a non-centered global origin.
// The other three models are pulled in from BentleyCampus.dgn. Each has an Azimuthal Equal Area GCS made from placemarks.
// In this tests, they are GCS Transformed to ExtonBlock's GCS, with scaling. The ranges are nearly the same as the Reprojection case.
TEST_F(ConverterTests, GCSMultiFilesGCSTransformWithScale)
    {
    m_noGcs       = true;

    // Note: The root model is converted to a BIM Spatial model because it has a GCS, which the converter treats as evidence that it is not a drawing model, even though it is 2D.

    // ExtonBlock.dgn is a design file that has a block with the Pennsylvania State Plane South GCS, with a block from -75.7, 40.1 to -75.6, 40.0 degrees.
    // We will first convert it.
    LineUpFiles(L"outputGCS.bim", L"ExtonBlock.dgn", false);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    RootModelConverter::RootModelChoice rmc;

    // Now we will pull in the Bentley Campus models, one at a time. The first time we pull them in, it will be the default method (reprojection).
    MakeWritableCopyOf (m_v8FileName, L"BentleyCampus.dgn");
    // Import "Building One"
    rmc.SetModelName ("Building 1");
    m_params.SetRootModelChoice(rmc);
    m_params.SetGcsCalculationMethod (iModelBridge::GCSCalculationMethod::UseGcsTransformWithScaling);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    // Import "TPB Building"
    rmc.SetModelName ("TPB Building");
    m_params.SetRootModelChoice(rmc);
    m_params.SetGcsCalculationMethod (iModelBridge::GCSCalculationMethod::UseGcsTransformWithScaling);
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // Import "2D Site Data"
    rmc.SetModelName ("2D Site Data");
    m_params.SetRootModelChoice(rmc);
    m_params.SetGcsCalculationMethod (iModelBridge::GCSCalculationMethod::UseGcsTransformWithScaling);
    DoConvert (m_dgnDbFileName, m_v8FileName);

    // Compare model ranges
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE (db->IsDbOpen());

    ValidateModelRange (*db.get(), "2D Site Data", 775473.879, 82772.289, -0.0005, 776530.835, 83720.499, 0.0005);
    ValidateModelRange (*db.get(), "Building 1", 775953.179, 83335.971, -0.0005, 776088.567, 83467.104, 0.0005);
    ValidateModelRange (*db.get(), "TPB Building", 775885.795, 83307.686, -0.0005, 775960.121, 83398.667, 0.0005);
    ValidateProjectExtents(774782.254, 76257.877, -5.839, 783576.865, 87155.225, 20.159, db->GeoLocation().GetProjectExtents());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                           Sam.Wilson             01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static PhysicalModelPtr insertPhysicalModel(DgnDbR db, Utf8CP partitionName)
    {
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    PhysicalPartitionCPtr partition = PhysicalPartition::CreateAndInsert(*rootSubject, partitionName);
    EXPECT_TRUE(partition.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
static DgnCategoryCPtr insertSpatialCategory(DgnDbR db, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance = DgnSubCategory::Appearance(), DgnCategory::Rank rank = DgnCategory::Rank::User)
    {
    SpatialCategory category(db.GetDictionaryModel(), categoryName, rank);
    return category.Insert(appearance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void convertSomeElements(DgnDbR outputBim, Bentley::DgnFileR v8File, Bentley::DgnModelR rootModel)
    {
    // For purposes of this test, I will create a model and a category in the output BIM.
    //  In a real converter, you would may have created many models and categories.
    auto bimModelFoo = insertPhysicalModel(outputBim, "Foo");
    auto bimCategoryBar = insertSpatialCategory(outputBim, "Bar");

    //  Initialize the ConverterLibrary helper object. Do all this initialization once, before converting any elements.
    RootModelConverter::RootModelSpatialParams params;
    params.SetKeepHostAlive(true);
    // Call params.AddDrawingOrSheetFile to add files to be processed by ConvertAllDrawingsAndSheets
    ConverterLibrary cvt(outputBim, params);

    // If you choose to add model mappings as you go along as this test does below, then
    //  you must at least record file mappings up front, before you try to map in any levels or styles.
    cvt.RecordFileMapping(v8File);

    // If you plan to convert sheets and/or drawings, you must call this up front:
    auto subject = Subject::CreateAndInsert(*outputBim.Elements().GetRootSubject(), "ConverterTests_UseConverterAsLibrary");
    cvt.SetJobSubject(*subject);

    //  Set up mappings for levels and styles.
    //  You must record a mapping for every input V8 level that is used by elements that you plan to convert.
    //  The ConverterLibrary will not write to the output BIM, but it does need to know these mappings.
    //      Here is an example of defining a custom level->category mapping.
    cvt.RecordLevelMappingForModel(DGNV8_LEVEL_DEFAULT_LEVEL_ID, bimCategoryBar->GetDefaultSubCategoryId(), v8File);

    //      You don't have to define mappings one by one.
    //      Instead, you can ask the converter to convert all of the levels in the V8 file to BIM categories with the same names.
    //      If you do that, you must first tell the converter to convert linestyles and to create the "Default" category.
    cvt.ConvertAllLineStyles(v8File);
    cvt.ConvertAllSpatialLevels(v8File);

    //  Set up the source GCS -> BIM GCS transformation.
    //  To do that, I need to know the root model of the source data. In this test, I arbitrarily decide on the first one. A real converter would know the root model.
    SubjectPtr noJobSubject = Subject::Create(*outputBim.Elements().GetRootSubject(), "DummyJobSubject");
    JobSubjectUtils::InitializeProperties(*noJobSubject, "Dummy");

    cvt.SetRootModelAndSubject(rootModel, *noJobSubject);


    //  Convert spatial elements
    //      In this test, I step through all of the elements in all of the spatial models.
    //      A real converter could pick and choose which elements to convert.
    for (int i=0; i<v8File.GetLoadedDgnModelCount(); ++i)
        {
        auto v8Model = v8File.GetLoadedModelByIndex(i);

        if (!v8Model->Is3D() || v8Model->IsSheet())
            continue;

        ResolvedModelMapping modelMapping = cvt.RecordModelMapping(*v8Model, *bimModelFoo);

        for (auto v8El : *v8Model->GetGraphicElementsP())
            {
            BeAssert(v8El != nullptr);
            DgnV8Api::EditElementHandle v8eh(v8El);
            ElementConversionResults results = cvt.ConvertElement(v8eh);

            if (!results.m_element.IsValid())
                continue;       // the element's geometry could not be converted for some reason?!

            // Note that element(s) in results are just in memory. They have *not* been written to the output BIM.

            ASSERT_TRUE(results.m_element->GetModelId() == bimModelFoo->GetModelId()) << "The element should have been mapped into the BIM's Foo model";

            auto geom = results.m_element->ToGeometrySource();
            if (nullptr != geom)
                {
                ASSERT_TRUE(geom->GetCategoryId() == bimCategoryBar->GetCategoryId()) << "The element should have been assigned to the Bar category";

                // You can get the element's geomstream like this:
                //      GeometryStreamCR geomStream = geom->GetGeometryStream();
                //
                //  Note that the geometry has been transformed to meters.
                //
                //  You could create a GeometryBuilder on some other element, passing in this converted GeometryStream as the second argument
                //  in order to create a new element with the same geometry as this one.

                //  Note that if the original V8 element was a cell, then results will contain multiple elements.
                //      m_element represents the header
                //      m_childElements represents the children, each of which could have its own children.
                //  This is normally meant to be written to a BIM as an assembly.
                }

            // Show how you can convert a curve vector in isolation
            DgnV8Api::ICurvePathQuery* curveQuery = dynamic_cast<DgnV8Api::ICurvePathQuery*>(&v8eh.GetHandler());
            if (nullptr != curveQuery)
                {
                CurveVectorPtr v8Cv;
                ASSERT_EQ(0, curveQuery->GetCurveVector(v8eh, v8Cv));
                BentleyApi::CurveVectorPtr bimCv;
                Converter::ConvertCurveVector(bimCv, *v8Cv, nullptr);
                ASSERT_TRUE(bimCv.IsValid());
                // ConvertCurveVector does not transform the units.
                // You must do that explicitly.
                bimCv->TransformInPlace(modelMapping.GetTransform());
                }
            }
        }

    // Convert drawings and sheets
    cvt.ConvertAllDrawingsAndSheets();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, UseConverterAsLibrary)
    {
    //  Get the output BIM.
    //  A real converter will have created a BIM and set up its subjects and partitions according to some domain-specific logic.
    //      For this test, I use the converter to create a quick, mostly empty BIM. The fact that I am using the converter here has no relation to
    //      my use of the converter as a library in convertSomeElements.
    LineUpFiles(L"UseConverterAsLibrary-TheReal.bim", L"Test3d.dgn", true);
    BeFileName outputBimFileName(m_dgnDbFileName);
    auto outputBim = OpenExistingDgnDb(m_dgnDbFileName);

    //  Open some V8 file as the source.
    //  A real converter would use the DgnV8 API directly to open a pre-existing V8 file.
    //      For purposes of this test, I will just create and write a few elements to the sample V8 file.
    //      (Note that I am doing this after converting the file above, so these elements won't be in the BIM.)
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    v8editor.AddLine(nullptr, v8editor.m_defaultModel, Bentley::DPoint3d::From(0,0,0));
    v8editor.AddCellWithTwoArcs(nullptr, L"UserCell");
    auto threeDModel = v8editor.m_defaultModel;

    if (true)
        {
        // Create a drawing model ...
        DgnV8Api::DgnModelStatus modelStatus;
        Bentley::DgnModelP drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing1", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);

        // ... and attach the 3D model as a reference to the new drawing model
        Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        DgnV8Api::DgnAttachment* attachment;
        ASSERT_EQ(BentleyApi::SUCCESS, drawingModel->CreateDgnAttachment(attachment, *moniker, threeDModel->GetModelName(), true));
        ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
        v8editor.Save();

        //  Default (3D)
        //      ^
        //      DgnAttachment
        //      |
        //  Drawing1
        }

    if (true)
        {
        // Create a sheet model ...
        DgnV8Api::DgnModelStatus modelStatus;
        Bentley::DgnModelP sheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);

        // ... and attach the drawing as a reference to the new sheet model
        Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        DgnV8Api::DgnAttachment* attachment;
        ASSERT_EQ(BentleyApi::SUCCESS, sheetModel->CreateDgnAttachment(attachment, *moniker, L"Drawing1", true));
        ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
        v8editor.Save();

        //  Default (3D)
        //      ^
        //      DgnAttachment
        //      |
        //  Drawing1
        //      ^
        //      DgnAttachment
        //      |
        //  Sheet1
        }

    v8editor.Save();

    //  Now show how to use ConverterLibrary to convert element geometry and levels
    convertSomeElements(*outputBim, *v8editor.m_file, *v8editor.m_defaultModel);

    outputBim->SaveChanges();
    wprintf(L"%ls\n", outputBim->GetFileName().c_str());
    v8editor.Save();
    }

struct TestXDomain : XDomain
    {
    int m_counts[2] {};
    bool m_onBeginCalled{};
    bool m_onFinishCalled{};

    void _DetermineElementParams(DgnClassId&, DgnCode& code, DgnCategoryId&, DgnV8EhCR v8eh, Converter& cvt, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const&) override
        {
        if (DgnV8Api::LINE_ELM != v8eh.GetElementType())
            return;

        Utf8PrintfString codeValue("TestXDomain-%d", m_counts[0]);
        code = cvt.CreateCode(codeValue.c_str());
        ++m_counts[0];
        }

    void _ProcessResults(ElementConversionResults&, DgnV8EhCR v8eh, ResolvedModelMapping const&, Converter&) override
        {
        if (DgnV8Api::LINE_ELM != v8eh.GetElementType())
            return;

        ++m_counts[1];
        }

    void _OnBeginConversion(Converter&, DgnV8ModelR rootModel) override {m_onBeginCalled=true;}
    void _OnFinishConversion(Converter&) override {m_onFinishCalled=true;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, XDomainTest)
    {
    LineUpFiles(L"XDomainTest.bim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    v8editor.AddLine(nullptr, v8editor.m_defaultModel, Bentley::DPoint3d::From(0,0,0));
    v8editor.Save();

    TestXDomain testXdomain;
    XDomain::Register(testXdomain);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    EXPECT_TRUE(testXdomain.m_onBeginCalled);
    EXPECT_TRUE(testXdomain.m_onFinishCalled);
    EXPECT_EQ(1, testXdomain.m_counts[0]);
    EXPECT_EQ(1, testXdomain.m_counts[1]);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db.IsValid());
    if (true)
        {
        EC::ECSqlStatement stmt;
        ASSERT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "SELECT COUNT(*) FROM bis.element WHERE (CodeValue LIKE 'TestXDomain%')"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        }

    XDomain::UnRegister(testXdomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyApi::bvector<DgnModelCPtr> getModelsByName(DgnDbR db, Utf8CP modelName)
    {
    BentleyApi::bvector<DgnModelCPtr> models;

    auto stmt = db.GetPreparedECSqlStatement("select el.ecinstanceid from bis.element el, bis.model m WHERE (el.ecinstanceid = m.ecinstanceid) AND (el.CodeValue = ?)");
    stmt->BindText(1, modelName, EC::IECSqlBinder::MakeCopy::No);
    while (BentleyApi::BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        auto modelId = stmt->GetValueId<DgnModelId>(0);
        models.push_back(db.Models().GetModel(modelId));
        }
    return models;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterTests::CheckNoDupXsas(DgnDbR db, Utf8CP kind)
    {
    Utf8PrintfString totalCountSql   ("select count(*) from (select          scope.id, identifier from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " where kind='%s')", kind);
    Utf8PrintfString distinctCountSql("select count(*) from (select distinct scope.id, identifier from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " where kind='%s')", kind);

    EC::ECSqlStatement totalCountStmt;
    ASSERT_EQ(EC::ECSqlStatus::Success, totalCountStmt.Prepare(db, totalCountSql.c_str()));
    EC::ECSqlStatement distinctCountStmt;
    ASSERT_EQ(EC::ECSqlStatus::Success, distinctCountStmt.Prepare(db, distinctCountSql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, totalCountStmt.Step());
    ASSERT_EQ(BE_SQLITE_ROW, distinctCountStmt.Step());
    ASSERT_EQ(totalCountStmt.GetValueInt(0), distinctCountStmt.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, CommonReferences)
    {
    // The input files to this test have PW doc GUIDs. That is how we detect the common (embedded) reference.
    const Utf8CP s_master1Name = "master3d1 Package";
    const Utf8CP s_master2Name = "master3d2 Package";
    const Utf8CP s_refName = "ref3d";
    BentleyApi::BeFileName masterPackageFile1 = GetInputFileName(L"master3d1 Package.i.dgn");
    BentleyApi::BeFileName masterPackageFile2 = GetInputFileName(L"master3d2 Package.i.dgn");

    m_dgnDbFileName = GetOutputFileName(L"CommonReferences.bim");
    DeleteExistingDgnDb(m_dgnDbFileName);
    MakeWritableCopyOf(m_dgnDbFileName, m_seedDgnDbFileName, m_dgnDbFileName.GetFileNameAndExtension().c_str());

    m_v8FileName = masterPackageFile1;
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());
        ASSERT_EQ(1, getModelsByName(*db, s_master1Name).size());
        ASSERT_EQ(0, getModelsByName(*db, s_master2Name).size());
        ASSERT_EQ(1, getModelsByName(*db, s_refName).size());
        CheckNoDupXsas(*db, SyncInfo::ExternalSourceAspect::Kind::Element);
        }

    m_v8FileName = masterPackageFile2;
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());
        ASSERT_EQ(1, getModelsByName(*db, s_master1Name).size());
        ASSERT_EQ(1, getModelsByName(*db, s_master2Name).size());
        ASSERT_EQ(1, getModelsByName(*db, s_refName).size()) << "Since both master files reference the same ref3d, there should be only one copy of ref3d in the iModel";
        CheckNoDupXsas(*db, SyncInfo::ExternalSourceAspect::Kind::Element);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, CommonReferencesWithRecipe)
    {
    // The input files to this test have PW doc GUIDs. That is how we detect the common (embedded) reference.
    const Utf8CP s_master1Name = "master_common";
    const Utf8CP s_master2Name = "master_common-v1";
    const Utf8CP s_refName = "commonref";
    const Utf8CP s_refName_v1 = "commonref-v1";
    BentleyApi::BeFileName masterPackageFile1 = GetInputFileName(L"master_common.i.dgn");
    BentleyApi::BeFileName masterPackageFile2 = GetInputFileName(L"master_common-v1.i.dgn");

    m_dgnDbFileName = GetOutputFileName(L"CommonReferencesWithRecipe.bim");
    DeleteExistingDgnDb(m_dgnDbFileName);
    MakeWritableCopyOf(m_dgnDbFileName, m_seedDgnDbFileName, m_dgnDbFileName.GetFileNameAndExtension().c_str());

    m_params.SetMatchOnEmbeddedFileBasename(true);  // Must opt into this filename-based matching. It is off by default.
    iModelBridge::Params::FileIdRecipe recipe;
    recipe.m_suffixRegex = "-v[0-9]$";
    m_params.SetEmbeddedFileIdRecipe(recipe);
    ASSERT_TRUE(m_params.GetEmbeddedFileIdRecipe() != nullptr);


    m_v8FileName = masterPackageFile1;
    DoConvert(m_dgnDbFileName, m_v8FileName, /*wantExternalDataModels*/false);
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());
        ASSERT_EQ(1, getModelsByName(*db, s_master1Name).size());
        ASSERT_EQ(0, getModelsByName(*db, s_master2Name).size());
        ASSERT_EQ(1, getModelsByName(*db, s_refName).size());
        ASSERT_EQ(0, getModelsByName(*db, s_refName_v1).size());
        CheckNoDupXsas(*db, SyncInfo::ExternalSourceAspect::Kind::Element);
        countModels(*db, 0, 2);
        }

    m_v8FileName = masterPackageFile2;
    DoConvert(m_dgnDbFileName, m_v8FileName, /*wantExternalDataModels*/false);
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());
        ASSERT_EQ(1, getModelsByName(*db, s_master1Name).size());
        ASSERT_EQ(1, getModelsByName(*db, s_master2Name).size());
        ASSERT_EQ(1, getModelsByName(*db, s_refName).size());
        ASSERT_EQ(0, getModelsByName(*db, s_refName_v1).size());
        CheckNoDupXsas(*db, SyncInfo::ExternalSourceAspect::Kind::Element);
        countModels(*db, 0, 3);
        }

    m_params.SetMatchOnEmbeddedFileBasename(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterTests, CommonReferencesNoDocGuids)
    {
    // The input files to this test DO NOT have PW doc GUIDs. So, we have to use the common embedded reference file's basename to detect that it's common.

    m_params.SetMatchOnEmbeddedFileBasename(true);  // Must opt into this filename-based matching. It is off by default.

    const Utf8CP s_master1Name = "master3d1 Package";
    const Utf8CP s_master2Name = "master3d2 Package";
    const Utf8CP s_refName = "commonref";

    BentleyApi::BeFileName inFile = GetInputFileName(L"referencesCommon.i.dgn");
    BentleyApi::BeFileName masterPackageFile1, masterPackageFile2;
    MakeWritableCopyOf(masterPackageFile1, inFile, L"master3d1 Package.i.dgn");
    MakeWritableCopyOf(masterPackageFile2, inFile, L"master3d2 Package.i.dgn");

    m_dgnDbFileName = GetOutputFileName(L"CommonReferencesNoDocGuids.bim");
    DeleteExistingDgnDb(m_dgnDbFileName);
    MakeWritableCopyOf(m_dgnDbFileName, m_seedDgnDbFileName, m_dgnDbFileName.GetFileNameAndExtension().c_str());

    m_v8FileName = masterPackageFile1;
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());
        ASSERT_EQ(1, getModelsByName(*db, s_master1Name).size());
        ASSERT_EQ(0, getModelsByName(*db, s_master2Name).size());
        ASSERT_EQ(1, getModelsByName(*db, s_refName).size());
        CheckNoDupXsas(*db, SyncInfo::ExternalSourceAspect::Kind::Element);
        }

    m_v8FileName = masterPackageFile2;
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());
        ASSERT_EQ(1, getModelsByName(*db, s_master1Name).size());
        ASSERT_EQ(1, getModelsByName(*db, s_master2Name).size());
        ASSERT_EQ(1, getModelsByName(*db, s_refName).size()) << "Since both master files reference the same commonref (by name), there should be only one copy of commonref in the iModel";
        CheckNoDupXsas(*db, SyncInfo::ExternalSourceAspect::Kind::Element);
        }

    m_params.SetMatchOnEmbeddedFileBasename(false);
    }

//========================================================================================
// @bsiclass                                    Sam.Wilson          11/17
//========================================================================================
struct ExternalSourceAspectTests : ConverterTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExternalSourceAspectTests, Performance)
    {
    LineUpFiles(L"ExternalSourceAspectTests_Performance.bim", L"Test3d.dgn", false);
    
    static constexpr int s_insertElementCount = 1000;
    static constexpr int s_insertAspectCount = 1000;
    static constexpr int s_otherElementCount = 0;
    static constexpr int s_selectRepeatCount = 100000;

    DgnV8Api::ElementId lastElementsId = 0;
    DgnV8Api::ModelId theModelId = 0;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        for (int i=0; i<s_insertElementCount; ++i)
            {
            v8editor.AddLine(&lastElementsId, v8editor.m_defaultModel, Bentley::DPoint3d::From(0,0,0));
            }
        v8editor.Save();

        theModelId = v8editor.m_defaultModel->GetModelId();
        }

    if (true)
        {
        BentleyApi::StopWatch timer(true);
        DoConvert(m_dgnDbFileName, m_v8FileName);
        timer.Stop();
        printf("ExternalSourceAspectTests.Performance convert = %lf\n", timer.GetElapsedSeconds());
        }

    DgnModelId modelId;
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));

        auto repositoryLink = FindRepositoryLinkByFilename(*db, m_v8FileName);
        SyncInfo::V8ModelExternalSourceAspectIteratorByV8Id models(*repositoryLink, theModelId);
        auto it = models.begin();
        ASSERT_TRUE(it != models.end());
        modelId = it->GetModelId();

        if (true)
            {
            EC::ECSqlStatement totalCountStmt;
            ASSERT_EQ(EC::ECSqlStatus::Success, totalCountStmt.Prepare(*db, "select count(*) from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " where (kind='Element' AND scope.id=?)"));
            totalCountStmt.BindId(1, modelId);
            BentleyApi::StopWatch timer(true);
            ASSERT_EQ(BE_SQLITE_ROW, totalCountStmt.Step());
            ASSERT_EQ(s_insertElementCount + s_otherElementCount, totalCountStmt.GetValueInt(0));
            timer.Stop();
            printf("ImodelJsTest.MeasureInsertPerformance select kind=Element and scope 1 time = %lf\n", timer.GetElapsedSeconds());
            }

        if (true)
            {
            EC::ECSqlStatement stmt;
            ASSERT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "select * from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " where (kind='Element' AND scope.id=? AND identifier=?)"));
            stmt.BindId(1, modelId);
            stmt.BindText(2, SyncInfo::V8ElementExternalSourceAspect::FormatSourceId(lastElementsId).c_str(), EC::IECSqlBinder::MakeCopy::Yes);

            BentleyApi::StopWatch timer(true);

            for (int i=0; i<s_selectRepeatCount; ++i)
                {
                stmt.Reset();
                ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
                }

            timer.Stop();
            printf("ImodelJsTest.MeasureInsertPerformance select by kind, scope, identifier, repeated %d times = %lf\n", s_selectRepeatCount, timer.GetElapsedSeconds());

            auto qplan = db->ExplainQuery(stmt.GetNativeSql());
            ASSERT_TRUE(!qplan.Contains("SCAN"));
            // TODO: When we get the next index in place, assert that qplan searches that index.
            printf("%s\n", qplan.c_str());
            }
        }

    DgnElementId theElementId; // The BIS element ID of the last V8 element
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        auto el = FindV8ElementInDgnDb(*db, lastElementsId)->CopyForEdit();

        auto aspectClass = iModelExternalSourceAspect::GetAspectClass(*db);

        BentleyApi::StopWatch timer(true);

        for (int i=0; i<s_insertAspectCount; ++i)
            {
            char buf[10];
            itoa(i, buf, 10);
            auto instance = iModelExternalSourceAspect::CreateInstance(DgnElementId(modelId.GetValue()), "Foo", buf, nullptr, *aspectClass);
            auto aspect = iModelExternalSourceAspect(instance.get());
            ASSERT_EQ(DgnDbStatus::Success, aspect.AddAspect(*el));
            }

        ASSERT_TRUE(el->Update().IsValid());

        timer.Stop();
        printf("ImodelJsTest.MeasureInsertPerformance update element, adding %d aspects = %lf\n", s_insertAspectCount, timer.GetElapsedSeconds());

        theElementId = el->GetElementId();
        }

    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));

        if (true)
            {
            EC::ECSqlStatement totalCountStmt;
            ASSERT_EQ(EC::ECSqlStatus::Success, totalCountStmt.Prepare(*db, "select count(*) from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " where (Element.Id=? and kind='Foo')"));
            totalCountStmt.BindId(1, theElementId);
            BentleyApi::StopWatch timer(true);
            ASSERT_EQ(BE_SQLITE_ROW, totalCountStmt.Step());
            ASSERT_EQ(s_insertElementCount + s_otherElementCount, totalCountStmt.GetValueInt(0));
            timer.Stop();
            printf("ImodelJsTest.MeasureInsertPerformance select Element.Id and kind=Foo 1 time = %lf\n", timer.GetElapsedSeconds());
            }

        if (true)
            {
            ASSERT_NE(s_insertAspectCount-1, lastElementsId); // make sure that the identifier of the last Foo aspect on this element doesn't happen to match the identifier of the 'Element' aspect on this element.

            EC::ECSqlStatement stmt;
            ASSERT_EQ(EC::ECSqlStatus::Success, stmt.Prepare(*db, "select * from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " where (Element.Id=? AND identifier=?)"));
            stmt.BindId(1, theElementId);
            stmt.BindText(2, Utf8PrintfString("%d", s_insertAspectCount-1).c_str(), EC::IECSqlBinder::MakeCopy::Yes);       // The identifier of the last Foo aspect

            BentleyApi::StopWatch timer(true);

            auto selectRepeatCount = s_selectRepeatCount;
            for (int i=0; i<selectRepeatCount; ++i)
                {
                stmt.Reset();
                ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
                }

            timer.Stop();
            printf("ImodelJsTest.MeasureInsertPerformance select by Element.Id and identifier, repeated %d times = %lf\n", selectRepeatCount, timer.GetElapsedSeconds());

            auto qplan = db->ExplainQuery(stmt.GetNativeSql());
            ASSERT_TRUE(!qplan.Contains("SCAN"));
            // TODO: When we get the next index in place, assert that qplan searches that index.
            printf("%s\n", qplan.c_str());
            }
        }

    }

//========================================================================================
// @bsiclass                                    Sam.Wilson          11/17
//========================================================================================
struct TransformTests : ConverterTests
    {
    BentleyApi::DRange3d m_projectExtents;    // make a note of the project extents
    BentleyApi::DPoint3d m_obim1Initial, m_obim2Initial;
    DgnV8Api::ElementId m_elementId1, m_elementId2;
    double m_pointTolerance = 1.0e-10; // all values are near zero. The is the tolerance for comparing points, after applying a transform.

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyApi::ICurvePrimitivePtr GetCurvePrimitiveFromElement(DgnElementCR el)
    {
    auto geom = el.ToGeometrySource();
    if (nullptr == geom)
        return nullptr;
    size_t count = 0;
    BentleyApi::ICurvePrimitivePtr curveprim;
    for (auto iter : GeometryCollection(*geom))
        {
        GeometricPrimitivePtr geom = iter.GetGeometryPtr();
        if (GeometricPrimitive::GeometryType::CurvePrimitive != geom->GetGeometryType())
            return nullptr;
        curveprim = geom->GetAsICurvePrimitive();
        ++count;
        }
    EXPECT_EQ(1, count);
    return curveprim;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetLineStartPoint(BentleyApi::DPoint3dR pt, DgnElementCR el)
    {
    auto curveprim = GetCurvePrimitiveFromElement(el);
    ASSERT_TRUE(curveprim.IsValid());
    auto lineStr = curveprim->GetLineStringCP();
    ASSERT_TRUE(nullptr != lineStr);
    ASSERT_EQ(2, lineStr->size());
    pt = lineStr->at(0);                                            // element local coordinates
    el.ToGeometrySource()->GetPlacementTransform().Multiply(pt);    // => world coordinates
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssertEqualPoints(BentleyApi::DPoint3dCR ptbim, Bentley::DPoint3dCR ptce, double uscale)
    {
    ASSERT_DOUBLE_EQ(ptbim.x, uscale*ptce.x);
    ASSERT_DOUBLE_EQ(ptbim.y, uscale*ptce.y);
    ASSERT_DOUBLE_EQ(ptbim.z, uscale*ptce.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateWithSpatialDataTransform(BentleyApi::DPoint3dCR xlat, double rot, bool expectedChanges)
    {
    BentleyApi::Transform jobTrans = BentleyApi::Transform::FromAxisAndRotationAngle(BentleyApi::DRay3d::FromOriginAndVector(BentleyApi::DPoint3d::FromZero(), BentleyApi::DVec3d::From(0, 0, 1)), rot);
    jobTrans.SetTranslation(xlat);

    m_params.SetSpatialDataTransform(jobTrans);

    m_count = 0;
    DoUpdate(m_dgnDbFileName, m_v8FileName, false, expectedChanges);

    if (expectedChanges)
        {
        ASSERT_EQ(2, m_count) << "Both lines should have moved";
        }
    else
        {
        ASSERT_EQ(0, m_count);
        }

    // Verify that the lines moved as expected
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    auto bimLine1 = FindV8ElementInDgnDb(*db, m_elementId1);
    ASSERT_TRUE(bimLine1.IsValid());
    BentleyApi::DPoint3d obim1AfterUpdate;
    GetLineStartPoint(obim1AfterUpdate, *bimLine1);

    auto obim1InitialTransformed = BentleyApi::DPoint3d::FromProduct(jobTrans, m_obim1Initial);
    ASSERT_TRUE(obim1InitialTransformed.AlmostEqual(obim1AfterUpdate, m_pointTolerance)) << " Line1's origin should now be at the transformed location";

    auto bimLine2 = FindV8ElementInDgnDb(*db, m_elementId2);
    ASSERT_TRUE(bimLine2.IsValid());
    BentleyApi::DPoint3d obim2AfterUpdate;
    GetLineStartPoint(obim2AfterUpdate, *bimLine2);

    auto obim2InitialTransformed = BentleyApi::DPoint3d::FromProduct(jobTrans, m_obim2Initial);
    ASSERT_TRUE(obim2InitialTransformed.AlmostEqual(obim2AfterUpdate, m_pointTolerance)) << " Line2's origin should now be at the transformed location";

    // Check that the transform is reported in the JobSubject's "Transform" property
    SubjectCPtr jobSubject = GetFirstJobSubject(*db);
    BentleyApi::Transform transStored;
    ASSERT_EQ(BSISUCCESS, JobSubjectUtils::GetTransform(transStored, *jobSubject));
    ASSERT_TRUE(transStored.IsEqual(jobTrans));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SetupTransformCorrectionTest()
    {
    // Place two lines and remember where they started.
    auto o1 = Bentley::DPoint3d::From(1, 0, 0);
    auto o2 = Bentley::DPoint3d::From(2, 0, 0);

    {
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    v8editor.AddLine(&m_elementId1, v8editor.m_defaultModel, o1);
    v8editor.AddLine(&m_elementId2, v8editor.m_defaultModel, o2);
    v8editor.Save();
    }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    auto unitsScaleFactor = 1.0;

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    m_projectExtents = db->GeoLocation().GetProjectExtents();

    SubjectCPtr jobSubject = GetFirstJobSubject(*db);
    ASSERT_TRUE(jobSubject.IsValid()) << "There is always a job subject element";

    BentleyApi::Transform trans;
    ASSERT_EQ(BSISUCCESS, JobSubjectUtils::GetTransform(trans, *jobSubject)) << "The job subject should have a default identity transform property";

    auto bimLine1 = FindV8ElementInDgnDb(*db, m_elementId1);
    ASSERT_TRUE(bimLine1.IsValid());
    GetLineStartPoint(m_obim1Initial, *bimLine1);

    ASSERT_FALSE(0 == BentleyApi::BeNumerical::Compare(m_obim1Initial.x, 0.0));

    unitsScaleFactor = m_obim1Initial.x / o1.x;

    AssertEqualPoints(m_obim1Initial, o1, unitsScaleFactor);

    auto bimLine2 = FindV8ElementInDgnDb(*db, m_elementId2);
    ASSERT_TRUE(bimLine2.IsValid());
    GetLineStartPoint(m_obim2Initial, *bimLine2);
    AssertEqualPoints(m_obim2Initial, o2, unitsScaleFactor);
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransformTests, Test1)
    {
    LineUpFiles(L"TransformCorrectionFromParams.bim", L"Test3d.dgn", false);

    BentleyApi::DPoint3d xlat = BentleyApi::DPoint3d::From(1,1,1);
    double rot = 0.0;

    SetupTransformCorrectionTest();
    UpdateWithSpatialDataTransform(BentleyApi::DPoint3d::FromZero(), 0.0, false);
    UpdateWithSpatialDataTransform(BentleyApi::DPoint3d::FromOne(), 0.0, true);
    UpdateWithSpatialDataTransform(BentleyApi::DPoint3d::FromOne(), 0.0, false);
    UpdateWithSpatialDataTransform(BentleyApi::DPoint3d::FromZero(), Angle::PiOver2(), true);
    UpdateWithSpatialDataTransform(BentleyApi::DPoint3d::FromOne(), Angle::PiOver2(), true);
    UpdateWithSpatialDataTransform(BentleyApi::DPoint3d::FromOne(), Angle::PiOver2(), false);
    }

extern "C" iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeRegSubKey);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(ConverterTests, StandAloneAdapterTest)
//    {
//    LineUpFiles(L"ElementCRUD.bim", L"Test3d.dgn", true); // creates Test1.idgndb from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
//    //WString testName(BeTest::GetNameOfCurrentTest(), BentleyCharEncoding::Utf8);
//    //BeFileName outputFileName = GetOutputFileName(testName.c_str());
//
//
//    WCharCP args[] = {L"Dgnv8Bridge"};
//    iModelBridge* bridge = iModelBridge_getInstance(L"Dgnv8Bridge");
//    iModelBridgeSacAdapter::ParseCommandLineForBeTest(*bridge, {
//            {L"--input=", m_v8FileName.c_str()},
//            {L"--output=",m_dgnDbFileName.c_str()}//,
//            });
//
//    BentleyApi::BentleyStatus status = bridge->_Initialize(1, (WCharCP*) &args);
//    ASSERT_EQ(BentleyStatus::SUCCESS, status);
//
//    iModelBridgeSacAdapter::Params saparams;
//    iModelBridgeSacAdapter::Execute(*bridge, saparams);
//    ASSERT_TRUE(m_dgnDbFileName.DoesPathExist());
//
//    BentleyApi::BeSQLite::DbResult result;
//    DgnDbPtr db = DgnDb::OpenDgnDb(&result, m_dgnDbFileName, DgnDb::OpenParams(BentleyApi::BeSQLite::Db::OpenMode::Readonly));
//    ASSERT_TRUE(db.IsValid());
//    ASSERT_TRUE(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK == result);
//    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      5/19
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConverterTests, DetectDeletionsInEmbeddedFiles)
    {
    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=0");
    static const uint32_t s_v8PasswordCacheLifetime = 2*1000; // the timeout is 1 second. Wait for 2 to be safe.
    m_params.SetMatchOnEmbeddedFileBasename(true);

    m_dgnDbFileName = GetOutputFileName(L"DetectDeletionsInEmbeddedFiles.bim");
    DeleteExistingDgnDb(m_dgnDbFileName);
    MakeWritableCopyOf(m_dgnDbFileName, m_seedDgnDbFileName, m_dgnDbFileName.GetFileNameAndExtension().c_str());

    auto v1InputName = GetInputFileName(L"changesInEmbeddedFiles\\v1\\master.i.dgn");
    auto v2InputName = GetInputFileName(L"changesInEmbeddedFiles\\v2\\master.i.dgn");

    auto cvtDir = GetOutputDir();
    cvtDir.AppendToPath(L"DetectDeletionsInEmbeddedFiles");
    cvtDir.AppendToPath(L"cvt");
    BeFileName::EmptyAndRemoveDirectory(cvtDir.c_str());
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory(cvtDir.c_str()));

    m_v8FileName = cvtDir;
    m_v8FileName.AppendToPath(L"master.i.dgn");

    // Convert v1
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(v1InputName.c_str(), m_v8FileName.c_str()));
    DoConvert(m_dgnDbFileName, m_v8FileName);

    RepositoryLinkId masterRepositoryLinkId, ref__1__RepositoryLinkId, ref__2__RepositoryLinkId;
    DgnModelId masterModelId, ref__1__ModelId, ref__2__ModelId;
    DgnElementId ref__1___620, ref__2___620, ref__2___621;

    if (true)
        {
        auto db = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);

        auto link = FindRepositoryLinkByFilename(*db, m_v8FileName);
        ASSERT_TRUE(link.IsValid());
        masterRepositoryLinkId = RepositoryLinkId(link->GetElementId().GetValue());
        link = FindRepositoryLinkByFilename(*db, BentleyApi::BeFileName(L"Ref1.i.dgn"));
        ASSERT_TRUE(link.IsValid());
        ref__1__RepositoryLinkId = RepositoryLinkId(link->GetElementId().GetValue());
        link = FindRepositoryLinkByFilename(*db, BentleyApi::BeFileName(L"Ref2.i.dgn"));
        ASSERT_TRUE(link.IsValid());
        ref__2__RepositoryLinkId = RepositoryLinkId(link->GetElementId().GetValue());

        masterModelId = FindModelByV8ModelId(*db, masterRepositoryLinkId, 0);
        ref__1__ModelId = FindModelByV8ModelId(*db, ref__1__RepositoryLinkId, 0);
        ref__2__ModelId = FindModelByV8ModelId(*db, ref__2__RepositoryLinkId, 0);
        ASSERT_TRUE(masterModelId.IsValid());
        ASSERT_TRUE(ref__1__ModelId.IsValid());
        ASSERT_TRUE(ref__2__ModelId.IsValid());

        // Verify that V8 element '620' from Ref1.i.dgn is there
        auto el = FindV8ElementInDgnDb(*db, 620, ref__1__RepositoryLinkId);
        ASSERT_TRUE(el.IsValid());
        ref__1___620 = el->GetElementId();

        // Verify that V8 element '620' from Ref2.i.dgn is there
        el = FindV8ElementInDgnDb(*db, 620, ref__2__RepositoryLinkId);
        ASSERT_TRUE(el.IsValid());
        ref__2___620 = el->GetElementId();

        // Verify that V8 element '621' from Ref2.i.dgn is there
        el = FindV8ElementInDgnDb(*db, 621, ref__2__RepositoryLinkId);
        ASSERT_TRUE(el.IsValid());
        ref__2___621 = el->GetElementId();

        el = nullptr;
        link = nullptr;
        }

    // Update from v2 
    //  In v2, Ref1.i.dgn was deleted, and element #621 was removed from Ref2.i.dgn
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(v2InputName.c_str(), m_v8FileName.c_str()));

    // NEEDS WORK temporary work-around for V8 password cache entry lifetime
    BentleyApi::BeThreadUtilities::BeSleep(s_v8PasswordCacheLifetime);

    DoUpdate(m_dgnDbFileName, m_v8FileName, /*expectFailure*/false, /*expectUpdate*/true, /*detectDeletedDocuments*/true, /*onAllDocsProcessed*/true);

    if (true)
        {
        auto db = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);

        // Master.i.dgn's RepositoryLink and its Model should still be  there
        ASSERT_TRUE(FindRepositoryLinkByFilename(*db, m_v8FileName).IsValid());
        ASSERT_TRUE(db->Elements().GetElement(masterRepositoryLinkId).IsValid());
        ASSERT_TRUE(db->Models().GetModel(masterModelId).IsValid());
        ASSERT_TRUE(FindModelByV8ModelId(*db, masterRepositoryLinkId, 0).IsValid());

        // Ref1.i.dgn's RepositoryLink and its Model should NOT be there
        // ASSERT_FALSE(FindRepositoryLinkByFilename(*db, BentleyApi::BeFileName(L"Ref1.i.dgn")).IsValid()); Bridge cannot deleted the RepositoryLink safely. It can't guarantee that the repository link element was created by the bridge to which that file is assigned. It could happen that another bridge saw this repository/file first, while trying to find its own files via reference attachments.
        // ASSERT_FALSE(db->Elements().GetElement(ref__1__RepositoryLinkId).IsValid());
        ASSERT_FALSE(db->Models().GetModel(ref__1__ModelId).IsValid());             // The Bridge *can* delete the model in that file, because the bridge definition created it and owns it. 
        ASSERT_FALSE(FindModelByV8ModelId(*db, ref__1__RepositoryLinkId, 0).IsValid());

        // Ref2.i.dgn's RepositoryLink and its Model should still be there
        ASSERT_TRUE(FindRepositoryLinkByFilename(*db, BentleyApi::BeFileName(L"Ref2.i.dgn")).IsValid());
        ASSERT_TRUE(db->Elements().GetElement(ref__2__RepositoryLinkId).IsValid());
        ASSERT_TRUE(db->Models().GetModel(ref__2__ModelId).IsValid());
        ASSERT_TRUE(FindModelByV8ModelId(*db, ref__2__RepositoryLinkId, 0).IsValid());

        // Verify that V8 element '620' from Ref1.i.dgn is NOT there
        ASSERT_FALSE(db->Elements().GetElement(ref__1___620).IsValid());
        ASSERT_FALSE(FindV8ElementInDgnDb(*db, 620, ref__1__RepositoryLinkId).IsValid());

        // Verify that V8 element '620' from Ref2.i.dgn is there
        ASSERT_TRUE(db->Elements().GetElement(ref__2___620).IsValid());
        ASSERT_TRUE(FindV8ElementInDgnDb(*db, 620, ref__2__RepositoryLinkId).IsValid());

        // Verify that V8 element '621' from Ref2.i.dgn is NOT there
        ASSERT_FALSE(db->Elements().GetElement(ref__2___621).IsValid());
        ASSERT_FALSE(FindV8ElementInDgnDb(*db, 621, ref__2__RepositoryLinkId).IsValid());
        }

    m_params.SetMatchOnEmbeddedFileBasename(false);
    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=0");
    }

TEST_F(ConverterTests, EmbeddedFileIdRecipe)
    {
    LineUpFiles(L"EmbeddedFileIdRecipe.bim", L"Test3d.dgn", false);  // just so that I can bootstrap a RootModelConverter

    RootModelConverter::RootModelSpatialParams params(m_params);
    params.SetKeepHostAlive(true);
    params.SetInputFileName(m_v8FileName);
    params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());

    RootModelConverter converter(params);
    auto db = OpenExistingDgnDb(m_dgnDbFileName);
    converter.SetDgnDb(*db);
    converter.SetIsUpdating(false);
    converter.AttachSyncInfo();

    iModelBridge::Params::FileIdRecipe recipe;
    recipe.m_ignoreCase = true;
    recipe.m_ignoreExtension = true;
    recipe.m_ignorePackage = true;
    recipe.m_ignorePwDocId = true;
    recipe.m_suffixRegex = "";
    ASSERT_STREQ("abc", converter.ComputeEffectiveEmbeddedFileName("abc.ext", &recipe).c_str());
    ASSERT_STREQ("abc", converter.ComputeEffectiveEmbeddedFileName("def<111>abc.ext", &recipe).c_str());
    ASSERT_STREQ("abc", converter.ComputeEffectiveEmbeddedFileName(">abc.ext", &recipe).c_str());
    ASSERT_STREQ("abc", converter.ComputeEffectiveEmbeddedFileName(">abc.z", &recipe).c_str());
    ASSERT_STREQ("abc", converter.ComputeEffectiveEmbeddedFileName(">abc", &recipe).c_str());
    ASSERT_STREQ("abc", converter.ComputeEffectiveEmbeddedFileName(">ABC.EXT", &recipe).c_str());

    recipe.m_ignoreCase = false;
    ASSERT_STREQ("ABC", converter.ComputeEffectiveEmbeddedFileName(">ABC.EXT", &recipe).c_str());

    recipe.m_ignoreExtension = false;
    ASSERT_STREQ("ABC.EXT", converter.ComputeEffectiveEmbeddedFileName(">ABC.EXT", &recipe).c_str());

    recipe.m_ignorePackage = false;
    ASSERT_STREQ(">ABC.EXT", converter.ComputeEffectiveEmbeddedFileName(">ABC.EXT", &recipe).c_str());

    recipe.m_ignoreCase = true;
    recipe.m_ignoreExtension = true;
    recipe.m_ignorePackage = true;
    recipe.m_ignorePwDocId = true;

    recipe.m_suffixRegex = "_P[0-9]*\\-[0-9]*_S[0-9]*$";
    ASSERT_STRCASEEQ("133735_2A-EWR-OXD-OB_33-M3-CE-010001", converter.ComputeEffectiveEmbeddedFileName("133735_2A-EWR-OXD-OB_33-M3-CE-010001.dgn.i.dgn", &recipe).c_str());
    ASSERT_STRCASEEQ("133735_2A-EWR-OXD-OB_33-M3-CE-010001", converter.ComputeEffectiveEmbeddedFileName("133735_2A-EWR-OXD-OB_33-M3-CE-010001_P02-01_S2.dgn.i.dgn", &recipe).c_str());

    ASSERT_STRCASEEQ("Ph1-BBV-HS2SG1 Routewide LOD 3d", converter.ComputeEffectiveEmbeddedFileName("1MC08-BBV-DS-DMB-NS01_NL01-100001.i.dgn<24>Ph1-BBV-HS2SG1 Routewide LOD 3d.dgn.i.dgn", &recipe).c_str());
    }

void ConverterTests::ConvertWithCrash(DgnDbR db, CrashControlMonitor& monitor)
    {
    RootModelConverter::RootModelSpatialParams params(m_params);
    params.SetKeepHostAlive(true);
    params.SetInputFileName(m_v8FileName);

    TestRootModelCreator converter(params, this);
    params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());
    converter.SetDgnDb(db);
    converter.SetIsUpdating(false);
    converter.AttachSyncInfo();

    converter.SetMonitor(monitor);

    ASSERT_EQ(BentleyApi::SUCCESS, converter.InitRootModel());
    converter.MakeSchemaChanges();
    ASSERT_FALSE(converter.WasAborted());
    ASSERT_EQ(TestRootModelCreator::ImportJobCreateStatus::Success, converter.InitializeJob());
    ASSERT_EQ(BentleyApi::SUCCESS, converter.MakeDefinitionChanges());
    converter.ConvertData();
    }

bool ConverterTests::ConvertWithCrashCaught(DgnDbR db, CrashControlMonitor& monitor)
    {
    __try
        {
        ConvertWithCrash(db, monitor);
        return false;
        }
    __except (EXCEPTION_EXECUTE_HANDLER)
        {        
        }
    return  true;
    }

TEST_F(ConverterTests, CrashAndRerun)
    {
    LineUpFiles(L"CrashAndRerun.bim", L"Test3d.dgn", false);

    wchar_t const* model1Name = L"Model1";
    wchar_t const* model2Name = L"Model2";

    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
    
        Bentley::DgnModelP newModel1 = v8editor.m_file->CreateNewModel(&modelStatus, model1Name, DgnV8Api::DgnModelType::Normal, /*is3D*/ true);
        DgnV8Api::ElementId eid1;
        v8editor.AddLine(&eid1, newModel1);
    
        Bentley::DgnModelP newModel2 = v8editor.m_file->CreateNewModel(&modelStatus, model2Name, DgnV8Api::DgnModelType::Normal, /*is3D*/ true);
        DgnV8Api::ElementId eid2;
        v8editor.AddLine(&eid2, newModel2);

        auto parentModel = v8editor.m_defaultModel;

        v8editor.AddAttachment(m_v8FileName, nullptr, Bentley::DPoint3d::FromZero(), newModel1->GetModelNameCP());
        v8editor.AddAttachment(m_v8FileName, nullptr, Bentley::DPoint3d::FromZero(), newModel2->GetModelNameCP());
    
        ASSERT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        }

    // Do the initial conversion but crash half way through, after converting model1 but before converting model2.
    if (true)
        {
        auto db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_TRUE(db.IsValid());

        RefCountedPtr<CrashControlMonitor> monitor = new CrashControlMonitor();
        monitor->m_modelsAndActions[model1Name] = CrashControlMonitor::ModelAction::Save;
        monitor->m_modelsAndActions[model2Name] = CrashControlMonitor::ModelAction::Crash;

        ASSERT_TRUE(ConvertWithCrashCaught(*db, *monitor));

        db->AbandonChanges();
        db->CloseDb();
        }

    if (true)
        {
        auto db = OpenExistingDgnDb(m_dgnDbFileName);
        auto model1Found = getModelsByName(*db, Utf8String(model1Name).c_str());
        auto model2Found = getModelsByName(*db, Utf8String(model2Name).c_str());
        ASSERT_EQ(1, model1Found.size());
        ASSERT_EQ(1, model2Found.size());
        countElements(*model1Found[0], 1);
        countElements(*model2Found[0], 0);  // because of the crash, model2 should not have been converted
        }

    // Re-run the converter, this time with no crash. The converter should convert model2 (and leave model1 as is)

    DoUpdate(m_dgnDbFileName, m_v8FileName);

    if (true)
        {
        auto db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_TRUE(db.IsValid());

        auto model1Found = getModelsByName(*db, Utf8String(model1Name).c_str());
        auto model2Found = getModelsByName(*db, Utf8String(model2Name).c_str());
        ASSERT_EQ(1, model1Found.size());
        ASSERT_EQ(1, model2Found.size());
        countElements(*model1Found[0], 1);
        countElements(*model2Found[0], 1);  // this time, model2 should have been converted
        }
    }

TEST_F(ConverterTests, MappingErrors)
    {
    LineUpFiles(L"MappingErrors.bim", L"Test3d.dgn", false);

    // Convert 
        {
        RootModelConverter::RootModelSpatialParams params(m_params); // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
        params.SetKeepHostAlive(true);
        params.SetIsUpdating(false);
        params.SetInputFileName(m_v8FileName);
        params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());
        params.SetBridgeJobName("first job");

        RootModelConverter creator(params);
        auto db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_TRUE(db.IsValid());
        creator.SetDgnDb(*db);
        creator.AttachSyncInfo();
        ASSERT_EQ(BentleyApi::SUCCESS, creator.InitRootModel());
        creator.MakeSchemaChanges();
        ASSERT_FALSE(creator.WasAborted());
        ASSERT_EQ(RootModelConverter::ImportJobLoadStatus::FailedNotFound, creator.FindJob()) << "Should not find the job -- it's new";
        ASSERT_EQ(RootModelConverter::ImportJobCreateStatus::Success, creator.InitializeJob());
        creator.MakeDefinitionChanges();
        creator.ConvertData();
        ASSERT_FALSE(creator.WasAborted());
        db->SaveChanges();
        }

    //  Verify that we cannot *initialize* the *same* job a second time. (The bridge framework never tries to do this.)
        {
        RootModelConverter::RootModelSpatialParams params(m_params); // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
        params.SetKeepHostAlive(true);
        params.SetIsUpdating(false);
        params.SetInputFileName(m_v8FileName);
        params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());
        params.SetBridgeJobName("first job");

        RootModelConverter creator(params);
        auto db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_TRUE(db.IsValid());
        creator.SetDgnDb(*db);
        creator.AttachSyncInfo();
        ASSERT_EQ(BentleyApi::SUCCESS, creator.InitRootModel());
        BentleyApi::BeTest::SetFailOnAssert(false);
        ASSERT_EQ(BentleyApi::SUCCESS, creator.MakeSchemaChanges());
        ASSERT_EQ(RootModelConverter::ImportJobCreateStatus::FailedExistingRoot, creator.InitializeJob()) << "Should not create a new job when one exists for this root";
        ASSERT_EQ(RootModelConverter::ImportJobLoadStatus::Success, creator.FindJob()) << "Should find the existing job";
        BentleyApi::BeTest::SetFailOnAssert(true);
        }

    //  Verify that we cannot initialize a *different* job with the *same* masterfile as the first job. The bridge framework may try to do this.
        {
        RootModelConverter::RootModelSpatialParams params(m_params); // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
        params.SetKeepHostAlive(true);
        params.SetIsUpdating(false);
        params.SetInputFileName(m_v8FileName);
        params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());
        params.SetBridgeJobName("second job");

        RootModelConverter creator(params);
        auto db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_TRUE(db.IsValid());
        creator.SetDgnDb(*db);
        creator.AttachSyncInfo();
        ASSERT_EQ(BentleyApi::SUCCESS, creator.InitRootModel());
        ASSERT_NE(BentleyApi::SUCCESS, creator.MakeSchemaChanges() );
        ASSERT_EQ(RootModelConverter::ImportJobCreateStatus::FailedExistingRoot, creator.InitializeJob()) << "Should not create a new job when one exists for this root";
        }
    }