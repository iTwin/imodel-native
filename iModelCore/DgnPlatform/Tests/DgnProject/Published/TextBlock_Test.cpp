/*--------------------------------------------------------------------------------------+
|     $Source: Tests/DgnProject/Published/TextBlock_Test.cpp $
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/

#include "DgnHandlersTests.h"
#include <typeinfo>

USING_NAMESPACE_BENTLEY

/*=================================================================================**//**
* @bsiclass                                                     Jeff.Marker     10/2009
+===============+===============+===============+===============+===============+======*/
class TextBlockTest :
    public GenericDgnModelTestFixture
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     10/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: DgnModelR   GetDgnModelR    () const    { return *this->GetDgnModelP (); }
    public: DgnProjectR GetDgnProject () const    { return GetDgnModelP()->GetDgnProject(); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     10/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: TextBlockTest () :
        GenericDgnModelTestFixture (__FILE__, false /*2D*/)
        {
        }

    }; // TextBlockTest

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TextBlockTest, TextBlockPropertiesCloneAndEqualityTests)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlockProperties object, clones and mutates it, and ensures it reports equal/non-equal as expected.",
    //                                    "");

    TextBlockPropertiesPtr  lhsTbProps  = TextBlockProperties::Create (this->GetDgnModelR ());
    TextBlockPropertiesPtr  rhsTbProps  = TextBlockProperties::Create (this->GetDgnModelR ());

    ASSERT_TRUE (BackDoor::TextBlock::TextBlockProperties_Equals (*lhsTbProps, *rhsTbProps)) << L"Two blank TextBlockProperties should be equal, but report false";
    
    lhsTbProps->SetIsVertical (true);

    rhsTbProps = lhsTbProps->Clone ();

    ASSERT_TRUE (BackDoor::TextBlock::TextBlockProperties_Equals (*lhsTbProps, *rhsTbProps)) << L"Mutated and cloned TextBlockProperties should be equal, but report false";

    lhsTbProps = TextBlockProperties::Create (this->GetDgnModelR ());

    ASSERT_FALSE (BackDoor::TextBlock::TextBlockProperties_Equals (*lhsTbProps, *rhsTbProps)) << L"Mutated TextBlockProperties should NOT be equal, but report true";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TextBlockTest, ParagraphPropertiesCloneAndEqualityTests)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a ParagraphProperties object, clones and mutates it, and ensures it reports equal/non-equal as expected.",
    //                                    "");

    ParagraphPropertiesPtr  lhsParaProps    = ParagraphProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  rhsParaProps    = ParagraphProperties::Create (this->GetDgnModelR ());

    ASSERT_TRUE (BackDoor::TextBlock::ParagraphProperties_Equals (*lhsParaProps, *rhsParaProps)) << L"Two blank ParagraphProperties should be equal, but report false";

    lhsParaProps->SetLineSpacingValue (500.0);

    rhsParaProps = lhsParaProps->Clone ();

    ASSERT_TRUE (BackDoor::TextBlock::ParagraphProperties_Equals (*lhsParaProps, *rhsParaProps)) << L"Mutated and cloned ParagraphProperties should be equal, but report false";

    rhsParaProps->SetJustification (TextElementJustification::CenterMiddle);

    ASSERT_FALSE (BackDoor::TextBlock::ParagraphProperties_Equals (*lhsParaProps, *rhsParaProps)) << L"Mutated ParagraphProperties should NOT be equal, but report true";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TextBlockTest, RunPropertiesCloneAndEqualityTests)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a RunProperties object, clones and mutates it, and ensures it reports equal/non-equal as expected.",
    //                                    "");

    DgnFontCR           runPropsFont    = DgnFontManager::GetDefaultTrueTypeFont ();
    DPoint2d            runPropsSize    = { 1000.0, 1000.0 };
    RunPropertiesPtr    lhsRunProps     = RunProperties::Create (runPropsFont, runPropsSize, this->GetDgnModelR ());
    RunPropertiesPtr    rhsRunProps     = RunProperties::Create (runPropsFont, runPropsSize, this->GetDgnModelR ());

    ASSERT_TRUE (BackDoor::TextBlock::RunProperties_Equals (*lhsRunProps, *rhsRunProps)) << L"Two blank RunProperties should be equal, but report false";
                                                                                        
    lhsRunProps->SetIsBold (true);                                                      
                                                                                        
    rhsRunProps = lhsRunProps->Clone ();                                                
                                                                                        
    ASSERT_TRUE (BackDoor::TextBlock::RunProperties_Equals (*lhsRunProps, *rhsRunProps)) << L"Mutated and cloned RunProperties should be equal, but report false";
                                                                                        
    rhsRunProps->SetIsItalic (true);                                                    
                                                                                        
    ASSERT_FALSE (BackDoor::TextBlock::RunProperties_Equals (*lhsRunProps, *rhsRunProps)) << L"Mutated RunProperties should NOT be equal, but report true";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TextBlockTest, TextBlockEqualsPositiveUnitTest)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Mutates a representative set of TextBlock aspects, and ensures that the equality checker returns true.",
    //                                    "");

    static  const   WCharCP    LOREM_IPSUM_1   = L"Lorem ipsum";
    static  const   WCharCP    LOREM_IPSUM_2   = L"dolor sit amet";

    TextBlockPropertiesPtr  tbProps         = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps       = ParagraphProperties::Create (this->GetDgnModelR ());
    DgnFontCR               runPropsFont    = DgnFontManager::GetDefaultTrueTypeFont ();
    DPoint2d                runPropsSize    = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps        = RunProperties::Create (runPropsFont, runPropsSize, this->GetDgnModelR ());

    TextBlockPtr lhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    TextBlockPtr rhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());

    EXPECT_TRUE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (*lhs, *rhs)) << L"Two empty TextBlocks should be equal, but report false";

    lhs->AppendText (LOREM_IPSUM_1);
    rhs->AppendText (LOREM_IPSUM_1);

    EXPECT_TRUE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (*lhs, *rhs)) << L"Adding the same text to two empty TextBlocks should be equal, but report false";

    tbProps->SetIsVertical (true);
    paraProps->SetLineSpacingValue (500.0);
    runProps->SetIsBold (true);

    lhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    lhs->AppendText (LOREM_IPSUM_1);

    rhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    rhs->AppendText (LOREM_IPSUM_1);

    EXPECT_TRUE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (*lhs, *rhs)) << L"Adding the same text to two formatted TextBlocks should be equal, but report false";

    paraProps->SetLineSpacingValue (0.0);
    lhs->SetParagraphPropertiesForAdd (*paraProps);
    rhs->SetParagraphPropertiesForAdd (*paraProps);

    runProps->SetIsBold (false);
    lhs->SetRunPropertiesForAdd (*runProps);
    rhs->SetRunPropertiesForAdd (*runProps);

    lhs->AppendParagraphBreak ();
    rhs->AppendParagraphBreak ();

    lhs->AppendText (LOREM_IPSUM_2);
    rhs->AppendText (LOREM_IPSUM_2);

    EXPECT_TRUE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (*lhs, *rhs)) << L"Adding a new paragraph to two formatted TextBlocks should be equal, but report false";

    DPoint3d origin;
    origin.Init (1.0, 2.0, 3.0);
    lhs->SetUserOrigin (origin);
    rhs->SetUserOrigin (origin);

    EXPECT_TRUE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (*lhs, *rhs)) << L"Changing the origin of two TextBlocks should be equal, but report false";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TextBlockTest, TextBlockEqualsNegativeUnitTest)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Mutates a representative set of TextBlock aspects, and ensures that the equality checker returns false.",
    //                                    "");

    static  const   WCharCP    LOREM_IPSUM_1   = L"Lorem ipsum";
    static  const   WCharCP    LOREM_IPSUM_2   = L"dolor sit amet";

    TextBlockPropertiesPtr  tbProps         = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps       = ParagraphProperties::Create (this->GetDgnModelR ());
    DgnFontCR               runPropsFont    = DgnFontManager::GetDefaultTrueTypeFont ();
    DPoint2d                runPropsSize    = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps        = RunProperties::Create (runPropsFont, runPropsSize, this->GetDgnModelR ());

    TextBlockPtr lhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    lhs->AppendText (LOREM_IPSUM_1);

    TextBlockPropertiesPtr tbPropsMutated = tbProps->Clone ();
    tbPropsMutated->SetIsVertical (true);

    TextBlockPtr rhs = TextBlock::Create (*tbPropsMutated, *paraProps, *runProps, this->GetDgnModelR ());
    rhs->AppendText (LOREM_IPSUM_1);

    EXPECT_FALSE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (*lhs, *rhs)) << L"Two TextBlock with same text but different TextBlockProperties should be NOT equal, but report true";

    lhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    lhs->AppendText (LOREM_IPSUM_1);

    rhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    rhs->AppendText (LOREM_IPSUM_1);
    rhs->AppendText (LOREM_IPSUM_2);

    EXPECT_FALSE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (*lhs, *rhs)) << L"Two TextBlock with different text but same properties should be NOT equal, but report true";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TextBlockTest, CombineRunsUnitTest)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates multiple TextBlock objects that should be equivalent, even though they were created differently, and ensures that they are considered equal.",
    //                                    "Ensures that formatted runs are combined as appropriate (e.g. with like-formatting).");

    static  const   WCharCP    LOREM_IPSUM_1AB = L"Lorem ipsum dolor sit amet";
    static  const   WCharCP    LOREM_IPSUM_1A  = L"Lorem ipsum ";
    static  const   WCharCP    LOREM_IPSUM_1B  = L"dolor sit amet";

    TextBlockPropertiesPtr  tbProps         = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps       = ParagraphProperties::Create (this->GetDgnModelR ());
    DgnFontCR               runPropsFont    = DgnFontManager::GetDefaultTrueTypeFont ();
    DPoint2d                runPropsSize    = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps        = RunProperties::Create (runPropsFont, runPropsSize, this->GetDgnModelR ());

    TextBlockPtr lhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    lhs->AppendText (LOREM_IPSUM_1AB);

    TextBlockPtr rhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    rhs->AppendText (LOREM_IPSUM_1A);
    rhs->AppendText (LOREM_IPSUM_1B);

    EXPECT_TRUE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (*lhs, *rhs)) << L"Like-formatted runs should have been combined, but TextBlocks report NOT equal";
    
    lhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    lhs->AppendText (LOREM_IPSUM_1AB);

    rhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    rhs->AppendText (LOREM_IPSUM_1A);
    
    runProps->SetIsBold (true);
    rhs->SetRunPropertiesForAdd (*runProps);
    
    rhs->AppendText (LOREM_IPSUM_1B);

    EXPECT_FALSE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (*lhs, *rhs)) << L"Unlike-formatted runs should NOT have been combined, but TextBlocks report equal";
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void roundTripTextBlock (TextBlockCR original, WCharCP assertMessage, bool& attempted, EditElementHandleR fileEeh)
    {
    attempted = false;
    
    TextBlock::ToElementResult toElemResult = (TextBlock::ToElementResult)TextHandlerBase::CreateElement (fileEeh, NULL, original);
    ASSERT_EQ (TextBlock::TO_ELEMENT_RESULT_Success, toElemResult) << L"Could not generate text element(s).";
    
    ITextQueryCP textQuery = fileEeh.GetITextQuery ();
    ASSERT_TRUE (NULL != textQuery) << L"Generated element does not support ITextQuery.";
    ASSERT_TRUE (textQuery->IsTextElement (fileEeh)) << L"Generated element does not think it's a text element.";
    
    T_ITextPartIdPtrVector textParts;
    textQuery->GetTextPartIds (fileEeh, *ITextQueryOptions::CreateDefault (), textParts);
    ASSERT_EQ (1, textParts.size ()) << L"Generated element does not have one piece of text.";
    
    TextBlockPtr roundTripped = textQuery->GetTextPart (fileEeh, *textParts.front ());
    ASSERT_TRUE (roundTripped.IsValid ()) << L"Generated element could create a TextBlock for its single text part.";
    
    attempted = true;
    
    EXPECT_TRUE (BackDoor::TextBlock::TextBlock_EqualsWithCompareContentAndLocation (original, *roundTripped)) << (assertMessage ? assertMessage : L"TextBlock round-trip failed.");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void roundTripTextBlock (TextBlockCR original, WCharCP assertMessage, bool& attempted)
    {
    EditElementHandle fileEeh;
    roundTripTextBlock (original, assertMessage, attempted, fileEeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void roundTripSingleLineSingleFormat (TextBlockPropertiesCR tbProps, ParagraphPropertiesCR paraProps, RunPropertiesCR runProps, DgnModelR dgnModel, WCharCP message, bool& roundTripAttempted)
    {
    TextBlockPtr lhs = TextBlock::Create (tbProps, paraProps, runProps, dgnModel);
    lhs->AppendText (L"Lorem ipsum dolor sit amet.");
    roundTripTextBlock (*lhs, message, roundTripAttempted);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void roundTripMultiLineSingleFormat (TextBlockPropertiesCR tbProps, ParagraphPropertiesCR paraProps, RunPropertiesCR runProps, DgnModelR dgnModel, WCharCP message, bool& roundTripAttempted)
    {
    // While node properties are not strictly required at run-time, and their absence will still produce valid TextBlocks,
    //  when read back in they will be present, thus we will set them here to make equality checking easier.
    
    TextBlockPtr lhs = TextBlock::Create (tbProps, paraProps, runProps, dgnModel);
    BackDoor::TextBlock::TextBlock_SetNodeProperties (*lhs, runProps);
    
    lhs->AppendText (L"Lorem ipsum dolor sit amet,");
    lhs->AppendLineBreak ();
    lhs->AppendText (L"consectetur adipiscing elit.");
    lhs->AppendParagraphBreak ();
    lhs->AppendText (L"Vestibulum ante quam, tincidunt sed");
    lhs->AppendLineBreak ();
    lhs->AppendText (L"elementum a, malesuada aliquet massa.");
    lhs->AppendLineBreak ();
    lhs->AppendText (L"Vivamus quis leo nisi.");
    
    roundTripTextBlock (*lhs, message, roundTripAttempted);
    }

#define DO_ROUNDTRIP(ASSERT_MESSAGE)                                                            \
roundTripFunc (*tbProps, *paraProps, *runProps, dgnModel, ASSERT_MESSAGE, roundTripAttempted);  \
if (!roundTripAttempted)                                                                        \
    return;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void roundTripSingleFormat (DgnModelR dgnModel, void (*roundTripFunc)(TextBlockPropertiesCR, ParagraphPropertiesCR, RunPropertiesCR, DgnModelR, WCharCP, bool&))
    {
    TextBlockPropertiesPtr  tbProps             = TextBlockProperties::Create (dgnModel);
    ParagraphPropertiesPtr  paraProps           = ParagraphProperties::Create (dgnModel);
    DPoint2d                runPropsSize        = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps            = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);
    bool                    roundTripAttempted  = false;
    TextBlockPtr            lhs;

    //...............................................................................................................................................

    // No formatting
    DO_ROUNDTRIP (L"Round-trip failed for no formatting")

    //...............................................................................................................................................
    
    // Note that only RunProperties can dictate text style ID.
    //  Do not set text style ID on just TextBlockProperties or ParagraphProperties and expect it to round-trip.
    
    // Purposefully ignoring these text node-only properties: SetMaxCharactersPerLine, SetDocumentWidth
    tbProps = TextBlockProperties::Create (dgnModel);   tbProps->SetAnnotationScale (0.5);      DO_ROUNDTRIP (L"Round-trip failed for TextBlockProperties::SetAnnotationScale")
    tbProps = TextBlockProperties::Create (dgnModel);   tbProps->SetIsBackwards (true);         DO_ROUNDTRIP (L"Round-trip failed for TextBlockProperties::SetIsBackwards")
    tbProps = TextBlockProperties::Create (dgnModel);   tbProps->SetIsUpsideDown (true);        DO_ROUNDTRIP (L"Round-trip failed for TextBlockProperties::SetIsUpsideDown")
    tbProps = TextBlockProperties::Create (dgnModel);   tbProps->SetIsViewIndependent (true);   DO_ROUNDTRIP (L"Round-trip failed for TextBlockProperties::SetIsViewIndependent")
    tbProps = TextBlockProperties::Create (dgnModel);   tbProps->SetIsVertical (true);          DO_ROUNDTRIP (L"Round-trip failed for TextBlockProperties::SetIsVertical")
    
    tbProps = TextBlockProperties::Create (dgnModel);
    
    //...............................................................................................................................................
    
    IndentationDataPtr indentation = IndentationData::Create ();
    indentation->SetFirstLineIndent (1000.0);
    indentation->SetHangingIndent (4000.0);
    
    T_DoubleVector tabStops;
    tabStops.push_back (2000.0);
    tabStops.push_back (4000.0);
    tabStops.push_back (6000.0);
    
    indentation->SetTabStops (tabStops);
    
    // Justification is tricky... since TextBlock has to round-trip text nodes, technically there is a "node justification", as well as justification
    //  for each individual paragraph. Setting paragraph properties to a justification is all you really want, but simply doing that leaves the
    //  node justification unset. If we persist to a text element (which we will here), we loose the ability to have a unique node justification; when
    //  read back in, it will be assumed to be the first paragraph's justification (for lack of anything better). Thus, unless you manually set node
    //  justification on the source, the equals check will fail when round-tripped. I do not want to put a special case in equality checking for this.
    
    // It should also be noted that some RunProperties are inter-related, and thus cannot be tested in isolation
    //  (e.g. you need to enable italics for custom slant angle to be round-tripped).
    
    // Purposefully ignoring these text node-only properties: SetIsFullJustified, SetLineSpacingType, SetLineSpacingValue
    paraProps = ParagraphProperties::Create (dgnModel); paraProps->SetJustification (TextElementJustification::CenterMiddle);  DO_ROUNDTRIP (L"Round-trip failed for ParagraphProperties::SetJustification")
    tbProps = TextBlockProperties::Create (dgnModel);
    
    paraProps = ParagraphProperties::Create (dgnModel); paraProps->SetIndentation (*indentation);                   DO_ROUNDTRIP (L"Round-trip failed for ParagraphProperties::SetIndentation")
    
    paraProps = ParagraphProperties::Create (dgnModel);
    
    //...............................................................................................................................................
    
    UInt32      sampleColor         = 5;
    Int32       sampleLineStyle     = 2;
    UInt32      sampleLineWeight    = 3;
    DPoint2d    sampleDPoint2d      = { 123.0, 456.0 };
    
    // Purposefully ignoring SetShouldIgnoreLSB because that requires an MText document type, which cannot be directly created.
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetColor (2);                                                 DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetColor")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetIsBold (true);                                             DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetIsBold")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetIsItalic (true);                                           DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetIsItalic")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetIsItalic (true);                                           DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetIsItalic")
                                                                                                            runProps->SetCustomSlantAngle (PI / 6.0);                               DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetCustomSlantAngle")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetIsUnderlined (true);                                       DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetIsUnderlined")
                                                                                                            runProps->SetUnderlineOffset (100.0);                                   DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetUnderlineOffset")
                                                                                                            runProps->SetShouldUseUnderlineStyle (true);                            DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetShouldUseUnderlineStyle")
                                                                                                            runProps->SetUnderlineStyle (   &sampleColor,
                                                                                                                                            &sampleLineStyle,
                                                                                                                                            &sampleLineWeight);                     DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetUnderlineStyle")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetIsOverlined (true);                                        DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetIsOverlined")
                                                                                                            runProps->SetOverlineOffset (100.0);                                    DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetOverlineOffset")
                                                                                                            runProps->SetShouldUseOverlineStyle (true);                             DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetShouldUseOverlineStyle")
                                                                                                            runProps->SetOverlineStyle (    &sampleColor,
                                                                                                                                            &sampleLineStyle,
                                                                                                                                            &sampleLineWeight);                     DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetOverlineStyle")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetCharacterSpacingType (CharacterSpacingType::FixedWidth);  DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetCharacterSpacingType")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetCharacterSpacingValue (200.0);                             DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetCharacterSpacingValue")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetShouldUseBackground (true);                                DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetShouldUseBackground")
                                                                                                            runProps->SetBackgroundStyle (  &sampleColor,
                                                                                                                                            &sampleColor,
                                                                                                                                            &sampleLineStyle,
                                                                                                                                            &sampleLineWeight,
                                                                                                                                            &sampleDPoint2d);                       DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetBackgroundStyle")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetRunOffset (sampleDPoint2d);                                DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetRunOffset")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetIsSubScript (true);                                        DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetIsSubScript")
    runProps = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, dgnModel);   runProps->SetIsSuperScript (true);                                      DO_ROUNDTRIP (L"Round-trip failed for RunProperties::SetIsSuperScript")
    
    //...............................................................................................................................................
    
