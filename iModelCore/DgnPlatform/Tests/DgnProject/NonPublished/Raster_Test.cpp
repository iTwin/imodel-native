/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/Raster_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>
//#include <DgnPlatform/RasterHandlers.h>
#include <Logging/bentleylogging.h>
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_BENTLEY_LOGGING

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"RasterTest"))

#if defined (DGNPLATFORM_HAVE_DGN_IMPORTER)

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnRaster
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnRasterTest : public ::testing::Test
    {
    public:
        ScopedDgnHost           m_host;
        DgnDbPtr      project;
        DgnDbP file;
        DgnModelP model0;

        void SetupProject ();
        uint32_t GetModelColorIndex(RgbColorDef);
        void GetViewStates(bool *);
        void CreateAttachmentTest(bool);
    };

struct TestRasterProperties
    {
    public:
        bool tSnappable, tLocked, tViewIndependentState, tOpenReadWrite, tInvertState, tPrintState, tClipState, tTransparencyState, tBinaryInvertState;
        bool* tViewStates;
        long tDisplayOrder;
        int tViewStatesCount;
        uint32_t tForegroundColor, tBackgroundColor;
        uint8_t tForegroundTransparency, tBackgroundTransparency, tImageTransparency;
        DVec3d tUVector, tVVector;
        DPoint3d tOrigin;
        DPoint2d tExtent, tScanResolution;
        WString tSourceUrl, tAttachDescription, tLogicalName;

        void SetTestRasterProperties (WString sourceUrl, WString attachDescription, WString logicalName, bool snappable, bool locked, 
            bool viewIndependentState, bool openReadWrite, DVec3d uVector, 
            DVec3d vVector, DPoint3d origin, DPoint2d extent, DPoint2d scanResolution, bool* viewStates, int viewStatesCount, bool invertState,
            bool printState, bool clipState, bool transparencyState, bool binaryInvertState, long displayOrder, uint32_t foregroundColor, 
            uint32_t backgroundColor, uint8_t foregroundTransparency, uint8_t backgroundTransparency, uint8_t imageTransparency);
        void IsEqual (TestRasterProperties testStyle);
    };

