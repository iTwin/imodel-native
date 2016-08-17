/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/AnnotationTable_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/AnnotationTable.h>
#include <ECDb/ECSqlStatement.h>
#include <ECObjects/DesignByContract.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM

// Map of aspect name to count
typedef    bmap<Utf8String, size_t>     AspectCountMap;
typedef    bpair<Utf8String, size_t>    AspectCountEntry;

struct TestAnnotationTableAspectDescr
    {
    Utf8String  m_className;
    bool        m_isUniqueAspect;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<TestAnnotationTableAspectDescr> const& getAspectDescrs ()
    {
    static bvector<TestAnnotationTableAspectDescr> s_aspectDescrs;

    if ( ! s_aspectDescrs.empty())
        return s_aspectDescrs;

    s_aspectDescrs = 
        {
        { BIS_SCHEMA(BIS_CLASS_AnnotationTableHeader),   true    },
        { BIS_SCHEMA(BIS_CLASS_AnnotationTableRow),      false   },
        { BIS_SCHEMA(BIS_CLASS_AnnotationTableColumn),   false   },
        { BIS_SCHEMA(BIS_CLASS_AnnotationTableCell),     false   },
        { BIS_SCHEMA(BIS_CLASS_AnnotationTableMerge),    false   },
        { BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology),false   },
        { BIS_SCHEMA(BIS_CLASS_AnnotationTableEdgeRun),  false   },
        };

    return s_aspectDescrs;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ExpectedAspectCounts
{
private:
    AspectCountMap m_expectedCounts;

public:
    /* ctor */  ExpectedAspectCounts ()
        {
        // Every table has exactly one table data aspect
        AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableHeader), 1);

        // Every table at least one symbology aspect
        AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 1);
        }

    /* ctor */  ExpectedAspectCounts (uint32_t rowsExpected, uint32_t colsExpected, uint32_t cellsExpected, uint32_t mergesExpected)
        :
        ExpectedAspectCounts()
        {
        if (0 < rowsExpected)
            AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableRow), rowsExpected);

        if (0 < colsExpected)
            AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableColumn), colsExpected);

        if (0 < cellsExpected)
            AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableCell), cellsExpected);

        if (0 < mergesExpected)
            AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableMerge), mergesExpected);
        }

    void        AddEntry (Utf8CP className, size_t count)
        {
        m_expectedCounts[className] = count;
        }

    Utf8String  BuildSelectCountString (TestAnnotationTableAspectDescr const& aspectDescr)
        {
        Utf8String sqlString ("SELECT count(*) FROM ");
        sqlString.append (aspectDescr.m_className);
        sqlString.append (" WHERE ElementId=?");
        return sqlString;
        }

    int         GetActualCount (TestAnnotationTableAspectDescr const& aspectDescr, DgnElementId elementId, DgnDbCR db)
        {
        Utf8String sqlString = BuildSelectCountString (aspectDescr);
        ECSqlStatement statement;
        ECSqlStatus prepareStatus = statement.Prepare (db, sqlString.c_str());
        EXPECT_TRUE (ECSqlStatus::Success == prepareStatus);

        statement.BindId(1, elementId);

        return (BE_SQLITE_ROW == statement.Step()) ? static_cast<size_t>(statement.GetValueInt (0)) : 0;
        }

    void        GetActualCounts (AspectCountMap& counts, DgnElementId elementId, DgnDbCR db)
        {
        bvector<TestAnnotationTableAspectDescr> const& aspectDescrs = getAspectDescrs ();

        for (TestAnnotationTableAspectDescr const& aspectDescr : aspectDescrs)
            {
            int count = GetActualCount (aspectDescr, elementId, db);

            if (0 != count)
                counts[aspectDescr.m_className] = count;
            }
        }

    void        VerifyCounts (AnnotationTableCR table)
        {
        AspectCountMap actualCounts;

        GetActualCounts (actualCounts, table.GetElementId(), table.GetDgnDb());

        for (AspectCountEntry const& entry: m_expectedCounts)
            {
            Utf8String const&                   className       = entry.first;
            size_t                              expectedCount   = entry.second;
            AspectCountMap::const_iterator      matchingEntry   = actualCounts.find(className);
            size_t                              actualCount     = 0;

            if (actualCounts.end() != matchingEntry)
                actualCount = (*matchingEntry).second;

            EXPECT_EQ (expectedCount, actualCount) << "Aspect count mismatch for table: " << className.c_str();

            actualCounts.erase (className);
            }

        // There are aspects on the element that were not expected.
        if (actualCounts.empty())
            return;

        for (AspectCountEntry const& entry: actualCounts)
            {
            Utf8String const&                   className   = entry.first;
            size_t                              actualCount = entry.second;

            EXPECT_EQ (0, actualCount) << "Aspect count mismatch for table: " << className.c_str();
            }
        }
};

