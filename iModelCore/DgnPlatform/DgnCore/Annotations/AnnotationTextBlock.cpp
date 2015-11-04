//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationTextBlock.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextStylePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextBlockPersistence.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextBlock::AnnotationTextBlock(DgnDbR project) :
    T_Super()
    {
    // Making additions or changes? Please check AnnotationTextBlock::Reset.
    m_dgndb = &project;
    m_documentWidth = 0.0;
    m_justification = HorizontalJustification::Left;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextBlockPtr AnnotationTextBlock::Create(DgnDbR project, AnnotationTextStyleId styleID)
    {
    auto doc = AnnotationTextBlock::Create(project);
    doc->SetStyleId(styleID, SetAnnotationTextStyleOptions::Direct);

    return doc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextBlockPtr AnnotationTextBlock::Create(DgnDbR project, AnnotationTextStyleId styleID, AnnotationParagraphR par)
    {
    auto doc = AnnotationTextBlock::Create(project, styleID);
    doc->GetParagraphsR().push_back(&par);

    return doc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextBlockPtr AnnotationTextBlock::Create(DgnDbR project, AnnotationTextStyleId styleID, Utf8CP content)
    {
    auto run = AnnotationTextRun::Create(project, styleID, content);
    auto par = AnnotationParagraph::Create(project, styleID, *run);
    auto doc = AnnotationTextBlock::Create(project, styleID, *par);
    
    return doc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlock::CopyFrom(AnnotationTextBlockCR rhs)
    {
    m_dgndb = rhs.m_dgndb;
    m_documentWidth = rhs.m_documentWidth;
    m_justification = rhs.m_justification;
    m_styleID = rhs.m_styleID;
    m_styleOverrides = rhs.m_styleOverrides;

    m_paragraphs.clear();
    m_paragraphs.reserve(rhs.m_paragraphs.size());
    for (auto const& rhsParagraph : rhs.m_paragraphs)
        m_paragraphs.push_back(rhsParagraph->Clone());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlock::Reset()
    {
    // Making additions or changes? Please check AnnotationTextBlock::AnnotationTextBlock.
    m_documentWidth = 0.0;
    m_justification = HorizontalJustification::Left;
    m_styleID.Invalidate();
    m_styleOverrides.ClearAllProperties();
    m_paragraphs.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlock::SetStyleId(AnnotationTextStyleId value, SetAnnotationTextStyleOptions options)
    {
    m_styleID = value;

    if (!isEnumFlagSet(SetAnnotationTextStyleOptions::PreserveOverrides, options))
        m_styleOverrides.ClearAllProperties();
    
    if (!isEnumFlagSet(SetAnnotationTextStyleOptions::DontPropogate, options))
        {
        for (auto& paragraph : m_paragraphs)
            paragraph->SetStyleId(value, options);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlock::AppendParagraph()
    {
    // I debated seeding a paragraph from doc style vs. doing nothing if empty... as a utility method, I think using the document as a seed is more useful.
    // You can get fine-grained control by using other more direct means if necessary.
    
    AnnotationParagraphPtr par;

    if (m_paragraphs.empty())
        {
        par = AnnotationParagraph::Create(*m_dgndb, m_styleID);
        }
    else
        {
        auto seedPar = m_paragraphs.back();

        par = AnnotationParagraph::Create(*m_dgndb);
        par->SetStyleId(seedPar->GetStyleId(), SetAnnotationTextStyleOptions::Direct);
        par->GetStyleOverridesR() = seedPar->GetStyleOverrides();
        }
    
    m_paragraphs.push_back(par);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlock::AppendRun(AnnotationRunBaseR run)
    {
    // I debated ensuring a paragraph vs. doing nothing if empty... as a utility method, I think creating a new paragraph is more useful.
    // You can get fine-grained control by using other more direct means if necessary.
    
    if (m_paragraphs.empty())
        m_paragraphs.push_back(AnnotationParagraph::Create(*m_dgndb, m_styleID));

    m_paragraphs.back()->GetRunsR().push_back(&run);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2015
//---------------------------------------------------------------------------------------
AnnotationTextBlock::ToStringOpts AnnotationTextBlock::ToStringOpts::CreateForPlainText()
    {
    ToStringOpts opts;
    opts.m_paragraphSeparator = " ";
    opts.m_lineBreakString = " ";
    opts.m_fractionSeparator = "/";

    return opts;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2015
//---------------------------------------------------------------------------------------
Utf8String AnnotationTextBlock::ToString(ToStringOpts const& opts) const
    {
    Utf8String str;

    for (size_t iParagraph = 0; iParagraph < m_paragraphs.size(); ++iParagraph)
        {
        if (iParagraph > 0)
            str += opts.GetParagraphSeparator();

        for (AnnotationRunBasePtr run : m_paragraphs[iParagraph]->m_runs)
            {
            switch (run->GetType())
                {
                case AnnotationRunType::Text: str += ((AnnotationTextRunCR)*run).GetContent(); break;
                case AnnotationRunType::LineBreak: str += opts.GetLineBreakString(); break;
                case AnnotationRunType::Fraction:
                    {
                    AnnotationFractionRunCR fraction = (AnnotationFractionRunCR)*run;
                    str += fraction.GetNumeratorContent();
                    str += opts.GetFractionSeparator();
                    str += fraction.GetDenominatorContent();
                    
                    break;
                    }

                default:
                    BeAssert(false);
                    break;
                }
            }
        }

    return str;
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

static const uint8_t CURRENT_MAJOR_VERSION = 1;
static const uint8_t CURRENT_MINOR_VERSION = 0;
static const double DEFAULT_DOCUMENTWIDTH_VALUE = 0.0;
static const int64_t DEFAULT_JUSTIFICATION_VALUE = (int64_t)AnnotationTextBlock::HorizontalJustification::Left;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void appendIntegerSetter
(
FB::AnnotationTextBlockSetters& setters,
decltype(declval<FB::AnnotationTextBlockSetter>().key()) fbProp,
decltype(declval<FB::AnnotationTextBlockSetter>().integerValue()) value,
decltype(declval<FB::AnnotationTextBlockSetter>().integerValue()) defaultValue
)
    {
    if (value == defaultValue)
        return;

    setters.push_back(FB::AnnotationTextBlockSetter(fbProp, value, 0.0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void appendRealSetter
(
FB::AnnotationTextBlockSetters& setters,
decltype(declval<FB::AnnotationTextBlockSetter>().key()) fbProp,
decltype(declval<FB::AnnotationTextBlockSetter>().realValue()) value,
decltype(declval<FB::AnnotationTextBlockSetter>().realValue()) defaultValue
)
    {
    if (value == defaultValue)
        return;

    setters.push_back(FB::AnnotationTextBlockSetter(fbProp, 0, value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockPersistence::EncodeAsFlatBuf(FB::AnnotationTextBlockRunOffsets& runOffsets, flatbuffers::FlatBufferBuilder& encoder, AnnotationRunBaseCR run)
    {
    //.............................................................................................
    FB::AnnotationTextStyleSetters runOverrides;
    POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::EncodeAsFlatBuf(runOverrides, run.m_styleOverrides), ERROR);

    FB::AnnotationTextStyleSetterVectorOffset runOverridesOffset;
    if (!runOverrides.empty())
        runOverridesOffset = encoder.CreateVectorOfStructs(runOverrides);

    //.............................................................................................
    Offset<String> textContentOffset;
    Offset<String> fractionDenominatorContentOffset;
    Offset<String> fractionNumeratorContentOffset;

    switch (run.GetType())
        {
        case AnnotationRunType::Text:
            {
            auto const* textRun = dynamic_cast<AnnotationTextRunCP>(&run);
            
            if (!textRun->m_content.empty())
                textContentOffset = encoder.CreateString(textRun->m_content);

            break;
            }

        case AnnotationRunType::Fraction:
            {
            auto const* fractionRun = dynamic_cast<AnnotationFractionRunCP>(&run);
            
            if (!fractionRun->m_denominatorContent.empty())
                fractionDenominatorContentOffset = encoder.CreateString(fractionRun->m_denominatorContent);

            if (!fractionRun->m_numeratorContent.empty())
                fractionNumeratorContentOffset = encoder.CreateString(fractionRun->m_numeratorContent);

            break;
            }

        case AnnotationRunType::LineBreak:
            {
            break;
            }

        default:
            BeAssert(false); // Unknown/unexpected AnnotationRunType.
            break;
        }
    
    //.............................................................................................
    FB::AnnotationTextBlockRunBuilder fbRun(encoder);
    // No properties in this version.
    
    fbRun.add_styleId(run.m_styleID.GetValue());
    if (!runOverrides.empty())
        fbRun.add_styleOverrides(runOverridesOffset);
    
    switch (run.GetType())
        {
        case AnnotationRunType::Text:
            {
            fbRun.add_type(FB::AnnotationTextBlockRunType_Text);
            
            auto const* textRun = dynamic_cast<AnnotationTextRunCP>(&run);
            
            if (!textRun->m_content.empty())
                fbRun.add_text_content(textContentOffset);

            switch (textRun->GetSubSuperScript())
                {
                case AnnotationTextRunSubSuperScript::SubScript: fbRun.add_text_subsuperscript(FB::AnnotationTextRunSubSuperScript_SubScript); break;
                case AnnotationTextRunSubSuperScript::SuperScript: fbRun.add_text_subsuperscript(FB::AnnotationTextRunSubSuperScript_SuperScript); break;
                // Otherwise neither, don't store anything.
                }
            
            break;
            }

        case AnnotationRunType::Fraction:
            {
            fbRun.add_type(FB::AnnotationTextBlockRunType_Fraction);
            
            auto const* fractionRun = dynamic_cast<AnnotationFractionRunCP>(&run);

            if (!fractionRun->m_denominatorContent.empty())
                fbRun.add_fraction_denominatorContent(fractionDenominatorContentOffset);

            if (!fractionRun->m_numeratorContent.empty())
                fbRun.add_fraction_numeratorContent(fractionNumeratorContentOffset);

            break;
            }

        case AnnotationRunType::LineBreak:
            {
            fbRun.add_type(FB::AnnotationTextBlockRunType_LineBreak);
            
            break;
            }

        default:
            BeAssert(false); // Unknown/unexpected AnnotationRunType.
            break;
        }
    

    runOffsets.push_back(fbRun.Finish());
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockPersistence::EncodeAsFlatBuf(FB::AnnotationTextBlockParagraphOffsets& paragraphOffsets, flatbuffers::FlatBufferBuilder& encoder, AnnotationParagraphCR paragraph)
    {
    //.............................................................................................
    FB::AnnotationTextBlockRunOffsets runOffsets;
    for (auto const& run : paragraph.m_runs)
        POSTCONDITION(SUCCESS == EncodeAsFlatBuf(runOffsets, encoder, *run), ERROR);
    
    FB::AnnotationTextBlockRunOffsetVectorOffset runOffsetsOffset;
    if (!runOffsets.empty())
        runOffsetsOffset = encoder.CreateVector(runOffsets);
    
    //.............................................................................................
    FB::AnnotationTextBlockParagraphBuilder fbParagraph(encoder);
    // No properties in this version.
    fbParagraph.add_styleId(paragraph.m_styleID.GetValue());
    // No style overrides in this version.
    
    if (!runOffsets.empty())
        fbParagraph.add_runs(runOffsetsOffset);
    
    //.............................................................................................
    paragraphOffsets.push_back(fbParagraph.Finish());
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockPersistence::EncodeAsFlatBuf(Offset<FB::AnnotationTextBlock>& documentOffset, FlatBufferBuilder& encoder, AnnotationTextBlockCR document)
    {
    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    FB::AnnotationTextBlockSetters propSetters;
    appendRealSetter(propSetters, FB::AnnotationTextBlockProperty_DocumentWidth, document.m_documentWidth, DEFAULT_DOCUMENTWIDTH_VALUE);
    appendIntegerSetter(propSetters, FB::AnnotationTextBlockProperty_Justification, (int64_t)document.m_justification, DEFAULT_JUSTIFICATION_VALUE);

    FB::AnnotationTextBlockSetterVectorOffset propSettersOffset;
    if (!propSetters.empty())
        propSettersOffset = encoder.CreateVectorOfStructs(propSetters);

    //.............................................................................................
    FB::AnnotationTextStyleSetters docOverrides;
    POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::EncodeAsFlatBuf(docOverrides, document.m_styleOverrides), ERROR);

    FB::AnnotationTextStyleSetterVectorOffset docOverridesOffset;
    if (!docOverrides.empty())
        docOverridesOffset = encoder.CreateVectorOfStructs(docOverrides);

    //.............................................................................................
    FB::AnnotationTextBlockParagraphOffsets paragraphOffsets;
    for (auto const& paragraph : document.m_paragraphs)
        POSTCONDITION(SUCCESS == EncodeAsFlatBuf(paragraphOffsets, encoder, *paragraph), ERROR);

    FB::AnnotationTextBlockParagraphOffsetVectorOffset paragraphOffsetsOffset;
    if (!paragraphOffsets.empty())
        paragraphOffsetsOffset = encoder.CreateVector(paragraphOffsets);
    
    //.............................................................................................
    FB::AnnotationTextBlockBuilder fbDocument(encoder);
    fbDocument.add_majorVersion(CURRENT_MAJOR_VERSION);
    fbDocument.add_minorVersion(CURRENT_MINOR_VERSION);

    if (!propSetters.empty())
        fbDocument.add_properties(propSettersOffset);
    
    fbDocument.add_styleId(document.m_styleID.GetValue());
    if (!docOverrides.empty())
        fbDocument.add_styleOverrides(docOverridesOffset);

    if (!paragraphOffsets.empty())
        fbDocument.add_paragraphs(paragraphOffsetsOffset);

    //.............................................................................................
    documentOffset = fbDocument.Finish();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockPersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, AnnotationTextBlockCR document)
    {
    FlatBufferBuilder encoder;
    
    //.............................................................................................
    Offset<FB::AnnotationTextBlock> documentOffset;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(documentOffset, encoder, document), ERROR);

    //.............................................................................................
    encoder.Finish(documentOffset);
    buffer.resize(encoder.GetSize());
    memcpy(&buffer[0], encoder.GetBufferPointer(), encoder.GetSize());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockPersistence::DecodeFromFlatBuf(AnnotationTextBlockR document, FB::AnnotationTextBlock const& fbDocument)
    {
    document.Reset();
    
    PRECONDITION(fbDocument.has_majorVersion(), ERROR);
    if (fbDocument.majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;
    
    if (fbDocument.has_properties())
        {
        for (auto const& propSetter : *fbDocument.properties())
            {
            switch (propSetter.key())
                {
                case FB::AnnotationTextBlockProperty_DocumentWidth: document.SetDocumentWidth(propSetter.realValue()); break;
                case FB::AnnotationTextBlockProperty_Justification: document.SetJustification((AnnotationTextBlock::HorizontalJustification)propSetter.integerValue()); break;
                }
            }
        }
    
    PRECONDITION(fbDocument.has_styleId(), ERROR);
    document.SetStyleId(AnnotationTextStyleId((uint64_t)fbDocument.styleId()), SetAnnotationTextStyleOptions::Direct);
    
    if (fbDocument.has_styleOverrides())
        POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::DecodeFromFlatBuf(document.m_styleOverrides, *fbDocument.styleOverrides()), ERROR);

    if (fbDocument.has_paragraphs())
        {
        for (auto const& fbParagraph : *fbDocument.paragraphs())
            {
            AnnotationParagraphPtr paragraph = AnnotationParagraph::Create(document.GetDbR());
            PRECONDITION(fbParagraph.has_styleId(), ERROR);
            paragraph->SetStyleId(AnnotationTextStyleId((uint64_t)fbParagraph.styleId()), SetAnnotationTextStyleOptions::Direct);

            if (fbParagraph.has_runs())
                {
                for (auto const& fbRun : *fbParagraph.runs())
                    {
                    AnnotationRunBasePtr run;
                    PRECONDITION(fbRun.has_type(), ERROR);
                    switch (fbRun.type())
                        {
                        case FB::AnnotationTextBlockRunType_Text:
                            {
                            AnnotationTextRunPtr textRun = AnnotationTextRun::Create(document.GetDbR());
                            run = textRun;
                            
                            PRECONDITION(fbRun.has_text_content(), ERROR);
                            textRun->SetContent(fbRun.text_content()->c_str());
                            
                            if (fbRun.has_text_subsuperscript())
                                {
                                switch (fbRun.text_subsuperscript())
                                    {
                                    case FB::AnnotationTextRunSubSuperScript_SubScript: textRun->SetIsSubScript(true); break;
                                    case FB::AnnotationTextRunSubSuperScript_SuperScript: textRun->SetIsSuperScript(true); break;
                                    // Otherwise neither, didn't store anything.
                                    }
                                }
                            
                            break;
                            }

                        case FB::AnnotationTextBlockRunType_Fraction:
                            {
                            AnnotationFractionRunPtr fractionRun = AnnotationFractionRun::Create(document.GetDbR());
                            run = fractionRun;
                            
                            PRECONDITION(fbRun.has_fraction_denominatorContent(), ERROR);
                            fractionRun->SetDenominatorContent(fbRun.fraction_denominatorContent()->c_str());
                            
                            PRECONDITION(fbRun.has_fraction_numeratorContent(), ERROR);
                            fractionRun->SetNumeratorContent(fbRun.fraction_numeratorContent()->c_str());

                            break;
                            }

                        case FB::AnnotationTextBlockRunType_LineBreak:
                            {
                            AnnotationLineBreakRunPtr lineBreakRun = AnnotationLineBreakRun::Create(document.GetDbR());
                            run = lineBreakRun;

                            break;
                            }
                        }

                    if (!run.IsValid())
                        continue;

                    PRECONDITION(fbRun.has_styleId(), ERROR);
                    run->SetStyleId(AnnotationTextStyleId((uint64_t)fbRun.styleId()), SetAnnotationTextStyleOptions::Direct);
                    
                    if (fbRun.has_styleOverrides())
                        POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::DecodeFromFlatBuf(run->m_styleOverrides, *fbRun.styleOverrides()), ERROR);

                    paragraph->GetRunsR().push_back(run);
                    }
                }
            
            document.GetParagraphsR().push_back(paragraph);
            }
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockPersistence::DecodeFromFlatBuf(AnnotationTextBlockR document, ByteCP buffer, size_t numBytes)
    {
    auto const& fbDocument = *GetRoot<FB::AnnotationTextBlock>(buffer);
    return DecodeFromFlatBuf(document, fbDocument);
    }