#ifdef DGN_IMPORTER_REORG_WIP
    // Just get a sampling of text style properties.
    DgnTextStylePtr textStyle = DgnTextStyle::Create (L"GeneratedStyleForRoundTripSingleFormat", *dgnModel.GetDgnProject ());
    textStyle->SetProperty (DgnTextStyleProperty::Vertical,         (bool)true);                            // Resides on TextBlockProperties
    textStyle->SetProperty (DgnTextStyleProperty::Justification,    (UInt32)TextElementJustification::CenterMiddle);   // Resides on ParagraphProperties
    textStyle->SetProperty (DgnTextStyleProperty::Width,            (double)1000.0);                        // Resides on RunProperties
    textStyle->SetProperty (DgnTextStyleProperty::Height,           (double)1000.0);                        // Resides on RunProperties
    textStyle->SetProperty (DgnTextStyleProperty::HasColor,        (bool)true);                            // Resides on RunProperties
    textStyle->SetProperty (DgnTextStyleProperty::Color,            (UInt32)6);                             // Resides on RunProperties
    
    textStyle->Add();
    
    tbProps     = TextBlockProperties::Create (*textStyle, dgnModel);
    paraProps   = ParagraphProperties::Create (*textStyle, dgnModel);
    runProps    = RunProperties::Create (*textStyle, dgnModel);
    
    DO_ROUNDTRIP (L"Round-trip failed for text style formatting")
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TextBlockTest, RoundTripSingleLineSingleFormat)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates single-line single-format TextBlocks, generate elements, read them back, and ensures that the TextBlocks are equal.",
    //                                    "");

    roundTripSingleFormat (this->GetDgnModelR (), roundTripSingleLineSingleFormat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TextBlockTest, RoundTripMultiLineSingleFormat)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates multi-line single-format TextBlocks, generate elements, read them back, and ensures that the TextBlocks are equal.",
    //                                    "");

    roundTripSingleFormat (this->GetDgnModelR (), roundTripMultiLineSingleFormat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TextBlockTest, RoundTripMultiFormat)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates multi-format TextBlocks, generate elements, read them back, and ensures that the TextBlocks are equal.",
    //                                    "");

    TextBlockPropertiesPtr  tbProps             = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps           = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize        = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps            = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    roundTripAttempted  = false;
    TextBlockPtr            lhs;
    
    //...............................................................................................................................................
    
    lhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    BackDoor::TextBlock::TextBlock_SetNodeProperties (*lhs, *runProps);
    
    lhs->AppendText (L"Lorem ipsum dolor sit amet, ");
    
    runProps->SetIsBold (true);
    runProps->SetColor (6);
    lhs->SetRunPropertiesForAdd (*runProps);
    
    lhs->AppendText (L"consectetur adipiscing elit.");
    
    roundTripTextBlock (*lhs, L"Round-trip failed for single-line multi-format", roundTripAttempted);
    if (!roundTripAttempted)
        return;
    
    //...............................................................................................................................................
    
    runProps    = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    lhs         = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    
    BackDoor::TextBlock::TextBlock_SetNodeProperties (*lhs, *runProps);
    
    lhs->AppendText (L"Lorem ipsum dolor sit amet, ");
    
    runProps->SetIsBold (true);
    runProps->SetColor (6);
    lhs->SetRunPropertiesForAdd (*runProps);
    
    lhs->AppendText (L"consectetur adipiscing elit.");
    lhs->AppendLineBreak ();
    
    runProps->SetCharacterSpacingType (CharacterSpacingType::FixedWidth);
    runProps->SetCharacterSpacingValue (100.0);
    lhs->SetRunPropertiesForAdd (*runProps);
    
    lhs->AppendText (L"Vestibulum ante quam, tincidunt sed elementum a, malesuada aliquet massa.");
    lhs->AppendParagraphBreak ();
    
    lhs->AppendText (L"Vivamus quis leo nisi.");
    
    roundTripTextBlock (*lhs, L"Round-trip failed for multi-line multi-format", roundTripAttempted);
    if (!roundTripAttempted)
        return;
    
    //...............................................................................................................................................
    
    runProps    = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    lhs         = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    
    BackDoor::TextBlock::TextBlock_SetNodeProperties (*lhs, *runProps);
    
    lhs->AppendText (L"Lorem ipsum dolor sit amet, ");
    lhs->AppendLineBreak ();
    lhs->AppendText (L"consectetur adipiscing elit.");
    
    IndentationDataPtr indentation = IndentationData::Create ();
    indentation->SetFirstLineIndent (4000.0);
    
    T_DoubleVector tabStops;
    tabStops.push_back (5000.0);
    tabStops.push_back (10000.0);
    tabStops.push_back (15000.0);
    
    indentation->SetTabStops (tabStops);
        
    paraProps->SetIndentation (*indentation);
    lhs->SetParagraphPropertiesForAdd (*paraProps);
    
    lhs->AppendParagraphBreak ();
    lhs->AppendText (L"Vestibulum ante quam, tincidunt sed ");
    lhs->AppendLineBreak ();
    lhs->AppendText (L"elementum a,");
    lhs->AppendTab ();
    lhs->AppendText (L"malesuada");
    lhs->AppendTab ();
    lhs->AppendText (L"aliquet massa.");

    roundTripTextBlock (*lhs, L"Round-trip failed for multi-paragraph multi-format", roundTripAttempted);
    if (!roundTripAttempted)
        return;
    
    // Don't forget to reset paraProps and runProps if adding another test.
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef DGN_IMPORTER_REORG_WIP
TEST_F (TextBlockTest, RoundTripOverrides)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates TextBlocks based on text styles, changes the TextBlock's properties, and ensures that overrides are set and persisted.",
    //                                    "");

    UInt32      sampleColor     = 7;
    DPoint2d    sampleDPoint2d  = { 123.0, 456.0 };
    
    DgnTextStylePtr textStyle = DgnTextStyle::Create (L"GeneratedStyleForRoundTripOverrides", *this->GetDgnModelR ().GetDgnProject ());
    
    textStyle->SetProperty (DgnTextStyleProperty::Vertical,             (bool)true);
    textStyle->SetProperty (DgnTextStyleProperty::Justification,        (UInt32)TextElementJustification::RightTop);
    
    textStyle->SetProperty (DgnTextStyleProperty::Width,                (double)1000.0);
    textStyle->SetProperty (DgnTextStyleProperty::Height,               (double)1000.0);
    textStyle->SetProperty (DgnTextStyleProperty::HasColor,            (bool)true);
    textStyle->SetProperty (DgnTextStyleProperty::Color,                (UInt32)6);
    textStyle->SetProperty (DgnTextStyleProperty::ShouldUseBackground,  (bool)true);
    textStyle->SetProperty (DgnTextStyleProperty::BackgroundBorderLineStyle,      (Int32)0);
    textStyle->SetProperty (DgnTextStyleProperty::BackgroundBorderWeight,     (UInt32)2);
    textStyle->SetProperty (DgnTextStyleProperty::BackgroundBorderColor,      (UInt32)3);
    textStyle->SetProperty (DgnTextStyleProperty::BackgroundFillColor,  (UInt32)4);
    textStyle->SetProperty (DgnTextStyleProperty::BackgroundBorderPaddingFactor,     (DPoint2d)sampleDPoint2d);
    
    textStyle->Add ();
    
    TextBlockPropertiesPtr  tbProps             = TextBlockProperties::Create (*textStyle, this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps           = ParagraphProperties::Create (*textStyle, this->GetDgnModelR ());
    RunPropertiesPtr        runProps            = RunProperties::Create (*textStyle, this->GetDgnModelR ());
    bool                    roundTripAttempted  = false;
    TextBlockPtr            lhs;
    
    tbProps->SetIsVertical (false);
    paraProps->SetJustification (TextElementJustification::LeftTop);
    runProps->SetFontSize (sampleDPoint2d);
    runProps->SetIsBold (true);
    runProps->SetBackgroundStyle (&sampleColor, NULL, NULL, NULL, NULL);
    
    lhs = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    BackDoor::TextBlock::TextBlock_SetNodeProperties (*lhs, *runProps);
    
    lhs->AppendText (L"Lorem ipsum dolor sit amet,");
    lhs->AppendParagraphBreak ();
    lhs->AppendText (L"consectetur adipiscing elit.");
    
    ASSERT_FALSE (tbProps->IsBackwardsOverridden ());
    ASSERT_FALSE (tbProps->IsUpsideDownOverridden ());
    ASSERT_TRUE (tbProps->IsVerticalOverridden ());
    ASSERT_FALSE (tbProps->IsMaxCharactersPerLineOverridden ());
    
    ASSERT_TRUE (paraProps->IsJustificationOverridden ());
    ASSERT_FALSE (paraProps->IsFullJustifiedOverridden ());
    ASSERT_FALSE (paraProps->IsLineSpacingTypeOverridden ());
    ASSERT_FALSE (paraProps->IsLineSpacingValueOverridden ());
    
    ASSERT_FALSE (runProps->IsFontOverridden ());
    ASSERT_FALSE (runProps->IsShxBigFontOverridden ());
    ASSERT_FALSE (runProps->IsHasColorOverridden ());
    ASSERT_FALSE (runProps->IsColorOverridden ());
    ASSERT_TRUE (runProps->IsBoldOverridden ());
    ASSERT_FALSE (runProps->IsItalicOverridden ());
    ASSERT_FALSE (runProps->IsCustomSlantAngleOverridden ());
    ASSERT_FALSE (runProps->IsUnderlinedOverridden ());
    ASSERT_FALSE (runProps->IsShouldUseUnderlineStyleOverridden ());
    ASSERT_FALSE (runProps->IsUnderlineOffsetOverridden ());
    ASSERT_FALSE (runProps->IsUnderlineColorOverridden ());
    ASSERT_FALSE (runProps->IsUnderlineLineStyleOverridden ());
    ASSERT_FALSE (runProps->IsUnderlineWeightOverridden ());
    ASSERT_FALSE (runProps->IsOverlinedOverridden ());
    ASSERT_FALSE (runProps->IsShouldUseOverlineStyleOverridden ());
    ASSERT_FALSE (runProps->IsOverlineOffsetOverridden ());
    ASSERT_FALSE (runProps->IsOverlineColorOverridden ());
    ASSERT_FALSE (runProps->IsOverlineLineStyleOverridden ());
    ASSERT_FALSE (runProps->IsOverlineWeightOverridden ());
    ASSERT_FALSE (runProps->IsCharacterSpacingTypeOverridden ());
    ASSERT_FALSE (runProps->IsCharacterSpacingValueOverridden ());
    ASSERT_FALSE (runProps->IsShouldUseBackgroundOverridden ());
    ASSERT_TRUE (runProps->IsBackgroundFillColorOverridden ());
    ASSERT_FALSE (runProps->IsBackgroundBorderColorOverridden ());
    ASSERT_FALSE (runProps->IsBackgroundBorderLineStyleOverridden ());
    ASSERT_FALSE (runProps->IsBackgroundBorderWeightOverridden ());
    ASSERT_FALSE (runProps->IsBackgroundBorderPaddingOverridden ());
    ASSERT_FALSE (runProps->IsRunOffsetOverridden ());
    ASSERT_FALSE (runProps->IsSubScriptOverridden ());
    ASSERT_FALSE (runProps->IsSuperScriptOverridden ());
    ASSERT_TRUE (runProps->IsWidthOverridden ());
    ASSERT_TRUE (runProps->IsHeightOverridden ());

    roundTripTextBlock (*lhs, L"Round-trip failed for multi-paragraph multi-format", roundTripAttempted);
    if (!roundTripAttempted)
        return;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, SingleAtLengthEdf)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with a single EDF (at-length), and round-trips.",
    //                                    "");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    
    textBlock->AppendEnterDataField (L"EdfValue", 8, EdfJustification::Left);

    CaretPtr        caret = textBlock->CreateStartCaret ();
    EdfCharStreamCP edf   = dynamic_cast<EdfCharStreamCP>(caret->GetCurrentRunCP ());
    
    EXPECT_NE ((EdfCharStreamCP)NULL, edf) << L"The first run is not an EdfCharStream.";
    
    if (NULL != edf)
        EXPECT_EQ (0, edf->GetString ().compare (L"EdfValue")) << L"The value string does not match.";

    EXPECT_NE (SUCCESS, caret->MoveToNextRun ()) << L"More than one run exists.";
    
    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, SingleOverLengthEdf)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with a single EDF (over-length), and round-trips.",
    //                                    "Ensures that the EDF gets truncated correctly because it was over-length.");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    
    // This will throw an BeDataAssert because the string we are providing exceeds the total length we are providing.
    BeTest::SetFailOnAssert (false);
        {
        textBlock->AppendEnterDataField (L"EdfValueTooBig", 8, EdfJustification::Left);
        }
    BeTest::SetFailOnAssert (true);

    CaretPtr        caret = textBlock->CreateStartCaret ();
    EdfCharStreamCP edf   = dynamic_cast<EdfCharStreamCP>(caret->GetCurrentRunCP ());
    
    EXPECT_NE ((EdfCharStreamCP)NULL, edf) << L"The first run is not an EdfCharStream.";
    
    if (NULL != edf)
        EXPECT_EQ (0, edf->GetString ().compare (L"EdfValue")) << L"The value string was not truncated correctly.";

    EXPECT_NE (SUCCESS, caret->MoveToNextRun ()) << L"More than one run exists.";

    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, SingleLeftUnderLengthEdf)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with a single EDF (under-length), and round-trips.",
    //                                    "Ensures that the EDF gets padded correctly because it was under-length.");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    
    textBlock->AppendEnterDataField (L"EdfValue", 13, EdfJustification::Left);

    CaretPtr        caret = textBlock->CreateStartCaret ();
    EdfCharStreamCP edf   = dynamic_cast<EdfCharStreamCP>(caret->GetCurrentRunCP ());
    
    EXPECT_NE ((EdfCharStreamCP)NULL, edf) << L"The first run is not an EdfCharStream.";
    
    if (NULL != edf)
        EXPECT_EQ (0, edf->GetString ().compare (L"EdfValue     ")) << L"The value string was not padded correctly.";

    EXPECT_NE (SUCCESS, caret->MoveToNextRun ()) << L"More than one run exists.";
    
    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, SingleCenterUnderLengthEdf)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with a single EDF (under-length, center), and round-trips.",
    //                                    "Ensures that the EDF gets padded correctly because it was under-length.");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    
    textBlock->AppendEnterDataField (L"EdfValue", 13, EdfJustification::Center);

    CaretPtr        caret = textBlock->CreateStartCaret ();
    EdfCharStreamCP edf   = dynamic_cast<EdfCharStreamCP>(caret->GetCurrentRunCP ());
    
    EXPECT_NE ((EdfCharStreamCP)NULL, edf) << L"The first run is not an EdfCharStream.";
    
    if (NULL != edf)
        EXPECT_EQ (0, edf->GetString ().compare (L"  EdfValue   ")) << L"The value string was not padded correctly.";

    EXPECT_NE (SUCCESS, caret->MoveToNextRun ()) << L"More than one run exists.";
    
    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, SingleRightUnderLengthEdf)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with a single EDF (under-length, right), and round-trips.",
    //                                    "Ensures that the EDF gets padded correctly because it was under-length.");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    
    textBlock->AppendEnterDataField (L"EdfValue", 13, EdfJustification::Right);

    CaretPtr        caret = textBlock->CreateStartCaret ();
    EdfCharStreamCP edf   = dynamic_cast<EdfCharStreamCP>(caret->GetCurrentRunCP ());
    
    EXPECT_NE ((EdfCharStreamCP)NULL, edf) << L"The first run is not an EdfCharStream.";
    
    if (NULL != edf)
        EXPECT_EQ (0, edf->GetString ().compare (L"     EdfValue")) << L"The value string was not padded correctly.";

    EXPECT_NE (SUCCESS, caret->MoveToNextRun ()) << L"More than one run exists.";
    
    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, SingleBlankEdf)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with a single EDF (under-length, blank), and round-trips.",
    //                                    "Ensures that the EDF gets padded correctly because it was under-length.");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());
    
    textBlock->AppendEnterDataField (L"", 10, EdfJustification::Left);

    CaretPtr        caret = textBlock->CreateStartCaret ();
    EdfCharStreamCP edf   = dynamic_cast<EdfCharStreamCP>(caret->GetCurrentRunCP ());
    
    EXPECT_NE ((EdfCharStreamCP)NULL, edf) << L"The first run is not an EdfCharStream.";
    
    if (NULL != edf)
        EXPECT_EQ (0, edf->GetString ().compare (L"          ")) << L"The value string was not padded correctly.";

    EXPECT_NE (SUCCESS, caret->MoveToNextRun ()) << L"More than one run exists.";
    
    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, CombiningCharStreamThenEdf)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with a CharStream and then an EDF (under-length), and round-trips.",
    //                                    "Ensures that a single text element was produced (even though we had two runs).");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());

    textBlock->AppendText (L"Lorem ipsum ");
    textBlock->AppendEnterDataField (L"dolar", 10, EdfJustification::Left);
    
    EditElementHandle fileEeh;
    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted, fileEeh);
    if (!wasRoundTripAttempted)
        return;
    
    EXPECT_EQ (TEXT_ELM, fileEeh.GetLegacyType ()) << L"Persisted element is a not a single text element.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, CombiningEdfThenCharStream)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with an EDF (under-length) and then a CharStream, and round-trips.",
    //                                    "Ensures that a single text element was produced (even though we had two runs).");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());

    textBlock->AppendEnterDataField (L"Lorem", 10, EdfJustification::Left);
    textBlock->AppendText (L" ipsum dolar");
    
    EditElementHandle fileEeh;
    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted, fileEeh);
    if (!wasRoundTripAttempted)
        return;
    
    EXPECT_EQ (TEXT_ELM, fileEeh.GetLegacyType ()) << L"Persisted element is a not a single text element.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, CombiningManyCharStreamsEdfs)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with several EDFs (under-length) and CharStreams, and round-trips.",
    //                                    "Ensures that a single text element was produced (even though we had multiple runs).");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());

    textBlock->AppendText (L"Lorem ipsum ");
    textBlock->AppendEnterDataField (L"dolar", 5, EdfJustification::Left);
    textBlock->AppendText (L" sit amet, ");
    textBlock->AppendEnterDataField (L"consectetur", 15, EdfJustification::Center);
    textBlock->AppendText (L"adipiscing elit.");
    
    EditElementHandle fileEeh;
    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted, fileEeh);
    if (!wasRoundTripAttempted)
        return;
    
    EXPECT_EQ (TEXT_ELM, fileEeh.GetLegacyType ()) << L"Persisted element is a not a single text element.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, CombiningManyCharStreamsEdfsInTextnode)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Creates a TextBlock with several EDFs (under-length), CharStreams, and breaks, and then round-trips.",
    //                                    "Ensures that the appropriate number of text element was produced (even though we had multiple runs per line).");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());

    BackDoor::TextBlock::TextBlock_SetNodeProperties (*textBlock, *runProps);
    
    textBlock->AppendText (L"Lorem ipsum ");
    textBlock->AppendEnterDataField (L"dolar", 5, EdfJustification::Left);
    textBlock->AppendLineBreak ();
    textBlock->AppendText (L" sit amet, ");
    textBlock->AppendEnterDataField (L"consectetur", 15, EdfJustification::Center);
    textBlock->AppendText (L"adipiscing elit.");
    
    EditElementHandle fileEeh;
    roundTripTextBlock (*textBlock, L"Round-trip failed.", wasRoundTripAttempted, fileEeh);
    if (!wasRoundTripAttempted)
        return;
    
    size_t numChildren = 0;
    
    EXPECT_EQ (TEXT_NODE_ELM, fileEeh.GetLegacyType ()) << L"Persisted element is a not a text node.";

    for (ChildElemIter childIter (fileEeh, (ExposeChildrenReason)100); childIter.IsValid (); childIter = childIter.ToNext ())
        ++numChildren;
    
    EXPECT_EQ (2, numChildren) << L"Runs were not combined into 2 child elements.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, EnforcingAtomicEdfsOnInsert)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Ensures the TextBlock API does not allow you to insert text in the middle of an atomic (e.g. EDF) run.",
    //                                    "Even though the API technically allows you to insert anywhere, we must prevent the corruption of atomic runs.");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
