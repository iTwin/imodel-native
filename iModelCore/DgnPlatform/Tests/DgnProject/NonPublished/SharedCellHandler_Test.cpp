#include "DgnHandlersTests.h"

#if defined (NEEDS_WORK_DGNITEM)
USING_NAMESPACE_BENTLEY_DGNPLATFORM

size_t const NUM_LINES = 3;
size_t const NUM_POINTS = 2;

WCharCP SHARED_SHELL_DEFINITION_NAME = L"test";

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     KyleDeeds       06/09
+---------------+---------------+---------------+---------------+---------------+------*/
class SharedCellHandlerTest : public GenericDgnModelTestFixture
{
private:
    typedef GenericDgnModelTestFixture T_Super;
protected:
    RotMatrix       m_rMatrix;
    EditElementHandle  m_eehLines[NUM_LINES];
    EditElementHandle  m_eeh;
    EditElementHandle  m_eehDef;

    DPoint3d            m_points[NUM_LINES][NUM_POINTS];

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
protected: virtual void SetUp () override
    {
    T_Super::SetUp ();

    ITxnManagerR txnMgr = GetDgnModelP()->GetDgnProject().GetTxnManager();
    txnMgr.Activate ();

    m_rMatrix.InitIdentity ();
    SharedCellHandlerTest::CreateDef (m_eehDef, SHARED_SHELL_DEFINITION_NAME, m_eehLines);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       06/09
+---------------+---------------+---------------+---------------+---------------+------*/
public: SharedCellHandlerTest ()
    : GenericDgnModelTestFixture (__FILE__, true)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       06/09
+---------------+---------------+---------------+---------------+---------------+------*/
public: virtual ~SharedCellHandlerTest ()
    {
    m_eehDef.Invalidate ();
    m_eeh.Invalidate ();
    for (size_t i = 0; i < NUM_LINES; i++)
        {
        m_eehLines[i].Invalidate ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       06/09
+---------------+---------------+---------------+---------------+---------------+------*/
public: void SharedCellSetUp (WCharCP definitionName = SHARED_SHELL_DEFINITION_NAME);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       06/09
+---------------+---------------+---------------+---------------+---------------+------*/
public: void CreateDef (EditElementHandleR, WCharCP, EditElementHandle*);
};


*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandlerTest::SharedCellSetUp (WCharCP definitionName)
    {
    DPoint3d origin[] = {0.0, 0.0, 0.0};
    DPoint3d scale[] = {1.0, 1.0, 1.0};

    SharedCellHandler::CreateSharedCellElement (m_eeh, NULL, definitionName, origin, &m_rMatrix, scale, Is3d(), *GetDgnModelP());
    SharedCellHandler::CreateSharedCellComplete (m_eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandlerTest::CreateDef (EditElementHandleR defHandle, WCharCP name, EditElementHandle* Lines)
    {
    for (size_t i = 0; i < NUM_LINES; i++)
        {
        GeneratePoints (m_points[i], NUM_POINTS);
        for (size_t j = 0; j < NUM_POINTS; j++)
            {
            m_points[i][j].x *= i+7;
            m_points[i][j].y *= i+5;
            m_points[i][j].z *= i+6;
            }
        }

    SharedCellDefHandler::CreateSharedCellDefElement (defHandle, SHARED_SHELL_DEFINITION_NAME, Is3d(), *GetDgnModelP());

    for (size_t j = 0; j < NUM_LINES; j++)
        {
        LineHandler::CreateLineElement (Lines[j], NULL, (DSegment3dCR) (*m_points[j]), Is3d(), *GetDgnModelP());
        ASSERT_EQ (SUCCESS, SharedCellDefHandler::AddChildElement (defHandle, Lines[j]));
        }

    SharedCellDefHandler::AddChildComplete (defHandle);
    defHandle.AddToModel (/*GetDgnModelP()*/);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerTest, CreateSharedCellElement)
    {
    SharedCellHandlerTest::SharedCellSetUp ();
    size_t numChildren = 0;

    for (ChildElemIter child (m_eehDef); child.IsValid (); child = child.ToNext ())
        {
        DgnElementCP elm = child.GetElementCP ();
        EXPECT_TRUE (isDPoint3dNear (m_points[numChildren][0], elm->ToLine_3d().start, EPSILON));
        EXPECT_TRUE (isDPoint3dNear (m_points[numChildren][1], elm->ToLine_3d().end, EPSILON));
        numChildren++;
        }

    ASSERT_EQ (NUM_LINES, numChildren);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerTest, SharedCellDef_FindDefinitionByName_Valid)
    {
    DgnProjectP def = m_eehDef.GetDgnProject ();
    ASSERT_EQ (m_eehDef.GetElementRef (), SharedCellDefHandler::FindDefinitionByName (L"test", *def).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerTest, SharedCellDef_FindDefinitionByName_InValid)
    {
    DgnProjectP def = m_eehDef.GetDgnProject ();
    ASSERT_TRUE (!SharedCellDefHandler::FindDefinitionByName (L"fail", *def).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* This test demostrates what happens when two definitions with the same name are created.
*   The first one is overriden. This is not the correct way to use CreateSharedCellDef just
*   an example show what happens.  Normally the caller of the function needs to make sure
*   they are not inserting multiple definitions with the same name.
* @bsimethod                                                     KyleDeeds      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerTest, SharedCellDef_NonUniqueDefs)
    {
    DPoint3d points[NUM_LINES][NUM_POINTS];
    for (size_t i = 0; i < NUM_LINES; i++)
        {
        GeneratePoints (points[i], NUM_POINTS);
        for (size_t j = 0; j < NUM_POINTS; j++)
            {
            points[i][j].x *= i+200;
            points[i][j].y *= i+125;
            points[i][j].z *= i+654;
            }
        }

    SharedCellDefHandler::CreateSharedCellDefElement (m_eehDef, L"test", Is3d(), *GetDgnModelP());

    for (size_t j = 0; j < NUM_LINES; j++)
        {
        LineHandler::CreateLineElement (m_eehLines[j], NULL, (DSegment3dCR) (*points[j]), Is3d(), *GetDgnModelP());
        ASSERT_EQ (SUCCESS, SharedCellDefHandler::AddChildElement (m_eehDef, m_eehLines[j]));
        }

    SharedCellDefHandler::AddChildComplete (m_eehDef);
    
    // This would normally trigger an assertion failure.
    BeTest::SetFailOnAssert (false);
    bool addFailed = SUCCESS != m_eehDef.AddToModel (/*GetDgnModelP()*/);
    BeTest::SetFailOnAssert (true);

    ASSERT_TRUE( addFailed );
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    KyleDeeds      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
struct SharedCellHandlerInterfaces : public GenericDgnModelTestFixture
{
protected:
EditElementHandle      m_eeh;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
public: SharedCellHandlerInterfaces() 
    : GenericDgnModelTestFixture (__FILE__, true)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
public: virtual ~SharedCellHandlerInterfaces () 
    {
    m_eeh.Invalidate();
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerInterfaces, DisplayHandler_IsRenderable_SharedCell)
    {
    CreateElement(DPT_CreateElement::nSharedCell, m_eeh, *GetDgnModelP(), Is3d());
    DisplayHandlerP disp = dynamic_cast <DisplayHandlerP> (&m_eeh.GetHandler ());
    ASSERT_TRUE (NULL != disp);
    ASSERT_FALSE (disp->IsRenderable(m_eeh));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerInterfaces, DisplayHandler_GetTransformOrigin_SharedCell)
    {
    CreateElement(DPT_CreateElement::nSharedCell, m_eeh, *GetDgnModelP(), Is3d());

    DisplayHandlerP disp = dynamic_cast <DisplayHandlerP> (&m_eeh.GetHandler ());
    ASSERT_TRUE (NULL != disp);

    DPoint3d act;
    DPoint3d exp = {0, 0, 0};

    disp->GetTransformOrigin(m_eeh, act);
    ASSERT_TRUE(isDPoint3dNear (act, exp, EPSILON));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerInterfaces, DisplayHandler_GetOrientation_SharedCell)
    {
    CreateElement(DPT_CreateElement::nSharedCell, m_eeh, *GetDgnModelP(), Is3d());
    DisplayHandlerP disp = dynamic_cast <DisplayHandlerP> (&m_eeh.GetHandler ());
    size_t const ROWS = 3;
    size_t const COLS = 3;
    RotMatrix act;
    RotMatrix exp;
    exp.InitIdentity ();

    ASSERT_TRUE (NULL != disp);
    disp->GetOrientation (m_eeh, act);
    for(size_t i = 0; i<ROWS; i++)
        {
        for(size_t j = 0; j<COLS; j++)
            ASSERT_EQ(act.form3d[i][j], exp.form3d[i][j]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerInterfaces, DisplayHandler_IsPlanar_SharedCell)
    {
    CreateElement(DPT_CreateElement::nSharedCell, m_eeh, *GetDgnModelP(), Is3d());
    DisplayHandlerP disp = dynamic_cast <DisplayHandlerP> (&m_eeh.GetHandler ());
    ASSERT_TRUE (NULL != disp);

    DPoint3d point;
    DVec3d normal;
    DVec3d def = DVec3d::From (0.0, 0.0, 1.0);

    ASSERT_FALSE(disp->IsPlanar(m_eeh, &normal, &point, &def));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerInterfaces, DisplayHandler_GetSnapOrigin_SharedCell)
    {
    CreateElement(DPT_CreateElement::nSharedCell, m_eeh, *GetDgnModelP(), Is3d());
    DisplayHandlerP disp = dynamic_cast <DisplayHandlerP> (&m_eeh.GetHandler ());
    ASSERT_TRUE (NULL != disp);

    DPoint3d point;
    DPoint3d expPoint = {0, 0, 0};

    disp->GetSnapOrigin(m_eeh, point);
    ASSERT_TRUE (isDPoint3dNear(expPoint, point, EPSILON));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SharedCellHandlerInterfaces, DisplayHandler_GetElemDisplayParams_SharedCell)
    {
    CreateElement(DPT_CreateElement::nSharedCell, m_eeh, *GetDgnModelP(), Is3d());

    ElementPropertiesGetterPtr propQuery = ElementPropertiesGetter::Create (m_eeh);

    ASSERT_EQ (0, propQuery->GetColor ());
    ASSERT_EQ (0, propQuery->GetLineStyle ());
    ASSERT_EQ (0, propQuery->GetWeight ());
    ASSERT_TRUE (LevelId(LEVEL_DEFAULT_LEVEL_ID) == propQuery->GetLevel ());
    }

// ***
// *** WIP_DGNDB    Need tests for:
// ***                  import shared cell instance as component of normal cell
// ***                  import anonymous shared cell instance as component of normal cell
// ***                  import 2 shared cell instances, verifying that only one sc def is (deep-)copied into project
// ***                  import 2 instances of same named shared cell from different files, verifying that only one sc def is (deep-)copied into project
// ***                  import 2 instances of same anonymous shared cell from same and different files, verifying that only one sc def is (deep-)copied into project
// ***                  import 2 instances of different named shared cells from same and different files, verifying that two sc defs are (deep-)copied into project
// ***                  import 2 instances of differing anonymous shared cells from same and different files, verifying that two sc defs are (deep-)copied into project


#endif