#define TEST_MODEL_NAME     "TestModel"
#define TEST_CATEGORY_NAME  "TestCategory"
#define TEST_TEXTSTYLE_NAME "TestTextStyle"

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct AnnotationTableTest : ::testing::Test
{
private:

Dgn::ScopedDgnHost m_testHost;
DgnDbPtr m_dgnDb;

public:

static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;

AnnotationTableTest() { }

public: static void SetUpTestCase();
public: static void TearDownTestCase();

DgnDbR                  GetDgnDb()
    {
    if (m_dgnDb.IsNull())
        m_dgnDb = DgnDbTestUtils::OpenSeedDbCopy(s_seedFileInfo.fileName);

    return *m_dgnDb;
    }

DgnModelId              GetModelId()            { return GetDgnDb().Models().QueryModelId(DgnModel::CreateModelCode(TEST_MODEL_NAME)); }
DgnCategoryId           GetCategoryId()         { return DgnCategory::QueryCategoryId (TEST_CATEGORY_NAME, GetDgnDb()); }
DgnElementId            GetTextStyleId()        { return AnnotationTextStyle::QueryId (GetDgnDb(), TEST_TEXTSTYLE_NAME); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTablePtr    CreateBasicTable (int numRows = 5, int numCols = 3)
    {
    DgnDbR          db          = GetDgnDb();
    DgnModelId      modelId     = GetModelId();
    DgnCategoryId   categoryId  = GetCategoryId();

    AnnotationTable::CreateParams    createParams (db, modelId, AnnotationTable::QueryClassId(db), categoryId);
    AnnotationTablePtr               tableElement = AnnotationTable::Create(numRows, numCols, GetTextStyleId(), 0, createParams);
    EXPECT_TRUE (tableElement.IsValid());

    return tableElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    InsertElement (AnnotationTableR element)
    {
    AnnotationTableCPtr  insertedElement = element.Insert();
    EXPECT_TRUE(insertedElement.IsValid());

    DgnElementId elementId = insertedElement->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    return elementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void        UpdateElement (AnnotationTableR element)
    {
    AnnotationTableCPtr  updatedElement = element.Update();
    EXPECT_TRUE(updatedElement.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    CreateBasicTablePersisted (int numRows = 5, int numCols = 3)
    {
    AnnotationTablePtr   tableElement = CreateBasicTable (numRows, numCols);
    return InsertElement (*tableElement);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct MergeDescr
    {
    AnnotationTableCellIndex    m_rootIndex;
    uint32_t                    m_rowSpan;
    uint32_t                    m_colSpan;

    MergeDescr (AnnotationTableCellIndexCR i, uint32_t r, uint32_t c) : m_rootIndex(i), m_rowSpan(r), m_colSpan(c) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void     BuildExpectedCellList (bvector<AnnotationTableCellIndex>& expectedCells, uint32_t numRows, uint32_t numCols, bvector<AnnotationTableCellIndex> const* exclusions)
    {
    for (uint32_t iRow = 0; iRow < numRows; iRow++)
        {
        for (uint32_t iCol = 0; iCol < numCols; iCol++)
            {
            AnnotationTableCellIndex index (iRow, iCol);

            if (NULL != exclusions)
                {
                if (exclusions->end() != std::find (exclusions->begin(), exclusions->end(), index))
                    continue;
                }

            expectedCells.push_back (index);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void     VerifyCellCollection (AnnotationTableCR table, bvector<AnnotationTableCellIndex> const* exclusions)
    {
    bvector<AnnotationTableCellIndex> expectedCells;
    BuildExpectedCellList (expectedCells, table.GetRowCount(), table.GetColumnCount(), exclusions);

    uint32_t iCell = 0;

    for (AnnotationTableCellCR cell : table.GetCellCollection())
        {
        AnnotationTableCellIndex  foundIndex      = cell.GetIndex();
        AnnotationTableCellIndex  expectedIndex   = expectedCells[iCell++];

        ASSERT_TRUE (expectedIndex == foundIndex);
        }

    ASSERT_EQ (expectedCells.size(), iCell);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void     VerifyCellsWithMergeBlocks (AnnotationTableCR table, bvector<MergeDescr> mergeBlocks)
    {
    // Verify that the expected cells were consumed
    bvector<AnnotationTableCellIndex> cellsThatWereConsumed;

    for (MergeDescr const& merge : mergeBlocks)
        {
        bool    skippedFirst = false;
        size_t  oldConsumedCount = cellsThatWereConsumed.size();

        for (uint32_t iRow = merge.m_rootIndex.row; iRow < merge.m_rootIndex.row + merge.m_rowSpan; iRow++)
            {
            for (uint32_t iCol = merge.m_rootIndex.col; iCol < merge.m_rootIndex.col + merge.m_colSpan; iCol++)
                {
                if ( ! skippedFirst)
                    { skippedFirst = true; continue; }

                cellsThatWereConsumed.push_back (AnnotationTableCellIndex (iRow, iCol));
                }
            }

        // double check
        EXPECT_EQ (merge.m_rowSpan * merge.m_colSpan - 1, cellsThatWereConsumed.size() - oldConsumedCount);
        }

    AnnotationTableTest::VerifyCellCollection (table, &cellsThatWereConsumed);
    }

}; // AnnotationTableTest

DgnDbTestUtils::SeedDbInfo AnnotationTableTest::s_seedFileInfo;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableTest::SetUpTestCase()
    {
    ScopedDgnHost tempHost;

    //  Request a root seed file.
    DgnDbTestUtils::SeedDbInfo rootSeedInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, true));

    AnnotationTableTest::s_seedFileInfo = rootSeedInfo;
    AnnotationTableTest::s_seedFileInfo.fileName.SetName(L"AnnotationTableTest/AnnotationTableTest.bim");

    // Make a copy of the root seed which will be customized as a seed for tests in this group
    DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(rootSeedInfo.fileName, AnnotationTableTest::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());

    // Create a category
    DgnCategory category(DgnCategory::CreateParams(*db, TEST_CATEGORY_NAME, DgnCategory::Scope::Physical));
    DgnSubCategory::Appearance appearance;
    category.Insert(appearance);

    ASSERT_TRUE (category.GetCategoryId().IsValid());

    // Create a text style
    AnnotationTextStylePtr textStyle = AnnotationTextStyle::Create(*db);
    textStyle->SetName(TEST_TEXTSTYLE_NAME);
    textStyle->SetHeight(0.25);
    textStyle->SetFontId(db->Fonts().AcquireId(DgnFontManager::GetAnyLastResortFont()));
    textStyle->Insert();

    ASSERT_TRUE(textStyle->GetElementId().IsValid());

    // Create a 2d model
    DgnModelPtr model = new GeometricModel2d(GeometricModel2d::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_GeometricModel2d)), DgnModel::CreateModelCode(TEST_MODEL_NAME)));
    ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

    ASSERT_TRUE(model->GetModelId().IsValid());

#define WANT_VIEW
#if defined (WANT_VIEW)
    // This is only here to aid in debugging so you can open the file in a viewer and see the element you just created.
    //.........................................................................................
    DrawingViewDefinition view(*db, "AnnotationTableTest", model->GetModelId());
    EXPECT_TRUE(view.Insert().IsValid());

    DRange3d  madeUpRange = DRange3d::From (DPoint3d::From(-10.0, -10.0, -10.0), DPoint3d::From(10.0, 10.0, 10.0));

    ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);

    DrawingViewController viewController(view);
    viewController.SetStandardViewRotation(StandardView::Top);
    viewController.LookAtVolume(madeUpRange, nullptr, &viewMargin);
    //viewController.LookAtVolume(insertedAnnotationElement->CalculateRange3d(), nullptr, &viewMargin);
    viewController.GetViewFlagsR().SetRenderMode(Render::RenderMode::Wireframe);
    viewController.ChangeCategoryDisplay(category.GetCategoryId(), true);
    viewController.ChangeModelDisplay(model->GetModelId(), true);

    EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
#endif

    // Save the customized seed for use by all the tests in this group
    db->SaveSettings();
    db->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void AnnotationTableTest::TearDownTestCase()
    {
    // Note: leave your subdirectory in place. Don't remove it. That allows the 
    // base class to detect and throw an error if two groups try to use a directory of the same name.
    // Don't worry about stale data. The test runner will clean out everything at the start of the program.
    // You can empty the directory, if you want to save space.
    DgnDbTestUtils::EmptySubDirectory(AnnotationTableTest::s_seedFileInfo.fileName.GetDirectoryName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableTest, BasicCreate)
    {
    AnnotationTablePtr   tableElement = CreateBasicTable ();

    EXPECT_EQ (5, tableElement->GetRowCount ());
    EXPECT_EQ (3, tableElement->GetColumnCount ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableTest, BasicPersist)
    {
    int numRows = 5;
    int numCols = 3;

    DgnElementId elementId = CreateBasicTablePersisted (numRows, numCols);

    // Purge the cache so that we don't get a cached element.
    GetDgnDb().Memory().PurgeUntil(0);

    AnnotationTableCPtr readTableElement = AnnotationTable::Get(GetDgnDb(), elementId);
    ASSERT_TRUE(readTableElement.IsValid());

    EXPECT_EQ (numRows, readTableElement->GetRowCount ());
    EXPECT_EQ (numCols, readTableElement->GetColumnCount ());

    // Expect the minimum aspects on the element
    ExpectedAspectCounts expectedCounts;
    expectedCounts.VerifyCounts(*readTableElement);

    GetDgnDb().SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableTest, PersistTwoTables)
    {
    int numRows1 = 5, numCols1 = 3;
    int numRows2 = 8, numCols2 = 2;

    DgnElementId elementId1 = CreateBasicTablePersisted (numRows1, numCols1);
    DgnElementId elementId2 = CreateBasicTablePersisted (numRows2, numCols2);

    // Purge the cache so that we don't get a cached element.
    GetDgnDb().Memory().PurgeUntil(0);

    AnnotationTableCPtr readTableElement = AnnotationTable::Get(GetDgnDb(), elementId1);
    ASSERT_TRUE(readTableElement.IsValid());

    EXPECT_EQ (numRows1, readTableElement->GetRowCount ());
    EXPECT_EQ (numCols1, readTableElement->GetColumnCount ());

    // Expect the minimum aspects on the element
    ExpectedAspectCounts expectedCounts;
    expectedCounts.VerifyCounts(*readTableElement);

    readTableElement = AnnotationTable::Get(GetDgnDb(), elementId2);
    ASSERT_TRUE(readTableElement.IsValid());

    EXPECT_EQ (numRows2, readTableElement->GetRowCount ());
    EXPECT_EQ (numCols2, readTableElement->GetColumnCount ());

    // Expect the minimum aspects on the element
    expectedCounts.VerifyCounts(*readTableElement);

    GetDgnDb().SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableTest, PersistRowAndColumnAspects)
    {
    int numRows1 = 5, numCols1 = 3;
    int numRows2 = 8, numCols2 = 2;

    typedef bpair <int, double> ExpectedRowHeight;

    // Set the height of even rows to the 2 * (1+index) (2, def, 6, def, 10).
    bvector<ExpectedRowHeight> rowHeights1;
    for (int iRow = 0; iRow < numRows1; iRow++)
        {
        if (0 == iRow % 2)
            rowHeights1.push_back (ExpectedRowHeight (iRow, 2.0 * (1+iRow)));
        }

    // Set the height of odd rows to 3*index (3, def, 9, def, 15, def, 21, def).
    bvector<ExpectedRowHeight> rowHeights2;
    for (int iRow = 0; iRow < numRows2; iRow++)
        {
        if (0 != iRow % 2)
            rowHeights2.push_back (ExpectedRowHeight (iRow, 3.0 * iRow));
        }

    AnnotationTablePtr   tableElement1 = CreateBasicTable (numRows1, numCols1);
    AnnotationTablePtr   tableElement2 = CreateBasicTable (numRows2, numCols2);

    for (ExpectedRowHeight const& entry : rowHeights1)
        tableElement1->GetRow (entry.first)->SetHeight(entry.second);

    for (ExpectedRowHeight const& entry : rowHeights2)
        tableElement2->GetRow (entry.first)->SetHeight(entry.second);

    // Insert both tables.
    DgnElementId elementId1 = InsertElement (*tableElement1);
    DgnElementId elementId2 = InsertElement (*tableElement2);

    tableElement1 = nullptr;
    tableElement2 = nullptr;

    // Purge the cache so that we don't get a cached element.
    GetDgnDb().Memory().PurgeUntil(0);

    AnnotationTableCPtr readTableElement = AnnotationTable::Get(GetDgnDb(), elementId1);
    ASSERT_TRUE(readTableElement.IsValid());

    for (ExpectedRowHeight const& entry : rowHeights1)
        EXPECT_EQ (entry.second, readTableElement->GetRow (entry.first)->GetHeight());

    // Expect row aspects for the rows with non-default heights.
    ExpectedAspectCounts expectedCounts1;
    expectedCounts1.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableRow), rowHeights1.size());
    expectedCounts1.VerifyCounts(*readTableElement);

    readTableElement = AnnotationTable::Get(GetDgnDb(), elementId2);
    ASSERT_TRUE(readTableElement.IsValid());

    for (ExpectedRowHeight const& entry : rowHeights2)
        EXPECT_EQ (entry.second, readTableElement->GetRow (entry.first)->GetHeight());

    // Expect row aspects for the rows with non-default heights.
    ExpectedAspectCounts expectedCounts2;
    expectedCounts2.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableRow), rowHeights2.size());
    expectedCounts2.VerifyCounts(*readTableElement);

    GetDgnDb().SaveChanges();
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct AnnotationTableTestAction
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _CreateTable (AnnotationTablePtr&, DgnDbR, DgnModelId, DgnCategoryId, DgnElementId) { return false; }
    virtual void    _PreAction (AnnotationTableR) {}
    virtual void    _DoAction (AnnotationTableR) = 0;
    virtual void    _VerifyAction (AnnotationTableCR) const = 0;
    };

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct AnnotationTableActionTest : public AnnotationTableTest
{
 private:
    typedef AnnotationTableTest T_Super;

    DgnElementId    m_tableElementId;

public:
    void SetUp () override
        {
        T_Super::SetUp();

        m_tableElementId.Invalidate();
        }

    AnnotationTablePtr CreateTable (AnnotationTableTestAction& testAction)
        {
        AnnotationTablePtr    table;

        if (testAction._CreateTable (table, GetDgnDb(), GetModelId(), GetCategoryId(), GetTextStyleId()))
            return table;

        return CreateBasicTable();
        }

    void ReadConstTable (AnnotationTableCPtr& table)
        {
        EXPECT_TRUE (m_tableElementId.IsValid());

        table = AnnotationTable::Get(GetDgnDb(), m_tableElementId);
        EXPECT_TRUE (table.IsValid());
        }

    void ReadEditableTable (AnnotationTablePtr& table)
        {
        EXPECT_TRUE (m_tableElementId.IsValid());

        table = AnnotationTable::GetForEdit(GetDgnDb(), m_tableElementId);
        EXPECT_TRUE (table.IsValid());
        }

    void AddTableToDb (AnnotationTableR table)
        {
        ASSERT_TRUE ( ! m_tableElementId.IsValid());
        m_tableElementId = InsertElement (table);
        ASSERT_TRUE (m_tableElementId.IsValid());
        }

    void UpdateTableInDb (AnnotationTableR table)
        {
        ASSERT_EQ (m_tableElementId, table.GetElementId());
        UpdateElement (table);
        ASSERT_EQ (m_tableElementId, table.GetElementId());
        }

    void DoCreateTableTest (AnnotationTableTestAction& testAction)
        {
        AnnotationTablePtr seedTable = CreateTable(testAction);
        EXPECT_TRUE (seedTable.IsValid());

        testAction._PreAction (*seedTable);
        testAction._DoAction  (*seedTable);

        AddTableToDb (*seedTable);
        seedTable = nullptr;

#if defined (FOR_DEBUGGING)
        // Put a break point on Reopen in order to open the file in an viewing application
        CloseTestFile();
        ReopenTestFile();
#endif
        // Purge the cache so that we don't get a cached element.
        GetDgnDb().Memory().PurgeUntil(0);

        AnnotationTableCPtr    foundTable;

        ReadConstTable (foundTable);
        EXPECT_TRUE (foundTable.IsValid());

        testAction._VerifyAction (*foundTable);

        GetDgnDb().SaveChanges();
        }

    void DoModifyTableTest (AnnotationTableTestAction& testAction)
        {
        AnnotationTablePtr seedTable = CreateTable(testAction);
        EXPECT_TRUE (seedTable.IsValid());

        testAction._PreAction (*seedTable);

        AddTableToDb (*seedTable);
        seedTable = nullptr;

#if defined (FOR_DEBUGGING)
// Put a break point on Reopen in order to open the file in an viewing application
CloseTestFile();
ReopenTestFile();
#endif

#if defined (FOR_DEBUGGING)
// Purge the cache so that we don't get a cached element.
GetDgnDb().Elements().Purge(0);
#endif

        AnnotationTablePtr        applyActionTable;

        ReadEditableTable (applyActionTable);
        EXPECT_TRUE (applyActionTable.IsValid());

        testAction._DoAction  (*applyActionTable);

        UpdateTableInDb (*applyActionTable);
        applyActionTable = nullptr;

#if defined (FOR_DEBUGGING)
        // Put a break point on Reopen in order to open the file in an viewing application
        CloseTestFile();
        ReopenTestFile();
#endif
        // Purge the cache so that we don't get a cached element.
        GetDgnDb().Memory().PurgeUntil(0);

        AnnotationTableCPtr    postActionTable;

        ReadConstTable (postActionTable);
        EXPECT_TRUE (postActionTable.IsValid());

        testAction._VerifyAction (*postActionTable);

        GetDgnDb().SaveChanges();
        }
};

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct NoAction : AnnotationTableTestAction
{
public:
    /* ctor */  NoAction () {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        // Don't do anything, a table is created and written by the test logic
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        EXPECT_EQ (5, table.GetRowCount ());
        EXPECT_EQ (3, table.GetColumnCount ());

        // Expect the minimum aspects on the element
        ExpectedAspectCounts expectedCounts;
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_NoAction)
    {
    NoAction   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_NoAction)
    {
    NoAction   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct OverrideRowHeightAction : AnnotationTableTestAction
{
private:
    double  m_overrideHeight;
    int     m_index;

public:
    /* ctor */  OverrideRowHeightAction (double v, int i) : m_overrideHeight (v), m_index (i) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        double  defaultRowHeight  = table.GetDefaultRowHeight();
        EXPECT_TRUE (defaultRowHeight != m_overrideHeight);

        table.GetRow(m_index)->SetHeight (m_overrideHeight);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        double  defaultRowHeight  = table.GetDefaultRowHeight();

        for (uint32_t iRow = 0; iRow < table.GetRowCount(); iRow++)
            {
            double expectedHeight = (m_index == iRow) ? m_overrideHeight : defaultRowHeight;

            EXPECT_EQ (expectedHeight, table.GetRow(iRow)->GetHeight());
            }

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableRow), 1);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_OverrideRowHeight)
    {
    int     rowIndex = 1;
    double  overrideHeightValue = 0.75;

    OverrideRowHeightAction   testAction (overrideHeightValue, rowIndex);
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_OverrideRowHeight)
    {
    int     columnIndex = 1;
    double  overrideHeightValue = 0.75;

    OverrideRowHeightAction   testAction (overrideHeightValue, columnIndex);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct OverrideColumnWidthAction : AnnotationTableTestAction
{
private:
    double  m_overrideWidth;
    int     m_index;

public:
    /* ctor */  OverrideColumnWidthAction (double v, int i) : m_overrideWidth (v), m_index (i) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        double  defaultColumnWidth  = table.GetDefaultColumnWidth();
        EXPECT_TRUE (defaultColumnWidth != m_overrideWidth);

        table.GetColumn(m_index)->SetWidth (m_overrideWidth);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        double  defaultColumnWidth  = table.GetDefaultColumnWidth();

        for (uint32_t iColumn = 0; iColumn < table.GetColumnCount(); iColumn++)
            {
            double expectedWidth = (m_index == iColumn) ? m_overrideWidth : defaultColumnWidth;

            EXPECT_EQ (expectedWidth, table.GetColumn(iColumn)->GetWidth());
            }

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableColumn), 1);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_OverrideColumnWidth)
    {
    int     colIndex = 1;
    double  overrideWidthValue = 0.75;

    OverrideColumnWidthAction   testAction (overrideWidthValue, colIndex);
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_OverrideColumnWidth)
    {
    int     columnIndex = 1;
    double  overrideWidthValue = 0.75;

    OverrideColumnWidthAction   testAction (overrideWidthValue, columnIndex);
    DoModifyTableTest (testAction);
    }


/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct ClearRowHeightAction : AnnotationTableTestAction
{
private:
    int  m_index;

public:
    /* ctor */  ClearRowHeightAction (int i) : m_index (i) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        double  overrideHeight = 0.75;
        double  defaultRowHeight  = table.GetDefaultRowHeight();
        EXPECT_TRUE (defaultRowHeight != overrideHeight);

        table.GetRow(m_index)->SetHeight (overrideHeight);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        table.GetRow(m_index)->SetHeightFromContents();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        double  defaultRowHeight  = table.GetDefaultRowHeight();

        for (uint32_t iRow = 0; iRow < table.GetRowCount(); iRow++)
            {
            double expectedHeight = defaultRowHeight;

            EXPECT_EQ (expectedHeight, table.GetRow(iRow)->GetHeight());
            }

        // Expect just the minimum instances on the element (we removed the row)
        ExpectedAspectCounts expectedCounts;
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_ClearRowHeight)
    {
    int  rowIndex = 1;

    ClearRowHeightAction   testAction (rowIndex);
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_ClearRowHeight)
    {
    int  rowIndex = 1;

    ClearRowHeightAction   testAction (rowIndex);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct SetCellTextAction : AnnotationTableTestAction
{
private:
    Utf8String                  m_applyString;
    AnnotationTableCellIndex    m_cellIndex;

public:
    /* ctor */  SetCellTextAction (Utf8CP v, AnnotationTableCellIndexCR i) : m_applyString (v), m_cellIndex (i) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        DgnElementId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId, m_applyString.c_str());

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        AnnotationTableCellP  foundCell = table.GetCell (m_cellIndex);
        AnnotationTextBlockCP foundTextBlock = foundCell->GetTextBlock();
        Utf8String            foundString = foundTextBlock->ToString();

        EXPECT_STREQ (m_applyString.c_str(), foundString.c_str());

        AnnotationTableCellIndex  anotherCell (m_cellIndex.row - 1, m_cellIndex.col - 1);
        foundCell = table.GetCell (anotherCell);

        EXPECT_TRUE (NULL == foundCell->GetTextBlock());

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableCell), 1);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_SetCellText)
    {
    Utf8CP                      cellString = "Hello Table";
    AnnotationTableCellIndex    cellIndex (1, 1);

    SetCellTextAction   testAction (cellString, cellIndex);
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_SetCellText)
    {
    Utf8CP                      cellString = "Hello Table";
    AnnotationTableCellIndex    cellIndex (1, 1);

    SetCellTextAction   testAction (cellString, cellIndex);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct ChangeCellTextAction : AnnotationTableTestAction
{
private:
    Utf8String                  m_applyString;
    AnnotationTableCellIndex    m_cellIndex;

public:
    /* ctor */  ChangeCellTextAction (Utf8CP v, AnnotationTableCellIndexCR i) : m_applyString (v), m_cellIndex (i) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        DgnElementId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId, "abcdefghi");

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        DgnElementId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId, m_applyString.c_str());

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        AnnotationTableCellP    foundCell = table.GetCell (m_cellIndex);
        AnnotationTextBlockCP   foundTextBlock = foundCell->GetTextBlock();
#if defined (NEEDSWORK)
        Utf8String              foundString = foundTextBlock->ToString();
        
        EXPECT_STREQ (m_applyString.c_str(), foundString.c_str());
#else
        EXPECT_TRUE (nullptr != foundTextBlock);
#endif

        AnnotationTableCellIndex  anotherCell (m_cellIndex.row - 1, m_cellIndex.col - 1);
        foundCell = table.GetCell (anotherCell);

        EXPECT_TRUE (NULL == foundCell->GetTextBlock());

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableCell), 1);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_ChangeCellText)
    {
    Utf8CP                      cellString = "Hello Table";
    AnnotationTableCellIndex    cellIndex (1, 1);

    ChangeCellTextAction   testAction (cellString, cellIndex);
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_ChangeCellText)
    {
    Utf8CP                      cellString = "Hello Table";
    AnnotationTableCellIndex    cellIndex (1, 1);

    ChangeCellTextAction   testAction (cellString, cellIndex);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct ClearCellTextAction : AnnotationTableTestAction
{
private:
    AnnotationTableCellIndex  m_cellIndex;

public:
    /* ctor */  ClearCellTextAction (AnnotationTableCellIndexCR i) : m_cellIndex (i) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        DgnElementId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId, "abcdefghi");

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        DgnElementId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId);

        // textBlock is empty

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        AnnotationTableCellP  foundCell = table.GetCell (m_cellIndex);
        EXPECT_TRUE (NULL == foundCell->GetTextBlock());

        AnnotationTableCellIndex  anotherCell (m_cellIndex.row - 1, m_cellIndex.col - 1);

        foundCell = table.GetCell (anotherCell);
        EXPECT_TRUE (NULL == foundCell->GetTextBlock());

        // Expect just the minimum instances on the element (we removed cell)
        ExpectedAspectCounts expectedCounts;
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_ClearCellText)
    {
    AnnotationTableCellIndex  cellIndex (1, 1);

    ClearCellTextAction   testAction (cellIndex);
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_ClearCellText)
    {
    AnnotationTableCellIndex  cellIndex (1, 1);

    ClearCellTextAction   testAction (cellIndex);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct DeleteRowAction : AnnotationTableTestAction
{
private:
    uint32_t        m_rowIndex;
    uint32_t        m_rowCount;

public:
    /* ctor */  DeleteRowAction (uint32_t r) : m_rowIndex (r), m_rowCount(0) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    07/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        m_rowCount = table.GetRowCount();

        for (uint32_t iRow = 0; iRow < m_rowCount; iRow++)
            table.GetRow(iRow)->SetHeight (10.0*(1+iRow));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    07/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        // Delete a row
        table.DeleteRow (m_rowIndex);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    07/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        EXPECT_EQ (m_rowCount - 1, table.GetRowCount());

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableRow), m_rowCount - 1);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_DeleteRow)
    {
    uint32_t          rowIndex = 1;

    DeleteRowAction   testAction (rowIndex);
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_DeleteRow)
    {
    uint32_t          rowIndex = 1;

    DeleteRowAction   testAction (rowIndex);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct MergeCellsAction : AnnotationTableTestAction
{
private:
    AnnotationTableCellIndex    m_rootIndex;
    uint32_t                    m_rowSpan;
    uint32_t                    m_colSpan;
    bool                        m_expectFail;

public:
    /* ctor */  MergeCellsAction (AnnotationTableCellIndex i, uint32_t r, uint32_t c, bool e) : m_rootIndex (i), m_rowSpan (r), m_colSpan (c), m_expectFail(e) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool    _CreateTable (AnnotationTablePtr& table, DgnDbR db, DgnModelId mid, DgnCategoryId cid, DgnElementId tsid) override
        {
        uint32_t          numRows  = 3;
        uint32_t          numCols  = 3;

        //       0     1     2   
        //    |------------------
        //  0 |     |     |     |
        //    |-----+-----+-----+
        //  1 |     |     |     |
        //    |-----+-----+-----|
        //  2 |     |     |     |
        //    |-----+-----+-----+

        AnnotationTable::CreateParams    createParams (db, mid, AnnotationTable::QueryClassId(db), cid);
        table = AnnotationTable::Create (numRows, numCols, tsid, 0, createParams);

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        // Merge a block of cells
        bool          failed = (SUCCESS != table.MergeCells (m_rootIndex, m_rowSpan, m_colSpan));
        ASSERT_EQ (m_expectFail, failed);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        bvector <AnnotationTableTest::MergeDescr> mergeBlocks;

        if ( ! m_expectFail)
            mergeBlocks.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, m_colSpan));

        AnnotationTableTest::VerifyCellsWithMergeBlocks (table, mergeBlocks);

        ExpectedAspectCounts expectedCounts (0, 0, 0, (uint32_t) mergeBlocks.size());
        expectedCounts.VerifyCounts(table);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCells_TopLeft)
    {
    AnnotationTableCellIndex    cellIndex (0, 0);
    uint32_t                    rowSpan = 2;
    uint32_t                    colSpan = 2;

        //       0     1     2   
        //    |------------------
        //  0 |           |     |
        //    |           +-----+
        //  1 |           |     |
        //    |-----+-----+-----+
        //  2 |     |     |     |
        //    |-----+-----+-----+

    MergeCellsAction   testAction (cellIndex, rowSpan, colSpan, false);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCells_BottomRight)
    {
    AnnotationTableCellIndex    cellIndex (1, 1);
    uint32_t                    rowSpan = 2;
    uint32_t                    colSpan = 2;

        //       0     1     2   
        //    |-----+-----+-----|
        //  0 |     |     |     |
        //    |-----+-----+-----|
        //  1 |     |           |
        //    |-----+           |
        //  2 |     |           |
        //    |-----+-----+-----|

    MergeCellsAction   testAction (cellIndex, rowSpan, colSpan, false);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCells_FullRow)
    {
    AnnotationTableCellIndex    cellIndex (0, 0);
    uint32_t                    rowSpan = 1;
    uint32_t                    colSpan = 3;

        //       0     1     2  
        //    |-----------------|
        //  0 |                 |
        //    |-----+-----+-----|
        //  1 |     |     |     |
        //    |-----+-----+-----|
        //  2 |     |     |     |
        //    |-----+-----+-----|

    MergeCellsAction   testAction (cellIndex, rowSpan, colSpan, false);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCells_FullColumn)
    {
    AnnotationTableCellIndex    cellIndex (0, 2);
    uint32_t                    rowSpan = 3;
    uint32_t                    colSpan = 1;

        //       0     1     2
        //    |-----------------|
        //  0 |     |     |     |
        //    |-----+-----+     |
        //  1 |     |     |     |
        //    |-----+-----+     |
        //  2 |     |     |     |
        //    |-----+-----+-----|

    MergeCellsAction   testAction (cellIndex, rowSpan, colSpan, false);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCells_TooWide)
    {
    AnnotationTableCellIndex    cellIndex (0, 1);
    uint32_t                    rowSpan = 1;
    uint32_t                    colSpan = 3;

        //       0     1     2 
        //    |----------------|
        //  0 |     | fail     xxxx
        //    |-----+-----+----|
        //  1 |     |     |    |
        //    |-----+-----+----|
        //  2 |     |     |    |
        //    |-----+-----+----|

    MergeCellsAction   testAction (cellIndex, rowSpan, colSpan, true);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCells_TooTall)
    {
    AnnotationTableCellIndex    cellIndex (1, 2);
    uint32_t                    rowSpan = 3;
    uint32_t                    colSpan = 1;

        //       0     1     2
        //    |-----------------|
        //  0 |     |     |     |  origin 1,2
        //    |-----+-----+-----|  span   3,1
        //  1 |     |     |     |
        //    |-----+-----+ fail|
        //  2 |     |     |     |
        //    |-----+-----+     |
        //                 xxxxx

    MergeCellsAction   testAction (cellIndex, rowSpan, colSpan, true);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct MergeCellsWithExistingAction : AnnotationTableTestAction
{
private:
    AnnotationTableCellIndex  m_rootIndex;
    uint32_t        m_rowSpan;
    uint32_t        m_colSpan;
    bool            m_expectFail;
    bool            m_expectConsume;

    AnnotationTableTest::MergeDescr      m_existingMerge;

public:
    /* ctor */  MergeCellsWithExistingAction (AnnotationTableTest::MergeDescr e, AnnotationTableCellIndex i, uint32_t r, uint32_t c, bool f, bool consume) : 
        m_rootIndex (i), m_rowSpan (r), m_colSpan (c), m_expectFail(f), m_expectConsume(consume),
        m_existingMerge (e) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    03/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool    _CreateTable (AnnotationTablePtr& table, DgnDbR db, DgnModelId mid, DgnCategoryId cid, DgnElementId tsid) override
        {
        uint32_t          numRows  = 4;
        uint32_t          numCols  = 4;

        AnnotationTable::CreateParams    createParams (db, mid, AnnotationTable::QueryClassId(db), cid);
        table = AnnotationTable::Create (numRows, numCols, tsid, 0, createParams);

        table->MergeCells (m_existingMerge.m_rootIndex, m_existingMerge.m_rowSpan, m_existingMerge.m_colSpan);

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    03/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        // Merge a block of cells
        bool          failed = (SUCCESS != table.MergeCells (m_rootIndex, m_rowSpan, m_colSpan));
        ASSERT_EQ (m_expectFail, failed);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    03/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        bvector <AnnotationTableTest::MergeDescr> mergeBlocks;

        if ( ! m_expectConsume)
            mergeBlocks.push_back (m_existingMerge);

        if ( ! m_expectFail)
            mergeBlocks.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, m_colSpan));

        AnnotationTableTest::VerifyCellsWithMergeBlocks (table, mergeBlocks);

        ExpectedAspectCounts expectedCounts (0, 0, 0, (uint32_t) mergeBlocks.size());
        expectedCounts.VerifyCounts(table);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_AboveLeft)
    {
    MergeDescr                  old (AnnotationTableCellIndex (2, 1), 2, 2);
    AnnotationTableCellIndex    cellIndex (0, 0);
    uint32_t                    rowSpan = 2;
    uint32_t                    colSpan = 2;

    //       0     1     2     3             0     1     2     3             0     1     2     3
    //    |-----------------------|  +    |-----------------------|  =    |-----------------------|
    //  0 |     |     |     |     |  +  0 |origin 0,0 |     |     |  =  0 |           |     |     |
    //    |-----+-----+-----+-----|  +    |span   2,2 +-----+-----|  =    |   new     +-----+-----|
    //  1 |     |     |     |     |  +  1 |           |     |     |  =  1 |           |     |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|
    //  2 |     |           |     |  +  2 |     |     |     |     |  =  2 |     |           |     |
    //    |-----+    old    +-----|  +    |-----+-----+-----+-----|  =    |-----+    old    +-----|
    //  3 |     |           |     |  +  3 |     |     |     |     |  =  3 |     |           |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, false, false);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_AboveRight)
    {
    MergeDescr                  old (AnnotationTableCellIndex (2, 1), 2, 2);
    AnnotationTableCellIndex    cellIndex (0, 2);
    uint32_t                    rowSpan = 2;
    uint32_t                    colSpan = 2;

    //       0     1     2     3            0     1     2     3              0     1     2     3  
    //    |-----------------------|  +    |-----------------------|  =    |-----------------------|
    //  0 |     |     |     |     |  +  0 |     |     |origin 0,2 |  =  0 |     |     |           |
    //    |-----+-----+-----+-----|  +    |-----+-----+span   2,2 |  =    |-----+-----+    new    |
    //  1 |     |     |     |     |  +  1 |     |     |           |  =  1 |     |     |           |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|
    //  2 |     |           |     |  +  2 |     |     |     |     |  =  2 |     |           |     |
    //    |-----+    old    +-----|  +    |-----+-----+-----+-----|  =    |-----+    old    +-----|
    //  3 |     |           |     |  +  3 |     |     |     |     |  =  3 |     |           |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, false, false);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_ConsumedLeft)
    {
    MergeDescr                  old (AnnotationTableCellIndex (0, 0), 1, 2);
    AnnotationTableCellIndex    cellIndex (0, 0);
    uint32_t                    rowSpan = 1;
    uint32_t                    colSpan = 3;

    //       0     1     2     3            0     1     2     3              0     1     2     3  
    //    |-----------------------|  +    |-----------------------|  =    |-----------------------|
    //  0 |    old    |     |     |  +  0 |       new       |     |  =  0 |     consumed    |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|
    //  1 |     |     |     |     |  +  1 |     |     |     |     |  =  1 |     |     |     |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|
    //  2 |     |     |     |     |  +  2 |     |     |     |     |  =  2 |     |     |     |     |
    //    |-----+-----+-----|-----|  =    |-----+-----+-----|-----|       |-----+-----+-----|-----|
    //  3 |     |     |     |     |  +  3 |     |     |     |     |  =  3 |     |     |     |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, false, true);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_ConsumedRight)
    {
    MergeDescr                  old (AnnotationTableCellIndex (1, 1), 1, 2);
    AnnotationTableCellIndex    cellIndex (1, 0);
    uint32_t                    rowSpan = 1;
    uint32_t                    colSpan = 3;

    //       0     1     2     3            0     1     2     3              0     1     2     3  
    //    |-----------------------|  +    |-----------------------|  =    |-----------------------|
    //  0 |     |     |     |     |  +  0 |     |     |     |     |  =  0 |     |     |     |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|
    //  1 |     |    old    |     |  +  1 |       new       |     |  =  1 |     consumed    |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|
    //  2 |     |     |     |     |  +  2 |     |     |     |     |  =  2 |     |     |     |     |
    //    |-----+-----+-----|-----|  =    |-----+-----+-----|-----|       |-----+-----+-----|-----|
    //  3 |     |     |     |     |  +  3 |     |     |     |     |  =  3 |     |     |     |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, false, true);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_ConsumedFromAbove)
    {
    MergeDescr                  old (AnnotationTableCellIndex (2, 1), 2, 2);
    AnnotationTableCellIndex    cellIndex (0, 1);
    uint32_t                    rowSpan = 4;
    uint32_t                    colSpan = 2;

    //       0     1     2     3            0     1     2     3              0     1     2     3  
    //    |-----------------------|  +    |-----------------------|  =    |-----------------------|
    //  0 |     |     |     |     |  +  0 |     |           |     |  =  0 |     |           |     |
    //    |-----+-----+-----+-----|  +    |-----+           +-----|  =    |-----+           +-----|
    //  1 |     |     |     |     |  +  1 |     |           |     |  =  1 |     |           |     |
    //    |-----+-----+-----+-----|  +    |-----+    new    +-----|  =    |-----+  consumed +-----|
    //  2 |     |           |     |  +  2 |     |           |     |  =  2 |     |           |     |
    //    |-----+    old    +-----|  +    |-----+           +-----|  =    |-----+           +-----|
    //  3 |     |           |     |  +  3 |     |           |     |  =  3 |     |           |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, false, true);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_ConsumedLeftAndRight)
    {
    MergeDescr                  old (AnnotationTableCellIndex (2, 1), 2, 2);
    AnnotationTableCellIndex    cellIndex (2, 0);
    uint32_t                    rowSpan = 2;
    uint32_t                    colSpan = 4;

    //       0     1     2     3            0     1     2     3              0     1     2     3  
    //    |-----------------------|  +    |-----------------------|  =    |-----------------------|
    //  0 |     |     |     |     |  +  0 |     |     |     |     |  =  0 |     |     |     |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|
    //  1 |     |     |     |     |  +  1 |     |     |     |     |  =  1 |     |     |     |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|
    //  2 |     |           |     |  +  2 |                       |  =  2 |                       |
    //    |-----+    old    +-----|  +    |          new          |  =    |       consumed        |
    //  3 |     |           |     |  +  3 |                       |  =  3 |                       |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, false, true);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_ConsumedFromAboveLeft)
    {
    MergeDescr                  old (AnnotationTableCellIndex (2, 1), 2, 2);
    AnnotationTableCellIndex    cellIndex (0, 0);
    uint32_t                    rowSpan = 4;
    uint32_t                    colSpan = 3;

    //       0     1     2     3            0     1     2     3              0     1     2     3  
    //    |-----------------------|  +    |-----------------------|  =    |-----------------------|
    //  0 |     |     |     |     |  +  0 |                 |     |  =  0 |                 |     |
    //    |-----+-----+-----+-----|  +    |                 +-----|  =    |                 +-----|
    //  1 |     |     |     |     |  +  1 |                 |     |  =  1 |                 |     |
    //    |-----+-----+-----+-----|  +    |       new       +-----|  =    |    consumed     +-----|
    //  2 |     |           |     |  +  2 |                 |     |  =  2 |                 |     |
    //    |-----+    old    +-----|  +    |                 +-----|  =    |                 +-----|
    //  3 |     |           |     |  +  3 |                 |     |  =  3 |                 |     |
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =    |-----+-----+-----+-----|

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, false, true);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_OverlapOneCell)
    {
    MergeDescr                  old (AnnotationTableCellIndex (2, 1), 2, 2);
    AnnotationTableCellIndex    cellIndex (0, 2);
    uint32_t                    rowSpan = 3;
    uint32_t                    colSpan = 1;

    //       0     1     2     3            0     1     2     3         
    //    |-----------------------|  +    |-----------------------|  =  
    //  0 |     |     |     |     |  +  0 |     |     |     |     |  =  
    //    |-----+-----+-----+-----|  +    |-----+-----+     +-----|  =  
    //  1 |     |     |     |     |  +  1 |     |     | new |     |  =   FAIL
    //    |-----+-----+-----+-----|  +    |-----+-----+     +-----|  =   DUE TO
    //  2 |     |           |     |  +  2 |     |     |     |     |  =   OVERLAP
    //    |-----+    old    +-----|  +    |-----+-----+-----+-----|  =  
    //  3 |     |           |     |  +  3 |     |     |     |     |  =  
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =  

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, true, false);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_OverlapTwoCell)
    {
    MergeDescr                  old (AnnotationTableCellIndex (2, 1), 2, 2);
    AnnotationTableCellIndex    cellIndex (0, 1);
    uint32_t                    rowSpan = 3;
    uint32_t                    colSpan = 2;

    //       0     1     2     3            0     1     2     3         
    //    |-----------------------|  +    |-----------------------|  =  
    //  0 |     |     |     |     |  +  0 |     |           |     |  =  
    //    |-----+-----+-----+-----|  +    |-----+           +-----|  =  
    //  1 |     |     |     |     |  +  1 |     |    new    |     |  =   FAIL
    //    |-----+-----+-----+-----|  +    |-----+           +-----|  =   DUE TO
    //  2 |     |           |     |  +  2 |     |           |     |  =   OVERLAP
    //    |-----+    old    +-----|  +    |-----+-----+-----+-----|  =  
    //  3 |     |           |     |  +  3 |     |     |     |     |  =  
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =  

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, true, false);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_OverlapAcrossTop)
    {
    MergeDescr                  old (AnnotationTableCellIndex (2, 1), 2, 2);
    AnnotationTableCellIndex    cellIndex (2, 0);
    uint32_t                    rowSpan = 1;
    uint32_t                    colSpan = 4;

    //       0     1     2     3            0     1     2     3         
    //    |-----------------------|  +    |-----------------------|  =  
    //  0 |     |     |     |     |  +  0 |     |     |     |     |  =  
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =  
    //  1 |     |     |     |     |  +  1 |     |     |     |     |  =   FAIL
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =   DUE TO
    //  2 |     |           |     |  +  2 |          new          |  =   OVERLAP
    //    |-----+    old    +-----|  +    |-----+-----+-----+-----|  =  
    //  3 |     |           |     |  +  3 |     |     |     |     |  =  
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =  

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, true, false);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, MergeCellsWithExisting_OverlapAcrossBottom)
    {
    MergeDescr                  old (AnnotationTableCellIndex (2, 1), 2, 2);
    AnnotationTableCellIndex    cellIndex (3, 0);
    uint32_t                    rowSpan = 1;
    uint32_t                    colSpan = 4;

    //       0     1     2     3            0     1     2     3         
    //    |-----------------------|  +    |-----------------------|  =  
    //  0 |     |     |     |     |  +  0 |     |     |     |     |  =  
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =  
    //  1 |     |     |     |     |  +  1 |     |     |     |     |  =   FAIL
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =   DUE TO
    //  2 |     |           |     |  +  2 |     |     |     |     |  =   OVERLAP
    //    |-----+    old    +-----|  +    |-----+-----+-----+-----|  =  
    //  3 |     |           |     |  +  3 |          new          |  =  
    //    |-----+-----+-----+-----|  +    |-----+-----+-----+-----|  =  

    MergeCellsWithExistingAction   testAction (old, cellIndex, rowSpan, colSpan, true, false);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct DeleteMergedCellsAction : AnnotationTableTestAction
{
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
enum class DeleteTarget
    {
    Nothing         = 0,    // No delete

    RowBefore       = 1,    // Delete the row preceeding the merge block
    RowWithRoot     = 2,    // Delete the row containing the root of the merge block
    RowInterior     = 3,    // Delete a row that intersects the merge block
    RowAllMerged    = 4,    // Delete all the rows that intersect the merge block
    RowAfter        = 5,    // Delete the row after the merge block

    ColumnBefore    = 6,    // Delete the column preceeding the merge block
    ColumnWithRoot  = 7,    // Delete the column containing the root of the merge block
    ColumnInterior  = 8,    // Delete a column that intersects the merge block
    ColumnAllMerged = 9,    // Delete all the column that intersect the merge block
    ColumnAfter     = 10,   // Delete the column after the merge block
    };

private:
        DeleteTarget              m_deleteTarget;
        AnnotationTableCellIndex  m_rootIndex;
        uint32_t                  m_rowSpan;
        uint32_t                  m_colSpan;
        uint32_t                  m_numMergeInstances;

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    /* ctor */  DeleteMergedCellsAction (DeleteTarget deleteTarget)
        :
        m_deleteTarget (deleteTarget),
        m_rootIndex (1, 1),
        m_rowSpan(2),
        m_colSpan(2),
        m_numMergeInstances(0)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool    _CreateTable (AnnotationTablePtr& table, DgnDbR db, DgnModelId mid, DgnCategoryId cid, DgnElementId tsid) override
        {
        uint32_t          numRows  = 4;
        uint32_t          numCols  = 4;

        // We want a table big enough that the merge block isn't on the edges.  So that
        // we can insert and delete rows/cols before after the merge.

        //       0     1     2     3  
        //    |-----------------------|
        //  0 |     |     |     |     |
        //    |-----+-----+-----+-----|
        //  1 |     |           |     |
        //    |-----+           +-----|
        //  2 |     |           |     |
        //    |-----+-----+-----+-----|
        //  3 |     |     |     |     |
        //    |-----+-----+-----+-----|

        AnnotationTable::CreateParams    createParams (db, mid, AnnotationTable::QueryClassId(db), cid);
        table = AnnotationTable::Create (numRows, numCols, tsid, 0, createParams);

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        // Merge a block of cells
        table.MergeCells (m_rootIndex, m_rowSpan, m_colSpan);

        // At this point, the table should have one merge instance
        m_numMergeInstances = 1;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        bvector<uint32_t>     rowsToDelete;
        bvector<uint32_t>     colsToDelete;
        uint32_t              rootRowChange       = 0;
        uint32_t              rootColChange       = 0;
        uint32_t              rowSpanChange       = 0;
        uint32_t              colSpanChange       = 0;
        uint32_t              numInstancesChange  = 0;

        switch (m_deleteTarget)
            {
            case DeleteTarget::Nothing:         { rootRowChange = 0; rowSpanChange = 0; numInstancesChange = 0;                                                                                      break; }

            case DeleteTarget::RowBefore:       { rootRowChange = 1; rowSpanChange = 0; numInstancesChange = 0; rowsToDelete.push_back (m_rootIndex.row - 1);                                        break; }
            case DeleteTarget::RowWithRoot:     { rootRowChange = 0; rowSpanChange = 1; numInstancesChange = 0; rowsToDelete.push_back (m_rootIndex.row);                                            break; }
            case DeleteTarget::RowInterior:     { rootRowChange = 0; rowSpanChange = 1; numInstancesChange = 0; rowsToDelete.push_back (m_rootIndex.row + 1);                                        break; }
            case DeleteTarget::RowAllMerged:    { rootRowChange = 0; rowSpanChange = 2; numInstancesChange = 1; rowsToDelete.push_back (m_rootIndex.row); rowsToDelete.push_back (m_rootIndex.row);  break; }
            case DeleteTarget::RowAfter:        { rootRowChange = 0; rowSpanChange = 0; numInstancesChange = 0; rowsToDelete.push_back (m_rootIndex.row + m_rowSpan);                                break; }

            case DeleteTarget::ColumnBefore:    { rootColChange = 1; colSpanChange = 0; numInstancesChange = 0; colsToDelete.push_back (m_rootIndex.col - 1);                                        break; }
            case DeleteTarget::ColumnWithRoot:  { rootColChange = 0; colSpanChange = 1; numInstancesChange = 0; colsToDelete.push_back (m_rootIndex.col);                                            break; }
            case DeleteTarget::ColumnInterior:  { rootColChange = 0; colSpanChange = 1; numInstancesChange = 0; colsToDelete.push_back (m_rootIndex.col + 1);                                        break; }
            case DeleteTarget::ColumnAllMerged: { rootColChange = 0; colSpanChange = 2; numInstancesChange = 1; colsToDelete.push_back (m_rootIndex.col); colsToDelete.push_back (m_rootIndex.col);  break; }
            case DeleteTarget::ColumnAfter:     { rootColChange = 0; colSpanChange = 0; numInstancesChange = 0; colsToDelete.push_back (m_rootIndex.col + m_colSpan);                                break; }

            default:                            { FAIL(); }
            }

        for (uint32_t const& row : rowsToDelete)
            EXPECT_EQ (SUCCESS, table.DeleteRow (row));

        for (uint32_t const& col : colsToDelete)
            EXPECT_EQ (SUCCESS, table.DeleteColumn (col));

        m_rootIndex.row     -= rootRowChange;
        m_rootIndex.col     -= rootColChange;
        m_rowSpan           -= rowSpanChange;
        m_colSpan           -= colSpanChange;
        m_numMergeInstances -= numInstancesChange;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        bvector <AnnotationTableTest::MergeDescr> mergeBlocks;

        if (1 == m_numMergeInstances)
            mergeBlocks.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, m_colSpan));

        AnnotationTableTest::VerifyCellsWithMergeBlocks (table, mergeBlocks);

        ExpectedAspectCounts expectedCounts (0, 0, 0, (uint32_t) mergeBlocks.size());
        expectedCounts.VerifyCounts(table);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, DeleteMergedCells_Nothing)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::Nothing);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_RowBefore)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::RowBefore);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_RowWithRoot)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::RowWithRoot);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_RowInterior)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::RowInterior);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_RowAllMerged)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::RowAllMerged);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_RowAfter)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::RowAfter);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_ColumnBefore)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::ColumnBefore);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_ColumnWithRoot)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::ColumnWithRoot);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_ColumnInterior)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::ColumnInterior);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_ColumnAllMerged)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::ColumnAllMerged);
    DoModifyTableTest (testAction);
    }

