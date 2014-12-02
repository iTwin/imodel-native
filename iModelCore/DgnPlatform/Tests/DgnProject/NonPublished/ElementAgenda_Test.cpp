/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementAgenda_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
class ElementAgendaTest : public GenericDgnModelTestFixture
{
protected:
EditElementHandle      m_eeh;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAgendaTest()
    : GenericDgnModelTestFixture (__FILE__, true)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~ElementAgendaTest ()
    {
    m_eeh.Invalidate();
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAgendaTest, GetSource)
    {
    ElementAgenda agenda;
    ModifyElementSource source = ModifyElementSource::Selected;
    ModifyElementSource actual;

    agenda.SetSource(source);
    actual  = agenda.GetSource();

    ASSERT_TRUE(source == actual);
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAgendaTest, InsertElemRef)
    {
    ElementAgenda agenda;
    CreateElement(DPT_CreateElement::LineString, m_eeh, *GetDgnModelP(), Is3d());

    ASSERT_EQ(SUCCESS, m_eeh.AddToModel (/**GetDgnModelP()*/));

    ElementRefP elm  = m_eeh.GetElementRef ();
    agenda.Insert (elm);

    EditElementHandleP curr  = agenda.GetFirstP ();
    DgnElementCP actual  = curr->GetElementCP ();

    EditElementHandle line;
    CreateElement(DPT_CreateElement::LineString, line, *GetDgnModelP(), Is3d());
    DgnElementCP expected  = line.GetElementCP ();
    for(size_t i  = 0; i < 5; i++)
        ASSERT_TRUE(isDPoint3dNear(expected->ToLine_String_3d().vertice[i], actual->ToLine_String_3d().vertice[i], EPSILON));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAgendaTest, InsertElemRef_NullRef)
    {
    ElementAgenda agenda;

    ElementRefP elm  = NULL;
    EditElementHandleP actual;
    actual  = agenda.Insert (elm);
    ASSERT_EQ(NULL, actual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAgendaTest, Find)
    {
    ElementAgenda agenda;
    CreateElement(DPT_CreateElement::LineString, m_eeh, *GetDgnModelP(), Is3d());

    ASSERT_EQ(SUCCESS, m_eeh.AddToModel (/**GetDgnModelP()*/));

    ElementRefP elm  = m_eeh.GetElementRef ();
    agenda.Insert (elm);

    EditElementHandleCP res  = agenda.Find(elm, 0 , 1);
    DgnElementCP actual  = res->GetElementCP ();

    EditElementHandle line;
    CreateElement(DPT_CreateElement::LineString, line, *GetDgnModelP(), Is3d());
    DgnElementCP expected  = line.GetElementCP ();

    for(size_t i  = 0; i < 5; i++)
        ASSERT_TRUE(isDPoint3dNear(expected->ToLine_String_3d().vertice[i], actual->ToLine_String_3d().vertice[i], EPSILON));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAgendaTest, Find_InvalidIndex)
    {
    ElementAgenda agenda;
    CreateElement(DPT_CreateElement::LineString, m_eeh, *GetDgnModelP(), Is3d());

    ASSERT_EQ(SUCCESS, m_eeh.AddToModel (/**GetDgnModelP()*/));

    ElementRefP elm  = m_eeh.GetElementRef ();
    agenda.Insert (elm);

    ASSERT_EQ(NULL, agenda.Find(elm, 1 , 0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAgendaTest, Find_WrongElement)
    {
    ElementAgenda agenda;
    CreateElement(DPT_CreateElement::LineString, m_eeh, *GetDgnModelP(), Is3d());

    ASSERT_EQ(SUCCESS, m_eeh.AddToModel (/**GetDgnModelP()*/));

    ElementRefP elm  = m_eeh.GetElementRef ();
    agenda.Insert (elm);

    EditElementHandle line;
    CreateElement(DPT_CreateElement::Line, line, *GetDgnModelP(), Is3d());
    ASSERT_EQ(SUCCESS, line.AddToModel (/**GetDgnModelP()*/));

    ElementRefP wrong  = line.GetElementRef ();
    ASSERT_EQ(NULL, agenda.Find(wrong, 0, 1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAgendaTest, Find_EmptyAgenda)
    {
    ElementAgenda agenda;

    EditElementHandle line;
    CreateElement(DPT_CreateElement::Line, line, *GetDgnModelP(), Is3d());
    ASSERT_EQ(SUCCESS, line.AddToModel (/**GetDgnModelP()*/));

    ElementRefP wrong  = line.GetElementRef ();
    ASSERT_EQ(NULL, agenda.Find(wrong, 0, 1));
    }
#endif
