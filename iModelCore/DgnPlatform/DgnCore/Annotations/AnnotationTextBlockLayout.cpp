//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationTextBlockLayout.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static DRange2d createZeroDRange2d()
    {
    DRange2d range;
    range.low.Zero();
    range.high.Zero();

    return range;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static DRange2d createDRange2dWithHeight(double height)
    {
    DRange2d range;
    range.low.Zero();
    range.high.Zero();
    
    range.high.y = height;

    return range;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static void scaleRange(DRange2dR range, double scale)
    {
    range.low.Scale(scale);
    range.high.Scale(scale);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static void moveRange(DRange2dR range, DVec2dCR offset)
    {
    range.low.Add(offset);
    range.high.Add(offset);
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationLayoutRun::AnnotationLayoutRun(AnnotationRunBaseCR seedRun) :
    T_Super()
    {
    m_seedRun = &seedRun;
    m_charOffset = 0;
    m_isValid = false;
    m_layoutRange.Init();
    m_offsetFromLine.Zero();

    switch (m_seedRun->GetType())
        {
        case AnnotationRunType::Text: m_numChars = ((AnnotationTextRunCR)seedRun).GetContent().size(); break;
        case AnnotationRunType::Fraction: m_numChars = 1; break;
        case AnnotationRunType::LineBreak: m_numChars = 0; break;

        default:
            BeAssert(false); // Unknown/unexpected AnnotationRunType.
            m_numChars = 0;
            break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationLayoutRun::CopyFrom(AnnotationLayoutRunCR rhs)
    {
    m_seedRun = rhs.m_seedRun;
    m_charOffset = rhs.m_charOffset;
    m_numChars = rhs.m_numChars;
    m_isValid = rhs.m_isValid;
    m_layoutRange = rhs.m_layoutRange;
    m_subRanges = rhs.m_subRanges;
    m_offsetFromLine = rhs.m_offsetFromLine;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLayoutRun::GetSubRange(DRange2dR subRange, SubRange subRangeType) const
    {
    const_cast<AnnotationLayoutRunP>(this)->Update();
    
    auto foundSubRange = m_subRanges.find(subRangeType);
    if (m_subRanges.end() == foundSubRange)
        return ERROR;
    
    subRange = foundSubRange->second;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static DRange2d computeRangeForText(DRange2dP justificationRange, Utf8CP chars, size_t numChars, AnnotationTextStyleCR effectiveStyle, AnnotationTextRunSubSuperScript subSuperScript)
    {
    // I'm doing this so that blank runs space correctly without special casing later.
    DRange2d range = createDRange2dWithHeight(effectiveStyle.GetHeight());
    
    if (0 == numChars)
        return range;

    DgnFontCR font = effectiveStyle.ResolveFont();
    
    DgnGlyphLayoutContext layoutContext;
    layoutContext.m_string = Utf8String(chars, numChars);
    layoutContext.m_drawSize = DPoint2d::From(effectiveStyle.GetHeight(), (effectiveStyle.GetHeight() * effectiveStyle.GetWidthFactor()));
    layoutContext.m_isBold = effectiveStyle.IsBold();
    layoutContext.m_isItalic = (effectiveStyle.IsItalic() && DgnFontType::TrueType == font.GetType());
    
    DgnGlyphLayoutResult layoutResult;
    if (SUCCESS != font.LayoutGlyphs(layoutResult, layoutContext))
        {
        BeAssert(false);
        return range;
        }
    
    range = layoutResult.m_range;

    if (NULL != justificationRange)
        *justificationRange = layoutResult.m_justificationRange;

    // Completely ignore if both.
    if (AnnotationTextRunSubSuperScript::Neither != subSuperScript)
        {
        DVec2d offset = DVec2d::From(0.0, 0.0);
        double scale = 1.0;
        
        if (AnnotationTextRunSubSuperScript::SubScript == subSuperScript)
            {
            offset = DVec2d::From(0.0, (effectiveStyle.GetHeight() * effectiveStyle.GetSubScriptOffsetFactor()));
            scale = effectiveStyle.GetSubScriptScale();
            }
        else if (AnnotationTextRunSubSuperScript::SuperScript == subSuperScript)
            {
            offset = DVec2d::From(0.0, (effectiveStyle.GetHeight() * effectiveStyle.GetSuperScriptOffsetFactor()));
            scale = effectiveStyle.GetSuperScriptScale();
            }
        
        scaleRange(range, scale);
        moveRange(range, offset);
        
        if (NULL != justificationRange)
            {
            scaleRange(*justificationRange, scale);
            moveRange(*justificationRange, offset);
            }
        }
    
    return range;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static DRange2d computeRangeForTextRun(DRange2dP justificationRange, AnnotationTextRunCR run, size_t charOffset, size_t numChars)
    {
    return computeRangeForText(justificationRange, run.GetContent().c_str() + charOffset, numChars, *run.CreateEffectiveStyle(), run.GetSubSuperScript());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static DRange2d computeRangeForFractionRun(DRange2dP numeratorRange, DRange2dP denominatorRange, AnnotationFractionRunCR run)
    {
    AnnotationTextStylePtr effectiveStyle = run.CreateEffectiveStyle();

    DRange2d localNumeratorRange;
    if (NULL == numeratorRange)
        numeratorRange = &localNumeratorRange;

    DRange2d localDenominatorRange;
    if (NULL == denominatorRange)
        denominatorRange = &localDenominatorRange;

    *numeratorRange = computeRangeForText(NULL, run.GetNumeratorContent().c_str(), run.GetNumeratorContent().size(), *effectiveStyle, AnnotationTextRunSubSuperScript::Neither);
    scaleRange(*numeratorRange, effectiveStyle->GetStackedFractionScale());
    
    *denominatorRange = computeRangeForText(NULL, run.GetDenominatorContent().c_str(), run.GetDenominatorContent().size(), *effectiveStyle, AnnotationTextRunSubSuperScript::Neither);
    scaleRange(*denominatorRange, effectiveStyle->GetStackedFractionScale());
    
    switch (effectiveStyle->GetStackedFractionType())
        {
        case AnnotationStackedFractionType::HorizontalBar:
            if (numeratorRange->XLength() > denominatorRange->XLength())
                moveRange(*denominatorRange, DVec2d::From(((numeratorRange->XLength() - denominatorRange->XLength()) / 2.0), 0.0));
            else
                moveRange(*numeratorRange, DVec2d::From(((denominatorRange->XLength() - numeratorRange->XLength()) / 2.0), 0.0));

            moveRange(*numeratorRange, DVec2d::From(0.0, (1.5 * denominatorRange->YLength())));

            break;

        case AnnotationStackedFractionType::DiagonalBar:
            moveRange(*numeratorRange, DVec2d::From(0.0, (denominatorRange->YLength())));
            moveRange(*denominatorRange, DVec2d::From(numeratorRange->XLength(), 0.0));

            break;

        default:
            BeAssert(false); // Unknown / unexpected AnnotationStackedFractionType.
            break;
        }

    DRange2d range = createZeroDRange2d();
    range.Extend(*numeratorRange);
    range.Extend(*denominatorRange);
    
    return range;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static DRange2d computeRangeForLineBreakRun(AnnotationLineBreakRunCR run)
    {
    // I'm doing this so that blank lines space correctly without special casing later.
    return createDRange2dWithHeight(run.CreateEffectiveStyle()->GetHeight());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationLayoutRun::Update()
    {
    if (m_isValid)
        return;
    
    m_isValid = true;

    m_subRanges.clear();

    switch (m_seedRun->GetType())
        {
        case AnnotationRunType::Text:
            {
            DRange2d justificationRange;
            m_layoutRange = computeRangeForTextRun(&justificationRange, (AnnotationTextRunCR)*m_seedRun, m_charOffset, m_numChars);
            
            m_subRanges[SubRange::JustificationRange] = justificationRange;
            
            break;
            }
        case AnnotationRunType::Fraction:
            {
            DRange2d numeratorRange;
            DRange2d denominatorRange;
            m_layoutRange = computeRangeForFractionRun(&numeratorRange, &denominatorRange, (AnnotationFractionRunCR)*m_seedRun);

            m_subRanges[SubRange::FractionDenominator] = denominatorRange;
            m_subRanges[SubRange::FractionNumerator] = numeratorRange;

            break;
            }
        case AnnotationRunType::LineBreak:
            {
            m_layoutRange = computeRangeForLineBreakRun((AnnotationLineBreakRunCR)*m_seedRun);
            break;
            }
        default:
            {
            BeAssert(false); // Unknown/unexpected AnnotationRunType.
            break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
bool AnnotationLayoutRun::CanWrap() const
    {
    // Only text runs can be split.
    return (AnnotationRunType::Text == m_seedRun->GetType());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
bool AnnotationLayoutRun::Wrap(AnnotationLayoutRunPtr& leftOver, double availableWidth, bool shouldForceLeadingUnit)
    {
#ifndef WIP_FONT
    // WordBoundaryServices is removed; need to re-write to use ICU on every platform instead of using it to branch per-platform.
    return false;
#else
    leftOver = NULL;
    
    PRECONDITION(CanWrap(), false);
    
    AnnotationTextRunCR textRun = (AnnotationTextRunCR)*m_seedRun;
    AnnotationTextStylePtr effectiveStyle = textRun.CreateEffectiveStyle();
    
    // WordBoundaryServices is based on WString... do operations in terms of WString, then translate back...
    WString testContentW;
    BeStringUtilities::Utf8ToWChar(testContentW, textRun.GetContent().c_str() + m_charOffset, m_numChars);

    size_t breakPosW = 0;
    size_t peekBreakPosW;
    double runningWidth = 0.0;
    while ((peekBreakPosW = WordBoundaryServices::FindNextWordBoundary(WordBoundaryReason::WordWrapping, testContentW.c_str(), testContentW.size(), breakPosW)) > breakPosW)
        {
        bool forceCurrentUnit = (shouldForceLeadingUnit && (0 == breakPosW));
        
        // Note that we track a runningWidth; this is an optimization so that we don't have to recompute preceding character ranges over and over again.
        // Caveat: This assumes (and to the best of my knowledge) that characters before a break point cannot affect the shaping of characters after a break point.
        Utf8String testContentU8;
        BeStringUtilities::WCharToUtf8(testContentU8, testContentW.c_str() + breakPosW, (peekBreakPosW - breakPosW));
        
        // If we don't fit, the previous break position is the split point.
        DRange2d justificationRange;
        DRange2d testContentRange = computeRangeForText(&justificationRange, testContentU8.c_str(), testContentU8.size(), *effectiveStyle, textRun.GetSubSuperScript());
        if (!forceCurrentUnit && (runningWidth + justificationRange.XLength()) > availableWidth)
            break;
        
        runningWidth += testContentRange.XLength();

        // Otherwise we fit; keep trying.
        breakPosW = peekBreakPosW;
        }

    // Does the whole thing fit? No leftOver and return that we don't have to wrap.
    if (breakPosW >= testContentW.size())
        return false;
    
    // Otherwise create leftOver and trim this run.
    size_t breakPosLog = BeStringUtilities::ComputeNumLogicalChars(testContentW.c_str(), breakPosW);
    size_t breakPosU8 = BeStringUtilities::ComputeByteOffsetOfLogicalChar(textRun.GetContent().c_str() + m_charOffset, breakPosLog);
    
    leftOver = Clone();
    leftOver->SetCharOffset(m_charOffset + breakPosU8);
    leftOver->SetNumChars(m_numChars - breakPosU8);
    
    SetNumChars(breakPosU8);

    return true;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
bool AnnotationLayoutRun::AffectsJustification() const
    {
    // Control runs do not affect justification.
    return (AnnotationRunType::LineBreak != m_seedRun->GetType());
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationLayoutLine::AnnotationLayoutLine() :
    T_Super()
    {
    m_isValid = false;
    m_layoutRange.Init();
    m_offsetFromDocument.Zero();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationLayoutLine::CopyFrom(AnnotationLayoutLineCR rhs)
    {
    m_isValid = rhs.m_isValid;
    m_layoutRange = rhs.m_layoutRange;
    m_offsetFromDocument = rhs.m_offsetFromDocument;
    
    m_runs.clear();
    m_runs.reserve(rhs.m_runs.size());
    for (auto const& rhsRun : rhs.m_runs)
        m_runs.push_back(rhsRun->Clone());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationLayoutLine::Update()
    {
    if (m_isValid)
        return;
    
    m_isValid = true;
    m_layoutRange = createZeroDRange2d();
    m_justificationRange = createZeroDRange2d();

    for (auto const& run : m_runs)
        {
        DVec2d runOffset = DVec2d::From(m_layoutRange.high.x, 0.0);
        run->SetOffsetFromLine(runOffset);

        //.........................................................................................
        DRange2d runLayoutRange = run->GetLayoutRange();
        moveRange(runLayoutRange, runOffset);

        m_layoutRange.Extend(runLayoutRange);

        //.........................................................................................
        if (run->AffectsJustification())
            {
            DRange2d runJustificationRange;
            if (SUCCESS != run->GetSubRange(runJustificationRange, AnnotationLayoutRun::SubRange::JustificationRange))
                runJustificationRange = run->GetLayoutRange();
        
            moveRange(runJustificationRange, runOffset);

            m_justificationRange.Extend(runJustificationRange);
            }
        }
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextBlockLayout::AnnotationTextBlockLayout(AnnotationTextBlockCR doc) :
    T_Super()
    {
    m_doc = &doc;
    m_isValid = false;
    m_layoutRange.Init();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlockLayout::CopyFrom(AnnotationTextBlockLayoutCR rhs)
    {
    m_doc = rhs.m_doc;
    m_isValid = rhs.m_isValid;
    m_layoutRange = rhs.m_layoutRange;

    m_lines.clear();
    m_lines.reserve(rhs.m_lines.size());
    for (auto const& rhsLine : rhs.m_lines)
        m_lines.push_back(rhsLine->Clone());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationLayoutLinePtr AnnotationTextBlockLayout::FlushLine(AnnotationLayoutLineR line)
    {
    // Based on comments in Update, we want to guarantee that each layout line has at least one run.
    if (line.GetRuns().empty())
        {
        // Based on comments in Update, if we're empty, there should always be a preceeding run, and it should always be a line break.
        if (m_lines.empty() || m_lines.back()->GetRuns().empty())
            {
            BeAssert(false);
            return AnnotationLayoutLine::Create();
            }
            
        // While we could technically make the following code agnostic of run type, this should always be true, and other things must be fixed instead of working around it here.
        AnnotationRunBaseCR previousRun = m_lines.back()->GetRuns().back()->GetSeedRun();
        if (AnnotationRunType::LineBreak != previousRun.GetType())
            {
            BeAssert(false);
            return AnnotationLayoutLine::Create();
            }

        AnnotationLineBreakRunPtr lineBreak = ((AnnotationLineBreakRunCR)previousRun).CloneAsLineBreakRun();
        line.AppendRun(*AnnotationLayoutRun::Create(*lineBreak));
        }
    
    // Line origin is its baseline.
    DVec2d lineOffset = DVec2d::From(0.0, -line.GetLayoutRange().YLength());
    
    // Place it below any exiting lines.
    if (!m_lines.empty())
        {
        lineOffset.y += m_lines.back()->GetOffsetFromDocument().y;
    
        AnnotationTextStylePtr effectiveDocStyle = m_doc->CreateEffectiveStyle();
        lineOffset.y -= (effectiveDocStyle->GetLineSpacingFactor() * effectiveDocStyle->GetHeight());
        }
    
    line.SetOffsetFromDocument(lineOffset);

    // Update document range from compute line range and position.
    DRange2d lineRange = line.GetLayoutRange();
    moveRange(lineRange, lineOffset);

    m_layoutRange.Extend(lineRange);

    m_lines.push_back(&line);

    return AnnotationLayoutLine::Create();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlockLayout::PopulateLines()
    {
    // The approach is to fill up a line, and flush it (e.g. post-process and add to the internal collection) as-needed. This allows lines to only be built as-needed, and not potentially pruned after processing the next run.
    //  It should be noted that AnnotationTextBlock has "line break" runs, but not "paragraph break" runs; the presence of a new paragraph object indicates this.
    //  All blank lines interior to a paragraph will contain a data (vs. layout) line break run. A potential trailing blank line of a paragraph may have no data runs, but will always have been preceeded by a line break.
    //  For ease of algorithms, we synthesize a new layout line break run for these trailing lines based on the preceeding run.

    double docWidth = m_doc->GetDocumentWidth();
    bool isWrapped = (docWidth > 0.0);
    
    AnnotationLayoutLinePtr line = AnnotationLayoutLine::Create();

    for (size_t iParagraph = 0; iParagraph < m_doc->GetParagraphs().size(); ++iParagraph)
        {
        if (iParagraph > 0)
            line = FlushLine(*line);

        for (auto const& run : m_doc->GetParagraphs()[iParagraph]->GetRuns())
            {
            AnnotationLayoutRunPtr layoutRun = AnnotationLayoutRun::Create(*run);

            // Line break? It always "fits" and causes us to flush the line.
            if (AnnotationRunType::LineBreak == run->GetType())
                {
                line->AppendRun(*layoutRun);
                line = FlushLine(*line);
                continue;
                }

            double effectiveRunWidth = (isWrapped ? layoutRun->GetLayoutRange().XLength() : 0.0);
            double effectiveRemainingWidth = (isWrapped ? (docWidth - line->GetLayoutRange().XLength()) : numeric_limits<double>::max());

            // Do we fit (no wrapping or narrow enough)? Append and go around to the next run.
            if (effectiveRunWidth < effectiveRemainingWidth)
                {
                line->AppendRun(*layoutRun);
                continue;
                }

            // Can't fit, but can't wrap? Force on the line if it's the first thing; otherwise flush and add to the next line.
            if (!layoutRun->CanWrap())
                {
                if (line->GetRuns().empty())
                    {
                    line->AppendRun(*layoutRun);
                    line = FlushLine(*line);
                    }
                else
                    {
                    line = FlushLine(*line);
                    line->AppendRun(*layoutRun);
                    }

                continue;
                }

            // Otherwise, keep splitting the run into lines until the whole thing is appended.
            AnnotationLayoutRunPtr leftOver;
            while (layoutRun->Wrap(leftOver, effectiveRemainingWidth, line->GetRuns().empty()))
                {
                line->AppendRun(*layoutRun);
                line = FlushLine(*line);
                effectiveRemainingWidth = docWidth;

                layoutRun = leftOver;
                }

            line->AppendRun(*layoutRun);
            }
        }

    if (!line->GetRuns().empty())
        FlushLine(*line);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlockLayout::JustifyLines()
    {
    if ((m_lines.size() <= 1) || (AnnotationTextBlock::HorizontalJustification::Left == m_doc->GetJustification()))
        return;

    double effectiveDocumentWidth = m_doc->GetDocumentWidth();
    if (0.0 == effectiveDocumentWidth)
        {
        for (auto const& line : m_lines)
            {
            double lineJustificationWidth = line->GetJustificationRange().XLength();
            if (lineJustificationWidth > effectiveDocumentWidth)
                effectiveDocumentWidth = lineJustificationWidth;
            }
        }

    for (auto& line : m_lines)
        {
        DVec2d offsetFromDocument = line->GetOffsetFromDocument();
        
        switch (m_doc->GetJustification())
            {
            case AnnotationTextBlock::HorizontalJustification::Center: offsetFromDocument.x += ((effectiveDocumentWidth - line->GetJustificationRange().XLength()) / 2.0); break;
            case AnnotationTextBlock::HorizontalJustification::Right: offsetFromDocument.x += (effectiveDocumentWidth - line->GetJustificationRange().XLength()); break;
            
            default:
                BeAssert(false); // Unknown/unexpected AnnotationTextBlock::HorizontalJustification.
                break;
            }

        line->SetOffsetFromDocument(offsetFromDocument);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlockLayout::Update()
    {
    if (m_isValid)
        return;

    m_isValid = true;
    m_lines.clear();
    m_layoutRange = createZeroDRange2d();

    PopulateLines();
    JustifyLines();
    }