TEST_F (AnnotationTableActionTest, DeleteMergedCells_ColumnAfter)
    {
    DeleteMergedCellsAction testAction (DeleteMergedCellsAction::DeleteTarget::ColumnAfter);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct InsertMergedCellsAction : AnnotationTableTestAction
{
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
enum class InsertTarget
    {
    Nothing         = 0,    // No delete

    RowBefore       = 1,    // Insert a row before the merge block
    RowJustBefore   = 2,    // Insert a row immediately before the rootIndex of the merge block
    RowInterior     = 3,    // Insert a row that intersects the merge block
    RowJustAfter    = 4,    // Insert a row immediately after the last row of the merge block
    RowAfter        = 5,    // Insert a row after the merge block

    ColumnBefore    = 6,    // Insert a column before the merge block
    ColumnJustBefore= 7,    // Insert a column immediately before the rootIndex of the merge block
    ColumnInterior  = 8,    // Insert a column that intersects the merge block
    ColumnJustAfter = 9,    // Insert a column immediately after the last column of the merge block
    ColumnAfter     = 10,   // Insert a column after the merge block
    };

private:
        InsertTarget                                m_insertTarget;
        AnnotationTableCellIndex                    m_rootIndex;
        uint32_t                                    m_rowSpan;
        uint32_t                                    m_colSpan;
        bvector <AnnotationTableTest::MergeDescr>   m_expectedMerges;

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    /* ctor */  InsertMergedCellsAction (InsertTarget insertTarget)
        :
        m_insertTarget (insertTarget),
        m_rootIndex (1, 1),
        m_rowSpan(2),
        m_colSpan(2)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool    _CreateTable (AnnotationTablePtr& table, DgnDbR db, DgnModelId mid, DgnCategoryId cid, DgnElementId tsid) override
        {
        uint32_t          numRows  = 4;
        uint32_t          numCols  = 4;

        // We want a table big enough that the merge block isn't on the edges.  So that
        // we can insert and delete rows/cols before after the merge.

        //       0     1     2     3  
        //    |-----------------------|
        //  0 |     |     |     |     |
        //    |-----+-----+-----+-----|
        //  1 |     |           |     |
        //    |-----+           +-----|
        //  2 |     |           |     |
        //    |-----+-----+-----+-----|
        //  3 |     |     |     |     |
        //    |-----+-----+-----+-----|

        AnnotationTable::CreateParams    createParams (db, mid, AnnotationTable::QueryClassId(db), cid);
        table = AnnotationTable::Create (numRows, numCols, tsid, 0, createParams);

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        // Merge a block of cells
        table.MergeCells (m_rootIndex, m_rowSpan, m_colSpan);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        switch (m_insertTarget)
            {
            case InsertTarget::Nothing:
                {
                // Do nothing.

                // Expect no change to the existing merge.
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, m_colSpan));
                break;
                }
            case InsertTarget::RowBefore:
                {
                // Add the row before the row that's above the merge.
                table.InsertRow (m_rootIndex.row - 1, TableInsertDirection::Before);

                // Expect the merge to move down by one row.
                AnnotationTableCellIndex rootIndex (m_rootIndex.row + 1, m_rootIndex.col);
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (rootIndex, m_rowSpan, m_colSpan));
                break;
                }
            case InsertTarget::RowJustBefore:
                {
                // Add the row immediately before the merge.
                table.InsertRow (m_rootIndex.row, TableInsertDirection::Before);

                // Expect a new merge since the cells were merged in the seed row.
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, 1, m_colSpan));

                // Also expect the original merge to move down by one row.
                AnnotationTableCellIndex rootIndex (m_rootIndex.row + 1, m_rootIndex.col);
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (rootIndex, m_rowSpan, m_colSpan));
                break;
                }
            case InsertTarget::RowInterior:
                {
                // Add the row within the merge.
                table.InsertRow (m_rootIndex.row, TableInsertDirection::After);

                // Expect the merge to grow by one row.
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan+1, m_colSpan));
                break;
                }
            case InsertTarget::RowJustAfter:
                {
                // Add the row immediately after the merge.
                table.InsertRow (m_rootIndex.row + m_rowSpan - 1, TableInsertDirection::After);

                // Expect no change to the original merge.
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, m_colSpan));

                // Also expect a new merge since the cells were merged in the seed row.
                AnnotationTableCellIndex rootIndex (m_rootIndex.row + m_rowSpan, m_rootIndex.col);
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (rootIndex, 1, m_colSpan));
                break;
                }
            case InsertTarget::RowAfter:
                {
                // Add the row after the row that's past the merge.
                table.InsertRow (m_rootIndex.row + m_rowSpan, TableInsertDirection::After);

                // Expect no change to the merge.
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, m_colSpan));
                break;
                }
            case InsertTarget::ColumnBefore:
                {
                // Add a column before the column that's above the merge.
                table.InsertColumn (m_rootIndex.col - 1, TableInsertDirection::Before);

                // Expect the merge to move right by one column.
                AnnotationTableCellIndex rootIndex (m_rootIndex.row, m_rootIndex.col + 1);
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (rootIndex, m_rowSpan, m_colSpan));
                break;
                }
            case InsertTarget::ColumnJustBefore:
                {
                // Add a column immediately before the merge.
                table.InsertColumn (m_rootIndex.col, TableInsertDirection::Before);

                // Expect a new merge since the cells were merged in the seed column.
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, 1));

                // Also expect the original merge to move right by one column.
                AnnotationTableCellIndex rootIndex (m_rootIndex.row, m_rootIndex.col + 1);
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (rootIndex, m_rowSpan, m_colSpan));
                break;
                }
            case InsertTarget::ColumnInterior:
                {
                // Add the row within the merge.
                table.InsertColumn (m_rootIndex.col, TableInsertDirection::After);

                // Expect the merge to grow by one column.
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, m_colSpan+1));
                break;
                }
            case InsertTarget::ColumnJustAfter:
                {
                // Add a column immediately after the merge.
                table.InsertColumn (m_rootIndex.col + m_colSpan - 1, TableInsertDirection::After);

                // Expect no change to the original merge.
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, m_colSpan));

                // Also expect a new merge since the cells were merged in the seed row.
                AnnotationTableCellIndex rootIndex (m_rootIndex.row, m_rootIndex.col + m_colSpan);
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (rootIndex, m_rowSpan, 1));
                break;
                }
            case InsertTarget::ColumnAfter:
                {
                // Add the row after the row that's past the merge.
                table.InsertColumn (m_rootIndex.col + m_colSpan, TableInsertDirection::After);

                // Expect no change to the merge.
                m_expectedMerges.push_back (AnnotationTableTest::MergeDescr (m_rootIndex, m_rowSpan, m_colSpan));
                break;
                }
            default:
                {
                FAIL();
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        AnnotationTableTest::VerifyCellsWithMergeBlocks (table, m_expectedMerges);

        ExpectedAspectCounts expectedCounts (0, 0, 0, (uint32_t) m_expectedMerges.size());
        expectedCounts.VerifyCounts(table);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_Nothing)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::Nothing);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_RowBefore)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::RowBefore);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_RowJustBefore)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::RowJustBefore);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_RowInterior)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::RowInterior);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_RowJustAfter)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::RowJustAfter);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_RowAfter)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::RowAfter);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_ColumnBefore)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::ColumnBefore);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_ColumnJustBefore)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::ColumnJustBefore);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_ColumnInterior)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::ColumnInterior);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_ColumnJustAfter)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::ColumnJustAfter);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertMergedCells_ColumnAfter)
    {
    InsertMergedCellsAction testAction (InsertMergedCellsAction::InsertTarget::ColumnAfter);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct EdgeColorSetter
    {
    AnnotationTableR         m_table;
    uint32_t                        m_rowIndex;
    AnnotationTableSymbologyValues  m_symb;

    EdgeColorSetter (AnnotationTableR table, uint32_t rowIndex, ColorDef colorVal)
        : m_table(table), m_rowIndex(rowIndex)
        {
        m_symb.SetLineColor(colorVal);
        }

    void SetColor (uint32_t colIndex, uint32_t numCells, bool top)
        {
        TableCellListEdges edges = top ? TableCellListEdges::Top : TableCellListEdges::Bottom;
        bvector<AnnotationTableCellIndex> cells;

        for (uint32_t iCol = 0; iCol < numCells; iCol++)
            cells.push_back (AnnotationTableCellIndex (m_rowIndex, colIndex + iCol));

        m_table.SetEdgeSymbology (m_symb, edges, cells);
        }
    };

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct ExpectedEdgeColor
{
uint32_t        m_columnIndex;
ColorDef        m_color;
bool            m_isGap;

/* ctor  */ ExpectedEdgeColor (uint32_t i, ColorDef v) : m_columnIndex(i), m_color (v), m_isGap (false) {}
/* ctor  */ ExpectedEdgeColor (uint32_t i)             : m_columnIndex(i), m_isGap (true) {}
};

typedef bvector<ExpectedEdgeColor> ExpectedEdgeColors;

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct EdgeColorVerifier
{
bool                        m_top;
uint32_t                    m_rowIndex;
ExpectedEdgeColors const&   m_expectedColors;

/* ctor */ EdgeColorVerifier (bvector<ExpectedEdgeColor> const& e, uint32_t rowIndex, bool top) : m_rowIndex (rowIndex), m_expectedColors (e), m_top (top) {}

void VerifyColors (AnnotationTableCR table)
    {
    // The test is responsible for providing an entry for every column
    if (m_expectedColors.size() != table.GetColumnCount())
        { FAIL(); return; }

    TableCellListEdges edges = m_top ? TableCellListEdges::Top : TableCellListEdges::Bottom;

    // Verify expected color for each cell in the row.
    for (uint32_t colIndex = 0; colIndex < table.GetColumnCount(); colIndex++)
        {
        AnnotationTableCellIndex                cellIndex(m_rowIndex, colIndex);

        if (nullptr == table.GetCell (cellIndex))
            {
            EXPECT_TRUE (m_expectedColors[colIndex].m_isGap) << "Expected a gap for column " << colIndex;
            continue;
            }

        bvector<AnnotationTableCellIndex>       cells;
        cells.push_back (cellIndex);

        bvector<AnnotationTableSymbologyValues> symbologies;
        table.GetEdgeSymbology (symbologies, edges, cells);

        EXPECT_TRUE (1 == symbologies.size());
        EXPECT_TRUE (symbologies[0].HasLineColor());

        ColorDef expectedColor = m_expectedColors[colIndex].m_color;
        EXPECT_EQ (expectedColor, symbologies[0].GetLineColor()) << "Unexpected color for column " << colIndex;
        }
    }
};

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct SetEdgeColorAction : AnnotationTableTestAction
{
private:
    ColorDef            m_colorVal;
    uint32_t            m_rowIndex;
    uint32_t            m_colStartIndex;
    uint32_t            m_numCols;
    bool                m_top;
    uint32_t            m_expectedRunCount;
    ExpectedEdgeColors  m_expectedColors;

public:
    /* ctor */  SetEdgeColorAction (ColorDefCR color, uint32_t row, uint32_t colStart, uint32_t numCols, bool top) : m_colorVal (color), m_rowIndex (row), m_colStartIndex (colStart), m_numCols (numCols), m_top (top) {}

    void    SetExpectedRunCount (uint32_t numRuns) { m_expectedRunCount = numRuns; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        EdgeColorSetter setter (table, m_rowIndex, m_colorVal);

        setter.SetColor (m_colStartIndex, m_numCols, m_top);

        for (uint32_t iCol = 0; iCol < table.GetColumnCount(); iCol++)
            {
            ColorDef    expectedColor;

            if (iCol >= m_colStartIndex && iCol < m_colStartIndex + m_numCols)
                expectedColor = m_colorVal;

            m_expectedColors.push_back (ExpectedEdgeColor (iCol, expectedColor));
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        EdgeColorVerifier verifier (m_expectedColors, m_rowIndex, m_top);
        verifier.VerifyColors (table);

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 2);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableEdgeRun),   m_expectedRunCount);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_SetEdgeColorAtStart)
    {
    ColorDef        colorVal = ColorDef::Green();
    uint32_t        rowIndex = 0;
    uint32_t        colStart = 0;
    uint32_t        colSpan  = 1;
    bool            isTop    = false;

    SetEdgeColorAction   testAction (colorVal, rowIndex, colStart, colSpan, isTop);

    // |oooo|----|----|
    testAction.SetExpectedRunCount(2);

    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_SetEdgeColorAtStart)
    {
    ColorDef        colorVal = ColorDef::Green();
    uint32_t        rowIndex = 0;
    uint32_t        colStart = 0;
    uint32_t        colSpan  = 1;
    bool            isTop    = false;

    SetEdgeColorAction   testAction (colorVal, rowIndex, colStart, colSpan, isTop);

    // |oooo|----|----|
    testAction.SetExpectedRunCount(2);

    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_SetEdgeColorInterior)
    {
    ColorDef        colorVal = ColorDef::Green();
    uint32_t        rowIndex = 0;
    uint32_t        colStart = 1;
    uint32_t        colSpan  = 1;
    bool            isTop    = false;

    SetEdgeColorAction   testAction (colorVal, rowIndex, colStart, colSpan, isTop);

    // |----|oooo|----|
    testAction.SetExpectedRunCount(3);

    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_SetEdgeColorInterior)
    {
    ColorDef        colorVal = ColorDef::Green();
    uint32_t        rowIndex = 0;
    uint32_t        colStart = 1;
    uint32_t        colSpan  = 1;
    bool            isTop    = false;

    SetEdgeColorAction   testAction (colorVal, rowIndex, colStart, colSpan, isTop);

    // |----|oooo|----|
    testAction.SetExpectedRunCount(3);

    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_SetEdgeColorAtEnd)
    {
    ColorDef        colorVal = ColorDef::Green();
    uint32_t        rowIndex = 0;
    uint32_t        colStart = 1;
    uint32_t        colSpan  = 2;
    bool            isTop    = false;

    SetEdgeColorAction   testAction (colorVal, rowIndex, colStart, colSpan, isTop);

    // |----|oooo|oooo|
    testAction.SetExpectedRunCount(2);

    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_SetEdgeColorAtEnd)
    {
    ColorDef        colorVal = ColorDef::Green();
    uint32_t        rowIndex = 0;
    uint32_t        colStart = 1;
    uint32_t        colSpan  = 2;
    bool            isTop    = false;

    SetEdgeColorAction   testAction (colorVal, rowIndex, colStart, colSpan, isTop);

    // |----|oooo|oooo|
    testAction.SetExpectedRunCount(2);

    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct MergeAdjacentEdgeRunsAction : AnnotationTableTestAction
{
private:
    uint32_t            m_rowIndex;
    ColorDef            m_colorVal;
    ExpectedEdgeColors  m_expectedColors;

public:
    /* ctor */  MergeAdjacentEdgeRunsAction () : m_rowIndex(0), m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        EdgeColorSetter setter (table, m_rowIndex, m_colorVal);

        // |----|----|----|
        setter.SetColor (0, 2, true);
        // |oooo|oooo|----|
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        EdgeColorSetter setter (table, m_rowIndex, m_colorVal);

        // |oooo|oooo|----|
        setter.SetColor (2, 1, true);
        // |oooo|oooo|oooo|

        m_expectedColors.push_back (ExpectedEdgeColor (0, m_colorVal));
        m_expectedColors.push_back (ExpectedEdgeColor (1, m_colorVal));
        m_expectedColors.push_back (ExpectedEdgeColor (2, m_colorVal));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        EdgeColorVerifier verifier (m_expectedColors, m_rowIndex, true);
        verifier.VerifyColors (table);

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 2);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableEdgeRun),   1);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_MergeAdjacentEdgeRuns)
    {
    MergeAdjacentEdgeRunsAction   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_MergeAdjacentEdgeRuns)
    {
    MergeAdjacentEdgeRunsAction   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct MergeNonAdjacentEdgeRunsAction : AnnotationTableTestAction
{
private:
    uint32_t            m_rowIndex;
    ColorDef            m_colorVal;
    ExpectedEdgeColors  m_expectedColors;

public:
    /* ctor */  MergeNonAdjacentEdgeRunsAction () : m_rowIndex(0), m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        EdgeColorSetter setter (table, m_rowIndex, m_colorVal);

        // |----|----|----|
        setter.SetColor (0, 1, true);
        setter.SetColor (2, 1, true);
        // |oooo|----|oooo|
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        EdgeColorSetter setter (table, m_rowIndex, m_colorVal);

        // |oooo|----|oooo|
        setter.SetColor (1, 1, true);
        // |oooo|oooo|oooo|

        m_expectedColors.push_back (ExpectedEdgeColor (0, m_colorVal));
        m_expectedColors.push_back (ExpectedEdgeColor (1, m_colorVal));
        m_expectedColors.push_back (ExpectedEdgeColor (2, m_colorVal));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        EdgeColorVerifier verifier (m_expectedColors, m_rowIndex, true);
        verifier.VerifyColors (table);

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 2);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableEdgeRun),   1);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_MergeNonAdjacentEdgeRuns)
    {
    MergeNonAdjacentEdgeRunsAction   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_MergeNonAdjacentEdgeRuns)
    {
    MergeNonAdjacentEdgeRunsAction   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct DeleteColumnJoinsEdgeRunsAction : AnnotationTableTestAction
{
private:
    uint32_t            m_rowIndex;
    ColorDef            m_colorVal;
    ExpectedEdgeColors  m_expectedColors;

public:
    /* ctor */  DeleteColumnJoinsEdgeRunsAction () : m_rowIndex(2), m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        EdgeColorSetter setter (table, m_rowIndex, m_colorVal);

        // |----|----|----|
        setter.SetColor (0, 1, true);
        setter.SetColor (2, 1, true);
        // |oooo|----|oooo|
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        // |oooo|----|oooo|
        table.DeleteColumn (1);
        // |oooo|oooo|

        m_expectedColors.push_back (ExpectedEdgeColor (0, m_colorVal));
        m_expectedColors.push_back (ExpectedEdgeColor (1, m_colorVal));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        EdgeColorVerifier verifier (m_expectedColors, m_rowIndex, true);
        verifier.VerifyColors (table);

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 2);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableEdgeRun),   1);
        expectedCounts.VerifyCounts(table);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_DeleteColumnJoinsEdgeRuns)
    {
    DeleteColumnJoinsEdgeRunsAction   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_DeleteColumnJoinsEdgeRuns)
    {
    DeleteColumnJoinsEdgeRunsAction   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct DeleteColumnRemovesSymbologyEntryAction : AnnotationTableTestAction
{
private:
    uint32_t            m_rowIndex;
    uint32_t            m_colIndex;
    ColorDef            m_colorVal;
    ExpectedEdgeColors  m_expectedColors;

public:
    /* ctor */  DeleteColumnRemovesSymbologyEntryAction () : m_rowIndex(2), m_colIndex(1), m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        EdgeColorSetter setter (table, m_rowIndex, m_colorVal);

        // |----|----|----|
        setter.SetColor (m_colIndex, 1, true);
        // |----|oooo|----|
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        // |----|oooo|----|
        table.DeleteColumn (m_colIndex);
        // |----|----|

        m_expectedColors.push_back (ExpectedEdgeColor (0, table.GetDefaultLineColor()));
        m_expectedColors.push_back (ExpectedEdgeColor (1, table.GetDefaultLineColor()));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        EdgeColorVerifier verifier (m_expectedColors, m_rowIndex, true);
        verifier.VerifyColors (table);

        ExpectedAspectCounts expectedCounts;
        // empty - we added symbology and an edge run, but then deleted the colum which used them.
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_DeleteColumnRemovesSymbologyEntry)
    {
    DeleteColumnRemovesSymbologyEntryAction   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_DeleteColumnRemovesSymbologyEntry)
    {
    DeleteColumnRemovesSymbologyEntryAction   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct InsertColumnExtendsSymbologyAction : AnnotationTableTestAction
{
private:
    uint32_t            m_rowIndex;
    uint32_t            m_colIndex;
    ColorDef            m_colorVal;
    ExpectedEdgeColors  m_expectedColors;

public:
    /* ctor */  InsertColumnExtendsSymbologyAction () : m_rowIndex(2), m_colIndex(1), m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        EdgeColorSetter setter (table, m_rowIndex, m_colorVal);

        // |----|----|----|
        setter.SetColor (m_colIndex, 1, true);
        // |----|oooo|----|
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        // |----|oooo|----|
        table.InsertColumn (m_colIndex, TableInsertDirection::After);
        // |----|oooo|oooo|----|

        m_expectedColors.push_back (ExpectedEdgeColor (0, table.GetDefaultLineColor()));
        m_expectedColors.push_back (ExpectedEdgeColor (1, m_colorVal));
        m_expectedColors.push_back (ExpectedEdgeColor (2, m_colorVal));
        m_expectedColors.push_back (ExpectedEdgeColor (3, table.GetDefaultLineColor()));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        EdgeColorVerifier verifier (m_expectedColors, m_rowIndex, true);
        verifier.VerifyColors (table);

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 2);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableEdgeRun),   3);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_InsertColumnExtendsSymbology)
    {
    InsertColumnExtendsSymbologyAction   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_InsertColumnExtendsSymbology)
    {
    InsertColumnExtendsSymbologyAction   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct InsertColumnNearGapAction : AnnotationTableTestAction
{
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
enum class InsertTarget
    {
    ColumnBefore    = 1,    // Insert a column before the merge block
    ColumnJustBefore= 2,    // Insert a column immediately before the rootIndex of the merge block
    ColumnInterior  = 3,    // Insert a column that intersects the merge block
    ColumnJustAfter = 4,    // Insert a column immediately after the last column of the merge block
    ColumnAfter     = 5,    // Insert a column after the merge block
    };

private:
        AnnotationTableCellIndex                    m_rootIndex;
        uint32_t                                    m_rowSpan;
        uint32_t                                    m_colSpan;
        InsertTarget                                m_insertTarget;

        uint32_t                                    m_rowIndex;
        ColorDef                                    m_colorA;
        ColorDef                                    m_colorB;

        uint32_t                                    m_expectedMergeCount;
        uint32_t                                    m_expectedRunCount;
        ExpectedEdgeColors                          m_expectedColors;

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    /* ctor */  InsertColumnNearGapAction (InsertTarget insertTarget)
        :
        m_rootIndex (1, 1),
        m_rowSpan(2),
        m_colSpan(2),
        m_rowIndex(2),
        m_colorA(ColorDef::Green()),
        m_colorB(ColorDef::Yellow()),
        m_insertTarget (insertTarget),
        m_expectedMergeCount (0)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool    _CreateTable (AnnotationTablePtr& table, DgnDbR db, DgnModelId mid, DgnCategoryId cid, DgnElementId textStyleId) override
        {
        uint32_t          numRows  = 4;
        uint32_t          numCols  = 4;

        // We want a table big enough that the merge block isn't on the edges.  So that
        // we can insert and delete rows/cols before and after the merge.
        AnnotationTable::CreateParams    createParams (db, mid, AnnotationTable::QueryClassId(db), cid);
        table = AnnotationTable::Create (numRows, numCols, textStyleId, 0, createParams);

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        table.MergeCells (m_rootIndex, m_rowSpan, m_colSpan);

        EdgeColorSetter setterA (table, m_rowIndex, m_colorA);
        // |----|         |----|
        setterA.SetColor (0, 1, true);
        // |AAAA|         |----|

        EdgeColorSetter setterB (table, m_rowIndex, m_colorB);
        // |----|         |----|
        setterB.SetColor (3, 1, true);
        // |AAAA|         |BBBB|

        //       0     1     2     3  
        //    |-----------------------|
        //  0 |     |     |     |     |
        //    |-----+-----+-----+-----|
        //  1 |     |           |     |
        //    |AAAAA+           +BBBBB|
        //  2 |     |           |     |
        //    |-----+-----+-----+-----|
        //  3 |     |     |     |     |
        //    |-----+-----+-----+-----|

        m_expectedMergeCount = 1;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 3);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableEdgeRun),   2);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableMerge),     1);
        expectedCounts.VerifyCounts(table);


        // This action is essentially the same as InsertMergedCellsAction, but the verification
        // is concentrated on the edge runs for row 1 which crosses the gap rather than the merges.

        switch (m_insertTarget)
            {
            case InsertTarget::ColumnBefore:
                {
                // Add a column before the column that's before the merge.
                table.InsertColumn (m_rootIndex.col - 1, TableInsertDirection::Before);

                // Expect the merge to move right by one column.
                m_expectedColors.push_back (ExpectedEdgeColor (0, m_colorA));
                m_expectedColors.push_back (ExpectedEdgeColor (1, m_colorA));
                m_expectedColors.push_back (ExpectedEdgeColor (2)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (3)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (4, m_colorB));

                break;
                }
            case InsertTarget::ColumnJustBefore:
                {
                // Add a column immediately before the merge.
                table.InsertColumn (m_rootIndex.col, TableInsertDirection::Before);

                // Expect a new merge since the cells were merged in the seed column.
                m_expectedMergeCount++;

                // Also expect the original merge to move right by one column.
                m_expectedColors.push_back (ExpectedEdgeColor (0, m_colorA));
                m_expectedColors.push_back (ExpectedEdgeColor (1)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (2)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (3)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (4, m_colorB));

                break;
                }
            case InsertTarget::ColumnInterior:
                {
                // Add the row within the merge.
                table.InsertColumn (m_rootIndex.col, TableInsertDirection::After);

                // Expect the merge to grow by one column.
                m_expectedColors.push_back (ExpectedEdgeColor (0, m_colorA));
                m_expectedColors.push_back (ExpectedEdgeColor (1)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (2)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (3)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (4, m_colorB));

                break;
                }
            case InsertTarget::ColumnJustAfter:
                {
                // Add a column immediately after the merge.
                table.InsertColumn (m_rootIndex.col + m_colSpan - 1, TableInsertDirection::After);

                // Expect no change to the original merge.
                // Also expect a new merge since the cells were merged in the seed row.
                m_expectedMergeCount++;

                m_expectedColors.push_back (ExpectedEdgeColor (0, m_colorA));
                m_expectedColors.push_back (ExpectedEdgeColor (1)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (2)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (3)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (4, m_colorB));

                break;
                }
            case InsertTarget::ColumnAfter:
                {
                // Add the row after the row that's past the merge.
                table.InsertColumn (m_rootIndex.col + m_colSpan, TableInsertDirection::After);

                // Expect no change to the merge.
                m_expectedColors.push_back (ExpectedEdgeColor (0, m_colorA));
                m_expectedColors.push_back (ExpectedEdgeColor (1)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (2)); // gap
                m_expectedColors.push_back (ExpectedEdgeColor (3, m_colorB));
                m_expectedColors.push_back (ExpectedEdgeColor (4, m_colorB));

                break;
                }
            default:
                {
                FAIL();
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        EdgeColorVerifier verifier (m_expectedColors, m_rowIndex, true);
        verifier.VerifyColors (table);

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 3);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableEdgeRun),   2);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableMerge),     m_expectedMergeCount);
        expectedCounts.VerifyCounts(table);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertColumnNearGap_Before)
    {
    InsertColumnNearGapAction testAction (InsertColumnNearGapAction::InsertTarget::ColumnBefore);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertColumnNearGap_JustBefore)
    {
    InsertColumnNearGapAction testAction (InsertColumnNearGapAction::InsertTarget::ColumnJustBefore);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertColumnNearGap_Interior)
    {
    InsertColumnNearGapAction testAction (InsertColumnNearGapAction::InsertTarget::ColumnInterior);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertColumnNearGap_JustAfter)
    {
    InsertColumnNearGapAction testAction (InsertColumnNearGapAction::InsertTarget::ColumnJustAfter);
    DoModifyTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, InsertColumnNearGap_After)
    {
    InsertColumnNearGapAction testAction (InsertColumnNearGapAction::InsertTarget::ColumnAfter);
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct SetDefaultTextSymbology : AnnotationTableTestAction
{
private:
    ColorDef      m_colorVal;

public:
    /* ctor */  SetDefaultTextSymbology () : m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        table.SetDefaultTextColor (m_colorVal);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        ColorDef  color = table.GetDefaultTextColor();
        EXPECT_EQ (color, m_colorVal);

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 2);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_SetDefaultTextSymbology)
    {
    SetDefaultTextSymbology   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_SetDefaultTextSymbology)
    {
    SetDefaultTextSymbology   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct ClearDefaultTextSymbology : AnnotationTableTestAction
{
private:
    ColorDef      m_tableColorVal;
    ColorDef      m_colorVal;

public:
    /* ctor */  ClearDefaultTextSymbology () : m_tableColorVal(0), m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        EXPECT_NE (m_tableColorVal, m_colorVal);

        table.SetDefaultTextColor (m_colorVal);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        table.ClearDefaultTextColor ();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        ColorDef  color = table.GetDefaultTextColor();
        EXPECT_EQ (color, m_tableColorVal);

        ExpectedAspectCounts expectedCounts;
        // empty - we added default text symbology, but then cleared it.
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_ClearDefaultTextSymbology)
    {
    ClearDefaultTextSymbology   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_ClearDefaultTextSymbology)
    {
    ClearDefaultTextSymbology   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct DefaultTextSymbSharesEntryWithEdgeRunAction : AnnotationTableTestAction
{
private:
    uint32_t            m_rowIndex;
    ColorDef            m_colorVal;
    ExpectedEdgeColors  m_expectedColors;

public:
    /* ctor */  DefaultTextSymbSharesEntryWithEdgeRunAction () : m_rowIndex(2), m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        EdgeColorSetter setter (table, m_rowIndex, m_colorVal);

        // Adds one symbology entry (total of two).

        // |----|----|----|
        setter.SetColor (0, 1, true);
        setter.SetColor (2, 1, true);
        // |oooo|----|oooo|

        m_expectedColors.push_back (ExpectedEdgeColor (0, m_colorVal));
        m_expectedColors.push_back (ExpectedEdgeColor (1, table.GetDefaultLineColor()));
        m_expectedColors.push_back (ExpectedEdgeColor (2, m_colorVal));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        // Should not add a new symbology entry

        table.SetDefaultTextColor (m_colorVal);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        ColorDef  color = table.GetDefaultTextColor();
        EXPECT_EQ (color, m_colorVal);

        EdgeColorVerifier verifier (m_expectedColors, m_rowIndex, true);
        verifier.VerifyColors (table);

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 2);
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableEdgeRun),   3);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_DefaultTextSymbSharesEntryWithEdgeRunAction)
    {
    DefaultTextSymbSharesEntryWithEdgeRunAction   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_DefaultTextSymbSharesEntryWithEdgeRunAction)
    {
    DefaultTextSymbSharesEntryWithEdgeRunAction   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct SetGetSymbologyAction : AnnotationTableTestAction
{
public:
    enum class EdgeSymbologyId  { A, B, C, D, E };

    struct SetInstruction
        {
        AnnotationTableSymbologyValues          m_symb;
        TableCellListEdges                      m_edges;
        bvector<AnnotationTableCellIndex>       m_cells;
        };

    struct GetInstruction
        {
        bvector<AnnotationTableSymbologyValues> m_expectedResults;
        TableCellListEdges                      m_edges;
        bvector<AnnotationTableCellIndex>       m_cells;
        };

private:
    bvector<SetInstruction> const&      m_setInstructions;
    bvector<GetInstruction> const&      m_getInstructions;

public:
    /* ctor */  SetGetSymbologyAction (bvector<SetInstruction> const& set, bvector<GetInstruction> const& get) : m_setInstructions (set), m_getInstructions (get) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    12/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        for (SetInstruction const& instruction : m_setInstructions)
            table.SetEdgeSymbology (instruction.m_symb, instruction.m_edges, instruction.m_cells);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    12/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        for (GetInstruction const& instruction : m_getInstructions)
            {
            bvector<AnnotationTableSymbologyValues> const& expected = instruction.m_expectedResults;
            bvector<AnnotationTableSymbologyValues>        actual;

            table.GetEdgeSymbology (actual, instruction.m_edges, instruction.m_cells);

            EXPECT_EQ (expected.size(), actual.size());

            for (AnnotationTableSymbologyValues const& expectedValue : expected)
                EXPECT_TRUE (SUCCESS == RemoveExpectedSymbology (expectedValue, actual));

            EXPECT_EQ (0, actual.size());
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    12/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool ActualMatchedExpectedSymbology (AnnotationTableSymbologyValuesCR expected, AnnotationTableSymbologyValuesCR actual)
        {
        if (expected.HasLineVisible() && expected.GetLineVisible() != actual.GetLineVisible())
            return false;

        if (expected.HasLineWeight() && expected.GetLineWeight() != actual.GetLineWeight())
            return false;

        if (expected.HasLineColor() && expected.GetLineColor() != actual.GetLineColor())
            return false;

        if (expected.HasLineStyle() && expected.GetLineStyleId() != actual.GetLineStyleId())
            return false;

        if (expected.HasLineStyle() && expected.GetLineStyleScale() != actual.GetLineStyleScale())
            return false;

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    12/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BentleyStatus RemoveExpectedSymbology (AnnotationTableSymbologyValuesCR expected, bvector<AnnotationTableSymbologyValues>& collection)
        {
        struct FindEquivalentPredicate
            {
            AnnotationTableSymbologyValuesCR  m_expected;

            FindEquivalentPredicate (AnnotationTableSymbologyValuesCR item) : m_expected (item) {}
            bool operator () (AnnotationTableSymbologyValues const& candidate)
                {
                return ActualMatchedExpectedSymbology (m_expected, candidate);
                }
            };

        FindEquivalentPredicate   predicate (expected);
        auto foundIter = std::find_if (collection.begin (), collection.end (), predicate);

        if (foundIter == collection.end())
            return ERROR;

        collection.erase (foundIter);
        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    12/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    static AnnotationTableSymbologyValues CreateSymbValues (EdgeSymbologyId symbId)
        {
        // Each one should be a unique combination.
        // These are for edges, so no fill color.
        AnnotationTableSymbologyValues symb;

        switch (symbId)
            {
            case EdgeSymbologyId::A:
                symb.SetLineColor  (ColorDef::Green());
                break;
            case EdgeSymbologyId::B:
                symb.SetLineColor  (ColorDef::Green());
                symb.SetLineWeight (2);
                break;
            case EdgeSymbologyId::C:
                symb.SetLineColor  (ColorDef::Yellow());
                symb.SetLineStyle  (DgnStyleId(4ULL), 2.0);
                symb.SetLineWeight (4);
                break;
            case EdgeSymbologyId::D:
                symb.SetLineStyle  (DgnStyleId(6ULL), 6.0);
                break;
            case EdgeSymbologyId::E:
                symb.SetLineWeight (8);
                break;
            }

        return symb;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_SetGetSymbology_Simple)
    {
    bvector<SetGetSymbologyAction::SetInstruction>    setInstructions;
    bvector<SetGetSymbologyAction::GetInstruction>    getInstructions;

    //       0     1     2
    //    |-----------------|
    //  0 |     |     |     |
    //    |-----+-----+-----|
    //  1 |     |     |     |
    //    |--a--+--a--+-----|
    //  2 |     b     |     |
    //    |-----+-----+-----|
    //  3 |     c     |     |
    //    |-----+-----+-----|
    //  4 |     |     |     |
    //    |-----|-----|-----|

    auto    symbIdA = SetGetSymbologyAction::EdgeSymbologyId::A;
    auto    symbIdB = SetGetSymbologyAction::EdgeSymbologyId::B;
    auto    symbIdC = SetGetSymbologyAction::EdgeSymbologyId::C;

    // Set symb A on two edges
    {
    SetGetSymbologyAction::SetInstruction  setInstruction;

    setInstruction.m_symb  = SetGetSymbologyAction::CreateSymbValues(symbIdA);
    setInstruction.m_edges = TableCellListEdges::Top;
    setInstruction.m_cells.push_back (AnnotationTableCellIndex (2, 0));
    setInstruction.m_cells.push_back (AnnotationTableCellIndex (2, 1));

    setInstructions.push_back (setInstruction);
    }

    // Set symb B on one edge
    {
    SetGetSymbologyAction::SetInstruction  setInstruction;

    setInstruction.m_symb  = SetGetSymbologyAction::CreateSymbValues(symbIdB);
    setInstruction.m_edges = TableCellListEdges::Right;
    setInstruction.m_cells.push_back (AnnotationTableCellIndex (2, 0));

    setInstructions.push_back (setInstruction);
    }

    // Set symb C on one edge
    {
    SetGetSymbologyAction::SetInstruction  setInstruction;

    setInstruction.m_symb  = SetGetSymbologyAction::CreateSymbValues(symbIdC);
    setInstruction.m_edges = TableCellListEdges::Left;
    setInstruction.m_cells.push_back (AnnotationTableCellIndex (3, 1));

    setInstructions.push_back (setInstruction);
    }

    // Expect symb A
    {
    SetGetSymbologyAction::GetInstruction  getInstruction;

    getInstruction.m_edges = TableCellListEdges::Bottom;
    getInstruction.m_cells.push_back (AnnotationTableCellIndex (1, 0));
    getInstruction.m_cells.push_back (AnnotationTableCellIndex (1, 1));
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdA));

    getInstructions.push_back (getInstruction);
    }

    // Expect symbs B and C
    {
    SetGetSymbologyAction::GetInstruction  getInstruction;

    getInstruction.m_edges = TableCellListEdges::Right;
    getInstruction.m_cells.push_back (AnnotationTableCellIndex (2, 0));
    getInstruction.m_cells.push_back (AnnotationTableCellIndex (3, 0));
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdB));
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdC));

    getInstructions.push_back (getInstruction);
    }

    SetGetSymbologyAction   testAction (setInstructions, getInstructions);
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_SetGetSymbology_OutsideInside)
    {
    bvector<SetGetSymbologyAction::SetInstruction>    setInstructions;
    bvector<SetGetSymbologyAction::GetInstruction>    getInstructions;

    //       0     1     2
    //    |-----------------|
    //  0 |     |     |     |
    //    |-----+-----+-----|
    //  1 |     |     |     |
    //    |--a--+--a--+--a--|
    //  2 a     c     c     a
    //    |--b--+--b--+--a--|
    //  3 a     c     a     |
    //    |--b--+--b--+-----|
    //  4 a     c     a     |
    //    |--a--|--a--|-----|

    auto    symbIdA = SetGetSymbologyAction::EdgeSymbologyId::A;
    auto    symbIdB = SetGetSymbologyAction::EdgeSymbologyId::B;
    auto    symbIdC = SetGetSymbologyAction::EdgeSymbologyId::C;

    bvector<AnnotationTableCellIndex>  cells;
    cells.push_back (AnnotationTableCellIndex (2, 0));
    cells.push_back (AnnotationTableCellIndex (2, 1));
    cells.push_back (AnnotationTableCellIndex (2, 2));
    cells.push_back (AnnotationTableCellIndex (3, 0));
    cells.push_back (AnnotationTableCellIndex (3, 1));
    cells.push_back (AnnotationTableCellIndex (4, 0));
    cells.push_back (AnnotationTableCellIndex (4, 1));

    // Set symb A on outside edges
    {
    SetGetSymbologyAction::SetInstruction  setInstruction;

    setInstruction.m_symb  = SetGetSymbologyAction::CreateSymbValues(symbIdA);
    setInstruction.m_edges = TableCellListEdges::Exterior;
    setInstruction.m_cells = cells;

    setInstructions.push_back (setInstruction);
    }

    // Set symb B on horizontal inside edges
    {
    SetGetSymbologyAction::SetInstruction  setInstruction;

    setInstruction.m_symb  = SetGetSymbologyAction::CreateSymbValues(symbIdB);
    setInstruction.m_edges = TableCellListEdges::InteriorHorizontal;
    setInstruction.m_cells = cells;

    setInstructions.push_back (setInstruction);
    }

    // Set symb C on vertical inside edges
    {
    SetGetSymbologyAction::SetInstruction  setInstruction;

    setInstruction.m_symb  = SetGetSymbologyAction::CreateSymbValues(symbIdC);
    setInstruction.m_edges = TableCellListEdges::InteriorVertical;
    setInstruction.m_cells = cells;

    setInstructions.push_back (setInstruction);
    }

    // Expect symb A only outside edges
    {
    SetGetSymbologyAction::GetInstruction  getInstruction;

    getInstruction.m_edges = TableCellListEdges::Exterior;
    getInstruction.m_cells = cells;
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdA));

    getInstructions.push_back (getInstruction);
    }

    // Expect symbs B and C on inside edges
    {
    SetGetSymbologyAction::GetInstruction  getInstruction;

    getInstruction.m_edges = TableCellListEdges::Interior;
    getInstruction.m_cells = cells;
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdB));
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdC));

    getInstructions.push_back (getInstruction);
    }

    // Expect symbs A, B and C on all edges
    {
    SetGetSymbologyAction::GetInstruction  getInstruction;

    getInstruction.m_edges = TableCellListEdges::All;
    getInstruction.m_cells = cells;
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdA));
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdB));
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdC));

    getInstructions.push_back (getInstruction);
    }

    // Expect symbs A, B on bottom of second row
    {
    SetGetSymbologyAction::GetInstruction  getInstruction;

    getInstruction.m_edges = TableCellListEdges::Bottom;
    getInstruction.m_cells.push_back (AnnotationTableCellIndex (2, 0));
    getInstruction.m_cells.push_back (AnnotationTableCellIndex (2, 1));
    getInstruction.m_cells.push_back (AnnotationTableCellIndex (2, 2));
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdA));
    getInstruction.m_expectedResults.push_back (SetGetSymbologyAction::CreateSymbValues(symbIdB));

    getInstructions.push_back (getInstruction);
    }

    SetGetSymbologyAction   testAction (setInstructions, getInstructions);
    DoCreateTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct SetDefaultFill : AnnotationTableTestAction
{
private:
    ColorDef      m_colorVal;

public:
    /* ctor */  SetDefaultFill () : m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        AnnotationTableSymbologyValues symbology;

        symbology.SetFillColor (m_colorVal);
        table.SetDefaultFill (symbology, TableRows::Odd);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        AnnotationTableSymbologyValues  symb;

        table.GetDefaultFill(symb, TableRows::Odd);
        EXPECT_EQ (true, symb.HasFillColor());
        EXPECT_EQ (m_colorVal, symb.GetFillColor());

        table.GetDefaultFill(symb, TableRows::Even);
        EXPECT_EQ (false, symb.HasFillColor());

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (BIS_SCHEMA(BIS_CLASS_AnnotationTableSymbology), 2);
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_SetDefaultFill)
    {
    SetDefaultFill   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_SetDefaultFill)
    {
    SetDefaultFill   testAction;
    DoModifyTableTest (testAction);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct ClearDefaultFill : AnnotationTableTestAction
{
private:
    ColorDef      m_colorVal;

public:
    /* ctor */  ClearDefaultFill () : m_colorVal(ColorDef::Green()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableR table) override
        {
        AnnotationTableSymbologyValues symbology;

        symbology.SetFillColor (m_colorVal);
        table.SetDefaultFill (symbology, TableRows::Odd);
        table.SetDefaultFill (symbology, TableRows::Even);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableR table) override
        {
        AnnotationTableSymbologyValues symbology;

        symbology.SetFillVisible (false);
        table.SetDefaultFill (symbology, TableRows::Odd);
        table.SetDefaultFill (symbology, TableRows::Even);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    11/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableCR table) const override
        {
        AnnotationTableSymbologyValues  symb;

        table.GetDefaultFill(symb, TableRows::Odd);
        EXPECT_EQ (true,  symb.HasFillVisible());
        EXPECT_EQ (false, symb.GetFillVisible());
        EXPECT_EQ (false, symb.HasFillColor());

        table.GetDefaultFill(symb, TableRows::Even);
        EXPECT_EQ (true,  symb.HasFillVisible());
        EXPECT_EQ (false, symb.GetFillVisible());
        EXPECT_EQ (false, symb.HasFillColor());

        ExpectedAspectCounts expectedCounts;
        // empty - we added default fill, but then cleared it.
        expectedCounts.VerifyCounts(table);
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Create_ClearDefaultFill)
    {
    ClearDefaultFill   testAction;
    DoCreateTableTest (testAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableActionTest, Modify_ClearDefaultFill)
    {
    ClearDefaultFill   testAction;
    DoModifyTableTest (testAction);
    }

#if defined (PERFORMANCE_TEST)

#include <random>
/*-------------------------------------------------------------------------------------*
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t   getRandomNumber (uint32_t mean, double sigma, uint32_t min, uint32_t max)
    {
    static std::default_random_engine s_generator;

    std::normal_distribution<double> distribution(mean, sigma);

    while (true)
        {
        double number = distribution(s_generator);

        //printf ("Generated %f\t with args <%d, %d, %d, %d>\n", number, mean, sigma, min, max);

        if ((number>min)&&(number<max))
            return (uint32_t) number;

        //printf ("Missed one. %f is outside of <%d, %d> with mean %d and sigma %d\n", number, min, max, mean, sigma);
        }
    }

/*-------------------------------------------------------------------------------------*
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String  getRandomLengthString ()
    {
    Utf8CP          seedString = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    Utf8CP          lastCharP  = seedString + strlen (seedString);
    uint32_t        firstWord = getRandomNumber(12, 4.0, 0, 24);
    uint32_t        wordCount = getRandomNumber(4, 0.5, 0, 12);

    uint32_t        iWord = 0;
    Utf8CP          currCharP = seedString;
    Utf8CP          firstInsertCharP = seedString;

    while (firstInsertCharP < lastCharP && iWord < firstWord)
        {
        if (isspace (*currCharP))
            ++iWord;

        currCharP++;

        if (isalpha (*currCharP))
            firstInsertCharP = currCharP;
        }

    iWord = 0;
    currCharP = firstInsertCharP;

    Utf8CP          lastInsertCharP  = currCharP;

    while (currCharP < lastCharP && iWord < wordCount)
        {
        if (isspace (*currCharP))
            ++iWord;

        currCharP++;

        if (isalpha (*currCharP))
            lastInsertCharP = currCharP - 1;
        }

    Utf8String  outStr;
    outStr.assign (firstInsertCharP, lastInsertCharP - firstInsertCharP);

    return outStr;
    }

#include <Bentley\BeTimeUtilities.h>
/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct AnnotationTablePerformanceTest : public AnnotationTableTest
{
 private:
    typedef AnnotationTableTest T_Super;

    uint32_t        m_numRows = 100;
    uint32_t        m_numCols = 100;
    DgnElementId    m_tableElementId;

public:
    AnnotationTablePerformanceTest ()
        {
        }

    void Reset () { m_tableElementId.Invalidate(); }

    void SetUp () override
        {
        T_Super::SetUp();

        Reset();
        }

    void TearDown () override
        {
        T_Super::TearDown();
        }

    AnnotationTablePtr CreateLargeTable (double& time)
        {
        StopWatch timer;
        timer.Start();

        DgnDbR          db          = GetDgnDb();
        DgnModelId      modelId     = GetModelId();
        DgnCategoryId   categoryId  = GetCategoryId();

        AnnotationTable::CreateParams    createParams (db, modelId, AnnotationTable::QueryClassId(db), categoryId);
        AnnotationTablePtr               tableElement = AnnotationTable::Create( m_numRows, m_numCols, GetTextStyleId(), 0, createParams);

        time = 1000.0 * timer.GetCurrentSeconds();

        EXPECT_TRUE (tableElement.IsValid());
        EXPECT_EQ (m_numRows, tableElement->GetRowCount ());
        EXPECT_EQ (m_numCols, tableElement->GetColumnCount ());

        return tableElement;
        }

    void FillTableWithText (double& time, AnnotationTableR table)
        {
        StopWatch timer;
        timer.Start();

        for (AnnotationTableCellR cell : table.GetCellCollection ())
            cell.SetTextString (getRandomLengthString().c_str());

        time = 1000.0 * timer.GetCurrentSeconds();
        }

    void ModifyCellText (AnnotationTableR table)
        {
        AnnotationTableCellP  cell = table.GetCell (AnnotationTableCellIndex (5,5));
        cell->SetTextString ("Hello");
        }

    void ReadTable (double& time, AnnotationTableCPtr& table)
        {
        ASSERT_TRUE (m_tableElementId.IsValid());

        StopWatch timer;
        timer.Start();
        table = AnnotationTable::Get(GetDgnDb(), m_tableElementId);
        time = 1000.0 * timer.GetCurrentSeconds();

        EXPECT_TRUE (table.IsValid());

        EXPECT_EQ (m_numRows, table->GetRowCount ());
        EXPECT_EQ (m_numCols, table->GetColumnCount ());
        }

    void ReadEditableTable (double& time, AnnotationTablePtr& table)
        {
        ASSERT_TRUE (m_tableElementId.IsValid());

        StopWatch timer;
        timer.Start();
        table = AnnotationTable::GetForEdit(GetDgnDb(), m_tableElementId);
        time = 1000.0 * timer.GetCurrentSeconds();

        EXPECT_TRUE (table.IsValid());

        EXPECT_EQ (m_numRows, table->GetRowCount ());
        EXPECT_EQ (m_numCols, table->GetColumnCount ());
        }

    void AddTableToDb (double& time, AnnotationTableR table)
        {
        ASSERT_TRUE ( ! m_tableElementId.IsValid());
        AnnotationTableCPtr  insertedElement;

        StopWatch timer;
        timer.Start();
        insertedElement = table.Insert();
        time = 1000.0 * timer.GetCurrentSeconds();

        EXPECT_TRUE(insertedElement.IsValid());

        m_tableElementId = insertedElement->GetElementId();
        ASSERT_TRUE (m_tableElementId.IsValid());
        }

    void ReplaceTableInDb (double& time, AnnotationTableR table)
        {
        ASSERT_EQ (m_tableElementId, table.GetElementId());

        StopWatch timer;
        timer.Start();
        UpdateElement (table);
        time = 1000.0 * timer.GetCurrentSeconds();

        ASSERT_EQ (m_tableElementId, table.GetElementId());
        }

    void DeleteTableFromModel (double& time, AnnotationTableCR table)
        {
        StopWatch timer;
        timer.Start();
        EXPECT_EQ (DgnDbStatus::Success, table.Delete());
        time = 1000.0 * timer.GetCurrentSeconds();
        }

    void DoCRUDTiming(uint32_t iter)
        {
        double  createTime, fillTime, addTime, readTime, replaceTime, deleteTime;

        AnnotationTablePtr createTable  = CreateLargeTable(createTime);

        FillTableWithText (fillTime, *createTable);

        AddTableToDb (addTime, *createTable);

        createTable = nullptr;

#if defined (FOR_DEBUGGING)
        // Put a break point on Reopen in order to open the file in an viewing application
        CloseTestFile();
        ReopenTestFile();
#endif
        // Purge the cache so that we don't get a cached element.
        GetDgnDb().Elements().Purge(0);

        AnnotationTablePtr   tableToModify;
        ReadEditableTable (readTime, tableToModify);

        ModifyCellText (*tableToModify);
        ReplaceTableInDb (replaceTime, *tableToModify);

        double dummyTime;
        AnnotationTableCPtr   foundTable;
        ReadTable (dummyTime, foundTable);

        DeleteTableFromModel (deleteTime, *foundTable);

        foundTable = nullptr;

#if defined (FOR_DEBUGGING)
        // Put a break point on Reopen in order to open the file in an viewing application
        CloseTestFile();
        ReopenTestFile();
#endif

        Reset();

        printf ("%d: create: %fms, fill: %fms, add: %fms, read: %fms, replace: %fms, delete: %fms\n", iter, createTime, fillTime, addTime, readTime, replaceTime, deleteTime);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTablePerformanceTest, CreateWriteReadLargeTable)
    {
    // The first call to Create is a lot slower than the rest.  Just make one
    // here and disregard the time it takes.
    //AnnotationTablePtr table = AnnotationTable::Create (5, 3, GetTextStyleId(), 1000.0, *GetDgnModelP());
    //table = nullptr;

    // Prepopulate the file with lots of tables.
    uint32_t initialCount = 100;
    
    for (uint32_t iIter = 0; iIter < initialCount; ++iIter)
        {
        double  createTime, fillTime, addTime;
    
        AnnotationTablePtr createTable  = CreateLargeTable(createTime);
        FillTableWithText (fillTime, *createTable);
        AddTableToDb (addTime, *createTable);
        Reset();
    
        printf ("%d: create: %fms, fill: %fms, add: %fms\n", iIter, createTime, fillTime, addTime);
        }

    uint32_t numIters = 5;

    for (uint32_t iIter = 0; iIter < numIters; ++iIter)
        DoCRUDTiming(iIter);
    }

#endif //PERFORMANCE_TEST