//    bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());

    textBlock->AppendText (L"Lorem ipsum ");
    textBlock->AppendEnterDataField (L"dolar", 5, EdfJustification::Left);
    textBlock->AppendText (L" sit amet.");
    
    CaretPtr insertCaret = textBlock->CreateStartCaret ();
    
    for (size_t i = 0; i < 15; ++i)
        insertCaret->MoveToNextCharacter ();
    
    // This will throw an BeDataAssert because the string we are providing exceeds the total length we are providing.
    BeTest::SetFailOnAssert (false);
        {
        textBlock->InsertText (*insertCaret, L"new text");
        }
    BeTest::SetFailOnAssert (true);

    CaretPtr runIter = textBlock->CreateStartCaret ();

    EXPECT_TRUE (typeid (CharStream) == typeid (*runIter->GetCurrentRunCP ())) << L"Unexpected run type.";
    runIter->MoveToNextRun ();
    
    EXPECT_TRUE (typeid (EdfCharStream) == typeid (*runIter->GetCurrentRunCP ())) << L"Unexpected run type.";
    runIter->MoveToNextRun ();
    
    EXPECT_TRUE (typeid (CharStream) == typeid (*runIter->GetCurrentRunCP ())) << L"Unexpected run type.";
    runIter->MoveToNextRun ();

    EXPECT_NE (SUCCESS, runIter->MoveToNextRun ()) << L"Too many runs.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
