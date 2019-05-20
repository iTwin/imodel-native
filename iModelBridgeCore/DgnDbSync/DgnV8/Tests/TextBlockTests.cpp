/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include <DgnPlatform/Annotations/TextAnnotationElement.h>


//----------------------------------------------------------------------------------------
// @bsiclass                                    Ridha.Malik                      06/17
//----------------------------------------------------------------------------------------
struct TextBlockTests : public ConverterTestBaseFixture
{   
    public:
    DgnV8Api::DgnTextStylePtr textstyle;
    void SetUpStyles(V8FileEditor& v8editor);
    void CreateTextBlock(DgnV8Api::TextBlockPtr &TxtBlock, V8FileEditor& v8editor);
    void CreateTextElement(DgnV8Api::EditElementHandle &eeh, DgnV8Api::TextBlockPtr &TxtBlock, V8FileEditor& v8editor);
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TextBlockTests::SetUpStyles(V8FileEditor& v8editor)
    {
    textstyle = DgnV8Api::DgnTextStyle::Create(L"MyTextStyle", *v8editor.m_defaultModel->GetDgnFileP());
    ASSERT_EQ(true, textstyle.IsValid());
    textstyle->SetProperty(DgnV8Api::TextStyle_Height, 10000.0);
    textstyle->SetProperty(DgnV8Api::TextStyle_Width, 10000.0);
    ASSERT_TRUE(L"MyTextStyle" == textstyle->GetName());
    ASSERT_TRUE(SUCCESS == textstyle->Add());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TextBlockTests::CreateTextBlock(DgnV8Api::TextBlockPtr &TxtBlock , V8FileEditor& v8editor) {
    //Create an empty text block
    TxtBlock = DgnV8Api::TextBlock::Create(*textstyle, *v8editor.m_defaultModel);
    ASSERT_EQ(true, TxtBlock.IsValid()) << "TextBlock::Create method failed.";
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TextBlockTests::CreateTextElement(DgnV8Api::EditElementHandle &eeh, DgnV8Api::TextBlockPtr &TxtBlock, V8FileEditor& v8editor)
{
    //Create TextBlock element and it to the model.
    EXPECT_EQ(DgnV8Api::TextBlockToElementResult::TEXTBLOCK_TO_ELEMENT_RESULT_Success, DgnV8Api::TextHandlerBase::CreateElement(eeh, NULL, *TxtBlock));
    EXPECT_EQ(true, eeh.IsValid());

    ASSERT_EQ(SUCCESS, eeh.AddToModel()) << "EditElementHandle::AddToModel method failed.";
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TextBlockTests, TextBlock)
    {
    LineUpFiles(L"TextBlock.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    m_wantCleanUp = false;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);

        SetUpStyles(v8editor);
        //Create an empty text block
        DgnV8Api::TextBlockPtr TxtBlock;
        CreateTextBlock(TxtBlock, v8editor);
        //Set some properties for the text block.

        DPoint3d origin = DPoint3d::From(100, 200, 300);
        TxtBlock->SetUserOrigin(origin);
        TxtBlock->AppendText(L"First.");
        EXPECT_STREQ(L"First.", TxtBlock->ToString().c_str());
        TxtBlock->AppendText(L"Second.");
        EXPECT_STREQ(L"First.Second.", TxtBlock->ToString().c_str());
        TxtBlock->AppendText(L"Third.");
        EXPECT_STREQ(L"First.Second.Third.", TxtBlock->ToString().c_str());
        TxtBlock->AppendText(L"Fourth.");
        EXPECT_STREQ(L"First.Second.Third.Fourth.", TxtBlock->ToString().c_str());
        TxtBlock->AppendParagraphBreak();
        EXPECT_STREQ(L"First.Second.Third.Fourth.\r", TxtBlock->ToString().c_str());

        DgnV8Api::EditElementHandle eeh;
        CreateTextElement(eeh, TxtBlock, v8editor);
        v8editor.Save();
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        auto physical = db->Elements().Get<PhysicalPartition>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_PhysicalPartition)));
        ASSERT_TRUE(physical.IsValid());
        auto physicalModel = physical->GetSub<PhysicalModel>();
        ASSERT_TRUE(physicalModel.IsValid());
        BentleyApi::RefCountedCPtr<TextAnnotation3d> txtan3d=db->Elements().Get<TextAnnotation3d>(findFirstElementByClass(*db, getBisClassId(*db, "TextAnnotation3d")));
        ASSERT_EQ(physicalModel->GetModelId(), txtan3d->GetModelId());
        TextAnnotationCP txtan = txtan3d->GetAnnotation();
        ASSERT_TRUE(txtan != nullptr);
        AnnotationTextBlockCP anTxtb=txtan->GetTextCP();
        AnnotationTextStyleId anTxtstyleID=anTxtb->GetStyleId();
        ASSERT_TRUE(anTxtstyleID.IsValid());
        AnnotationTextStyleCPtr anTxtstyle=AnnotationTextStyle::Get(*db, "MyTextStyle");
        ASSERT_TRUE(anTxtstyle.IsValid());
        ASSERT_DOUBLE_EQ(0.01 , anTxtstyle->GetHeight()); 
        ASSERT_TRUE(1 == anTxtb->GetParagraphs().size());
        Utf8String textStr = anTxtb->ToString().c_str();
        static const Utf8String teststring("First.Second.Third.Fourth.");
        EXPECT_TRUE(0 == strcmp(teststring.c_str(), textStr.c_str()));
        }
    }