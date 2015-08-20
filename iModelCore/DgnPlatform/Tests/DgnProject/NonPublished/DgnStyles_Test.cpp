/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnStyles_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

#ifdef STYLE_REWRITE_06

USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// @bsiclass                                                Algirdas.Mikoliunas   01/13
//=======================================================================================
struct TestStyleProperties
    {
    public:
        DgnStyleId tsId;
        DgnStyleType tsType;
        WString tsName;
        WString tsDescription;

        void SetTestStyleProperties (DgnStyleType type, WString name,  WString description)
            {
            tsType = type;
            tsName = name;
            tsDescription = description;
            };
        void IsEqual (TestStyleProperties testStyle)
            {
            EXPECT_TRUE (tsType == testStyle.tsType) << "Types don't match";
            EXPECT_STREQ (tsName.c_str(), testStyle.tsName.c_str()) << "Names don't match";
            EXPECT_STREQ (tsDescription.c_str(), testStyle.tsDescription.c_str()) << "Descriptions don't match";
            };
    };


/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnStyles
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnStylesTest : public ::testing::Test
    {
    public:
        ScopedDgnHost           m_host;
        DgnDbPtr      project;

        void SetupProject (WCharCP projFile, Db::OpenMode mode);
    };

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .dgndb project file
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnStylesTest::SetupProject (WCharCP projFile, Db::OpenMode mode)
    {
    DgnDbTestDgnManager tdm (projFile, __FILE__, mode);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Test for reading from line style table
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, ReadLineStyles)
    {
    SetupProject (L"SubStation_NoFence.i.idgndb", Db::OPEN_ReadWrite);
    
    //Get line styles
    DgnStyles& styleTable = project->Styles();
    DgnStyles::Iterator iterLineStyles = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    EXPECT_EQ(4, iterLineStyles.QueryCount()) <<"The expected styles count is 4 where as it is: " << iterLineStyles.QueryCount();

    //Iterate through each line style and make sure they have correct information
    TestStyleProperties testStyles[4], testStyle;
    testStyles[0].SetTestStyleProperties (DgnStyleType::Line, L"Continuous", L"");
    testStyles[1].SetTestStyleProperties (DgnStyleType::Line, L"DGN Style 4", L"");
    testStyles[2].SetTestStyleProperties (DgnStyleType::Line, L"DGN Style 5", L"");
    testStyles[3].SetTestStyleProperties (DgnStyleType::Line, L"REFXA$0$TRAZO_Y_PUNTO", L"");

    int i = 0;
    for (DgnStyles::Iterator::Entry const& entry : iterLineStyles)
        {
        WString entryNameW (entry.GetName(), true);
        WString entryDescriptionW (entry.GetDescription(), true);
        testStyle.SetTestStyleProperties (entry.GetType(), entryNameW.c_str(), entryDescriptionW.c_str());
        testStyle.IsEqual(testStyles[i]);
        i++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Read lines styles descending order
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, ReadLineStylesDsc)
    {
    SetupProject (L"SubStation_NoFence.i.idgndb", Db::OPEN_ReadWrite);

    //Get line styles
    DgnStyles& styleTable = project->Styles();
    DgnStyles::Iterator iterLineStyles = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameDsc);
    EXPECT_EQ (4, iterLineStyles.QueryCount()) <<"The expected styles count is 4 where as it is: " << iterLineStyles.QueryCount();

    //Iterate through each line style and make sure they have correct information
    TestStyleProperties testStyles[4], testStyle;
    testStyles[0].SetTestStyleProperties (DgnStyleType::Line, L"REFXA$0$TRAZO_Y_PUNTO", L"");
    testStyles[1].SetTestStyleProperties (DgnStyleType::Line, L"DGN Style 5", L"");
    testStyles[2].SetTestStyleProperties (DgnStyleType::Line, L"DGN Style 4", L"");
    testStyles[3].SetTestStyleProperties (DgnStyleType::Line, L"Continuous", L"");

    int i = 0;
    for (DgnStyles::Iterator::Entry const& entry : iterLineStyles)
        {
        WString entryNameW (entry.GetName(), true);
        WString entryDescriptionW (entry.GetDescription(), true);
        testStyle.SetTestStyleProperties (entry.GetType(), entryNameW.c_str(), entryDescriptionW.c_str());
        testStyle.IsEqual (testStyles[i]);
        i++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test new style insert
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertLineStyle)
    {
    SetupProject (L"SubStation_NoFence.i.idgndb", Db::OPEN_ReadWrite);

    //Get line styles
    DgnStyles& styleTable = project->Styles();
    DgnStyles::Iterator iterLineStyles = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    EXPECT_EQ (4, iterLineStyles.QueryCount()) << "The expected styles count is 4 where as it is: " << iterLineStyles.QueryCount();

    // Add new line style
    DgnStyleId newStyleId;
    EXPECT_EQ(BE_SQLITE_DONE, styleTable.LineStyles().Insert(newStyleId, "ATestLineStyle", 6, 3, 53, 0.0)) << "Insert line style return value should be BE_SQLITE_DONE";
    EXPECT_TRUE((int32_t)newStyleId.GetValue() > 0) << "Inserted line style id should be more than 0";

    // Assure that now we have 5 line styles
    DgnStyles::Iterator iterAddedLineStyles = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    EXPECT_EQ (5, iterAddedLineStyles.QueryCount()) <<"The expected styles count is 5 where as it is: " << iterAddedLineStyles.QueryCount();
    
    // Test inserted style properties
    DgnStyles::Iterator::Entry const& entry = iterAddedLineStyles.begin();
    WString entryNameW (entry.GetName(), true);
    WString entryDescriptionW (entry.GetDescription(), true);

    TestStyleProperties expectedTestStyle, testStyle;
    expectedTestStyle.SetTestStyleProperties (DgnStyleType::Line, L"ATestLineStyle", L"");
    testStyle.SetTestStyleProperties (entry.GetType(), entryNameW.c_str(), entryDescriptionW.c_str());
    expectedTestStyle.IsEqual (testStyle);
    }

/*---------------------------------------------------------------------------------**//**
* Test new style insert
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertLineStyleWithId)
    {
    SetupProject (L"SubStation_NoFence.i.idgndb", Db::OPEN_ReadWrite);

    //Get line styles
    DgnStyles& styleTable = project->Styles();
    DgnStyles::Iterator iterLineStyles = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    EXPECT_EQ (4, iterLineStyles.QueryCount()) <<"The expected styles count is 4 where as it is: " << iterLineStyles.QueryCount();

    // Add new line style
    DgnStyleId newStyleId;
    EXPECT_EQ(BE_SQLITE_DONE, styleTable.LineStyles().Insert(newStyleId, "ATestLineStyle", 6, 3, 53, 0.0)) << "Insert line style return value should be BE_SQLITE_DONE";
    
    // Assure that now we have 5 line styles
    DgnStyles::Iterator iterAddedLineStyles = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    EXPECT_EQ(5, iterAddedLineStyles.QueryCount()) <<"The expected styles count is 5 where as it is: " << iterAddedLineStyles.QueryCount();

    // Test inserted style properties
    DgnStyles::Iterator::Entry const& entry = iterAddedLineStyles.begin();
    WString entryNameW (entry.GetName(), true);
    WString entryDescriptionW (entry.GetDescription(), true);

    TestStyleProperties expectedTestStyle, testStyle;
    expectedTestStyle.SetTestStyleProperties (DgnStyleType::Line, L"ATestLineStyle", L"");
    testStyle.SetTestStyleProperties (entry.GetType(), entryNameW.c_str(), entryDescriptionW.c_str());
    expectedTestStyle.IsEqual(testStyle);

    EXPECT_EQ(20, entry.GetId().GetValue()) << "Inserted entry id is not equal to 20";
    }

/*---------------------------------------------------------------------------------**//**
* Test insert line style with already existing name
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertLineStyleWithExistingName)
    {
    SetupProject (L"SubStation_NoFence.i.idgndb", Db::OPEN_ReadWrite);
    
    //Get line styles
    DgnStyles& styleTable = project->Styles();
    DgnStyles::Iterator iterLineStyles = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    EXPECT_EQ(4, iterLineStyles.QueryCount()) <<"The expected styles count is 4 where as it is: " << iterLineStyles.QueryCount();
    
    // Add new line style
    DgnStyleId newStyleId;
    BeSQLite::DbResult insertResult = BE_SQLITE_ERROR;
    
    BeTest::SetFailOnAssert (false);
    insertResult = styleTable.LineStyles().Insert(newStyleId, "Continuous", 6, 3, 53, 0.0);
    BeTest::SetFailOnAssert (true);
    
    EXPECT_NE(BE_SQLITE_OK, insertResult) << "Line style with existing name insertion must return not BE_SQLITE_OK";
    
    DgnStyles::Iterator iter = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    EXPECT_EQ(4, iter.QueryCount()) <<"The expected styles count is 4 where as it is: " << iter.QueryCount();
    }

/*---------------------------------------------------------------------------------**//**
* Test update line style
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, UpdateLineStyleTable)
    {
    SetupProject (L"SubStation_NoFence.i.idgndb", Db::OPEN_ReadWrite);

    //Get line styles
    DgnStyles& styleTable = project->Styles();
    DgnStyles::Iterator iter = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    DgnStyles::Iterator::Entry const& entry = iter.begin();
    EXPECT_STREQ("Continuous", entry.GetName());

    // Update style name and other data
    EXPECT_EQ(SUCCESS, styleTable.LineStyles().Update(entry.GetId(), "ATestLineStyle", 8, 4, 54, 1.0)) << "UpdateLineStyle must return value SUCCESS";

    // Check if line style name changed
    DgnStyles::Iterator iterUpdated = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    DgnStyles::Iterator::Entry const& updatedEntry = iterUpdated.begin();
    EXPECT_STREQ("ATestLineStyle", updatedEntry.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* Test update line style with existing name
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, UpdateLineStyleWithExistingName)
    {
    SetupProject (L"SubStation_NoFence.i.idgndb", Db::OPEN_ReadWrite);

    //Get line styles
    DgnStyles& styleTable = project->Styles();
    DgnStyles::Iterator iter = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    DgnStyles::Iterator::Entry const& entry = iter.begin();
    EXPECT_STREQ("Continuous", entry.GetName());
    
    // This would normally trigger an assertion failure.
    BeTest::SetFailOnAssert (false);
    BentleyStatus updateResult = styleTable.LineStyles().Update(entry.GetId(), "DGN Style 4", 7, 3, 53, 0.0);
    BeTest::SetFailOnAssert (true);

    EXPECT_NE(SUCCESS, updateResult) << "UpdateLineStyle with existing name must not be successfull";
    
    // Check if line style name changed
    DgnStyles::Iterator iterUpdated = styleTable.LineStyles().MakeIterator(DgnStyleSort::NameAsc);
    DgnStyles::Iterator::Entry const& updatedEntry = iterUpdated.begin();
    EXPECT_STREQ("Continuous", updatedEntry.GetName());
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* Test iterator entry get data
* @bsimethod                                    Algirdas.Mikoliunas          02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, IteratorEntryGetData)
    {
    SetupProject (L"SubStation_NoFence.i.idgndb", Db::OPEN_ReadWrite);
    
    //Get display styles
    DgnStyles& styleTable = project->Styles();
    DgnStyles::Iterator iter = styleTable.DisplayStyles().MakeIterator(DgnStyleSort::NameAsc);
    DgnStyles::Iterator::Entry entry = iter.begin();

    Utf8String styleData = "<DisplayStyle Usages=\"1\"><Flags DisplayVisibleEdges=\"true\" IgnoreGeometryMaps=\"false\" VisibleEdgeColor=\"true\" VisibleEdgeStyle=\"false\" FillColor=\"true\"/><Overrides DisplayMode=\"6\" VisibleEdgeColor=\"0\" VisibleEdgeWeight=\"0\" HiddenEdgeWeight=\"0\" Transparency=\"0\" BackgroundColor=\"0\" FillColor=\"2310\" LineStyle=\"0\" LineWeight=\"0\" Material=\"0\"/></DisplayStyle>";
    EXPECT_STREQ(styleData.c_str(), (Utf8CP)entry.GetData());
    EXPECT_EQ(styleData.length() + 1, entry.GetDataSize());
    }
#endif

#endif
