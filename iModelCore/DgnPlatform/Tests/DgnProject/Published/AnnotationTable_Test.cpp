/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/AnnotationTable_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/AnnotationTable.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

// Map of aspect name to count
typedef    bmap<Utf8String, size_t>     AspectCountMap;
typedef    bpair<Utf8String, size_t>    AspectCountEntry;

struct TestAnnotationTableAspectDescr
    {
    Utf8String  m_tableName;
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
        { DGN_TABLE(DGN_CLASSNAME_AnnotationTableHeader),   true    },
        { DGN_TABLE(DGN_CLASSNAME_AnnotationTableRow),      false   },
        { DGN_TABLE(DGN_CLASSNAME_AnnotationTableColumn),   false   },
        { DGN_TABLE(DGN_CLASSNAME_AnnotationTableCell),     false   },
        { DGN_TABLE(DGN_CLASSNAME_AnnotationTableMerge),    false   },
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
        AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableHeader), 1);

        // Every table at least one symbology aspect
        //AddEntry (DGN_CLASSNAME_AnnotationTableSymbology, 1);
        }

    /* ctor */  ExpectedAspectCounts (uint32_t rowsExpected, uint32_t colsExpected, uint32_t cellsExpected, uint32_t mergesExpected)
        :
        ExpectedAspectCounts()
        {
        if (0 < rowsExpected)
            AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableRow), rowsExpected);

        if (0 < colsExpected)
            AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableColumn), colsExpected);

        if (0 < cellsExpected)
            AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableCell), cellsExpected);

        if (0 < mergesExpected)
            AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableMerge), mergesExpected);
        }

    void        AddEntry (Utf8CP tableName, size_t count)
        {
        m_expectedCounts[tableName] = count;
        }

    Utf8String  BuildSelectCountString (TestAnnotationTableAspectDescr const& aspectDescr)
        {
        Utf8String sqlString ("SELECT count(*) FROM ");
        sqlString.append (aspectDescr.m_tableName);

        Utf8CP  idPropertyName = aspectDescr.m_isUniqueAspect ? "ECInstanceId" : "ElementId";
        Utf8PrintfString whereStr(" WHERE %s=?", idPropertyName);
        sqlString.append (whereStr);

        return sqlString;
        }

    int         GetActualCount (TestAnnotationTableAspectDescr const& aspectDescr, DgnElementId elementId, DgnDbCR db)
        {
        Utf8String  sqlString = BuildSelectCountString (aspectDescr);
        Statement   statement;
        DbResult    prepareStatus = statement.Prepare (db, sqlString.c_str());
        EXPECT_TRUE (BE_SQLITE_OK == prepareStatus);

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
                counts[aspectDescr.m_tableName] = count;
            }
        }

    void        VerifyCounts (AnnotationTableElementCR table)
        {
        AspectCountMap actualCounts;

        GetActualCounts (actualCounts, table.GetElementId(), table.GetDgnDb());

        for (AspectCountEntry const& entry: m_expectedCounts)
            {
            Utf8String const&                   tableName       = entry.first;
            size_t                              expectedCount   = entry.second;
            AspectCountMap::const_iterator      matchingEntry   = actualCounts.find(tableName);
            size_t                              actualCount     = 0;

            if (actualCounts.end() != matchingEntry)
                actualCount = (*matchingEntry).second;

            EXPECT_EQ (expectedCount, actualCount) << "Aspect count mismatch for table: " << tableName.c_str();

            actualCounts.erase (tableName);
            }

        // There are aspects on the element that were not expected.
        if (actualCounts.empty())
            return;

        for (AspectCountEntry const& entry: actualCounts)
            {
            Utf8String const&                   tableName   = entry.first;
            size_t                              actualCount = entry.second;

            EXPECT_EQ (0, actualCount) << "Aspect count mismatch for table: " << tableName.c_str();
            }
        }
};

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct AnnotationTableTest : public GenericDgnModelTestFixture /* public testing::Test*/
{
private:
    const    Utf8CP m_modelName     = "TestModel";
    const    Utf8CP m_categoryName  = "TestCategory";

    typedef GenericDgnModelTestFixture T_Super;

    DgnModelId              m_modelId;
    DgnCategoryId           m_categoryId;
    AnnotationTextStyleId   m_textStyleId;

public:
AnnotationTableTest() : GenericDgnModelTestFixture (__FILE__, false /*2D*/, false /*needBriefcase*/)
    {
    }

void SetUp () override
    {
    T_Super::SetUp();

    // Create a category
    DgnCategory category(DgnCategory::CreateParams(*GetDgnProjectP(), m_categoryName, DgnCategory::Scope::Physical));
    DgnSubCategory::Appearance appearance;
    category.Insert(appearance);

    m_categoryId = category.GetCategoryId();
    ASSERT_TRUE(m_categoryId.IsValid());

    // Create a text style
    AnnotationTextStylePtr textStyle = AnnotationTextStyle::Create(*GetDgnProjectP());
    textStyle->SetName(GetTextStyleName());
    textStyle->SetHeight(GetTextStyleHeight());
    textStyle->SetFontId(GetDgnProjectP()->Fonts().AcquireId(DgnFontManager::GetAnyLastResortFont()));
    textStyle->Insert();

    m_textStyleId = textStyle->GetStyleId();
    ASSERT_TRUE(m_textStyleId.IsValid());

    // Create a physical model
    DgnModelPtr model = new PhysicalModel(PhysicalModel::CreateParams(*GetDgnProjectP(), DgnClassId(GetDgnProjectP()->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel)), DgnModel::CreateModelCode(m_modelName)));
    ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

    m_modelId = model->GetModelId();
    ASSERT_TRUE(m_modelId.IsValid());
    }