/*---------------------------------------------------------------------------------**//**
* Get view states
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRasterTest::GetViewStates (bool * testViewStates)
    {
    
    const int viewStatesCount = 8;
    int i;

    for (i = 0; i < viewStatesCount; i++)
        {
        testViewStates[i] = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelP DgnModels::getAndFill(DgnDbR db, DgnModelId modelID)
    {
    DgnModelP dgnModel = db.Models().GetModel (modelID);
    if (dgnModel == NULL)
        return NULL;

    dgnModel->FillModel();
    return  dgnModel;
    }
    
/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .bim project file
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRasterTest::SetupProject ()
    {
    
    DgnDbTestDgnManager::CreateProjectFromDgn (project, DgnDbTestDgnManager::GetOutputFilePath (L"RasterColorModes.bim"), DgnDbTestDgnManager::GetSeedFilePath (L"RasterColorModes.i.dgn.v8"));
    ASSERT_TRUE( project != NULL);

    file = &(dynamic_cast<DgnDb*>(project.get())->GetDgnFile());
    model0 = getAndFill(*file, DgnModelId(0));
    };

/*---------------------------------------------------------------------------------**//**
* Gets model color index from model
* @bsimethod                                    Algirdas.Mikoliunas            04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnRasterTest::GetModelColorIndex(RgbColorDef color) {

    uint32_t    colorIndex(0);
    EXPECT_EQ (SUCCESS, RasterFrameHandler::ColorIndexFromRgbInModel(colorIndex, (*model0), color));
    
    return colorIndex;
}

/*---------------------------------------------------------------------------------**//**
* Create attachment tests
* @param[bool] create attachment from existing attachment
* @bsimethod                                    Algirdas.Mikoliunas            04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRasterTest::CreateAttachmentTest(bool fromExisting) {

    SetupProject();
    EditElementHandle eeh;
    
    DPoint3d origin = DPoint3d::From (0.0, 500, 0.0);
    DVec3dCR uVect = DVec3d::From(300, 0, 0);
    DVec3dCR vVect = DVec3d::From(0, 300, 0);

    const int viewStatesCount = 8;
    bool testViewStates[viewStatesCount];
    GetViewStates(testViewStates);

    RgbColorDef whiteColor = {255, 255, 255}, greyColor = {53, 37, 53};
    uint32_t whiteColorIndex = GetModelColorIndex(whiteColor);
    uint32_t greyColorIndex = GetModelColorIndex(greyColor);

    TestRasterProperties rasterData, newRasterData;
    rasterData.SetTestRasterProperties (
        L"source_url", L"", L"", true, false, false, false, uVect, vVect, 
        origin, DPoint2d::From(300, 300), DPoint2d::From(600, 600),
        testViewStates, viewStatesCount, false, true, true, true, false, -5, whiteColorIndex, 
        greyColorIndex, 0, 255, 0
    );
    
    if (fromExisting) 
        {
        RasterFrameElementCollection rasterFrameCollection = RasterFrameElementCollection (*model0);
        RasterFrameElementIterator rasterIterator = rasterFrameCollection.begin();
        
        ElementHandleCR rasterEh = *rasterIterator;
        RasterFrameHandler::CreateRasterAttachment (eeh, &rasterEh, L"source_url", origin, uVect, vVect, (*model0));
        EXPECT_TRUE(eeh.IsValid());

        RasterFrameHandler* pQuery(dynamic_cast<RasterFrameHandler*>(&eeh.GetHandler()));
    
        newRasterData.SetTestRasterProperties
            (
            pQuery->GetSourceUrl(eeh), 
            pQuery->GetAttachDescription(eeh), 
            pQuery->GetLogicalName(eeh),   
            pQuery->GetSnappableState(eeh), 
            pQuery->GetLockedState(eeh), 
            pQuery->GetViewIndependentState(eeh), 
            pQuery->GetOpenReadWrite(eeh), 
            pQuery->GetU(eeh), 
            pQuery->GetV(eeh),
            pQuery->GetOrigin(eeh),
            pQuery->GetExtent(eeh),
            pQuery->GetScanningResolution(eeh),
            testViewStates,
            viewStatesCount,
            pQuery->GetInvertState(eeh),
            pQuery->GetPrintState(eeh),
            pQuery->GetClipState(eeh),
            pQuery->GetTransparencyState(eeh),
            pQuery->GetBinaryPrintInvertState(eeh),
            pQuery->GetDisplayOrder(eeh),
            pQuery->GetForegroundColor(eeh),
            pQuery->GetBackgroundColor(eeh),
            pQuery->GetForegroundTransparencyLevel(eeh),
            pQuery->GetBackgroundTransparencyLevel(eeh),
            pQuery->GetImageTransparencyLevel(eeh)
            );
        
        rasterData.IsEqual(newRasterData);
        }
    else 
        {
        RasterFrameHandler::CreateRasterAttachment (eeh, NULL, L"source_url", origin, uVect, vVect, (*model0));
        EXPECT_TRUE(eeh.IsValid());

        RasterFrameHandler* pQuery(dynamic_cast<RasterFrameHandler*>(&eeh.GetHandler()));
        DVec3dCR actualUVect = pQuery->GetU(eeh);
        DVec3dCR actualVVect = pQuery->GetV(eeh);
        DPoint3d actualOrigin = pQuery->GetOrigin(eeh);

        EXPECT_EQ (Utf8String(L"source_url"), Utf8String(pQuery->GetSourceUrl(eeh)));
        
        EXPECT_NEAR (uVect.x, actualUVect.x, 0.01);
        EXPECT_NEAR (uVect.y, actualUVect.y, 0.01);
        EXPECT_NEAR (uVect.z, actualUVect.z, 0.01);

        EXPECT_NEAR (vVect.x, actualVVect.x, 0.01);
        EXPECT_NEAR (vVect.y, actualVVect.y, 0.01);
        EXPECT_NEAR (vVect.z, actualVVect.z, 0.01);

        EXPECT_NEAR (origin.x, actualOrigin.x, 0.01);
        EXPECT_NEAR (origin.y, actualOrigin.y, 0.01);
        EXPECT_NEAR (origin.z, actualOrigin.z, 0.01);
        }
}

//=======================================================================================
// @bsiclass                                                Algirdas.Mikoliunas   04/13
//=======================================================================================

void TestRasterProperties::SetTestRasterProperties (WString sourceUrl, WString attachDescription, WString logicalName, bool snappable, bool locked, 
            bool viewIndependentState, bool openReadWrite, DVec3d uVector, 
            DVec3d vVector, DPoint3d origin, DPoint2d extent, DPoint2d scanResolution, bool* viewStates, int viewStatesCount, bool invertState,
            bool printState, bool clipState, bool transparencyState, bool binaryInvertState, long displayOrder, uint32_t foregroundColor, 
            uint32_t backgroundColor, uint8_t foregroundTransparency, uint8_t backgroundTransparency, uint8_t imageTransparency)
    {
    tSourceUrl = sourceUrl;
    tAttachDescription = attachDescription;
    tLogicalName = logicalName;
    tSnappable = snappable;
    tLocked = locked;
    tViewIndependentState = viewIndependentState;
    tOpenReadWrite = openReadWrite;
    tUVector = uVector;
    tVVector = vVector;
    tOrigin = origin;
    tExtent = extent;
    tScanResolution = scanResolution;
    tViewStates = viewStates;
    tViewStatesCount = viewStatesCount;
    tInvertState = invertState;
    tPrintState = printState;
    tClipState = clipState;
    tTransparencyState = transparencyState;
    tBinaryInvertState = binaryInvertState;
    tDisplayOrder = displayOrder;
    tForegroundColor = foregroundColor;
    tForegroundTransparency = foregroundTransparency;
    tBackgroundTransparency = backgroundTransparency;
    tImageTransparency = imageTransparency;
    };

void TestRasterProperties::IsEqual (TestRasterProperties testStyle)
    {

    EXPECT_EQ (Utf8String(tSourceUrl), Utf8String(testStyle.tSourceUrl));
    EXPECT_EQ (Utf8String(tAttachDescription), Utf8String(testStyle.tAttachDescription));
    EXPECT_EQ (Utf8String(tLogicalName), Utf8String(testStyle.tLogicalName));

    EXPECT_EQ (tSnappable, testStyle.tSnappable);
    EXPECT_EQ (tLocked, testStyle.tLocked);
    EXPECT_EQ (tViewIndependentState, testStyle.tViewIndependentState);
    EXPECT_EQ (tOpenReadWrite, testStyle.tOpenReadWrite);
    EXPECT_EQ (tInvertState, testStyle.tInvertState);
    EXPECT_EQ (tPrintState, testStyle.tPrintState);
    EXPECT_EQ (tClipState, testStyle.tClipState);
    EXPECT_EQ (tTransparencyState, testStyle.tTransparencyState);
    EXPECT_EQ (tBinaryInvertState, testStyle.tBinaryInvertState);
    EXPECT_EQ (tDisplayOrder, testStyle.tDisplayOrder);
    EXPECT_EQ (tForegroundColor, testStyle.tForegroundColor);
    
    EXPECT_NEAR (tUVector.x, testStyle.tUVector.x, 0.01);
    EXPECT_NEAR (tUVector.y, testStyle.tUVector.y, 0.01);
    EXPECT_NEAR (tUVector.z, testStyle.tUVector.z, 0.01);

    EXPECT_NEAR (tVVector.x, testStyle.tVVector.x, 0.01);
    EXPECT_NEAR (tVVector.y, testStyle.tVVector.y, 0.01);
    EXPECT_NEAR (tVVector.z, testStyle.tVVector.z, 0.01);

    EXPECT_NEAR (tOrigin.x, testStyle.tOrigin.x, 0.01);
    EXPECT_NEAR (tOrigin.y, testStyle.tOrigin.y, 0.01);
    EXPECT_NEAR (tOrigin.z, testStyle.tOrigin.z, 0.01);

    EXPECT_NEAR (tExtent.x, testStyle.tExtent.x, 0.01);
    EXPECT_NEAR (tExtent.y, testStyle.tExtent.y, 0.01);

    EXPECT_NEAR (tScanResolution.x, testStyle.tScanResolution.x, 0.01);
    EXPECT_NEAR (tScanResolution.y, testStyle.tScanResolution.y, 0.01);

    int i = 0;
    for (i = 0; i < tViewStatesCount; i++)
        {
        EXPECT_EQ(tViewStates[i], testStyle.tViewStates[i]);
        }

    EXPECT_EQ (tForegroundTransparency, testStyle.tForegroundTransparency);
    EXPECT_EQ (tBackgroundTransparency, testStyle.tBackgroundTransparency);
    EXPECT_EQ (tImageTransparency, testStyle.tImageTransparency);
    };

/*---------------------------------------------------------------------------------**//**
* Verify import of raster
* @bsimethod                                    Marc.Bedard                     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnRasterTest, ImportTest)
    {
    SetupProject();
    
    size_t gcount=0, gcount2=0;

    RasterFrameElementCollection rastersCollection(*model0);
    EXPECT_FALSE(rastersCollection.empty());

    FOR_EACH(ElementHandleCR rasterEh , rastersCollection)
        {
        RasterFrameHandler* pQuery(dynamic_cast<RasterFrameHandler*>(&rasterEh.GetHandler()));
        if (pQuery!=NULL)
            {
            gcount++;
            WString sourceUrl(pQuery->GetSourceUrl(rasterEh));

            LOG.tracev (L"Raster.ImportTest import Attachment Name=[%ls]", sourceUrl.c_str());
            }
        }

    for (RasterFrameElementIterator iter = rastersCollection.begin(); iter != rastersCollection.end(); ++iter)
        {

        RasterFrameHandler* pQuery(dynamic_cast<RasterFrameHandler*>(&(*iter).GetHandler()));
        if (pQuery!=NULL)
            {
            gcount2++;
            }
        }

    EXPECT_EQ( gcount, 6 );
    EXPECT_EQ( gcount, gcount2 );
    }

/*---------------------------------------------------------------------------------**//**
* Verify import parameters
* @bsimethod                           Algirdas.Mikoliunas                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnRasterTest, CheckImportParameters)
    {
    SetupProject();
    
    RgbColorDef blackColor = {0, 0, 0}, whiteColor = {255, 255, 255}, greyColor = {53, 37, 53};
    uint32_t blackColorIndex = GetModelColorIndex(blackColor);
    uint32_t whiteColorIndex = GetModelColorIndex(whiteColor);
    uint32_t greyColorIndex = GetModelColorIndex(greyColor);

    const int viewStatesCount = 8;
    int i;
    bool testViewStates[8];
    GetViewStates(testViewStates);
    
    TestRasterProperties rasterTests[6], rasterTest;
    rasterTests[0].SetTestRasterProperties (
        L"dgndb://{1}/SebPattern2.tif", L"", L"", true, false, false, false, DVec3d::From(2000, 0,0), DVec3d::From(0, 2000,0), 
        DPoint3d::From (0.0, 2100, 0.0), DPoint2d::From(2000, 2000), DPoint2d::From(600, 600),
        testViewStates, viewStatesCount, false, true, true, true, false, -5, whiteColorIndex, 
        greyColorIndex, 0, 255, 0
    );
    rasterTests[1].SetTestRasterProperties (
        L"dgndb://{2}/SebPattern16.tif", L"", L"", true, false, false, false, DVec3d::From(1999.51, 0,0), DVec3d::From(0, 1999.51,0), 
        DPoint3d::From (0.0, 4200, 0.0), DPoint2d::From(1999.51, 1999.51), DPoint2d::From(600, 600),
        testViewStates, viewStatesCount, false, true, true, false, false, -4, whiteColorIndex, 
        blackColorIndex, 0, 0, 0
    );
    rasterTests[2].SetTestRasterProperties (
        L"dgndb://{3}/SebPattern256.tif", L"", L"", true, false, false, false, DVec3d::From(2000, 0,0), DVec3d::From(0, 2000,0), 
        DPoint3d::From (0.0, 6300, 0.0), DPoint2d::From(2000, 2000), DPoint2d::From(600, 600),
        testViewStates, viewStatesCount, false, true, true, false, false, -3, whiteColorIndex, 
        blackColorIndex, 0, 0, 0
    );
    rasterTests[3].SetTestRasterProperties (
        L"dgndb://{4}/SebPatternGray.tif", L"", L"", true, false, false, false, DVec3d::From(1999.51, 0,0), DVec3d::From(0, 2000,0), 
        DPoint3d::From (0.0, 8400, 0.0), DPoint2d::From(1999.51, 2000), DPoint2d::From(600, 600),
        testViewStates, viewStatesCount, false, true, true, false, false, -2, whiteColorIndex, 
        blackColorIndex, 0, 0, 0
    );
    rasterTests[4].SetTestRasterProperties (
        L"dgndb://{5}/SebPatternMono.tif", L"", L"", true, false, false, false, DVec3d::From(2000, 0,0), DVec3d::From(0, 2000,0), 
        DPoint3d::From (0.0, 0.0, 0.0), DPoint2d::From(2000, 2000), DPoint2d::From(600, 600),
        testViewStates, viewStatesCount, false, true, true, true, false, -6, whiteColorIndex, 
        blackColorIndex, 0, 255, 0
    );
    rasterTests[5].SetTestRasterProperties (
        L"dgndb://{6}/SebPattern.tif", L"", L"", true, false, false, false, DVec3d::From(2000, 0,0), DVec3d::From(0, 2000,0), 
        DPoint3d::From (0.0, 10500, 0.0), DPoint2d::From(2000, 2000), DPoint2d::From(600, 600),
        testViewStates, viewStatesCount, false, true, true, false, false, -1, whiteColorIndex, 
        blackColorIndex, 0, 255, 0
    );


    size_t gcount = 0;

    FOR_EACH(ElementHandleCR rasterEh , RasterFrameElementCollection (*model0))
        {
        RasterFrameHandler* pQuery(dynamic_cast<RasterFrameHandler*>(&rasterEh.GetHandler()));
        if (pQuery!=NULL)
            {

            bool viewStates[viewStatesCount];

            for (i = 0; i < viewStatesCount; i++)
                {
                viewStates[i] = pQuery->GetViewState(rasterEh, i);
                }

            rasterTest.SetTestRasterProperties(
                pQuery->GetSourceUrl(rasterEh), 
                pQuery->GetAttachDescription(rasterEh), 
                pQuery->GetLogicalName(rasterEh), 
                pQuery->GetSnappableState(rasterEh), 
                pQuery->GetLockedState(rasterEh), 
                pQuery->GetViewIndependentState(rasterEh), 
                pQuery->GetOpenReadWrite(rasterEh), 
                pQuery->GetU(rasterEh), 
                pQuery->GetV(rasterEh),
                pQuery->GetOrigin(rasterEh),
                pQuery->GetExtent(rasterEh),
                pQuery->GetScanningResolution(rasterEh),
                viewStates,
                viewStatesCount,
                pQuery->GetInvertState(rasterEh),
                pQuery->GetPrintState(rasterEh),
                pQuery->GetClipState(rasterEh),
                pQuery->GetTransparencyState(rasterEh),
                pQuery->GetBinaryPrintInvertState(rasterEh),
                pQuery->GetDisplayOrder(rasterEh),
                pQuery->GetForegroundColor(rasterEh),
                pQuery->GetBackgroundColor(rasterEh),
                pQuery->GetForegroundTransparencyLevel(rasterEh),
                pQuery->GetBackgroundTransparencyLevel(rasterEh),
                pQuery->GetImageTransparencyLevel(rasterEh)
            );

            rasterTest.IsEqual(rasterTests[gcount]);
            gcount++;
            }
        }
    ASSERT_EQ( gcount, 6 );
    }

/*---------------------------------------------------------------------------------**//**
* Create raster attachment from existing
* @bsimethod                                    Algirdas.Mikoliunas            4/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnRasterTest, CreateAttachmentFromExisting)
    {
    CreateAttachmentTest(true);
    }

/*---------------------------------------------------------------------------------**//**
* Create raster attachment from null
* @bsimethod                                    Algirdas.Mikoliunas            4/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnRasterTest, CreateAttachmentFromNull)
    {
    CreateAttachmentTest(false);
    }

/*---------------------------------------------------------------------------------**//**
* Change raster attachment settings
* @bsimethod                                    Algirdas.Mikoliunas            4/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnRasterTest, ChangeAttachmentSettings)
    {
    SetupProject();

    const int viewStatesCount = 8;
    bool testViewStates[viewStatesCount], updatedTestViewStates[viewStatesCount];
    int i;

    for (i = 0; i < viewStatesCount; i++)
        {
        testViewStates[i] = i % 2 == 0;
        }

    RgbColorDef color1 = {11, 12, 13}, color2 = {222, 223, 224};
    uint32_t color1Index = GetModelColorIndex(color1);
    uint32_t color2Index = GetModelColorIndex(color2);

    TestRasterProperties rasterTest, updatedRasterTest;
    rasterTest.SetTestRasterProperties (
        L"source_url", L"attach_description", L"logical_name", false, true, false, true, DVec3d::From(111, 112, 113), DVec3d::From(222, 223, 224), 
        DPoint3d::From (5.0, 6, 7.0), DPoint2d::From(193.995, 386.25), DPoint2d::From(333, 334),
        testViewStates, viewStatesCount, true, false, false, false, true, -9, color1Index, 
        color2Index, 1, 44, 2
    );

    RasterFrameElementCollection rasterFrameCollection = RasterFrameElementCollection (*model0);
    RasterFrameElementIterator rasterIterator = rasterFrameCollection.begin();
    
    ElementHandleCR rasterEh = *rasterIterator;
    EditElementHandle eeh(rasterEh.GetElementRef(), model0);

    RasterFrameHandler* pQuery(dynamic_cast<RasterFrameHandler*>(&rasterEh.GetHandler()));
    ASSERT_TRUE(pQuery != NULL);

    EXPECT_TRUE (pQuery->SetSourceUrl(eeh, L"source_url"));
    EXPECT_TRUE (pQuery->SetAttachDescription(eeh, L"attach_description"));
    EXPECT_TRUE (pQuery->SetLogicalName(eeh, L"logical_name"));
    EXPECT_TRUE (pQuery->SetSnappableState(eeh, rasterTest.tSnappable));
    EXPECT_TRUE (pQuery->SetLockedState(eeh, rasterTest.tLocked));
    EXPECT_TRUE (pQuery->SetOpenReadWrite(eeh, rasterTest.tOpenReadWrite));
    EXPECT_TRUE (pQuery->SetU(eeh, rasterTest.tUVector, false));
    EXPECT_TRUE (pQuery->SetV(eeh, rasterTest.tVVector, false));
    EXPECT_TRUE (pQuery->SetOrigin(eeh, rasterTest.tOrigin, false));
    EXPECT_TRUE (pQuery->SetScanningResolution(eeh, rasterTest.tScanResolution));
    EXPECT_TRUE (pQuery->SetInvertState(eeh, rasterTest.tInvertState));
    EXPECT_TRUE (pQuery->SetPrintState(eeh, rasterTest.tPrintState));
    EXPECT_TRUE (pQuery->SetClipState(eeh, rasterTest.tClipState));
    EXPECT_TRUE (pQuery->SetTransparencyState(eeh, rasterTest.tTransparencyState));
    EXPECT_TRUE (pQuery->SetBinaryPrintInvertState(eeh, rasterTest.tBinaryInvertState));
    EXPECT_TRUE (pQuery->SetDisplayOrder(eeh, rasterTest.tDisplayOrder));
    EXPECT_TRUE (pQuery->SetForegroundColor(eeh, rasterTest.tForegroundColor));
    EXPECT_TRUE (pQuery->SetBackgroundColor(eeh, rasterTest.tBackgroundColor));
    EXPECT_TRUE (pQuery->SetForegroundTransparencyLevel(eeh, rasterTest.tForegroundTransparency));
    EXPECT_TRUE (pQuery->SetBackgroundTransparencyLevel(eeh, rasterTest.tBackgroundTransparency));
    EXPECT_TRUE (pQuery->SetImageTransparencyLevel(eeh, rasterTest.tImageTransparency));

    for (i = 0; i < viewStatesCount; i++)
        {
        pQuery->SetViewState(eeh, i, testViewStates[i]);
        }
    
    for (i = 0; i < viewStatesCount; i++)
        {
        updatedTestViewStates[i] = pQuery->GetViewState(eeh, i);
        }

    updatedRasterTest.SetTestRasterProperties(
        pQuery->GetSourceUrl(eeh), 
        pQuery->GetAttachDescription(eeh), 
        pQuery->GetLogicalName(eeh), 
        pQuery->GetSnappableState(eeh), 
        pQuery->GetLockedState(eeh), 
        pQuery->GetViewIndependentState(eeh), 
        pQuery->GetOpenReadWrite(eeh), 
        pQuery->GetU(eeh), 
        pQuery->GetV(eeh),
        pQuery->GetOrigin(eeh),
        pQuery->GetExtent(eeh),
        pQuery->GetScanningResolution(eeh),
        updatedTestViewStates,
        viewStatesCount,
        pQuery->GetInvertState(eeh),
        pQuery->GetPrintState(eeh),
        pQuery->GetClipState(eeh),
        pQuery->GetTransparencyState(eeh),
        pQuery->GetBinaryPrintInvertState(eeh),
        pQuery->GetDisplayOrder(eeh),
        pQuery->GetForegroundColor(eeh),
        pQuery->GetBackgroundColor(eeh),
        pQuery->GetForegroundTransparencyLevel(eeh),
        pQuery->GetBackgroundTransparencyLevel(eeh),
        pQuery->GetImageTransparencyLevel(eeh)
    );

    updatedRasterTest.IsEqual(rasterTest);
    }

/*---------------------------------------------------------------------------------**//**
* Create rgb color and try to get it
* @bsimethod                                    Algirdas.Mikoliunas            5/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnRasterTest, CreateAndRetrieveRgbColor)
    {
    SetupProject();

    uint32_t colorIndex = 0;
    RgbColorDef rgbColor = {1, 22, 133}, rgbColorRet;

    EXPECT_EQ(SUCCESS, RasterFrameHandler::ColorIndexFromRgbInModel(colorIndex, *model0, rgbColor));
    EXPECT_EQ(SUCCESS, RasterFrameHandler::RgbFromColorIndexInModel(rgbColorRet, *model0, colorIndex));

    EXPECT_EQ(rgbColor.red, rgbColorRet.red);
    EXPECT_EQ(rgbColor.green, rgbColorRet.green);
    EXPECT_EQ(rgbColor.blue, rgbColorRet.blue);
    }

/*---------------------------------------------------------------------------------**//**
* Raster transformation test
* @bsimethod                                    Algirdas.Mikoliunas            5/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnRasterTest, Transformation)
    {
    SetupProject();

    DPoint3d translation = DVec3d::From(1, 2, 3);
    double scalingX = 10, scalingY = 5, rotation = 45, anorthogonality = 90;

    Transform transformation = RasterTransformFacility::FromMatrixParameters(translation, scalingX, scalingY, rotation, anorthogonality);

    EXPECT_TRUE (RasterTransformFacility::IsValidRasterTransform(transformation));
    EXPECT_FALSE (RasterTransformFacility::Has3dRotation(transformation));

    // Check transformfacility setters/getters
    RasterTransformFacility::SetUV(transformation, DVec3d::From(5, 4, 3), DVec3d::From(9, 8, 7));

    DVec3d u = RasterTransformFacility::GetU(transformation);
    EXPECT_EQ (5, u.x);
    EXPECT_EQ (4, u.y);
    EXPECT_EQ (3, u.z);

    DVec3d v = RasterTransformFacility::GetV(transformation);
    EXPECT_EQ (9, v.x);
    EXPECT_EQ (8, v.y);
    EXPECT_EQ (7, v.z);

    RasterTransformFacility::SetAffinity(transformation, 20);
    EXPECT_NEAR (1.15, RasterTransformFacility::GetAffinity(transformation), 0.01);

    RasterTransformFacility::SetScalingX(transformation, 30);
    EXPECT_NEAR (30, RasterTransformFacility::GetScalingX(transformation), 0.01);

    RasterTransformFacility::SetScalingY(transformation, 40);
    EXPECT_NEAR (40, RasterTransformFacility::GetScalingY(transformation), 0.01);

    RasterTransformFacility::SetTranslation(transformation, DPoint3d::From(51, 52, 53));

    DVec3d trans = RasterTransformFacility::GetTranslation(transformation);
    EXPECT_EQ (51, trans.x);
    EXPECT_EQ (52, trans.y);
    EXPECT_EQ (53, trans.z);

    // Check isValidTransform and IsTranform3D
    RasterFrameElementCollection rasterFrameCollection = RasterFrameElementCollection (*model0);
    RasterFrameElementIterator rasterIterator = rasterFrameCollection.begin();
    
    ElementHandleCR rasterEh = *rasterIterator;
    EditElementHandle eeh(rasterEh.GetElementRef(), model0);

    RasterFrameHandler* pQuery(dynamic_cast<RasterFrameHandler*>(&rasterEh.GetHandler()));
    ASSERT_TRUE(pQuery != NULL);

    EXPECT_TRUE (pQuery->IsValidTransform(transformation));
    EXPECT_TRUE (pQuery->IsTransform3D(transformation));
    }
	
/*---------------------------------------------------------------------------------**//**
* Search Path
* @bsimethod                                    Julija.Suboc                     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnRasterTest, SearchPath)
    {
    SetupProject();
    RasterFrameElementCollection rastersCollection(*model0);
    //Get directory where file is placed
    DgnDbP    dgnFile = model0->GetDgnDb();
    WString fileDirectory = BeFileName::GetDirectoryName(dgnFile->GetFileName().c_str());
    FOR_EACH(ElementHandleCR rasterEh , rastersCollection)
        {
        RasterFrameHandler* pQuery(dynamic_cast<RasterFrameHandler*>(&rasterEh.GetHandler()));
        if (pQuery!=NULL)
            {
            WString searchDirectory = BeFileName::GetDirectoryName(pQuery->GetSearchPath(model0).c_str());
            EXPECT_TRUE(BeFileName::IsDirectory(searchDirectory.c_str()))<<"Search directory does not exists";
            int cmp = searchDirectory.CompareTo(fileDirectory);
            EXPECT_TRUE(cmp==0)<<"Raster search directory does not match file directory";
            }
        }
    }
#endif
