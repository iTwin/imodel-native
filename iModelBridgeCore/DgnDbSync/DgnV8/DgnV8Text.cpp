/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <DgnPlatform/Annotations/TextAnnotationElement.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
DgnFontId Converter::ConvertFontFromStyle(DgnV8Api::DgnTextStyle const& v8Style, DgnV8Api::TextStyleProperty v8Property)
    {
    DgnFont const* dbFont = nullptr; // start invalid, set if we can, fall back if we couldn't
    DgnV8Api::DgnFont const* v8Font = nullptr;
    v8Style.GetProperty(v8Property, v8Font);
    if (nullptr != v8Font)
        {
        uint32_t v8FontId;
        if (SUCCESS == v8Style.GetFile().GetDgnFontMapP()->GetFontNumber(v8FontId, *v8Font, false))
            dbFont = &_RemapV8Font(v8Style.GetFile(), v8FontId);
        }

    if (nullptr == dbFont)
        {
        if (nullptr != v8Font)
            {
            switch (v8Font->GetType())
                {
                case DgnV8Api::DgnFontType::Rsc: dbFont = &T_HOST.GetFontAdmin().GetLastResortRscFont(); break;
                case DgnV8Api::DgnFontType::Shx: dbFont = &T_HOST.GetFontAdmin().GetLastResortShxFont(); break;
                case DgnV8Api::DgnFontType::TrueType: dbFont = &T_HOST.GetFontAdmin().GetLastResortTrueTypeFont(); break;
                }
            }

        if (nullptr == dbFont)
            dbFont = &T_HOST.GetFontAdmin().GetAnyLastResortFont();
        }

    return m_dgndb->Fonts().AcquireId(*dbFont);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
void Converter::ConvertColorFromStyle(AnnotationColorType& dbColorType, ColorDef& dbColorValue, DgnV8Api::DgnTextStyle const& v8Style, DgnV8Api::TextStyleProperty v8Property, DgnV8Api::TextStyleProperty v8MaskProperty)
    {
    // Text in DgnDb always has a color; if DgnV8 style object has no color, have to fall back on something...
    dbColorType = AnnotationColorType::ByCategory;
    
    if (DgnV8Api::TextStyle_InvalidProperty != v8MaskProperty)
        {
        bool v8MaskValue;
        v8Style.GetProperty(v8MaskProperty, v8MaskValue);
        if (!v8MaskValue)
            return;
        }
    
    uint32_t v8Color;
    v8Style.GetProperty(v8Property, v8Color);
    if (DgnV8Api::COLOR_BYLEVEL == v8Color)
        return;

    DgnV8Api::IntColorDef v8ColorDef;
    if (SUCCESS != DgnV8Api::DgnColorMap::ExtractElementColorInfo(&v8ColorDef, nullptr, nullptr, nullptr, nullptr, v8Color, v8Style.GetFile()))
        return;

    dbColorType = AnnotationColorType::RGBA;
    dbColorValue = ColorDef(v8ColorDef.m_int);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr Converter::_ConvertV8TextStyle(DgnV8Api::DgnTextStyle const& v8Style)
    {
    AnnotationTextStylePtr dbStyle = AnnotationTextStyle::Create(*m_dgndb);
    dbStyle->SetName(Utf8String(v8Style.GetName().c_str()).c_str());
    
    // DgnV8 styles' units are in that of the dictionary model.
    double scaleFactor = (1.0 / v8Style.GetFile().GetDictionaryModel().GetModelInfo().GetUorPerMeter());

    AnnotationColorType dbColorType;
    ColorDef dbColorValue;
    ConvertColorFromStyle(dbColorType, dbColorValue, v8Style, DgnV8Api::TextStyle_Color, DgnV8Api::TextStyle_ColorFlag);
    
    DgnFontId dbFontId = ConvertFontFromStyle(v8Style, DgnV8Api::TextStyle_Font);

    double v8Height;
    v8Style.GetProperty(DgnV8Api::TextStyle_Height, v8Height);
    double dbHeight = (v8Height * scaleFactor);

    // DgnDb only supports Exact/Automatic line spacing; I don't think it's worth getting clever, just make something usable otherwise.
    // Note that DgnV8 *styles* report physical line spacing as a factor of text height (vs. text elements).
    uint32_t v8LineSpacingType;
    v8Style.GetProperty(DgnV8Api::TextStyle_LineSpacingType, v8LineSpacingType);
    double dbLineSpacingFactor = 0.5;
    if (((uint32_t)DgnV8Api::DgnLineSpacingType::Exact == v8LineSpacingType) || ((uint32_t)DgnV8Api::DgnLineSpacingType::Automatic == v8LineSpacingType))
        {
        double v8LineSpacingFactor;
        v8Style.GetProperty(DgnV8Api::TextStyle_LineSpacing, v8LineSpacingFactor);

        // "0.0" means something special DgnV8: the displacement from the top of one line to the top of the next is maximum unit height + maximum descender.
        // In DgnDb, linespacing is the extra padding on top of the unit height.
        if (0.0 == v8LineSpacingFactor)
            {
            DgnV8Api::DgnFont const* v8Font = nullptr;
            v8Style.GetProperty(DgnV8Api::TextStyle_Font, v8Font);
            if (nullptr != v8Font)
                v8LineSpacingFactor = v8Font->GetDescenderRatio();
            }

        dbLineSpacingFactor = v8LineSpacingFactor;
        }

    bool isBold;
    v8Style.GetProperty(DgnV8Api::TextStyle_Bold, isBold);

    bool isItalic;
    v8Style.GetProperty(DgnV8Api::TextStyle_Italics, isItalic);

    bool isUnderlined;
    v8Style.GetProperty(DgnV8Api::TextStyle_Underline, isUnderlined);

    // DgnV8 has a width factor property, but it is typically not set; physical width is the only reliable property to query.
    double v8Width;
    v8Style.GetProperty(DgnV8Api::TextStyle_Width, v8Width);
    double dbWidthFactor = ((v8Width * scaleFactor) / dbHeight);

    dbStyle->SetColorType(dbColorType);
    dbStyle->SetColorValue(dbColorValue);
    dbStyle->SetFontId(dbFontId);
    dbStyle->SetHeight(dbHeight);
    dbStyle->SetLineSpacingFactor(dbLineSpacingFactor);
    dbStyle->SetIsBold(isBold);
    dbStyle->SetIsItalic(isItalic);
    dbStyle->SetIsUnderlined(isUnderlined);
    dbStyle->SetWidthFactor(dbWidthFactor);

    // Other properties are not present in V8 text styles; keep defaults.
    
    return dbStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
DgnElementId Converter::_GetOrCreateTextStyleNoneId(DgnV8Api::DgnFile& v8File)
    {
    // The strategy for V8's Style (none) is to create a placeholder so-to-speak from V8's active settings, and then use it but override every setting in the text.
    // The fact we use V8's active settings to seed the style is arbitrary and generally irrelevant to the display, but I'm not sure there's a more reasonable alternative.
    // This allows for DgnDb's requirement to have a style, yet the elements will act like V8 if the user ever did get ahold of the style and tweaked it.

    if (!m_textStyleNoneId.IsValid())
        {
        Utf8String defaultName = ConverterDataStrings::GetString(ConverterDataStrings::V8StyleNone());
        if (defaultName.empty())
            {
            BeDataAssert(false);
            defaultName = "V8StyleNone";
            }
        
        AnnotationTextStyleCPtr dbDefaultStyle = AnnotationTextStyle::Get(*m_dgndb, defaultName.c_str());
        if (!dbDefaultStyle.IsValid())
            {
            DgnV8Api::DgnTextStylePtr v8ActiveStyle = DgnV8Api::DgnTextStyle::GetSettings(v8File);
            auto newDbDefaultStyle = _ConvertV8TextStyle(*v8ActiveStyle);
            newDbDefaultStyle->SetName(defaultName.c_str());
            newDbDefaultStyle->SetDescription(ConverterDataStrings::GetString(ConverterDataStrings::V8StyleNoneDescription()).c_str());

            dbDefaultStyle = newDbDefaultStyle->Insert();
            if (!dbDefaultStyle.IsValid())
                {
                ReportError(IssueCategory::MissingData(), Issue::Error(), L"V8StyleNone");
                return DgnElementId();
                }
            }
        
        m_textStyleNoneId = dbDefaultStyle->GetElementId();
        }

    return m_textStyleNoneId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
DgnElementId Converter::_RemapV8TextStyle(DgnV8Api::DgnFile& v8File, DgnV8Api::ElementId v8Id)
    {
    DgnV8Api::DgnTextStylePtr v8Style = DgnV8Api::DgnTextStyle::GetByID(v8Id, v8File);
    if (!v8Style.IsValid())
        return _GetOrCreateTextStyleNoneId(v8File);

    auto dbStyle = AnnotationTextStyle::Get(*m_dgndb, Utf8String(v8Style->GetName().c_str()).c_str());
    if (!dbStyle.IsValid())
        {
        auto newDbStyle = _ConvertV8TextStyle(*v8Style);
        newDbStyle->Insert();
        dbStyle = newDbStyle;
        }
    
    return dbStyle->GetElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
void Converter::ConvertTextStyleForDocument(AnnotationTextBlock& dbText, DgnV8Api::DgnTextStyle const& v8Style)
    {
    DgnElementId dbStyleId;
    DgnV8Api::TextStylePropertyMaskPtr v8Overrides;

    if (0 != v8Style.GetID())
        {
        // A real style? Get a real ID and compute overrides.
        dbStyleId = _RemapV8TextStyle(v8Style.GetFile(), v8Style.GetID());

        DgnV8Api::DgnTextStylePtr v8FileStyle = DgnV8Api::DgnTextStyle::GetByID(v8Style.GetID(), v8Style.GetFile());
        v8Overrides = v8Style.Compare(*v8FileStyle);
        }
    else
        {
        // Style (none)? Grab the default style and treat everything as overrides.
        dbStyleId = _GetOrCreateTextStyleNoneId(v8Style.GetFile());
        
        v8Overrides = DgnV8Api::TextStylePropertyMask::CreatePropMask();
        v8Overrides->SetAll();
        }
    
    // Set the ID, and then any required overrides.
    dbText.SetStyleId(dbStyleId, SetAnnotationTextStyleOptions::DontPropogate);
    
    // DgnV8 styles' units are in that of the dictionary model.
    double scaleFactor = (1.0 / v8Style.GetFile().GetDictionaryModel().GetModelInfo().GetUorPerMeter());
    
    // This is needed for several properties, so compute up-front and re-use.
    double v8Height;
    v8Style.GetProperty(DgnV8Api::TextStyle_Height, v8Height);
    double dbHeight = (v8Height * scaleFactor);

    if (v8Overrides->GetPropertyFlag(DgnV8Api::TextStyle_Height))
        dbText.GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::Height, dbHeight);

    if (v8Overrides->GetPropertyFlag(DgnV8Api::TextStyle_LineSpacing))
        {
        // DgnDb only supports Exact/Automatic line spacing; I don't think it's worth getting clever, just make something usable otherwise.
        // Note that DgnV8 *styles* report physical line spacing as a factor of text height (vs. text elements).
        uint32_t v8LineSpacingType;
        v8Style.GetProperty(DgnV8Api::TextStyle_LineSpacingType, v8LineSpacingType);
        double dbLineSpacingFactor = 0.5;
        if (((uint32_t)DgnV8Api::DgnLineSpacingType::Exact == v8LineSpacingType) || ((uint32_t)DgnV8Api::DgnLineSpacingType::Automatic == v8LineSpacingType))
            {
            double v8LineSpacingFactor;
            v8Style.GetProperty(DgnV8Api::TextStyle_LineSpacing, v8LineSpacingFactor);

            // "0.0" means something special DgnV8: the displacement from the top of one line to the top of the next is maximum unit height + maximum descender.
            // In DgnDb, linespacing is the extra padding on top of the unit height.
            if (0.0 == v8LineSpacingFactor)
                {
                DgnV8Api::DgnFont const* v8Font = nullptr;
                v8Style.GetProperty(DgnV8Api::TextStyle_Font, v8Font);
                if (nullptr != v8Font)
                    v8LineSpacingFactor = v8Font->GetDescenderRatio();
                }

            dbLineSpacingFactor = v8LineSpacingFactor;
            }

        dbText.GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::LineSpacingFactor, dbLineSpacingFactor);
        }

    if (v8Overrides->GetPropertyFlag(DgnV8Api::TextStyle_Width))
        {
        double v8Width;
        v8Style.GetProperty(DgnV8Api::TextStyle_Width, v8Width);
        double dbWidthFactor = ((v8Width * scaleFactor) / dbHeight);

        dbText.GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::WidthFactor, dbWidthFactor);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
void Converter::ConvertTextStyleForParagraph(AnnotationParagraph& dbParagraph, DgnV8Api::DgnTextStyle const& v8Style)
    {
    // DgnDb paragraphs don't currently have any unique properties, but track the style ID for future reference.
    DgnElementId dbStyleId;
    if (0 != v8Style.GetID())
        {
        // A real style? Get a real ID.
        dbStyleId = _RemapV8TextStyle(v8Style.GetFile(), v8Style.GetID());
        }
    else
        {
        // Style (none)? Grab the default style.
        dbStyleId = _GetOrCreateTextStyleNoneId(v8Style.GetFile());
        }

    // Set the ID.
    dbParagraph.SetStyleId(dbStyleId, SetAnnotationTextStyleOptions::DontPropogate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
void Converter::ConvertTextStyleForRun(AnnotationRunBase& dbRun, DgnV8Api::DgnTextStyle const& v8Style)
    {
    DgnElementId dbStyleId;
    DgnV8Api::TextStylePropertyMaskPtr v8Overrides;

    if (0 != v8Style.GetID())
        {
        // A real style? Get a real ID and compute overrides.
        dbStyleId = _RemapV8TextStyle(v8Style.GetFile(), v8Style.GetID());

        DgnV8Api::DgnTextStylePtr v8FileStyle = DgnV8Api::DgnTextStyle::GetByID(v8Style.GetID(), v8Style.GetFile());
        v8Overrides = v8Style.Compare(*v8FileStyle);
        }
    else
        {
        // Style (none)? Grab the default style and treat everything as overrides.
        dbStyleId = _GetOrCreateTextStyleNoneId(v8Style.GetFile());

        v8Overrides = DgnV8Api::TextStylePropertyMask::CreatePropMask();
        v8Overrides->SetAll();
        }

    // The following properties are "style" properties (i.e. potential overrides) in DgnDb, but "DOM" properties in DgnV8; therefore we must independently preserve them.
    // N.B. The following cannot actually come from DgnV8, so we don't have to bother preserving them: SubScriptOffsetFactor, SubScriptScale, SuperScriptOffsetFactor, SuperScriptScale.
    bool hadStackedFractionScaleOverride = dbRun.GetStyleOverrides().HasProperty(AnnotationTextStyleProperty::StackedFractionScale);
    AnnotationPropertyBag::T_Real stackedFractionScaleOverride = hadStackedFractionScaleOverride ? dbRun.GetStyleOverrides().GetRealProperty(AnnotationTextStyleProperty::StackedFractionScale) : 0.0;
    bool hadStackedFractionTypeOverride = dbRun.GetStyleOverrides().HasProperty(AnnotationTextStyleProperty::StackedFractionType);
    AnnotationPropertyBag::T_Integer stackedFractionTypeOverride = hadStackedFractionTypeOverride ? dbRun.GetStyleOverrides().GetIntegerProperty(AnnotationTextStyleProperty::StackedFractionType) : 0;
    
    // Set the ID, and then any required overrides. This call, by-default, will clear all overrides already on the run.
    dbRun.SetStyleId(dbStyleId, SetAnnotationTextStyleOptions::DontPropogate);

    if (v8Overrides->GetPropertyFlag(DgnV8Api::TextStyle_Color))
        {
        // Ignore the color flag value, because the TextBlock-based API will always report an accurante color for it, which DgnDb needs.
        AnnotationColorType dbColorType;
        ColorDef dbColorValue;
        ConvertColorFromStyle(dbColorType, dbColorValue, v8Style, DgnV8Api::TextStyle_Color, DgnV8Api::TextStyle_InvalidProperty);

        dbRun.GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::ColorType, (AnnotationTextStylePropertyBag::T_Integer)dbColorType);
        if (AnnotationColorType::RGBA == dbColorType)
            dbRun.GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::ColorValue, (AnnotationTextStylePropertyBag::T_Integer)dbColorValue.GetValue());
        }

    if (v8Overrides->GetPropertyFlag(DgnV8Api::TextStyle_Font))
        {
        DgnFontId dbFontId = ConvertFontFromStyle(v8Style, DgnV8Api::TextStyle_Font);
        dbRun.GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::FontId, (AnnotationTextStylePropertyBag::T_Integer)dbFontId.GetValue());
        }

    if (v8Overrides->GetPropertyFlag(DgnV8Api::TextStyle_Bold))
        {
        bool v8IsBold;
        v8Style.GetProperty(DgnV8Api::TextStyle_Bold, v8IsBold);
        dbRun.GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::IsBold, v8IsBold ? 1 : 0);
        }

    if (v8Overrides->GetPropertyFlag(DgnV8Api::TextStyle_Italics))
        {
        bool v8IsItalic;
        v8Style.GetProperty(DgnV8Api::TextStyle_Italics, v8IsItalic);
        dbRun.GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, v8IsItalic ? 1 : 0);
        }

    if (v8Overrides->GetPropertyFlag(DgnV8Api::TextStyle_Underline))
        {
        bool v8IsUnderlined;
        v8Style.GetProperty(DgnV8Api::TextStyle_Underline, v8IsUnderlined);
        dbRun.GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::IsUnderlined, v8IsUnderlined ? 1 : 0);
        }

    // Other properties are not present in V8 text styles; keep defaults.
    
    // Pursuant to above, restore DgnDb-only overrides.
    if (hadStackedFractionScaleOverride)
        dbRun.GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::StackedFractionScale, stackedFractionScaleOverride);
    
    if (hadStackedFractionTypeOverride)
        dbRun.GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::StackedFractionType, stackedFractionTypeOverride);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2018
