/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/Tag_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

#define _QUOTEME(x) L#x
#define TMPNAME(x) _QUOTEME(x)
#ifdef DGN_IMPORTER_REORG_WIP

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                   08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct TagTest : public DgnDbTestFixture
{
public:
    static WChar* tagStyleName;
    static WChar* tagSetName;
    static WChar* tagName;
    static char*    tagValue;
    TagTest ()
        {}
    
    virtual void    SetUp () override;
    DgnTextStylePtr GetTextStyle () const {return DgnTextStyle::GetByName (tagStyleName, GetDgnDb());}
    BentleyStatus   GetTagDef (DgnTagDefinitionR tagDef);
    BentleyStatus   CreateTagWithTextStyle (EditElementHandleR element, DgnTextStyleCR style);
    BentleyStatus   GetOrigin (DPoint3dR origin, ElementHandleCR tagElement);
};

WChar* TagTest::tagStyleName = L"TagStyle";
WChar* TagTest::tagSetName = L"TagSet";
WChar* TagTest::tagName = L"TagName";
char*    TagTest::tagValue = "DgnTagValue";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                   08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            TagTest::SetUp () 
    {
    SetupSeedProject();
    DgnTextStylePtr tagStyle = DgnTextStyle::Create (tagStyleName, GetDgnDb());
    tagStyle->SetProperty(DgnTextStyleProperty::Height, 10000.0);
    tagStyle->SetProperty(DgnTextStyleProperty::Width, 10000.0);

    BentleyStatus status = SUCCESS;
    status = tagStyle->Add();
    
    DgnTagDefinition tagDef;
    memset (&tagDef, 0, sizeof(tagDef));
    wcsncpy (tagDef.name, tagName, TAG_NAME_MAX);
    wcsncpy (tagDef.styleName, tagStyleName, TAG_NAME_MAX);
    tagDef.value.type = MS_TAGTYPE_CHAR;
    tagDef.value.val.stringVal = new char [20];
    strcpy(tagDef.value.val.stringVal, tagValue);
    EditElementHandle tagElment;
    if (SUCCESS == (status = TagSetHandler::Create (tagElment, &tagDef, 1, tagSetName, NULL, true, *GetDgnDb())))
        {
        tagElment.SetDgnModel (GetDgnDb().GetDictionaryModel());
        status = (BentleyStatus) tagElment.AddToModel ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                   08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TagTest::GetTagDef (DgnTagDefinitionR tagDef)
    {
    BentleyStatus status = SUCCESS;
    EditElementHandle tagElement;
    if (SUCCESS != (status = TagSetHandler::GetByName (tagElement, tagSetName, GetDgnDb())))
        return status;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                   08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_DGNCLIENTFX_TAGS
TEST_F (TagTest, CreateTag)
    {
    DgnTextStylePtr textStyle = GetTextStyle ();
    ASSERT_TRUE (textStyle.IsValid());
    
    DPoint3d loc;
    loc.Init(0, 0, 0);

    DgnTagDefinition tagDef;
    ASSERT_TRUE (SUCCESS == GetTagDef(tagDef)); //This tests tagset creation

    ITagCreateDataPtr tagData = ITagCreateData::Create (tagName, tagSetName, *textStyle, GetDgnDb());
    ASSERT_TRUE (tagData.IsValid());
    
    EditElementHandle lineElement;
    DSegment3d  segment;
    segment.Init (0.0, 0.0, 0.0, 10000.0, 0, 0);
    ASSERT_EQ (SUCCESS, LineHandler::CreateLineElement (lineElement, NULL, segment, Is3d(), *GetDgnModelP()));

    lineElement.AddToModel ();

    RotMatrix rotation;
    EditElementHandle tagElement;
    ASSERT_TRUE (SUCCESS == TagElementHandler::Create (tagElement, NULL, *tagData, *GetDgnModelP(), GetDgnModelP()->Is3d(), loc, rotation, lineElement.GetDgnElement()));

    tagElement.AddToModel ();

    //Create a non associated tag
    EditElementHandle tagElement2;
    loc.Init(1000, 0, 0);
    ASSERT_TRUE (SUCCESS == TagElementHandler::Create (tagElement2, NULL, *tagData, *GetDgnModelP(), GetDgnModelP()->Is3d(), loc, rotation, NULL));
    tagElement2.AddToModel ();

    DgnTagValue newVal;
    newVal.type = MS_TAGTYPE_DOUBLE;
    newVal.val.doubleVal = 2.0;
    newVal.size = sizeof(newVal.val.doubleVal);
    ASSERT_TRUE (SUCCESS == TagElementHandler::SetAttributeValue (tagElement, newVal));
    tagElement.AddToModel ();

    DgnTagValue storedValue;
    ASSERT_TRUE (SUCCESS == TagElementHandler::GetAttributeValue (tagElement, storedValue));
    EXPECT_EQ (storedValue.type, newVal.type);
    EXPECT_EQ (storedValue.val.doubleVal, newVal.val.doubleVal);

//  GetDgnDb()->ProcessChanges(DgnSaveReason::ApplInitiated);
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_DGNCLIENTFX_TAGS
TEST_F (TagTest, CreateEmptyTag)
    {
    DgnTextStylePtr textStyle = GetTextStyle ();
    ASSERT_TRUE (textStyle.IsValid());
    
    DPoint3d loc;
    loc.Init(0, 0, 0);

    DgnTagDefinition tagDef;
    ASSERT_TRUE (SUCCESS == GetTagDef(tagDef)); //This tests tagset creation

    ITagCreateDataPtr tagData = ITagCreateData::Create (tagName, tagSetName, *textStyle, GetDgnDb());
    ASSERT_TRUE (tagData.IsValid());
    
    EditElementHandle lineElement;
    DSegment3d  segment;
    segment.Init (0.0, 0.0, 0.0, 10000.0, 0, 0);
    ASSERT_EQ (SUCCESS, LineHandler::CreateLineElement (lineElement, NULL, segment, Is3d(), *GetDgnModelP()));

    lineElement.AddToModel ();

    RotMatrix rotation;
    EditElementHandle tagElement;
    ASSERT_TRUE (SUCCESS == TagElementHandler::Create (tagElement, NULL, *tagData, *GetDgnModelP(), GetDgnModelP()->Is3d(), loc, rotation, lineElement.GetDgnElement()));

    tagElement.AddToModel ();

    DgnElementP oldElem = tagElement.GetDgnElement();
    DgnTagValue newVal;
    newVal.type = MS_TAGTYPE_DOUBLE;
    newVal.val.doubleVal = 2.0;
    newVal.size = sizeof(newVal.val.doubleVal);
    ASSERT_TRUE (SUCCESS == TagElementHandler::SetAttributeValue (tagElement, newVal));
    tagElement.ReplaceInModel (oldElem);

    oldElem = tagElement.GetDgnElement();

    DgnTagValue storedValue;
    ASSERT_TRUE (SUCCESS == TagElementHandler::GetAttributeValue (tagElement, storedValue));
    EXPECT_EQ (storedValue.type, newVal.type);
    EXPECT_EQ (storedValue.val.doubleVal, newVal.val.doubleVal);

    newVal.type = MS_TAGTYPE_CHAR;
    newVal.val.stringVal = new char(0);
    newVal.size = 0;
    ASSERT_TRUE (SUCCESS == TagElementHandler::SetAttributeValue (tagElement, newVal));
    tagElement.ReplaceInModel (oldElem);

    ASSERT_TRUE (SUCCESS == TagElementHandler::GetAttributeValue (tagElement, storedValue));
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TagTest::CreateTagWithTextStyle (EditElementHandleR tagElement, DgnTextStyleCR style)
    {
    DPoint3d loc;
    loc.Init(0, 0, 0);

    DgnTagDefinition tagDef;
    if (SUCCESS != GetTagDef(tagDef))
        return ERROR;

    ITagCreateDataPtr tagData = ITagCreateData::Create (tagName, tagSetName, style, GetDgnDb());
    if (tagData.IsNull())
        return ERROR;
    
    return TagElementHandler::Create (tagElement, NULL, *tagData, *GetDgnModelP(), GetDgnModelP()->Is3d(), loc, RotMatrix::FromIdentity(), NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TagTest::GetOrigin (DPoint3dR origin, ElementHandleCR tagElement)
    {
    ITextEdit* textEdit = dynamic_cast<ITextEdit*> (&tagElement.GetHandler());
    if (NULL == textEdit)
        return ERROR;

    T_ITextPartIdPtrVector textPart;
    textEdit->GetTextPartIds(tagElement, *ITextQueryOptions::CreateDefault (), textPart);

    if (textPart.empty())
        return ERROR;

    TextBlockPtr text = textEdit->GetTextPart(tagElement, **textPart.begin());
    if (text.IsNull())
        return ERROR;
    
    origin = text->GetUserOrigin();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_DGNCLIENTFX_TAGS
TEST_F (TagTest, MultiJustifiedTag)
    {
    DgnTextStylePtr textStyle = GetTextStyle ();
    ASSERT_TRUE (textStyle.IsValid());
    
    for (uint32_t index = 0; index < 15; ++index)
        {
        switch (index)
            {
            case TextElementJustification::LeftCap:
            case TextElementJustification::LeftMarginCap:
            case TextElementJustification::CenterCap:  
            case TextElementJustification::RightCap:      
            case TextElementJustification::RightMarginCap:
                continue;
            }
            
        ASSERT_TRUE (SUCCESS == textStyle->SetProperty(DgnTextStyleProperty::Justification, index));

        EditElementHandle tagElement;
        ASSERT_TRUE (SUCCESS == CreateTagWithTextStyle (tagElement, *textStyle));

        DPoint3d origin;
        ASSERT_TRUE (SUCCESS == GetOrigin(origin, tagElement));

        ASSERT_TRUE (fabs(origin.x) < mgds_fc_epsilon);
        ASSERT_TRUE (fabs(origin.y) < mgds_fc_epsilon);
        }

    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                   11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TagTest, FindTagByName)
    {
    DgnTagDefinition tagDef;
    memset (&tagDef, 0, sizeof(tagDef));
    wcsncpy (tagDef.name, L"NewTag", TAG_NAME_MAX);
    wcsncpy (tagDef.styleName, L"NewTagStyle", TAG_NAME_MAX);
    tagDef.value.type = MS_TAGTYPE_CHAR;
    tagDef.value.val.stringVal = new char [20];
    strcpy(tagDef.value.val.stringVal, "NewTagValue");
    
    DgnTagDefinition tagDef2(tagDef);
    wcsncpy (tagDef2.name, L"NewTag2", TAG_NAME_MAX);
    
    DgnTagDefinition defs[2];
    defs[0] = tagDef;
    defs[1] = tagDef2;
    EditElementHandle tagElement;
    StatusInt status;
    if (SUCCESS == (status = TagSetHandler::Create (tagElement, defs, 2, L"NewTagSet", NULL, true, GetDgnDb())))
        {
        tagElement.SetDgnModel (GetDgnDb().GetDictionaryModel());
        status = (BentleyStatus) tagElement.AddToModel ();
        }
    
    EditElementHandle foundElement;
    if (SUCCESS != (status = TagSetHandler::GetByName (foundElement, L"NewTagSet", GetDgnDb())))
        return;
    
    DgnTagDefinition newtag;
    ASSERT_TRUE (SUCCESS == TagSetHandler::ExtractTagDefByName (foundElement, newtag,  L"NewTag2"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (TagAcadTest, AcadTextTest)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"bug.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnModelP dgnCache = tdm.GetAndFillDgnModelP();

    EditElementHandle tagElement;
    tagElement.FindByID (980, dgnCache);

    ASSERT_TRUE (tagElement.IsValid());
    ITextEdit* textEdit = dynamic_cast<ITextEdit*> (&tagElement.GetHandler());

    ASSERT_TRUE (NULL != textEdit);

    T_ITextPartIdPtrVector textPart;
    textEdit->GetTextPartIds(tagElement, *ITextQueryOptions::CreateDefault (), textPart);

    ASSERT_TRUE (!textPart.empty());

    TextBlockPtr text = textEdit->GetTextPart(tagElement, **textPart.begin());
    ASSERT_TRUE (text.IsValid());

    DPoint3d origin = text->GetUserOrigin();
    ASSERT_TRUE (fabs(origin.x - 6167154.0270009805) < mgds_fc_epsilon);
    ASSERT_TRUE (fabs(origin.y - 382184.49056941038) < mgds_fc_epsilon);

    DgnElementP oldElemRef = tagElement.GetDgnElement();
    CaretPtr start = text->CreateStartCaret();
    ASSERT_TRUE (start.IsValid());

    CharStreamCP charStream = dynamic_cast<CharStreamCP> (start->GetCurrentRunCP ());
    ASSERT_TRUE (NULL != charStream);

    RunPropertiesPtr        runPropsPtr = charStream->GetProperties().Clone();
    ParagraphCP             paragraph   = start->GetCurrentParagraphCP();
    ParagraphPropertiesCR   paraProps   = (NULL != paragraph) ? paragraph->GetProperties() : text->GetParagraphPropertiesForAdd ();
    TextBlockPropertiesPtr  tbProps     = text->GetProperties().Clone();
    
    runPropsPtr->SetIsBold(true);
    runPropsPtr->SetIsUnderlined(true);
    DPoint2d fontSize = {1000000, 1000000};
    runPropsPtr->SetFontSize(fontSize);
    tbProps->SetIsBackwards(true);
    tbProps->SetIsUpsideDown(true);

    TextBlockPtr text2 = TextBlock::Create(*tbProps, paraProps, *runPropsPtr, *dgnCache);
    text2->AppendText(charStream->GetString().c_str());
    ASSERT_TRUE(ITextEdit::ReplaceStatus_Success == textEdit->ReplaceTextPart (tagElement, **textPart.begin(), *text2));
    tagElement.ReplaceInModel(oldElemRef);
    oldElemRef = tagElement.GetDgnElement();

    TextBlockPtr assingedText = textEdit->GetTextPart(tagElement, **textPart.begin());
    start = assingedText->CreateStartCaret();
    ASSERT_TRUE (start.IsValid());
    
    charStream = dynamic_cast<CharStreamCP> (start->GetCurrentRunCP ());
    ASSERT_TRUE (NULL != charStream);

    RunPropertiesCR newRunProps = charStream->GetProperties();
    EXPECT_FALSE (newRunProps.IsBold()); //Should not work for RSC fonts
    EXPECT_TRUE (newRunProps.IsUnderlined());

    EXPECT_EQ(newRunProps.GetFontSize().x, 1000000);
    EXPECT_EQ(newRunProps.GetFontSize().y, 1000000);

    TextBlockPropertiesCR newTbProps = assingedText->GetProperties();
    EXPECT_EQ(newTbProps.IsBackwards(), true);
    EXPECT_EQ(newTbProps.IsUpsideDown(), true);

    DgnTagValue newVal;
    newVal.type = MS_TAGTYPE_DOUBLE;
    newVal.val.doubleVal = 2.0;
    newVal.size = sizeof(newVal.val.doubleVal);
    ASSERT_TRUE (SUCCESS == TagElementHandler::SetAttributeValue (tagElement, newVal));

    ASSERT_TRUE(ITextEdit::ReplaceStatus_Error == textEdit->ReplaceTextPart (tagElement, **textPart.begin(), *text2));
    
    tagElement.ReplaceInModel(oldElemRef);
//    dgnCache->GetDgnDb()->ProcessChanges(DgnSaveReason::ApplInitiated);
    }
#endif