DgnDbR                  GetDgnDb()              { return *GetDgnProjectP(); }
DgnModelId              GetModelId()            { return m_modelId; }
DgnCategoryId           GetCategoryId()         { return m_categoryId; }
AnnotationTextStyleId   GetTextStyleId()        { return m_textStyleId; }
Utf8CP                  GetTextStyleName()      { return "TextStyleForTable"; }
double                  GetTextStyleHeight()    { return 0.25; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableElementPtr    CreateBasicTable (int numRows = 5, int numCols = 3)
    {
    DgnDbR          db          = GetDgnDb();
    DgnModelId      modelId     = GetModelId();
    DgnCategoryId   categoryId  = GetCategoryId();

    AnnotationTableElement::CreateParams    createParams (db, modelId, AnnotationTableElement::QueryClassId(db), categoryId);
    AnnotationTableElementPtr               tableElement = AnnotationTableElement::Create(numRows, numCols, GetTextStyleId(), 0, createParams);
    EXPECT_TRUE (tableElement.IsValid());

    return tableElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    InsertElement (AnnotationTableElementR element)
    {
    AnnotationTableElementCPtr  insertedElement = element.Insert();
    EXPECT_TRUE(insertedElement.IsValid());

    DgnElementId elementId = insertedElement->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    return elementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void        UpdateElement (AnnotationTableElementR element)
    {
    AnnotationTableElementCPtr  updatedElement = element.Update();
    EXPECT_TRUE(updatedElement.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    CreateBasicTablePersisted (int numRows = 5, int numCols = 3)
    {
    AnnotationTableElementPtr   tableElement = CreateBasicTable (numRows, numCols);
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
static void     VerifyCellCollection (AnnotationTableElementCR table, bvector<AnnotationTableCellIndex> const* exclusions)
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
static void     VerifyCellsWithMergeBlocks (AnnotationTableElementCR table, bvector<MergeDescr> mergeBlocks)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AnnotationTableTest, BasicCreate)
    {
    AnnotationTableElementPtr   tableElement = CreateBasicTable ();

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

    // Shutdown and reopen the project so that we don't get a cached element.
    CloseTestFile();
    ReopenTestFile();

    AnnotationTableElementCPtr readTableElement = AnnotationTableElement::Get(GetDgnDb(), elementId);
    ASSERT_TRUE(readTableElement.IsValid());

    EXPECT_EQ (numRows, readTableElement->GetRowCount ());
    EXPECT_EQ (numCols, readTableElement->GetColumnCount ());

    // Expect the minimum aspects on the element
    ExpectedAspectCounts expectedCounts;
    expectedCounts.VerifyCounts(*readTableElement);
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

    // Shutdown and reopen the project so that we don't get a cached element.
    CloseTestFile();
    ReopenTestFile();

    AnnotationTableElementCPtr readTableElement = AnnotationTableElement::Get(GetDgnDb(), elementId1);
    ASSERT_TRUE(readTableElement.IsValid());

    EXPECT_EQ (numRows1, readTableElement->GetRowCount ());
    EXPECT_EQ (numCols1, readTableElement->GetColumnCount ());

    // Expect the minimum aspects on the element
    ExpectedAspectCounts expectedCounts;
    expectedCounts.VerifyCounts(*readTableElement);

    readTableElement = AnnotationTableElement::Get(GetDgnDb(), elementId2);
    ASSERT_TRUE(readTableElement.IsValid());

    EXPECT_EQ (numRows2, readTableElement->GetRowCount ());
    EXPECT_EQ (numCols2, readTableElement->GetColumnCount ());

    // Expect the minimum aspects on the element
    expectedCounts.VerifyCounts(*readTableElement);
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

    AnnotationTableElementPtr   tableElement1 = CreateBasicTable (numRows1, numCols1);
    AnnotationTableElementPtr   tableElement2 = CreateBasicTable (numRows2, numCols2);

    for (ExpectedRowHeight const& entry : rowHeights1)
        tableElement1->GetRow (entry.first)->SetHeight(entry.second);

    for (ExpectedRowHeight const& entry : rowHeights2)
        tableElement2->GetRow (entry.first)->SetHeight(entry.second);

    // Insert both tables.
    DgnElementId elementId1 = InsertElement (*tableElement1);
    DgnElementId elementId2 = InsertElement (*tableElement2);

    tableElement1 = nullptr;
    tableElement2 = nullptr;

    // Shutdown and reopen the project so that we don't get a cached element.
    CloseTestFile();
    ReopenTestFile();

    AnnotationTableElementCPtr readTableElement = AnnotationTableElement::Get(GetDgnDb(), elementId1);
    ASSERT_TRUE(readTableElement.IsValid());

    for (ExpectedRowHeight const& entry : rowHeights1)
        EXPECT_EQ (entry.second, readTableElement->GetRow (entry.first)->GetHeight());

    // Expect row aspects for the rows with non-default heights.
    ExpectedAspectCounts expectedCounts1;
    expectedCounts1.AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableRow), rowHeights1.size());
    expectedCounts1.VerifyCounts(*readTableElement);

    readTableElement = AnnotationTableElement::Get(GetDgnDb(), elementId2);
    ASSERT_TRUE(readTableElement.IsValid());

    for (ExpectedRowHeight const& entry : rowHeights2)
        EXPECT_EQ (entry.second, readTableElement->GetRow (entry.first)->GetHeight());

    // Expect row aspects for the rows with non-default heights.
    ExpectedAspectCounts expectedCounts2;
    expectedCounts2.AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableRow), rowHeights2.size());
    expectedCounts2.VerifyCounts(*readTableElement);
    }

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct AnnotationTableTestAction
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _CreateTable (AnnotationTableElementPtr&, DgnDbR, DgnModelId, DgnCategoryId, AnnotationTextStyleId) { return false; }
    virtual void    _PreAction (AnnotationTableElementR) {}
    virtual void    _DoAction (AnnotationTableElementR) = 0;
    virtual void    _VerifyAction (AnnotationTableElementCR) const = 0;
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

    AnnotationTableElementPtr CreateTable (AnnotationTableTestAction& testAction)
        {
        AnnotationTableElementPtr    table;

        if (testAction._CreateTable (table, GetDgnDb(), GetModelId(), GetCategoryId(), GetTextStyleId()))
            return table;

        return CreateBasicTable();
        }

    void ReadConstTable (AnnotationTableElementCPtr& table)
        {
        EXPECT_TRUE (m_tableElementId.IsValid());

        table = AnnotationTableElement::Get(GetDgnDb(), m_tableElementId);
        EXPECT_TRUE (table.IsValid());
        }

    void ReadEditableTable (AnnotationTableElementPtr& table)
        {
        EXPECT_TRUE (m_tableElementId.IsValid());

        table = AnnotationTableElement::GetForEdit(GetDgnDb(), m_tableElementId);
        EXPECT_TRUE (table.IsValid());
        }

    void AddTableToDb (AnnotationTableElementR table)
        {
        ASSERT_TRUE ( ! m_tableElementId.IsValid());
        m_tableElementId = InsertElement (table);
        ASSERT_TRUE (m_tableElementId.IsValid());
        }

    void UpdateTableInDb (AnnotationTableElementR table)
        {
        ASSERT_EQ (m_tableElementId, table.GetElementId());
        UpdateElement (table);
        ASSERT_EQ (m_tableElementId, table.GetElementId());
        }

    void DoCreateTableTest (AnnotationTableTestAction& testAction)
        {
        AnnotationTableElementPtr seedTable = CreateTable(testAction);
        EXPECT_TRUE (seedTable.IsValid());

        testAction._PreAction (*seedTable);
        testAction._DoAction  (*seedTable);

        AddTableToDb (*seedTable);
        seedTable = nullptr;

        // Shutdown and reopen the project so that we don't get a cached element.
        CloseTestFile();
        ReopenTestFile();

        AnnotationTableElementCPtr    foundTable;

        ReadConstTable (foundTable);
        EXPECT_TRUE (foundTable.IsValid());

        testAction._VerifyAction (*foundTable);
        }

    void DoModifyTableTest (AnnotationTableTestAction& testAction)
        {
        AnnotationTableElementPtr seedTable = CreateTable(testAction);
        EXPECT_TRUE (seedTable.IsValid());

        testAction._PreAction (*seedTable);

        AddTableToDb (*seedTable);
        seedTable = nullptr;

        AnnotationTableElementPtr        applyActionTable;

        ReadEditableTable (applyActionTable);
        EXPECT_TRUE (applyActionTable.IsValid());

        testAction._DoAction  (*applyActionTable);

        UpdateTableInDb (*applyActionTable);
        applyActionTable = nullptr;

        // Shutdown and reopen the project so that we don't get a cached element.
        CloseTestFile();
        ReopenTestFile();

        AnnotationTableElementCPtr    postActionTable;

        ReadConstTable (postActionTable);
        EXPECT_TRUE (postActionTable.IsValid());

        testAction._VerifyAction (*postActionTable);
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
    void    _DoAction (AnnotationTableElementR table) override
        {
        // Don't do anything, a table is created and written by the test logic
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
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
    void    _DoAction (AnnotationTableElementR table) override
        {
        double  defaultRowHeight  = table.GetDefaultRowHeight();
        EXPECT_TRUE (defaultRowHeight != m_overrideHeight);

        table.GetRow(m_index)->SetHeight (m_overrideHeight);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
        {
        double  defaultRowHeight  = table.GetDefaultRowHeight();

        for (uint32_t iRow = 0; iRow < table.GetRowCount(); iRow++)
            {
            double expectedHeight = (m_index == iRow) ? m_overrideHeight : defaultRowHeight;

            EXPECT_EQ (expectedHeight, table.GetRow(iRow)->GetHeight());
            }

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableRow), 1);
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
    void    _DoAction (AnnotationTableElementR table) override
        {
        double  defaultColumnWidth  = table.GetDefaultColumnWidth();
        EXPECT_TRUE (defaultColumnWidth != m_overrideWidth);

        table.GetColumn(m_index)->SetWidth (m_overrideWidth);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
        {
        double  defaultColumnWidth  = table.GetDefaultColumnWidth();

        for (uint32_t iColumn = 0; iColumn < table.GetColumnCount(); iColumn++)
            {
            double expectedWidth = (m_index == iColumn) ? m_overrideWidth : defaultColumnWidth;

            EXPECT_EQ (expectedWidth, table.GetColumn(iColumn)->GetWidth());
            }

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableColumn), 1);
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
    void    _PreAction (AnnotationTableElementR table) override
        {
        double  overrideHeight = 0.75;
        double  defaultRowHeight  = table.GetDefaultRowHeight();
        EXPECT_TRUE (defaultRowHeight != overrideHeight);

        table.GetRow(m_index)->SetHeight (overrideHeight);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableElementR table) override
        {
        table.GetRow(m_index)->SetHeightFromContents();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
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
    void    _DoAction (AnnotationTableElementR table) override
        {
        AnnotationTextStyleId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId, m_applyString.c_str());

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
        {
        AnnotationTableCellP  foundCell = table.GetCell (m_cellIndex);
        AnnotationTextBlockCP foundTextBlock = foundCell->GetTextBlock();
#if defined (NEEDSWORK)
        Utf8String            foundString = foundTextBlock->ToString();
        
        EXPECT_STREQ (m_applyString.c_str(), foundString.c_str());
#else
        EXPECT_TRUE (nullptr != foundTextBlock);
#endif

        AnnotationTableCellIndex  anotherCell (m_cellIndex.row - 1, m_cellIndex.col - 1);
        foundCell = table.GetCell (anotherCell);

        EXPECT_TRUE (NULL == foundCell->GetTextBlock());

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableCell), 1);
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
    void    _PreAction (AnnotationTableElementR table) override
        {
        AnnotationTextStyleId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId, "abcdefghi");

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableElementR table) override
        {
        AnnotationTextStyleId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId, m_applyString.c_str());

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
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
        expectedCounts.AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableCell), 1);
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
    void    _PreAction (AnnotationTableElementR table) override
        {
        AnnotationTextStyleId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId, "abcdefghi");

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableElementR table) override
        {
        AnnotationTextStyleId   textStyleId = table.GetTextStyleId(AnnotationTableRegion::Body);
        AnnotationTextBlockPtr  textBlock   = AnnotationTextBlock::Create(table.GetDgnDb(), textStyleId);

        // textBlock is empty

        AnnotationTableCellP  cell = table.GetCell (m_cellIndex);
        cell->SetTextBlock (*textBlock);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
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
    void    _PreAction (AnnotationTableElementR table) override
        {
        m_rowCount = table.GetRowCount();

        for (uint32_t iRow = 0; iRow < m_rowCount; iRow++)
            table.GetRow(iRow)->SetHeight (10.0*(1+iRow));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    07/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableElementR table) override
        {
        // Delete a row
        table.DeleteRow (m_rowIndex);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    07/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
        {
        EXPECT_EQ (m_rowCount - 1, table.GetRowCount());

        ExpectedAspectCounts expectedCounts;
        expectedCounts.AddEntry (DGN_TABLE(DGN_CLASSNAME_AnnotationTableRow), m_rowCount - 1);
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
    bool    _CreateTable (AnnotationTableElementPtr& table, DgnDbR db, DgnModelId mid, DgnCategoryId cid, AnnotationTextStyleId tsid) override
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

        AnnotationTableElement::CreateParams    createParams (db, mid, AnnotationTableElement::QueryClassId(db), cid);
        table = AnnotationTableElement::Create (numRows, numCols, tsid, 0, createParams);

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableElementR table) override
        {
        // Merge a block of cells
        bool          failed = (SUCCESS != table.MergeCells (m_rootIndex, m_rowSpan, m_colSpan));
        ASSERT_EQ (m_expectFail, failed);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
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
    bool    _CreateTable (AnnotationTableElementPtr& table, DgnDbR db, DgnModelId mid, DgnCategoryId cid, AnnotationTextStyleId tsid) override
        {
        uint32_t          numRows  = 4;
        uint32_t          numCols  = 4;

        AnnotationTableElement::CreateParams    createParams (db, mid, AnnotationTableElement::QueryClassId(db), cid);
        table = AnnotationTableElement::Create (numRows, numCols, tsid, 0, createParams);

        table->MergeCells (m_existingMerge.m_rootIndex, m_existingMerge.m_rowSpan, m_existingMerge.m_colSpan);

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    03/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableElementR table) override
        {
        // Merge a block of cells
        bool          failed = (SUCCESS != table.MergeCells (m_rootIndex, m_rowSpan, m_colSpan));
        ASSERT_EQ (m_expectFail, failed);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    03/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _VerifyAction (AnnotationTableElementCR table) const override
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
    bool    _CreateTable (AnnotationTableElementPtr& table, DgnDbR db, DgnModelId mid, DgnCategoryId cid, AnnotationTextStyleId tsid) override
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

        AnnotationTableElement::CreateParams    createParams (db, mid, AnnotationTableElement::QueryClassId(db), cid);
        table = AnnotationTableElement::Create (numRows, numCols, tsid, 0, createParams);

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableElementR table) override
        {
        // Merge a block of cells
        table.MergeCells (m_rootIndex, m_rowSpan, m_colSpan);

        // At this point, the table should have one merge instance
        m_numMergeInstances = 1;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableElementR table) override
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
    void    _VerifyAction (AnnotationTableElementCR table) const override
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
    bool    _CreateTable (AnnotationTableElementPtr& table, DgnDbR db, DgnModelId mid, DgnCategoryId cid, AnnotationTextStyleId tsid) override
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

        AnnotationTableElement::CreateParams    createParams (db, mid, AnnotationTableElement::QueryClassId(db), cid);
        table = AnnotationTableElement::Create (numRows, numCols, tsid, 0, createParams);

        for (uint32_t iRow = 0; iRow < table->GetRowCount(); ++iRow)
            table->GetRow(iRow)->SetHeight(10.0);

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _PreAction (AnnotationTableElementR table) override
        {
        // Merge a block of cells
        table.MergeCells (m_rootIndex, m_rowSpan, m_colSpan);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    04/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _DoAction (AnnotationTableElementR table) override
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
    void    _VerifyAction (AnnotationTableElementCR table) const override
        {
        AnnotationTableTest::VerifyCellsWithMergeBlocks (table, m_expectedMerges);

        ExpectedAspectCounts expectedCounts (table.GetRowCount(), 0, 0, (uint32_t) m_expectedMerges.size());
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