//---------------------------------------------------------------------------------------
static AnnotationTextBlock::HorizontalJustification remapV8Justification(DgnV8Api::TextElementJustification v8Justification)
    {
    switch (v8Justification)
        {
        case DgnV8Api::TextElementJustification::CenterBaseline:
        case DgnV8Api::TextElementJustification::CenterCap:
        case DgnV8Api::TextElementJustification::CenterDescender:
        case DgnV8Api::TextElementJustification::CenterMiddle:
        case DgnV8Api::TextElementJustification::CenterTop:
            return AnnotationTextBlock::HorizontalJustification::Center;

        case DgnV8Api::TextElementJustification::LeftBaseline:
        case DgnV8Api::TextElementJustification::LeftCap:
        case DgnV8Api::TextElementJustification::LeftDescender:
        case DgnV8Api::TextElementJustification::LeftMarginBaseline:
        case DgnV8Api::TextElementJustification::LeftMarginCap:
        case DgnV8Api::TextElementJustification::LeftMarginDescender:
        case DgnV8Api::TextElementJustification::LeftMarginMiddle:
        case DgnV8Api::TextElementJustification::LeftMarginTop:
        case DgnV8Api::TextElementJustification::LeftMiddle:
        case DgnV8Api::TextElementJustification::LeftTop:
            return AnnotationTextBlock::HorizontalJustification::Left;

        case DgnV8Api::TextElementJustification::RightBaseline:
        case DgnV8Api::TextElementJustification::RightCap:
        case DgnV8Api::TextElementJustification::RightDescender:
        case DgnV8Api::TextElementJustification::RightMarginBaseline:
        case DgnV8Api::TextElementJustification::RightMarginCap:
        case DgnV8Api::TextElementJustification::RightMarginDescender:
        case DgnV8Api::TextElementJustification::RightMarginMiddle:
        case DgnV8Api::TextElementJustification::RightMarginTop:
        case DgnV8Api::TextElementJustification::RightMiddle:
        case DgnV8Api::TextElementJustification::RightTop:
            return AnnotationTextBlock::HorizontalJustification::Right;
        }

    return AnnotationTextBlock::HorizontalJustification::Left;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
AnnotationTextBlockPtr Converter::_ConvertV8TextBlock(DgnV8Api::TextBlock const& v8Text)
    {
    AnnotationTextBlockPtr dbText = AnnotationTextBlock::Create(*m_dgndb);

    // V8 and Db store different properties at different DOM levels.
    // When Db subsumes at a higher level, just assume the first child's properties and homogenize.
    DgnV8Api::DgnTextStylePtr v8BlockStyle = v8Text.Begin().CreateEffectiveTextStyle();
    ConvertTextStyleForDocument(*dbText, *v8BlockStyle);

    // These are conveyed directly on the AnnotationTextBlock, not its style.
    dbText->SetJustification(remapV8Justification(v8BlockStyle->GetJustification()));
    dbText->SetDocumentWidth(v8Text.GetProperties().GetDocumentWidth() / v8Text.GetDgnModelR().GetModelInfo().GetUorPerMeter());

    DgnV8Api::ParagraphRange v8Paragraphs(v8Text);
    for (DgnV8Api::ParagraphIterator v8ParagraphIter = v8Paragraphs.begin(); v8ParagraphIter != v8Paragraphs.end(); ++v8ParagraphIter)
        {
        DgnV8Api::CaretPtr v8ParagraphStart = v8ParagraphIter.ToCaret();
        DgnV8Api::CaretPtr v8ParagraphEnd = v8ParagraphStart->Clone();
        v8ParagraphEnd->MoveToBackOfParagraph();

        AnnotationParagraphPtr dbParagraph = AnnotationParagraph::Create(*m_dgndb);
        DgnV8Api::DgnTextStylePtr v8ParagraphStyle = v8ParagraphStart->CreateEffectiveTextStyle();
        ConvertTextStyleForParagraph(*dbParagraph, *v8ParagraphStyle);
        
        DgnV8Api::RunRange v8Runs(*v8ParagraphStart, *v8ParagraphEnd);
        for (DgnV8Api::RunIterator v8RunIter = v8Runs.begin(); v8RunIter != v8Runs.end(); ++v8RunIter)
            {
            DgnV8Api::Run const& v8Run = *v8RunIter;
            AnnotationRunBasePtr dbRun;

            DgnV8Api::CharStream const* v8CharStream = nullptr;
            DgnV8Api::Fraction const* v8Fraction = nullptr;
            DgnV8Api::LineBreak const* v8LineBreak = nullptr;
            if (nullptr != (v8CharStream = dynamic_cast<DgnV8Api::CharStream const*>(&v8Run)))
                {
                AnnotationTextRunPtr dbTextRun = AnnotationTextRun::Create(*m_dgndb);
                dbTextRun->SetContent(Utf8String(v8CharStream->GetString().c_str()).c_str());
                dbRun = dbTextRun;
                }
            else if (nullptr != (v8LineBreak = dynamic_cast<DgnV8Api::LineBreak const*>(&v8Run)))
                {
                AnnotationLineBreakRunPtr dbLineBreak = AnnotationLineBreakRun::Create(*m_dgndb);
                dbRun = dbLineBreak;
                }
            else if (nullptr != (v8Fraction = dynamic_cast<DgnV8Api::Fraction const*>(&v8Run)))
                {
                AnnotationFractionRunPtr dbFraction = AnnotationFractionRun::Create(*m_dgndb);
                
                if (nullptr != v8Fraction->GetDenominator())
                    dbFraction->SetDenominatorContent(Utf8String(v8Fraction->GetDenominator()->GetString().c_str()).c_str());
                
                if (nullptr != v8Fraction->GetNumerator())
                    dbFraction->SetNumeratorContent(Utf8String(v8Fraction->GetNumerator()->GetString().c_str()).c_str());

                AnnotationStackedFractionType dbFractionType = AnnotationStackedFractionType::HorizontalBar;
                if (DgnV8Api::StackedFractionType::DiagonalBar == v8Fraction->GetFractionType())
                    dbFractionType = AnnotationStackedFractionType::DiagonalBar;

                dbFraction->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::StackedFractionType, (AnnotationTextStylePropertyBag::T_Integer)dbFractionType);

                dbRun = dbFraction;
                }
            else
                {
                // These DgnV8 run types remain:
                //  ParagraphBreak - we don't use this; the mere presence of another paragraph object denotes this
                //  Tab - We don't support tabs, and don't currently intend to emulate them another way
                // Note that the following more derived sub-classes were subsumed above:
                //  EdfCharStream (by CharStream) - We don't support enter data fields, just take their underlying string data
                //  DiagonalBarFraction (by Fraction) - We only have one type of Fraction class, and we set its type to match
                //  HorizontalBarFraction (by Fraction) - We only have one type of Fraction class, and we set its type to match
                //  NoBarFraction (by Fraction) - We don't support this kind of fraction, but it's converted as a horizontal bar fraction to retain data

                // Catch any unexpected types in debug builds.                
                BeAssert(
                    nullptr != dynamic_cast<DgnV8Api::ParagraphBreak const*>(&v8Run)
                    || nullptr != dynamic_cast<DgnV8Api::Tab const*>(&v8Run)
                    || nullptr != dynamic_cast<DgnV8Api::TextBlockNode const*>(&v8Run)                    
                    );
                
                continue;
                }

            if (dbRun.IsValid())
                {
                DgnV8Api::DgnTextStylePtr v8RunStyle = v8RunIter.ToCaret()->CreateEffectiveTextStyle();
                ConvertTextStyleForRun(*dbRun, *v8RunStyle);

                dbParagraph->AppendRun(*dbRun);
                }
            }

        dbText->AppendParagraph(*dbParagraph);
        }

    return dbText;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
void ConvertV8TextToDgnDbExtension::Register()
    {
    ConvertV8TextToDgnDbExtension* instance = new ConvertV8TextToDgnDbExtension();
    RegisterExtension(DgnV8Api::TextElemHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::TextNodeHandler::GetInstance(), *instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2017
//---------------+---------------+---------------+---------------+---------------+-------
BisConversionRule ConvertV8TextToDgnDbExtension::_DetermineBisConversionRule(DgnV8EhCR v8eh, DgnDbR dgndb, BisConversionTargetModelInfoCR)
    {
    DgnV8Api::DisplayHandler* v8DisplayHandler = v8eh.GetDisplayHandler();
    if (nullptr == v8DisplayHandler)
        return BisConversionRule::ToDefaultBisBaseClass;
    return BisConversionRule::ToAspectOnly;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
void ConvertV8TextToDgnDbExtension::_DetermineElementParams(DgnClassId& dbClass, DgnCode& dbCode, DgnCategoryId& dbCategory, DgnV8EhCR v8Eh, Converter& converter, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const& mm)
    {
    DgnV8Api::DisplayHandler* v8DisplayHandler = v8Eh.GetDisplayHandler();
    if (nullptr == v8DisplayHandler)
        return;
    
    dbClass = mm.GetDgnModel().Is3d() ? TextAnnotation3d::QueryDgnClassId(converter.GetDgnDb()) : TextAnnotation2d::QueryDgnClassId(converter.GetDgnDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2018
//---------------------------------------------------------------------------------------
static TextAnnotation::AnchorPoint remapV8AnchorPoint(DgnV8Api::TextElementJustification v8Justification)
    {
    switch (v8Justification)
        {
        
        case DgnV8Api::TextElementJustification::LeftCap:
        case DgnV8Api::TextElementJustification::LeftMarginCap:
        case DgnV8Api::TextElementJustification::LeftMarginTop:
        case DgnV8Api::TextElementJustification::LeftTop:
            return TextAnnotation::AnchorPoint::LeftTop;

        case DgnV8Api::TextElementJustification::LeftMarginMiddle:
        case DgnV8Api::TextElementJustification::LeftMiddle:
            return TextAnnotation::AnchorPoint::LeftMiddle;

        case DgnV8Api::TextElementJustification::LeftBaseline:
        case DgnV8Api::TextElementJustification::LeftDescender:
        case DgnV8Api::TextElementJustification::LeftMarginBaseline:
        case DgnV8Api::TextElementJustification::LeftMarginDescender:
            return TextAnnotation::AnchorPoint::LeftBottom;

        case DgnV8Api::TextElementJustification::CenterCap:
        case DgnV8Api::TextElementJustification::CenterTop:
            return TextAnnotation::AnchorPoint::CenterTop;

        case DgnV8Api::TextElementJustification::CenterMiddle:
            return TextAnnotation::AnchorPoint::CenterMiddle;

        case DgnV8Api::TextElementJustification::CenterBaseline:
        case DgnV8Api::TextElementJustification::CenterDescender:
            return TextAnnotation::AnchorPoint::CenterBottom;

        case DgnV8Api::TextElementJustification::RightCap:
        case DgnV8Api::TextElementJustification::RightMarginCap:
        case DgnV8Api::TextElementJustification::RightMarginTop:
        case DgnV8Api::TextElementJustification::RightTop:
            return TextAnnotation::AnchorPoint::RightTop;

        case DgnV8Api::TextElementJustification::RightMarginMiddle:
        case DgnV8Api::TextElementJustification::RightMiddle:
            return TextAnnotation::AnchorPoint::RightMiddle;

        case DgnV8Api::TextElementJustification::RightBaseline:
        case DgnV8Api::TextElementJustification::RightDescender:
        case DgnV8Api::TextElementJustification::RightMarginBaseline:
        case DgnV8Api::TextElementJustification::RightMarginDescender:
            return TextAnnotation::AnchorPoint::RightBottom;
        }

    return TextAnnotation::AnchorPoint::LeftTop;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
void ConvertV8TextToDgnDbExtension::_ProcessResults(ElementConversionResults& results, DgnV8EhCR v8Eh, ResolvedModelMapping const& v8mm, Converter& converter)
    {
    // We only ever expect a single result element; text has no public children.
    if (!results.m_element.IsValid() || !results.m_childElements.empty())
        {
        BeAssert (false);
        return;
        }
    
    // _DetermineElementParameters told the caller to create a text-based element.
    TextAnnotation2dP dbTextElem = dynamic_cast<TextAnnotation2dP>(results.m_element.get());
    TextAnnotation3dP dbSpatialTextElem = ((nullptr != dbTextElem) ? nullptr : dynamic_cast<TextAnnotation3dP>(results.m_element.get()));

    if ((nullptr == dbTextElem) && (nullptr == dbSpatialTextElem))
        {
        BeAssert(false);
        return;
        }

    DgnV8Api::EditElementHandle v8eeh(v8Eh, true);
    // if the v8 element thinks it is 3d, but it is being inserted into a 2d model, we need to flatten it
    DgnV8Api::DisplayHandler* v8DisplayHandler = v8Eh.GetDisplayHandler();
    if (v8DisplayHandler->Is3dElem(v8Eh.GetElementCP()) && !v8mm.GetDgnModel().Is3d())
        {
        Bentley::DVec3d flattenDir = Bentley::DVec3d::From(0, 0, 1);
        auto transform = v8mm.GetTransform();
        v8DisplayHandler->ConvertTo2d(v8eeh, (Bentley::Transform const&)(transform), flattenDir);
        LOG.infov("Flattening 3d text element %d to a 2d element since it is in a 2d model.\n", v8Eh.GetElementId());
        }

    //.............................................................................................
    // Get the V8 TextBlock
    DgnV8Api::ITextQuery const* v8TextQuery = v8eeh.GetITextQuery();
    if (nullptr == v8TextQuery)
        return;

    DgnV8Api::ITextQueryOptionsPtr v8TextQueryOptions = DgnV8Api::ITextQueryOptions::CreateDefault();
    DgnV8Api::T_ITextPartIdPtrVector v8TextPartIds;
    v8TextQuery->GetTextPartIds(v8eeh, *v8TextQueryOptions, v8TextPartIds);

    if (1 != v8TextPartIds.size())
        return;

    DgnV8Api::TextBlockPtr v8Text = v8TextQuery->GetTextPart(v8eeh, *v8TextPartIds[0]);
    if (!v8Text.IsValid())
        return;

    //.............................................................................................
    // Convert to a DB TextBlock
    DgnDbR db = converter.GetDgnDb();
    AnnotationTextBlockPtr dbText = converter._ConvertV8TextBlock(*v8Text);
    if (!dbText.IsValid())
        return;

    TextAnnotation annotation(db);
    annotation.SetText(dbText.get());
    annotation.SetAnchorPoint(remapV8AnchorPoint(v8Text->Begin().CreateEffectiveTextStyle()->GetJustification()));

    //.............................................................................................
    // Add the text data to the element.
    if (nullptr != dbTextElem)
        dbTextElem->SetAnnotation(&annotation);
    else
        dbSpatialTextElem->SetAnnotation(&annotation);
    
    // This class should only be used during DgnV8 conversion.
    // As such, we want to leave our caller's proxy placement and graphics on the element.
    TextAnnotationDataP dbTextItem = TextAnnotationData::GetP(*results.m_element);
    dbTextItem->m_isGeometrySuppressed = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  01/16
//--------------+---------------+---------------+---------------+---------------+--------
bool ConvertV8TextToDgnDbExtension::_GetBasisTransform(Bentley::Transform& transform, DgnV8EhCR eh, Converter& converter)
    {
    return eh.GetDisplayHandler()->GetBasisTransform(eh, transform);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
