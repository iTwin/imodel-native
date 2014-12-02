/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/MSElementDescr_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     Evan.Williams  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
class MSElementDescrTest : public testing::Test
{
protected:
ScopedDgnHost       m_host;
DgnDbTestDgnManager m_testDataManager;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Evan.Williams  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrTest() : m_testDataManager (L"2dMetricGeneral.idgndb")
    {
    BeAssert( NULL != GetDgnModelP() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Evan.Williams  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~MSElementDescrTest () { }

DgnModelP GetDgnModelP() {return m_testDataManager.GetDgnModelP();}
};

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void compareDescr(MSElementDescrCR one, MSElementDescrCR two)
    {
    ASSERT_TRUE(&one != &two);
    ASSERT_TRUE(&one.GetDgnModel() == &two.GetDgnModel());
    ASSERT_TRUE(one.GetElementRef() == two.GetElementRef());
    ASSERT_TRUE(one.GetItemId() ==  two.GetItemId());
    ASSERT_TRUE(one.GetElementHandler() == two.GetElementHandler());
    ASSERT_TRUE(one.Element().Size() == two.Element().Size());
    ASSERT_TRUE(0 == memcmp(&one.Element(), &two.Element(), one.Element().Size()));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MSElementDescrTest, Allocation)
    {
#if defined (NEEDS_WORK_DGNITEM)
    DgnModelR model = *GetDgnModelP();
    DSegment3d segment;
    segment.Init (-1000.0, 0.0, 0.0, 1000.0, 0.0, 0.0);

    EditElementHandle lineHandle;
    ASSERT_EQ (SUCCESS, LineHandler::CreateLineElement (lineHandle, NULL, segment, false, model));
    
    ASSERT_EQ (1, MSElementDescr::DebugGetExtantCount());
    MSElementDescrP line = lineHandle.GetElementDescrP();

    EditElementHandle    parentEeh;
    NormalCellHeaderHandler::CreateOrphanCellElement (parentEeh, L"parent", false, model);
    MSElementDescrP parent = parentEeh.GetElementDescrP();
    ASSERT_EQ (2, MSElementDescr::DebugGetExtantCount());
    ASSERT_EQ (SUCCESS, parent->AddComponent(*line));
    ASSERT_TRUE(parent == line->GetParent());

    BeTest::SetFailOnAssert(false);
    ASSERT_NE (SUCCESS, parent->AddComponent(*line)); // it is already parented, it shouldn't work twice
    BeTest::SetFailOnAssert(true);

    for (int i=0; i<10; ++i)
        {
        EditElementHandle line2;
        ASSERT_EQ (SUCCESS, LineHandler::CreateLineElement (line2, NULL, segment, false, model));
        ASSERT_EQ (SUCCESS, parent->AddComponent(*line2.GetElementDescrP()));
        }

    MSElementDescrPtr clone = parent->Duplicate();
    ASSERT_EQ (24, MSElementDescr::DebugGetExtantCount());
    compareDescr (*parent, *clone);

    int count = 0;
    for (ChildElemIter child (parentEeh, ExposeChildrenReason::Query); child.IsValid(); child = child.ToNext())
        ++count;
    ASSERT_EQ (11, count);

    count = 0;
    for (ChildEditElemIter child (parentEeh, ExposeChildrenReason::Query); child.IsValid(); child = child.ToNext())
        ++count;
    ASSERT_EQ (11, count);

    clone = NULL;

    ASSERT_EQ (12, MSElementDescr::DebugGetExtantCount());
    parentEeh.Invalidate();
    ASSERT_EQ (1, MSElementDescr::DebugGetExtantCount());
    lineHandle.Invalidate();
    ASSERT_EQ (0, MSElementDescr::DebugGetExtantCount());
#endif
    }
    
