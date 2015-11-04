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
AnnotationTableTest() : GenericDgnModelTestFixture (__FILE__, false /*2D*/)
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
    virtual bool    _CreateTable (AnnotationTableElementPtr&, AnnotationTextStyleId textStyleID, DgnModelId) { return false; }
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

        if (testAction._CreateTable (table, GetTextStyleId(), GetModelId()))
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

