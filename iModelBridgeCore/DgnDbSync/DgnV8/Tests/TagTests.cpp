/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct TagTests : public ConverterTestBaseFixture
{
    DgnV8Api::DgnTextStylePtr m_tagStyle;
    BentleyStatus GetTagDef(DgnTagDefinitionR tagDef, V8FileEditor& v8editor, WCharCP tagSetName);
    void CreateTagDef(V8FileEditor& v8editor, WCharCP tagsetName);
    void CreateTextStyle(V8FileEditor& v8editor);
    BentleyStatus AddTagElement(V8FileEditor& v8editor, DgnV8Api::ITagCreateDataPtr& tagData, ElementRefP targetElem);
    TagTests()
    {
        m_tagStyle = nullptr;
    }
};

#define STYLENAME   L"TagStyle"
#define TAGSETNAME  L"TagSet"
#define TAGNAME     L"Tag1"
#define TAGNAME2    L"Tag 2"
#define TAGVALUE    L"DgnTagValue 1"
#define TAGVALUE2   L"DgnTagValue 2"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TagTests::CreateTextStyle(V8FileEditor& v8editor)
    {
    m_tagStyle = DgnV8Api::DgnTextStyle::Create(STYLENAME, *v8editor.m_file);
    m_tagStyle->SetProperty(DgnV8Api::TextStyle_Height, 10000.0);
    m_tagStyle->SetProperty(DgnV8Api::TextStyle_Width, 10000.0);

    BentleyStatus status = SUCCESS;
    status = m_tagStyle->Add();
    
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
PUSH_STATIC_ANALYSIS_WARNING(6385) // I can't figure out how to silence these static analysis warning(s) in this function, so just ignoring.
void TagTests::CreateTagDef(V8FileEditor& v8editor, WCharCP tagSetName)
    {
    BentleyStatus status = SUCCESS;
    DgnV8Api::DgnTagDefinition m_tagDef[2];
    memset (&m_tagDef[0], 0, sizeof(m_tagDef[0]));
    wcsncpy(m_tagDef[0].name, TAGNAME, TAG_NAME_MAX);
    wcsncpy(m_tagDef[0].styleName, STYLENAME, TAG_NAME_MAX);
    m_tagDef[0].value = DgnV8Api::DgnTagValue(TAGVALUE);

    memset(&m_tagDef[1], 0, sizeof(m_tagDef[1]));
    wcsncpy(m_tagDef[1].name, TAGNAME2, TAG_NAME_MAX);
    wcsncpy(m_tagDef[1].styleName, STYLENAME, TAG_NAME_MAX);
    m_tagDef[1].value = DgnV8Api::DgnTagValue(TAGVALUE2);
    
    DgnV8Api::EditElementHandle tagElment;
    if (SUCCESS == (status = DgnV8Api::TagSetHandler::Create(tagElment, m_tagDef, 2, tagSetName, NULL, true, *v8editor.m_file)))
        {
        tagElment.SetModelRef (&v8editor.m_file->GetDictionaryModel());
        status = (BentleyStatus) tagElment.AddToModel ();
        }
    ASSERT_TRUE(SUCCESS == status);
    }
POP_STATIC_ANALYSIS_WARNING

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TagTests::GetTagDef(DgnTagDefinitionR tagDef, V8FileEditor& v8editor, WCharCP tagSetName)
    {
    BentleyStatus status = SUCCESS;
    DgnV8Api::EditElementHandle tagElement;
    if (SUCCESS != (status = DgnV8Api::TagSetHandler::GetByName(tagElement, tagSetName, *v8editor.m_file)))
        return status;    
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TagTests::AddTagElement(V8FileEditor& v8editor, DgnV8Api::ITagCreateDataPtr& tagData, ElementRefP targetElem)
    {
    DPoint3d loc;
    loc.Init(0, 0, 0);
    RotMatrix rotation = RotMatrix::FromIdentity();
    DgnV8Api::EditElementHandle tagElement;
    if (SUCCESS != DgnV8Api::TagElementHandler::Create(tagElement, NULL, *tagData, *v8editor.m_defaultModel, v8editor.m_defaultModel->Is3d(), loc, rotation, targetElem))
        return ERROR;
    tagElement.AddToModel();
    v8editor.Save();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TagTests, Basic)
    {
    LineUpFiles(L"Basic.bim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::ElementId eid1, eid2;
    v8editor.AddLine(&eid1);
    v8editor.AddLine(&eid2, nullptr, DPoint3d::FromXYZ(100,100,0));
    PersistentElementRefP lineElement1 = v8editor.m_defaultModel->FindElementByID(eid1);
    ASSERT_TRUE(lineElement1 != nullptr);
    PersistentElementRefP lineElement2 = v8editor.m_defaultModel->FindElementByID(eid2);
    ASSERT_TRUE(lineElement2 != nullptr);
    CreateTextStyle(v8editor);
    CreateTagDef(v8editor, TAGSETNAME);
    CreateTagDef(v8editor, L"UnUsed TagSET");

    DgnV8Api::ITagCreateDataPtr tagData = DgnV8Api::ITagCreateData::Create(TAGNAME, TAGSETNAME, *m_tagStyle, *v8editor.m_file);
    ASSERT_TRUE(tagData.IsValid());
    DgnV8Api::ITagCreateDataPtr tagData2 = DgnV8Api::ITagCreateData::Create(TAGNAME2, TAGSETNAME, *m_tagStyle, *v8editor.m_file);
    ASSERT_TRUE(tagData2.IsValid());

    ASSERT_TRUE(SUCCESS == AddTagElement(v8editor, tagData, lineElement1));
    ASSERT_TRUE(SUCCESS == AddTagElement(v8editor, tagData2, lineElement1));
    ASSERT_TRUE(SUCCESS == AddTagElement(v8editor, tagData2, lineElement2));

    DoConvert(m_dgnDbFileName, m_v8FileName);
    m_wantCleanUp = false;

        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        DgnElementCPtr lineElem1 = FindV8ElementInDgnDb(*db, eid1);
        ASSERT_TRUE(lineElem1.IsValid());
        DgnElementCPtr lineElem2 = FindV8ElementInDgnDb(*db, eid2);
        ASSERT_TRUE(lineElem2.IsValid());
        ECSqlStatement stmt;
        ECSqlStatus ecStatus = stmt.Prepare(*db, "SELECT Element.Id,Tag1 FROM v8tag.TagSet");
        ASSERT_TRUE(ECSqlStatus::Success == ecStatus);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        ASSERT_EQ(lineElem1->GetElementId().GetValue(), stmt.GetValueInt(0));
        ASSERT_STREQ("DgnTagValue 1", stmt.GetValueText(1));
        }

    DgnV8Api::ElementId eid3;
        {
        v8editor.AddLine(&eid3);
        PersistentElementRefP lineElement3 = v8editor.m_defaultModel->FindElementByID(eid3);
        ASSERT_TRUE(lineElement3 != nullptr);
        ASSERT_TRUE(SUCCESS == AddTagElement(v8editor, tagData, lineElement3));
        v8editor.Save();
        }

    DoUpdate(m_dgnDbFileName, m_v8FileName);
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        DgnElementCPtr lineElem3 = FindV8ElementInDgnDb(*db, eid3);
        ASSERT_TRUE(lineElem3.IsValid());
        ECSqlStatement stmt;
        ECSqlStatus ecStatus = stmt.Prepare(*db, "SELECT Tag1 FROM v8tag.TagSet WHERE Element.Id = ?");
        ASSERT_TRUE(ECSqlStatus::Success == ecStatus);
        stmt.BindId(1, lineElem3->GetElementId());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        ASSERT_STREQ("DgnTagValue 1", stmt.GetValueText(0));
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TagTests, TagVisibility)
    {
    LineUpFiles(L"Visibility.bim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    PersistentElementRefP lineElement = v8editor.m_defaultModel->FindElementByID(eid);
    ASSERT_TRUE(lineElement != nullptr);
    CreateTextStyle(v8editor);
    CreateTagDef(v8editor, TAGSETNAME);

    DgnV8Api::ITagCreateDataPtr tagData = DgnV8Api::ITagCreateData::Create(TAGNAME, TAGSETNAME, *m_tagStyle, *v8editor.m_file);
    ASSERT_TRUE(tagData.IsValid());
    tagData->SetTagVisibility(true);

    ASSERT_TRUE(SUCCESS == AddTagElement(v8editor, tagData, lineElement));

    DoConvert(m_dgnDbFileName, m_v8FileName);
    m_wantCleanUp = false;

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    DgnElementCPtr lineElem = FindV8ElementInDgnDb(*db, eid);
    ASSERT_TRUE(lineElem.IsValid());
    ECSqlStatement stmt;
    ECSqlStatus ecStatus = stmt.Prepare(*db, "SELECT Element.Id,Tag1 FROM v8tag.TagSet");
    ASSERT_TRUE(ECSqlStatus::Success == ecStatus);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(lineElem->GetElementId().GetValue(), stmt.GetValueInt(0));
    ASSERT_STREQ("DgnTagValue 1", stmt.GetValueText(1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TagTests, Empty)
    {
    LineUpFiles(L"Empty.bim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    PersistentElementRefP lineElement = v8editor.m_defaultModel->FindElementByID(eid);
    ASSERT_TRUE(lineElement != nullptr);
    CreateTextStyle(v8editor);
    CreateTagDef(v8editor, TAGSETNAME);

    DgnV8Api::ITagCreateDataPtr tagData = DgnV8Api::ITagCreateData::Create(TAGNAME, TAGSETNAME, *m_tagStyle, *v8editor.m_file);
    ASSERT_TRUE(tagData.IsValid());
    tagData->SetAttributeValue(DgnV8Api::DgnTagValue(L""));

    ASSERT_TRUE(SUCCESS == AddTagElement(v8editor, tagData, lineElement));
    DoConvert(m_dgnDbFileName, m_v8FileName);
    m_wantCleanUp = false;

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    DgnElementCPtr lineElem = FindV8ElementInDgnDb(*db, eid);
    ASSERT_TRUE(lineElem.IsValid());
    ECSqlStatement stmt;
    ECSqlStatus ecStatus = stmt.Prepare(*db, "SELECT Element.Id,Tag1 FROM v8tag.TagSet");
    ASSERT_TRUE(ECSqlStatus::Success == ecStatus);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(lineElem->GetElementId().GetValue(), stmt.GetValueInt(0));
    ASSERT_STREQ("", stmt.GetValueText(1));
    }

