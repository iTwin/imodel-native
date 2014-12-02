/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnModel_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
USING_NAMESPACE_BENTLEY_SQLITE
//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnModelTests : public testing::Test
    {
     public:
        ScopedDgnHost m_autoDgnHost;
        DgnProjectPtr m_project;    
        DgnModelP m_modelP;
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        // Prepares test data file
        //---------------------------------------------------------------------------------------
        void SetUp()
            {
            DgnDbTestDgnManager tdm(L"XGraphicsElements.idgndb", __FILE__, OPENMODE_READWRITE);
            m_project = tdm.GetDgnProjectP();
           
            }
         //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        //---------------------------------------------------------------------------------------
        void LoadModel(Utf8CP name)
            {
            DgnModels& modelTable =  m_project->Models();
            DgnModelId id = modelTable.QueryModelId(name);
            m_modelP =  modelTable.GetModelById (id);
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetGraphicElements)
    {
    LoadModel("Splines");
    UInt32 graphicElementCount = m_modelP->GetElementCount();
    ASSERT_TRUE(graphicElementCount > 0)<<"Please provide model with graphics elements, otherwise this test case makes no sense";
    GraphicElementRefList* graphicList = m_modelP->GetGraphicElementsP();
    int count = 0;
    if (graphicList != NULL)
        {
        for(PersistentElementRefP elm: *graphicList)
            {
            EXPECT_TRUE(elm->GetDgnModelP() == m_modelP);
            ++count;
            }
        }
    EXPECT_EQ(graphicElementCount, count);
    //Try to get elements from empty model
    LoadModel("Default");
    EXPECT_EQ(0, m_modelP->GetElementCount())<<"This model should be empty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetName)
    {
    LoadModel("Splines");
    Utf8String name(m_modelP->GetModelName());
    EXPECT_TRUE(name.CompareTo("Splines")==0);
    Utf8String newName("New Long model name Longer than expectedNew Long model name Longer"
        " than expectedNew Long model name Longer than expectedNew Long model name Longer than expectedNew Long model");
    DgnModelStatus status;
    DgnModels& modelTable =  m_project->Models();
    modelTable.CreateNewModelFromSeed(&status, newName.c_str(), m_modelP->GetModelId());
    EXPECT_TRUE(status == DGNMODEL_STATUS_Success)<<"Failed to create model";
    DgnModelId id = modelTable.QueryModelId(newName.c_str());
    ASSERT_TRUE(id.IsValid());
    m_modelP =  modelTable.GetModelById (id);
    Utf8String nameToVerify(m_modelP->GetModelName());
    EXPECT_TRUE(newName.CompareTo(nameToVerify.c_str())==0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, EmptyList)
    {
    LoadModel("Splines");
    m_modelP->Empty();
    ASSERT_EQ(0, m_modelP->GetElementCount())<<"Failed to empty element list in model";
    LoadModel("Splines");
    ASSERT_EQ(0, m_modelP->GetElementCount())<<"Failed to empty element list in model";
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetRange)
    {
    DgnDbTestDgnManager tdm(L"ModelRangeTest.idgndb", __FILE__, OPENMODE_READWRITE);
    m_project = tdm.GetDgnProjectP();
    LoadModel("RangeTest");
    //Use first function to get range
    DRange3d firstRange;
    StatusInt status = m_modelP->GetRange(firstRange);
    ASSERT_EQ(0, status)<<"Failed to get range";
    //Use second function to get range
    DRange3d secondRange;
    status = m_modelP->GetRange(secondRange);
    ASSERT_EQ(0, status)<<"Failed to get range";
    //Use third function to get range
    DRange3d thirdRange;
    EXPECT_EQ(BE_SQLITE_OK , m_modelP->QueryModelRange(thirdRange))<<"BE_SQLITE_OK is expected to be returned for successfull function results";
    
    //Verify ranges returned by second and third function
    EXPECT_TRUE(secondRange.IsEqual(thirdRange))<<"Diffrent ranges are returned for the same model.";
    //Verify first function results with second function results
    EXPECT_EQ(firstRange.low.x, secondRange.low.x)<<"x low does not match";
    EXPECT_EQ(firstRange.low.y, secondRange.low.y)<<"y low does not match";
    EXPECT_EQ(firstRange.low.z, secondRange.low.z)<<"z low does not match";
    EXPECT_EQ(firstRange.high.x, secondRange.high.x)<<"x high does not match";
    EXPECT_EQ(firstRange.high.y, secondRange.high.y)<<"y high does not match";
    EXPECT_EQ(firstRange.high.z, secondRange.high.z)<<"z high does not match";
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetRangeOfEmptyModelFromFileWithElementsInAnotherModel)
    {
    DgnDbTestDgnManager tdm(L"ModelRangeTest.idgndb", __FILE__, OPENMODE_READWRITE);
    m_project = tdm.GetDgnProjectP();
    LoadModel("Default");
    //Use first function to get range
    DRange3d firstRange;
    StatusInt status = m_modelP->GetRange(firstRange);
    ASSERT_EQ(0, status)<<"Failed to get range";
    //Use second function to get range
    DRange3d secondRange;
    status = m_modelP->GetRange(secondRange);
    ASSERT_EQ(0, status)<<"Failed to get range";
    //Use third function to get range
    DRange3d thirdRange;
    EXPECT_EQ(BE_SQLITE_OK , m_modelP->QueryModelRange(thirdRange))<<"BE_SQLITE_OK is expected to be returned for successfull function results";
    
    //Verify ranges returned by second and third function
    DRange3d emptyRange;
    emptyRange.InitFrom(0, 0, 0, 0, 0, 0);
    EXPECT_TRUE(thirdRange.IsEqual(emptyRange))<<"Diffrent ranges are returned for the same model.";
    EXPECT_TRUE(secondRange.IsEqual(emptyRange))<<"Diffrent ranges are returned for the same model.";
    //Verify first function results with second function results
    EXPECT_EQ(firstRange.low.x, emptyRange.low.x)<<"x low does not match";
    EXPECT_EQ(firstRange.low.y, emptyRange.low.y)<<"y low does not match";
    EXPECT_EQ(firstRange.low.z, emptyRange.low.z)<<"z low does not match";
    EXPECT_EQ(firstRange.high.x, emptyRange.high.x)<<"x high does not match";
    EXPECT_EQ(firstRange.high.y, emptyRange.high.y)<<"y high does not match";
    EXPECT_EQ(firstRange.high.z, emptyRange.high.z)<<"z high does not match";
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetRangeOfEmptyModelFromFileWithNoElements)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, OPENMODE_READWRITE);
    m_project = tdm.GetDgnProjectP();
    LoadModel("Default");
    //Use first function to get range
    DRange3d firstRange;
    StatusInt status = m_modelP->GetRange(firstRange);
    ASSERT_EQ(0, status)<<"Failed to get range";
    //Use second function to get range
    DRange3d secondRange;
    status = m_modelP->GetRange(secondRange);
    ASSERT_EQ(0, status)<<"Failed to get range";
    //Use third function to get range
    DRange3d thirdRange;
    EXPECT_EQ(BE_SQLITE_OK , m_modelP->QueryModelRange(thirdRange))<<"BE_SQLITE_OK is expected to be returned for successfull function results";
    
    //Verify ranges returned by second and third function
    DRange3d emptyRange;
    emptyRange.InitFrom(0, 0, 0, 0, 0, 0);
    EXPECT_TRUE(thirdRange.IsEqual(emptyRange))<<"Diffrent ranges are returned for the same model.";
    EXPECT_TRUE(secondRange.IsEqual(emptyRange))<<"Diffrent ranges are returned for the same model.";
    //Verify first function results with second function results
    EXPECT_EQ(firstRange.low.x, emptyRange.low.x)<<"x low does not match";
    EXPECT_EQ(firstRange.low.y, emptyRange.low.y)<<"y low does not match";
    EXPECT_EQ(firstRange.low.z, emptyRange.low.z)<<"z low does not match";
    EXPECT_EQ(firstRange.high.x, emptyRange.high.x)<<"x high does not match";
    EXPECT_EQ(firstRange.high.y, emptyRange.high.y)<<"y high does not match";
    EXPECT_EQ(firstRange.high.z, emptyRange.high.z)<<"z high does not match";
    }
