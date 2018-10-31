/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/RasterTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImportConfigEditor.h"
#include "ConverterTestsBaseFixture.h"

#include <Raster/RasterApi.h>
#include <Raster/RasterFileHandler.h>
#include <Raster/WmsHandler.h>

#define MAX_VIEWS   8

//=========================================================================================
// @bsiclass                                                    Eric.Paquet         9/2016
//=========================================================================================
struct RasterTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);

    template<class Model_T>
    BentleyApi::RefCountedPtr<Model_T> FindModel(DgnDbCR db, Utf8StringCR name);

    void ValidateModelRange(GeometricModel3dCR geomModel, Utf8StringCR name, double x0, double y0, double z0, double x1, double y1, double z1, double tol = 0.1);
    void ValidateModelViews(DgnDbR db, Utf8StringCR  name, uint32_t nbConvertedViews, bool modelIs3d, bool expectedViewState[MAX_VIEWS]);

    void CreateRasterAttachmentElement(WCharCP fileName, DgnV8Api::EditElementHandle& eeh, Bentley::DgnModelR model/*, bool addToModel = true*/, bool reLocate = false);
    void CreateRasterAttachmentElement(WCharCP fileName, Transform transform, DgnV8Api::EditElementHandle& eeh, Bentley::DgnModelR model/*, bool addToModel = true*/, bool reLocate = false);
    };

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
template<class Model_T>
BentleyApi::RefCountedPtr<Model_T> RasterTests::FindModel(DgnDbCR db, Utf8StringCR modelName)
    {
    auto& models = db.Models();

    for (ModelIteratorEntryCR entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel)))
        {
        BentleyApi::Dgn::DgnModelPtr model = models.GetModel(entry.GetModelId());
        if (!model.IsValid())
            continue;

        if (modelName.Equals(model->GetName().c_str()))
            return dynamic_cast<Model_T*>(model.get());
        }

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
void RasterTests::ValidateModelRange(GeometricModel3dCR geomModel, Utf8StringCR name, double x0, double y0, double z0, double x1, double y1, double z1, double tol)
    {
    BentleyApi::DRange3d expectedRange = BentleyApi::DRange3d::From(x0, y0, z0, x1, y1, z1);
    BentleyApi::AxisAlignedBox3d modelRange = geomModel.QueryModelRange();

    // Compare ranges. Ignore z values because QueryModelRange adds some depth (z = 1.0 or -1.0), which is irrelevant for us.
    bool areEqual = true;
    if ((fabs(modelRange.low.x - expectedRange.low.x) > tol) ||
        (fabs(modelRange.low.y - expectedRange.low.y) > tol) ||
        (fabs(modelRange.high.x - expectedRange.high.x) > tol) ||
        (fabs(modelRange.high.y - expectedRange.high.y) > tol))
        areEqual = false;

    if (!areEqual)
        {
        Utf8String msg;
        msg.Sprintf("Model range is wrong. Model: %s\nExpected: {%lf %lf}{%lf %lf}\nActual:   {%lf %lf}{%lf %lf}",
                    name.c_str(), x0, y0, x1, y1,
                    modelRange.low.x, modelRange.low.y, modelRange.high.x, modelRange.high.y);
        FAIL() << msg;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
void RasterTests::ValidateModelViews(DgnDbR db, Utf8StringCR  name, uint32_t nbConvertedViews, bool modelIs3d, bool expectedViewState[MAX_VIEWS])
    {
    bool    viewState[MAX_VIEWS] = {false, false, false, false, false, false, false, false};
    bool    viewExists[MAX_VIEWS] = {false, false, false, false, false, false, false, false};
    uint32_t viewExistsCount = 0;
    for (int viewNo = 0; viewNo < MAX_VIEWS; ++viewNo)
        {
        BentleyApi::Utf8String viewName;

        if (modelIs3d)
            viewName.Sprintf("Default Views - View %d", viewNo + 1);
        else
            viewName.Sprintf("2D Metric Design Model Views - View %d", viewNo + 1);

        ViewDefinitionCPtr  viewPtr = ViewDefinition::Get(db, ViewDefinition::QueryViewId(*GetJobDefinitionModel(db), viewName));
        if (viewPtr == nullptr || !viewPtr->IsSpatialView())
            continue; // This view was not converted

        viewExists[viewNo] = true;
        viewExistsCount++;

        SpatialViewControllerCPtr viewControllerPtr = viewPtr->LoadViewController()->_ToSpatialView();
        for (DgnModelId modelId : viewControllerPtr->GetViewedModels())
            {
            BentleyApi::Dgn::DgnModelPtr modelPtr = db.Models().GetModel(modelId);
            ASSERT_TRUE(modelPtr != nullptr);
            BentleyApi::Utf8String viewedModelName = modelPtr->GetName();
            if (viewedModelName.Equals(name.c_str()))
                {
                // Found the view in the viewed models
                viewState[viewNo] = true;
                break;
                }
            }
        }

    // Verify that the expected number of views was converted
    ASSERT_EQ(nbConvertedViews, viewExistsCount) << L"Number of converted views is not as expected";

    // Compare results
    bool areEqual = true;
    for (int viewNo = 0; viewNo < MAX_VIEWS; ++viewNo)
        {
        if (viewExists[viewNo])
            {
            if (viewState[viewNo] != expectedViewState[viewNo])
                {
                areEqual = false;
                break;
                }
            }
        }

    // Display error if view states were not correctly converted.
    if (!areEqual)
        {
        // Format current view values
        Utf8String msgCurrentViews("");
        for (int viewNo = 0; viewNo < MAX_VIEWS; ++viewNo)
            {
            if (viewExists[viewNo])
                msgCurrentViews.Sprintf("%s%s ", msgCurrentViews.c_str(), viewState[viewNo] ? "ON " : "OFF");
            else
                msgCurrentViews.Sprintf("%s--  ", msgCurrentViews.c_str());
            }

        // Format expected view values
        Utf8String msgExpectedViews("");
        for (int viewNo = 0; viewNo < MAX_VIEWS; ++viewNo)
            {
            msgExpectedViews.Sprintf("%s%s ", msgExpectedViews.c_str(), expectedViewState[viewNo] ? "ON " : "OFF");
            }

        // Compose final message
        Utf8String msg;
        msg.Sprintf("View states were not correctly converted. Model: %s\nExpected: %s\nCurrent:  %s", name.c_str(), msgExpectedViews.c_str(), msgCurrentViews.c_str());

        FAIL() << msg;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterTests::CreateRasterAttachmentElement(WCharCP fileName, DgnV8Api::EditElementHandle& eeh, Bentley::DgnModelR model/*, bool addToModel = true*/, bool reLocate)
    {
    Transform transform;
    memset(&transform, 0, sizeof(transform.form3d));
    int i = 1;
    if (reLocate)
        i = 10.5;

    transform.form3d[0][0] = -10000 * i;
    transform.form3d[1][1] = 5000 * i;
    transform.form3d[2][2] = 10000 * i;

    CreateRasterAttachmentElement(fileName, transform, eeh, model, reLocate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
void RasterTests::CreateRasterAttachmentElement(WCharCP fileName, Transform transform, DgnV8Api::EditElementHandle& eeh, Bentley::DgnModelR model, bool reLocate /* = false */)
    {
    DPoint2d  extentInUORVerifier;
    extentInUORVerifier.x = 50.0;
    extentInUORVerifier.y = 100.0;

    BentleyApi::BeFileName rasterFile;
    if (GetInputFileName(fileName).DoesPathExist())
        MakeWritableCopyOf(rasterFile, fileName);

    WString InputFilename(fileName);
    WString InputFilespec(rasterFile);

    DgnDocumentMonikerPtr pMoniker = DgnV8Api::DgnDocumentMoniker::CreateFromRawData(InputFilename.c_str(), InputFilespec.c_str(), NULL, /*NULL/*searchPath will be added by DgnRaster*/DgnV8Api::IRasterAttachmentQuery::GetSearchPath(&model).c_str(), false, NULL);
    DgnRasterOpenParamsPtr openParams = DgnV8Api::Raster::DgnRasterOpenParams::Create(pMoniker, true/*read only*/);

    EXPECT_EQ(SUCCESS, DgnV8Api::RasterFrameHandler::CreateRasterAttachment(eeh, NULL, *pMoniker, transform, extentInUORVerifier, model));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RasterTests, EmbedRaster_False)
    {
    LineUpFiles(L"rasterimport.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    WCharCP fileName = L"chkmrk.bmp";
    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();

    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(fileName, eeh, *model);
    eeh.AddToModel();
    v8editor.Save();

    ImportConfigEditor config;
    config.m_rasterImportAttachments = "false";
    config.CreateConfig();
    m_params.SetConfigFile(config.m_importFileName);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    if (true)
        {
        // Verify if raster is imported
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

        Utf8String name = "chkmrk.bmp";
        auto geomModelP = FindModel<GeometricModel3d>(*db, name);
        ASSERT_EQ(nullptr, geomModelP.get()) << L"Model not found: " << name;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RasterTests, EmbedRaster_True)
    {
    LineUpFiles(L"rasterimport.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();
    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(L"12TVL305030_saltLake.itiff", eeh, *model);
    eeh.AddToModel();
    v8editor.Save();

    // by default raster should be imported
    DoConvert(m_dgnDbFileName, m_v8FileName);

    Utf8String name = "12TVL305030_saltLake.itiff";
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_NE(nullptr, geomModelP.get()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -2.560000, 0, 0, 0, 1.280000, 0);

    WString extractedFileName(name.c_str());
    BentleyApi::BeFileName extractedFile = GetOutputFileName(extractedFileName.c_str());
    ASSERT_TRUE(extractedFile.DoesPathExist()) << L"Output file was not created from embedded raster: " << extractedFile.c_str();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
TEST_F(RasterTests, rasterDgnNoGCS)
    {
    LineUpFiles(L"rasterDgnNoGCS.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;
    m_noGcs = true; // as we don't want GCS conversion in this test.

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();
    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(L"12TVL305030_saltLake.itiff", eeh, *model);
    eeh.AddToModel();
    v8editor.Save();

    // by default raster should be imported
    DoConvert(m_dgnDbFileName, m_v8FileName);

    Utf8String name("12TVL305030_saltLake.itiff");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;

    ValidateModelRange(*geomModelP, name, -2.560000, 0, 0, 0, 1.280000, 0);

    // Raster should be copied at the same location than the Bim file
    WString extractedFileName(name.c_str());
    BentleyApi::BeFileName extractedFile = GetOutputFileName(extractedFileName.c_str());
    ASSERT_TRUE(extractedFile.DoesPathExist()) << L"Output file was not created: " << extractedFile.c_str();

    // no GCS
    BentleyApi::Dgn::DgnGCS* dgnGCS = db->GeoLocation().GetDgnGCS();
    ASSERT_EQ(nullptr, dgnGCS);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
TEST_F(RasterTests, RasterViewSettings)
    {
    LineUpFiles(L"rasterViewSettings.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();
    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(L"12TVL305030_saltLake.itiff", eeh, *model);

    IRasterAttachmentEditP rasterEdit = dynamic_cast<IRasterAttachmentEditP> (&eeh.GetHandler());
    ASSERT_TRUE(NULL != rasterEdit);
    // Make Raster visible in random Views
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 0, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 1, false));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 2, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 3, false));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 4, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 5, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 6, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 7, true));
    eeh.AddToModel();
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    Utf8String name("12TVL305030_saltLake.itiff");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    // Validate range. The raster is not reprojected.
    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -2.560000, 0, 0, 0, 1.280000, 0);

    bool expectedViewState[MAX_VIEWS] = {true, false, true, false, true, true, true, true};
    ValidateModelViews(*db, name, 1, true, expectedViewState);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
TEST_F(RasterTests, rasterAttachedTwice)
    {
    LineUpFiles(L"rasterAttachedTwice.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();
    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(L"12TVL305030_saltLake.itiff", eeh, *model);
    eeh.AddToModel();
    v8editor.Save();

    CreateRasterAttachmentElement(L"12TVL305030_saltLake.itiff", eeh, *model, true);
    eeh.AddToModel();
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    Utf8String name("12TVL305030_saltLake.itiff");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(2, SelectCountFromECClass(*db, RASTER_SCHEMA(RASTER_CLASSNAME_RasterModel)));

    // Validate range. The raster is not reprojected.
    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -2.560000, 0, 0, 0, 1.280000, 0);

    // Validate range. This raster is the same as the previous one, but attached at a different location.
    name = "12TVL305030_saltLake.itiff-1";
    geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -25.600000, 0, 0, 0, 12.800000, 0);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         10/2016
//-----------------------------------------------------------------------------------------
TEST_F(RasterTests, rasterExportNoRepro)
    {
    LineUpFiles(L"rasterExportNoRepro.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();
    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(L"12TVL305015_saltLake.jp2", eeh, *model);
    eeh.AddToModel();
    v8editor.Save();

    // Set m_rasterExportNonPortableFormats to true, so that the "jp2" file will be exported as itiff
    ImportConfigEditor config;
    config.m_rasterExportNonPortableFormats = "true";
    config.CreateConfig();
    m_params.SetConfigFile(config.m_importFileName);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    Utf8String name("12TVL305015_saltLake.itiff");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -2.560000, 0, 0, 0, 1.280000, 0);

    // Raster should be copied at the same location than the Bim file and should be exported to an "itiff"
    BentleyApi::BeFileName fileName(L"12TVL305015_saltLake.itiff");
    BentleyApi::BeFileName copiedFile = GetOutputFileName(fileName.c_str());
    ASSERT_TRUE(copiedFile.DoesPathExist()) << L"Output file was not created: " << copiedFile.c_str();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         10/2016
//-----------------------------------------------------------------------------------------
TEST_F(RasterTests, rasterNoExport)
    {
    LineUpFiles(L"rasterExportNoRepro.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();
    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(L"12TVL305015_saltLake.jp2", eeh, *model);
    eeh.AddToModel();
    v8editor.Save();

    // Set m_rasterExportNonPortableFormats to false, so that the "jp2" file will not be exported
    ImportConfigEditor config;
    config.m_rasterExportNonPortableFormats = "false";
    config.CreateConfig();
    m_params.SetConfigFile(config.m_importFileName);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    Utf8String name("12TVL305015_saltLake.jp2");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -2.560000, 0, 0, 0, 1.280000, 0);

    // Raster should be copied at the same location than the Bim file and should still be a "jp2"
    BentleyApi::BeFileName fileName(L"12TVL305015_saltLake.jp2");
    BentleyApi::BeFileName copiedFile = GetOutputFileName(fileName.c_str());
    ASSERT_TRUE(copiedFile.DoesPathExist()) << L"Output file was not created: " << copiedFile.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RasterTests, rasterVariousCases)
    {
    LineUpFiles(L"rasterVariousCases.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();
    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(L"deletedRaster.jp2", eeh, *model);
    eeh.AddToModel();
    v8editor.Save();

    CreateRasterAttachmentElement(L"12TVL290030_saltLake.jpg", eeh, *model);
    IRasterAttachmentEditP rasterEdit = dynamic_cast<IRasterAttachmentEditP> (&eeh.GetHandler());
    ASSERT_TRUE(NULL != rasterEdit);
    Transform identityTransform;
    memset(&identityTransform, 0, sizeof(identityTransform.form3d));
    identityTransform.form3d[0][0] = -20000;
    identityTransform.form3d[1][1] = 2.500;
    identityTransform.form3d[2][2] = 20000;

    rasterEdit->SetTransform(eeh, identityTransform);
    eeh.AddToModel();
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    // Try to find a non-existing raster
    Utf8String name("deletedRaster.jp2");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_FALSE(geomModelP.IsValid()) << L"Model should not be found: " << name;

    // jpg raster with no GCS, but with a transformation
    name = ("12TVL290030_saltLake.jpg");
    geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -5.120000, 0, 0, 0, 0.000640, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RasterTests, RasterViewSettings2d)
    {
    LineUpFiles(L"rasterViewSettings.ibim", L"Test2d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();
    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(L"12TVL305030_saltLake.itiff", eeh, *model);

    IRasterAttachmentEditP rasterEdit = dynamic_cast<IRasterAttachmentEditP> (&eeh.GetHandler());
    ASSERT_TRUE(NULL != rasterEdit);
    // Make Raster visible in random Views
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 0, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 1, false));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 2, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 3, false));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 4, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 5, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 6, true));
    ASSERT_EQ(true, rasterEdit->SetViewState(eeh, 7, true));
    eeh.AddToModel();
    v8editor.Save();

    m_params.SetConsiderNormal2dModelsSpatial(true); // convert default 2D model to 3D
    DoConvert(m_dgnDbFileName, m_v8FileName);

    Utf8String name("12TVL305030_saltLake.itiff");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    // Validate range. The raster is not reprojected.
    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -256.000000, 0, 0, 0, 128.000000, 0);

    bool expectedViewState[MAX_VIEWS] = {true, false, true, false, true, true, true, true};
    ValidateModelViews(*db, name, 0, true, expectedViewState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RasterTests, RasterInReferencedDgn)
    {
    BentleyApi::BeFileName fileName;
    BentleyApi::BeFileName refFile;
    {
    MakeWritableCopyOf(m_v8FileName, L"Test3d.dgn");
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();
    DgnV8Api::EditElementHandle eeh;
    CreateRasterAttachmentElement(L"12TVL305030_saltLake.itiff", eeh, *model);
    eeh.AddToModel();
    v8editor.Save();

    CreateRasterAttachmentElement(L"12TVL305030_saltLake.itiff", eeh, *model, true);
    eeh.AddToModel();
    v8editor.Save();

    MakeCopyOfFile(refFile, L"-ref");

    fileName = m_v8FileName;
    }
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeDeleteFile(m_v8FileName));

    LineUpFiles(L"rasterInDgnWithReferences.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    //  Add refV8File as an attachment to v8File
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(refFile.c_str());
    DgnV8Api::DgnAttachment* attachment;
    ASSERT_EQ(BentleyApi::SUCCESS, v8editor.m_defaultModel->CreateDgnAttachment(attachment, *moniker, v8editor.m_defaultModel->GetModelName(), true));
    ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
    v8editor.Save();

    // by default raster should be imported
    DoConvert(m_dgnDbFileName, m_v8FileName);

    Utf8String name = "12TVL305030_saltLake.itiff";
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    ASSERT_EQ(2, SelectCountFromECClass(*db, RASTER_SCHEMA(RASTER_CLASSNAME_RasterModel)));

    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_NE(nullptr, geomModelP.get()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -2.560000, 0, 0, 0, 1.280000, 0);

    // Validate range. This raster is the same as the previous one, but attached at a different location.
    name = "12TVL305030_saltLake.itiff-1";
    geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;
    ValidateModelRange(*geomModelP, name, -25.600000, 0, 0, 0, 12.800000, 0);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
TEST_F(RasterTests, RasterReprojected)
    {
    // Set up a GCS near Salt Lake City, rather than near Exton, PA.
    m_noGcs = true;

    iModelBridge::GCSDefinition gcsDef;
    gcsDef.m_isValid = true;
    gcsDef.m_originUors.Zero();
    gcsDef.m_azimuthAngle = 0.0;
    gcsDef.m_geoPoint.latitude = 40.50; // coordinates of Bentley in Exton, PA
    gcsDef.m_geoPoint.longitude = -111.50;
    gcsDef.m_geoPoint.elevation = 0;
    m_params.SetOutputGcsDefinition(gcsDef);

    BentleyApi::BeFileName rasterFile;
    MakeWritableCopyOf(rasterFile, L"12TVL305030_saltLake.itiff");
    LineUpFiles(L"rasterReprojected.ibim", L"rasterReprojected.dgn", true);

    Utf8String name("12TVL305030_saltLake.itiff");

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_NE(nullptr, geomModelP.get()) << L"Model not found: " << name;

    ValidateModelRange(*geomModelP, name, -27262.46035, 19477.5549, 0, -25753.41979, 20986.61109, 0);

    // Raster should be copied at the same location than the Bim file
    WString extractedFileName(name.c_str());
    BentleyApi::BeFileName extractedFile = GetOutputFileName(extractedFileName.c_str());
    ASSERT_TRUE(extractedFile.DoesPathExist()) << L"Output file was not created: " << extractedFile.c_str();
    }

//-----------------------------------------------------------------------------------------
// Test with a dgn that has a GCS in feet
// @bsimethod                                                   Eric.Paquet         10/2016
//-----------------------------------------------------------------------------------------
TEST_F(RasterTests, DISABLED_raster_GCSFeet)
    {
    BentleyApi::BeFileName rasterFile;
    MakeWritableCopyOf(rasterFile, L"raster_GCSFeet.itiff");

    LineUpFiles(L"raster_GCSFeet.ibim", L"raster_GCSFeet.dgn", true);

    Utf8String name("raster_GCSFeet.itiff");

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    auto geomModelP = FindModel<GeometricModel3d>(*db, name);
    ASSERT_TRUE(geomModelP.IsValid()) << L"Model not found: " << name;

    ValidateModelRange(*geomModelP, name, 183986.5872, 914770.8748, 0, 185486.5872, 913270.8748, 0);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         10/2016
//-----------------------------------------------------------------------------------------
TEST_F(RasterTests, rasterHideLevels)
    {
    BentleyApi::BeFileName rasterFile;
    MakeWritableCopyOf(rasterFile, L"12TVL305030_saltLake.itiff");
    MakeWritableCopyOf(rasterFile, L"12TVL305045_saltLake.itiff");
    LineUpFiles(L"rasterHideLevels.ibim", L"rasterHideLevels.dgn", true);

    Utf8String name("12TVL305030_saltLake.itiff");
    bool expectedViewState[MAX_VIEWS] = {true, false, false, true, true, true, true, true};
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ValidateModelViews(*db, name, 4, true, expectedViewState);

    name = "12TVL305045_saltLake.itiff";
    bool expectedViewState2[MAX_VIEWS] = {false, true, false, true, true, true, true, true};
    ValidateModelViews(*db, name, 4, true, expectedViewState2);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
TEST_F(RasterTests, Clip_DisjointWithOverlappingHoles)
    {
    BentleyApi::BeFileName rasterFile;
    MakeWritableCopyOf(rasterFile, L"new3d_clip_disjointWithOverlappingHoles.jpg");
    BentleyApi::BeFileName rasterFile2;
    MakeWritableCopyOf(rasterFile2, L"papi.jpg");
    LineUpFiles(L"new3d_clip_disjointWithOverlappingHoles.ibim", L"new3d_clip_disjointWithOverlappingHoles.dgn", true);

    Utf8String name("papi.jpg");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    auto rasterFileModelP = FindModel<BentleyApi::Raster::RasterFileModel>(*db, name);
    ASSERT_TRUE(rasterFileModelP.IsValid()) << L"RasterFileModel not found: " << name;

    BentleyApi::Raster::RasterClipCR clips = rasterFileModelP->GetClip();

    // validate boundary
    ASSERT_TRUE(clips.HasBoundary());
    BentleyApi::CurveVectorCP boundary = clips.GetBoundaryCP();
    ASSERT_NE(nullptr, boundary);

    ASSERT_EQ(BentleyApi::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString, boundary->HasSingleCurvePrimitive());
    ASSERT_EQ(BentleyApi::CurveVector::BOUNDARY_TYPE_Outer, boundary->GetBoundaryType());

    BentleyApi::bvector<BentleyApi::DPoint3d> points;
    points.emplace_back(BentleyApi::DPoint3d::From(0.015345799161655748, 0.019211138229038582, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.044238135759726072, 0.019211138229038582, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.044238135759726072, -0.12095117259477639, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.015345799161655748, -0.12095117259477639, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.015345799161655748, 0.019211138229038582, 0.00000000000000000));
    BentleyApi::CurveVectorPtr pExpectedBoundary = BentleyApi::CurveVector::CreateLinear(points, BentleyApi::CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE(pExpectedBoundary->IsSameStructureAndGeometry(*boundary));

    // Validate masks
    auto const& mask = clips.GetMasks();
    ASSERT_EQ(4, mask.size());

    points.clear();
    points.emplace_back(BentleyApi::DPoint3d::From(0.022124606309913277, -0.016073274851864466, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.031526080917221873, -0.016073274851864466, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.031526080917221873, -0.022269562027047454, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.022124606309913277, -0.022269562027047454, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.022124606309913277, -0.016073274851864466, 0.00000000000000000));
    BentleyApi::CurveVectorPtr pExpectedMask1 = BentleyApi::CurveVector::CreateLinear(points, BentleyApi::CurveVector::BOUNDARY_TYPE_Inner);
    ASSERT_TRUE(pExpectedMask1->IsSameStructureAndGeometry(*mask[0]));

    points.clear();
    points.emplace_back(BentleyApi::DPoint3d::From(0.031526080917221873, -0.032596707319019078, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.040239642748385936, -0.032596707319019078, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.040239642748385936, -0.038334010259003327, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.031526080917221873, -0.038334010259003327, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.031526080917221873, -0.032596707319019078, 0.00000000000000000));
    BentleyApi::CurveVectorPtr pExpectedMask2 = BentleyApi::CurveVector::CreateLinear(points, BentleyApi::CurveVector::BOUNDARY_TYPE_Inner);
    ASSERT_TRUE(pExpectedMask2->IsSameStructureAndGeometry(*mask[1]));

    points.clear();
    points.emplace_back(BentleyApi::DPoint3d::From(0.027202057752884781, -0.038334010259003327, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.035915619584048847, -0.038334010259003327, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.035915619584048847, -0.028605183702201498, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.027202057752884781, -0.028605183702201498, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.027202057752884781, -0.038334010259003327, 0.00000000000000000));
    BentleyApi::CurveVectorPtr pExpectedMask3 = BentleyApi::CurveVector::CreateLinear(points, BentleyApi::CurveVector::BOUNDARY_TYPE_Inner);
    ASSERT_TRUE(pExpectedMask3->IsSameStructureAndGeometry(*mask[2]));

    points.clear();
    points.emplace_back(BentleyApi::DPoint3d::From(-0.024448726522633722, -0.046562941904352242, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.096165313561374144, -0.046562941904352242, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(0.096165313561374144, -0.059185008372317577, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(-0.024448726522633722, -0.059185008372317577, 0.00000000000000000));
    points.emplace_back(BentleyApi::DPoint3d::From(-0.024448726522633722, -0.046562941904352242, 0.00000000000000000));
    BentleyApi::CurveVectorPtr pExpectedMask4 = BentleyApi::CurveVector::CreateLinear(points, BentleyApi::CurveVector::BOUNDARY_TYPE_Inner);
    ASSERT_TRUE(pExpectedMask4->IsSameStructureAndGeometry(*mask[3]));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
TEST_F(RasterTests, Clip_Bspline)
    {
    BentleyApi::BeFileName rasterFile;
    MakeWritableCopyOf(rasterFile, L"new2d_clip_bspline.jpg");
    BentleyApi::BeFileName rasterFile2;
    MakeWritableCopyOf(rasterFile2, L"popotons.jpg");
    m_params.SetConsiderNormal2dModelsSpatial(true);
    LineUpFiles(L"new2d_clip_bspline.ibim", L"new2d_clip_bspline.dgn", true);

    Utf8String name("popotons.jpg");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    auto rasterFileModelP = FindModel<BentleyApi::Raster::RasterFileModel>(*db, name);
    ASSERT_TRUE(rasterFileModelP.IsValid()) << L"RasterFileModel not found: " << name;

    BentleyApi::Raster::RasterClipCR clips = rasterFileModelP->GetClip();

    // validate boundary
    ASSERT_TRUE(clips.HasBoundary());
    BentleyApi::CurveVectorCP boundary = clips.GetBoundaryCP();
    ASSERT_NE(nullptr, boundary);

    ASSERT_EQ(BentleyApi::CurveVector::BOUNDARY_TYPE_Outer, boundary->GetBoundaryType());
    ASSERT_EQ(BentleyApi::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve, boundary->HasSingleCurvePrimitive());

    BentleyApi::MSBsplineCurveCP bcurve = boundary->front()->GetBsplineCurveCP();
    ASSERT_NE(nullptr, bcurve);

    ASSERT_EQ(4, bcurve->GetOrder());
    ASSERT_TRUE(bcurve->IsClosed());

    ASSERT_EQ(6, bcurve->GetNumPoles());
    ASSERT_TRUE(bcurve->GetPoleCP()[0].AlmostEqual(BentleyApi::DPoint3d::From(0.48077039352424622, 1.3205534695572478, 0.00000000000000000)));
    ASSERT_TRUE(bcurve->GetPoleCP()[1].AlmostEqual(BentleyApi::DPoint3d::From(1.8140962797501177, 1.4268651483599246, 0.00000000000000000)));
    ASSERT_TRUE(bcurve->GetPoleCP()[2].AlmostEqual(BentleyApi::DPoint3d::From(1.9058832067004494, 0.76966567939792230, 0.00000000000000000)));
    ASSERT_TRUE(bcurve->GetPoleCP()[3].AlmostEqual(BentleyApi::DPoint3d::From(1.0169992825498682, 0.32992191707775897, 0.00000000000000000)));
    ASSERT_TRUE(bcurve->GetPoleCP()[4].AlmostEqual(BentleyApi::DPoint3d::From(0.23439495802598689, 0.62952755733984822, 0.00000000000000000)));
    ASSERT_TRUE(bcurve->GetPoleCP()[5].AlmostEqual(BentleyApi::DPoint3d::From(0.38898346657391447, 1.4510268935423511, 0.00000000000000000)));

    ASSERT_EQ(13, bcurve->GetNumKnots());
    double knots[] = {-0.5, -0.3333333333333333, -0.166666666666666, 0.0, 0.16666666666666666, 0.3333333333333333,
                      0.5, 0.6666666666666666, 0.833333333333333, 1.0, 1.166666666666666, 1.3333333333333333, 1.5};

    for (auto i = 0; i < 13; ++i)
        ASSERT_NEAR(knots[i], bcurve->GetKnotCP()[i], 1.0E-8);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
TEST_F(RasterTests, Clip_ReprojectedPreserveCurves)
    {
    // Set up a GCS near Salt Lake City, rather than near Exton, PA.
    m_noGcs = true;

    iModelBridge::GCSDefinition gcsDef;
    gcsDef.m_isValid = true;
    gcsDef.m_originUors.Zero();
    gcsDef.m_azimuthAngle = 0.0;
    gcsDef.m_geoPoint.latitude = 40.50; // coordinates of Bentley in Exton, PA
    gcsDef.m_geoPoint.longitude = -111.50;
    gcsDef.m_geoPoint.elevation = 0;
    m_params.SetOutputGcsDefinition(gcsDef);

    BentleyApi::BeFileName rasterFile;
    MakeWritableCopyOf(rasterFile, L"12TVL305030_saltLake.itiff");

    LineUpFiles(L"rasterReprojected_clipped.ibim", L"rasterReprojected_clipped.dgn", true);

    Utf8String name("12TVL305030_saltLake.itiff");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    auto rasterFileModelP = FindModel<BentleyApi::Raster::RasterFileModel>(*db, name);
    ASSERT_TRUE(rasterFileModelP.IsValid()) << L"RasterFileModel not found: " << name;

    BentleyApi::Raster::RasterClipCR clips = rasterFileModelP->GetClip();

    // validate boundary
    ASSERT_TRUE(clips.HasBoundary());
    BentleyApi::CurveVectorCP boundary = clips.GetBoundaryCP();
    ASSERT_NE(nullptr, boundary);
    ASSERT_EQ(BentleyApi::CurveVector::BOUNDARY_TYPE_Outer, boundary->GetBoundaryType());
    ASSERT_EQ(BentleyApi::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, boundary->HasSingleCurvePrimitive());

    BentleyApi::DEllipse3dCP boundArc = boundary->front()->GetArcCP();
    ASSERT_NE(nullptr, boundArc);
    ASSERT_TRUE(boundArc->center.AlmostEqual(BentleyApi::DPoint3d::From(-26609.344918304880, 20180.189303782310, 0.00000000000000000)));
    ASSERT_TRUE(boundArc->vector0.AlmostEqual(BentleyApi::DVec3d::From(161.32271349750397, 596.35623265712081, 0.00000000000000000)));
    ASSERT_TRUE(boundArc->vector90.AlmostEqual(BentleyApi::DVec3d::From(-596.35681660809996, 161.31246650598567, 0.00000000000000000)));
    ASSERT_NEAR(0.0, boundArc->start, 1.0E-8);
    ASSERT_NEAR(6.2831853071795862, boundArc->sweep, 1.0E-8);

    // validate mask
    auto const& mask = clips.GetMasks();
    ASSERT_EQ(1, mask.size());
    ASSERT_EQ(BentleyApi::CurveVector::BOUNDARY_TYPE_Inner, mask[0]->GetBoundaryType());
    ASSERT_EQ(BentleyApi::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, mask[0]->HasSingleCurvePrimitive());

    BentleyApi::DEllipse3dCP maskArc = mask[0]->front()->GetArcCP();
    ASSERT_TRUE(maskArc->center.AlmostEqual(BentleyApi::DPoint3d::From(-26391.101051042126, 20003.617349713953, 0.00000000000000000)));
    ASSERT_TRUE(maskArc->vector0.AlmostEqual(BentleyApi::DVec3d::From(-0.57683426846378583, 101.25456914294709, 0.00000000000000000)));
    ASSERT_TRUE(maskArc->vector90.AlmostEqual(BentleyApi::DVec3d::From(-101.25510918002867, -0.57842744746026986, 0.00000000000000000)));
    ASSERT_NEAR(0.0, maskArc->start, 1.0E-8);
    ASSERT_NEAR(6.2831853071795862, maskArc->sweep, 1.0E-8);
    }

#if 0  //&&MM validate that V8 raster dlls are not trying to access the network.
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
TEST_F(RasterTests, WMS_Reprojected)
    {
    BentleyApi::BeFileName rasterFile;
    MakeWritableCopyOf(rasterFile, L"osm.xwms");
    LineUpFiles(L"Wms_World_OSM.ibim", L"Wms_World_OSM.dgn", true);

    Utf8String name("osm");
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    auto wmsModel = FindModel<BentleyApi::Raster::WmsModel>(*db, name);
    ASSERT_TRUE(wmsModel.IsValid()) << L"WmsModel not found: " << name;

    auto mapInfo = wmsModel->GetMap();

    ASSERT_STREQ("http://ows.terrestris.de/osm/service", mapInfo.m_url.c_str());
    ASSERT_TRUE(mapInfo.m_boundingBox.IsEqual(BentleyApi::DRange2d::From(-20037508.341676001, -3503549.8435043800, 4947.5329241454901, 11068715.659379501), 1.0E-8));

    ASSERT_STREQ("1.1.1", mapInfo.m_version.c_str());
    ASSERT_STREQ("TOPO-OSM-WMS", mapInfo.m_layers.c_str());
    ASSERT_STREQ("", mapInfo.m_styles.c_str());
    ASSERT_STREQ("SRS", mapInfo.m_csType.c_str());
    ASSERT_STREQ("EPSG:900913", mapInfo.m_csLabel.c_str());
    ASSERT_STREQ("image/png", mapInfo.m_format.c_str());
    ASSERT_STREQ("SERVICE=WMS", mapInfo.m_vendorSpecific.c_str());
    ASSERT_FALSE(mapInfo.m_transparent);
    ASSERT_EQ(BentleyApi::Raster::WmsMap::AxisOrder::Default, mapInfo.m_axisOrder);

    // These are arbitrary and might change in the future. We only need a high number that will generate enough resolution.
    ASSERT_EQ(262144, mapInfo.m_metaWidth);
    ASSERT_EQ(190597, mapInfo.m_metaHeight);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(RasterTests, CRUD)
    {
    LineUpFiles(L"RasterCRUD.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    WCharCP fileName = L"popotons.jpg";
    Bentley::DgnModelP model = v8editor.m_defaultModel->GetDgnModelP();

    DgnV8Api::EditElementHandle eeh;
    Transform identity = Transform::FromIdentity();

    CreateRasterAttachmentElement(fileName, identity, eeh, *model);
    eeh.AddToModel();
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

        Utf8String name("popotons.jpg");
        auto rasterModelP = FindModel<BentleyApi::Raster::RasterFileModel>(*db, name);
        ASSERT_NE(nullptr, rasterModelP.get()) << L"Model not found: " << name;
        }
    DgnV8Api::EditElementHandle eeh2;
    Transform transform2;
    memset(&transform2, 0, sizeof(transform2.form3d));
    transform2.form3d[0][0] = 2;
    transform2.form3d[0][3] = 200;
    transform2.form3d[1][1] = 2;
    transform2.form3d[1][3] = 200;
    transform2.form3d[2][2] = 1;

    CreateRasterAttachmentElement(L"12TVL290030_saltLake.jpg", transform2, eeh2, *model);
    eeh2.AddToModel();

    v8editor.Save();
    DoUpdate(m_dgnDbFileName, m_v8FileName);

        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

        Utf8String name2("12TVL290030_saltLake.jpg");
        auto rasterModel2P = FindModel<BentleyApi::Raster::RasterFileModel>(*db, name2);
        ASSERT_NE(nullptr, rasterModel2P.get()) << L"Model not found: " << name2;
        }

    IRasterAttachmentEditP rasterEdit = dynamic_cast<IRasterAttachmentEditP> (&eeh2.GetHandler());
    ASSERT_TRUE(NULL != rasterEdit);
    Transform transform3;
    memset(&transform3, 0, sizeof(transform3.form3d));
    transform3.form3d[0][0] = 1.5;
    transform3.form3d[0][3] = -150;
    transform3.form3d[1][1] = 1.5;
    transform3.form3d[1][3] = -150;
    transform3.form3d[2][2] = 1;

    rasterEdit->SetTransform(eeh2, transform3);
    eeh2.ReplaceInModel(eeh2.GetElementRef());
    v8editor.Save();
    DoUpdate(m_dgnDbFileName, m_v8FileName);

        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        // for DgnDb, all coordinates are stored as meters. Determine the ratio of DgnV8 storage units to meters
        auto toMeters = 1.0 / 1000000.0;

        BentleyApi::DPoint4d row0 = BentleyApi::DPoint4d::From(1.5 * toMeters, 0.0, 0.0, -150.0 * toMeters);
        BentleyApi::DPoint4d row1 = BentleyApi::DPoint4d::From(0.0, 1.5 * toMeters, 0.0, -150.0 * toMeters);
        BentleyApi::DPoint4d row2 = BentleyApi::DPoint4d::From(0.0, 0.0, 1.0 * toMeters, 0.0);

        BentleyApi::DPoint4d actual0, actual1, actual2, actual3;
        Utf8String name2("12TVL290030_saltLake.jpg");
        auto rasterModel2P = FindModel<BentleyApi::Raster::RasterFileModel>(*db, name2);
        ASSERT_NE(nullptr, rasterModel2P.get()) << L"Model not found: " << name2;
        rasterModel2P->GetSourceToWorld().GetRows(actual0, actual1, actual2, actual3);
        ASSERT_TRUE(row0.IsEqual(actual0, 0.0001));
        ASSERT_TRUE(row1.IsEqual(actual1, 0.0001));
        ASSERT_TRUE(row2.IsEqual(actual2, 0.0001));
        }

    // This test only works if either uploading to the reality data server or storing to local server
    Bentley::WString uploadConfigVar;
    Bentley::WString serverConfigVar;
    bool doCheck = false;
    if (SUCCESS == DgnV8Api::ConfigurationManager::GetVariable(uploadConfigVar, L"DGNDB_REALITY_MODEL_UPLOAD"))
        doCheck = true;
    else if (SUCCESS == DgnV8Api::ConfigurationManager::GetVariable(serverConfigVar, L"DGNDB_REALITY_MODEL_TEMPDIR"))
        doCheck = true;

    if (doCheck)
        {
        BentleyApi::BeFileName popotons = GetOutputFileName(L"popotons.jpg");
        time_t mtime;
        popotons.GetFileTime(nullptr, nullptr, &mtime);
        mtime += 1000;
        popotons.SetFileTime(nullptr, &mtime);
        DoUpdate(m_dgnDbFileName, m_v8FileName, false, true);
        }
    }

