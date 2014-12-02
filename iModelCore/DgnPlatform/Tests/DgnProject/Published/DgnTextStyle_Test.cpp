/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnTextStyle_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

//=======================================================================================
// Test fixture for testing DgnTextStyles.
// Note: This file used to have tests for old API. The same file has been retained 
// with tests for new API
// @bsiclass                            Majd Uddin                          07/2014
//=======================================================================================
struct DgnTextStylesTest : public ::testing::Test
{
public:
    ScopedDgnHost           m_host;
    DgnProjectPtr           project;

    void SetupProject(WCharCP projFile, FileOpenMode mode);
};

//=======================================================================================
// Set up method for DgnProject that has text styles
// @bsimethod                            Majd Uddin                          07/2014
//=======================================================================================
void DgnTextStylesTest::SetupProject(WCharCP projFile, FileOpenMode mode)
{
    DgnDbTestDgnManager tdm(projFile, __FILE__, mode);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);
}

//=======================================================================================
// Test to make sure that iterator gets the right count
// @bsimethod                            Majd Uddin                          07/2014
//=======================================================================================
TEST_F(DgnTextStylesTest, GetTextStyleCount)
{
    SetupProject(L"SubStation_NoFence.i.idgndb", OPENMODE_READONLY);

    //Get text styles count
    DgnStyles& styleTable = project->Styles();
    DgnStyles::Iterator iterTextStyles = styleTable.TextStyles().MakeIterator(DgnStyleSort::NameAsc);
    ASSERT_EQ(10, iterTextStyles.QueryCount()) << "The expected text styles count is 10 where as it is: " << iterTextStyles.QueryCount();

}
//=======================================================================================
// Query by id and name
// @bsimethod                            Majd Uddin                          07/2014
//=======================================================================================
TEST_F(DgnTextStylesTest, QueryTextStyle)
{
    SetupProject(L"SubStation_NoFence.i.idgndb", OPENMODE_READONLY);
    DgnStyles& styleTable = project->Styles();

    //Query by Id
    DgnStyleId styleId(1);
    DgnTextStylePtr textStyle = styleTable.TextStyles().QueryById(styleId);
    ASSERT_TRUE(textStyle != NULL);
    ASSERT_EQ(1, textStyle->GetId().GetValue());

    //Query by Name
    DgnTextStylePtr textStyle2 = styleTable.TextStyles().QueryByName("Device ID");
    ASSERT_TRUE(textStyle2 != NULL);
    ASSERT_TRUE(textStyle2->GetName().Equals("Device ID"));

}

//=======================================================================================
// Insert a new text style
// @bsimethod                            Majd Uddin                          07/2014
//=======================================================================================
TEST_F(DgnTextStylesTest, InsertTextStyle)
{
    SetupProject(L"SubStation_NoFence.i.idgndb", OPENMODE_READWRITE);
    DgnStyles& styleTable = project->Styles();

    //Create a text style
    DgnTextStylePtr textStyle = DgnTextStyle::Create(*project);
    //textStyle->SetId(DgnStyleId(25)); -- JM: SetId is no longer published. It is needed public internally, but users should not actually be able to modify the ID.
    textStyle->SetName("Test text style");

    textStyle->SetPropertyValue(DgnTextStyleProperty::Font, 5);
    textStyle->SetPropertyValue(DgnTextStyleProperty::IsBold, true);
    textStyle->SetPropertyValue(DgnTextStyleProperty::MaxCharactersPerLine, 500);
    textStyle->SetPropertyValue(DgnTextStyleProperty::CharacterSpacingValueFactor, 1.20);

    //Insert this text style
    DgnTextStylePtr textStyle2 = styleTable.TextStyles().Insert(*textStyle);
    ASSERT_TRUE(textStyle2 != NULL);
    ASSERT_TRUE(textStyle2->GetName().Equals("Test text style"));

    //The count now should be 10 + 1
    DgnStyles::Iterator iterTextStyles = styleTable.TextStyles().MakeIterator(DgnStyleSort::NameAsc);
    ASSERT_EQ(11, iterTextStyles.QueryCount()) << "The expected text styles count is 11 where as it is: " << iterTextStyles.QueryCount();

}

//=======================================================================================
// Delete a text style
// @bsimethod                            Majd Uddin                          07/2014
//=======================================================================================
TEST_F(DgnTextStylesTest, DeleteTextStyle)
{
    SetupProject(L"SubStation_NoFence.i.idgndb", OPENMODE_READWRITE);
    DgnStyles& styleTable = project->Styles();

    //delete text style
    DgnStyleId styleId(1);
    ASSERT_EQ(SUCCESS, styleTable.TextStyles().Delete(styleId));

    //The count now should be 10 - 1
    DgnStyles::Iterator iterTextStyles = styleTable.TextStyles().MakeIterator(DgnStyleSort::NameAsc);
    ASSERT_EQ(9, iterTextStyles.QueryCount()) << "The expected text styles count is 9 where as it is: " << iterTextStyles.QueryCount();

    //The id serarch shouldn't work
    DgnTextStylePtr textStyle = styleTable.TextStyles().QueryById(styleId);
    ASSERT_FALSE(textStyle != NULL);

}

//=======================================================================================
// Update an existing text style
// @bsimethod                            Majd Uddin                          07/2014
//=======================================================================================
TEST_F(DgnTextStylesTest, UpdateTextStyle)
{
    SetupProject(L"SubStation_NoFence.i.idgndb", OPENMODE_READWRITE);
    DgnStyles& styleTable = project->Styles();

    //Get a text style
    DgnStyleId styleId(1);
    DgnTextStylePtr textStyle = styleTable.TextStyles().QueryById(styleId);
    ASSERT_TRUE(textStyle != NULL);

    //update some properties
    textStyle->SetName("Test updated style");
    textStyle->SetPropertyValue(DgnTextStyleProperty::Font, 5);
    textStyle->SetPropertyValue(DgnTextStyleProperty::IsBold, true);

    //Update this text style
    ASSERT_EQ(SUCCESS, styleTable.TextStyles().Update(*textStyle));


    //Verify that it is updated
    DgnTextStylePtr textStyle2 = styleTable.TextStyles().QueryById(styleId);
    EXPECT_TRUE(textStyle2 != NULL);
    ASSERT_TRUE(textStyle2->GetName().Equals("Test updated style"));
    ASSERT_FALSE(textStyle2->GetName().Equals("Device ID")); //old name shouldn't exist

}