TEST_F (TextBlockTest, EnforcingAtomicEdfsOnDelete)
    {
    //TestDocumentation::DocumentTest (   "",
    //                                    "2D DGN",
    //                                    "Ensures the TextBlock API does not allow you to delete in the middle of an atomic (e.g. EDF) run.",
    //                                    "Even though the API technically allows you to delete anywhere, we must prevent the corruption of atomic runs.");

    TextBlockPropertiesPtr  tbProps                 = TextBlockProperties::Create (this->GetDgnModelR ());
    ParagraphPropertiesPtr  paraProps               = ParagraphProperties::Create (this->GetDgnModelR ());
    DPoint2d                runPropsSize            = { 1000.0, 1000.0 };
    RunPropertiesPtr        runProps                = RunProperties::Create (DgnFontManager::GetDefaultTrueTypeFont (), runPropsSize, this->GetDgnModelR ());
    //bool                    wasRoundTripAttempted   = false;    
    TextBlockPtr            textBlock               = TextBlock::Create (*tbProps, *paraProps, *runProps, this->GetDgnModelR ());

    textBlock->AppendText (L"Lorem ipsum ");
    textBlock->AppendEnterDataField (L"dolar", 5, EdfJustification::Left);
    textBlock->AppendText (L" sit amet.");
    
    CaretPtr insertCaret = textBlock->CreateStartCaret ();
    
    for (size_t i = 0; i < 15; ++i)
        insertCaret->MoveToNextCharacter ();
    
    // This will throw an BeDataAssert because the string we are providing exceeds the total length we are providing.
    BeTest::SetFailOnAssert (false);
        {
        textBlock->Remove (*insertCaret, *textBlock->CreateEndCaret ());
        }
    BeTest::SetFailOnAssert (true);

    CaretPtr runIter = textBlock->CreateStartCaret ();

    EXPECT_TRUE (typeid (CharStream) == typeid (*runIter->GetCurrentRunCP ())) << L"Unexpected run type.";
    runIter->MoveToNextRun ();
    
    EXPECT_TRUE (typeid (EdfCharStream) == typeid (*runIter->GetCurrentRunCP ())) << L"Unexpected run type.";
    runIter->MoveToNextRun ();
    
    EXPECT_NE (SUCCESS, runIter->MoveToNextRun ()) << L"Too many runs.";
    }
#endif
