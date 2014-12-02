//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationTextBlockDraw.cpp $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextBlockDrawPtr AnnotationTextBlockDraw::Create(AnnotationTextBlockLayoutCR layout) { return new AnnotationTextBlockDraw(layout); }
AnnotationTextBlockDraw::AnnotationTextBlockDraw(AnnotationTextBlockLayoutCR layout) :
    T_Super()
    {
    m_layout = &layout;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextBlockDrawPtr AnnotationTextBlockDraw::Clone() const { return new AnnotationTextBlockDraw(*this); }
AnnotationTextBlockDraw::AnnotationTextBlockDraw(AnnotationTextBlockDrawCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationTextBlockDrawR AnnotationTextBlockDraw::operator=(AnnotationTextBlockDrawCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationTextBlockDraw::CopyFrom(AnnotationTextBlockDrawCR rhs)
    {
    m_layout = rhs.m_layout;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextBlockLayoutCR AnnotationTextBlockDraw::GetLayout() const { return *m_layout; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static TextStringProperties textStringPropertiesFromAnnotationRun(AnnotationRunBaseCR run)
    {
    TextStringProperties tsProps;
    AnnotationTextStylePtr effectiveStyle = run.CreateEffectiveStyle();

    tsProps.SetColor(effectiveStyle->GetColorId());
    tsProps.SetFont(*DgnFontManager::ResolveFont(effectiveStyle->GetFontId(), run.GetDgnProjectR(), DgnFontVariant::DGNFONTVARIANT_DontCare));
    tsProps.SetFontSize({effectiveStyle->GetWidthFactor() * effectiveStyle->GetHeight(), effectiveStyle->GetHeight()});
    tsProps.SetIsBold(effectiveStyle->IsBold());
    tsProps.SetIsItalic(effectiveStyle->IsItalic());
    tsProps.SetIsUnderlined(effectiveStyle->IsUnderlined());
    
    return tsProps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static void adjustForSubOrSuperScript(TextStringR ts, AnnotationTextStyleCR style)
    {
    // Completely ignore if neither or both.
    if (!(style.IsSubScript() ^ style.IsSuperScript()))
        return;

    double offsetFactor = (style.IsSubScript() ? style.GetSubScriptOffsetFactor() : style.GetSuperScriptOffsetFactor());
    double scale = (style.IsSubScript() ? style.GetSubScriptScale() : style.GetSuperScriptScale());
    
    DPoint2d seedFontSize = ts.GetProperties().GetFontSize();
    DPoint3d seedLowerLeft = ts.GetLowerLeft();
        
    DPoint3d lowerLeft = seedLowerLeft;
    lowerLeft.y += (offsetFactor * seedFontSize.y);

    DPoint2d fontSize;
    fontSize.Scale(seedFontSize, scale);
        
    ts.SetFontSize(fontSize);
    ts.SetLowerLeft(lowerLeft);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockDraw::DrawTextRun(AnnotationLayoutRunCR layoutRun, ViewContextR context) const
    {
    AnnotationTextRunCR run = (AnnotationTextRunCR)layoutRun.GetSeedRun();
    
    if (run.GetContent().empty())
        return SUCCESS;
    
    WString contentW;
    BeStringUtilities::Utf8ToWChar(contentW, run.GetContent().c_str() + layoutRun.GetCharOffset(), layoutRun.GetNumChars());

    TextStringProperties tsProps = textStringPropertiesFromAnnotationRun(run);
    TextString ts(contentW.c_str(), NULL, NULL, tsProps);
    
    adjustForSubOrSuperScript(ts, *run.CreateEffectiveStyle());
    
#ifdef MERGE_0501_06_JEFF
    ts.Draw(context);
#endif
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockDraw::DrawFractionRun(AnnotationLayoutRunCR layoutRun, ViewContextR context) const
    {
    AnnotationFractionRunCR run = (AnnotationFractionRunCR)layoutRun.GetSeedRun();
    
    if (run.GetNumeratorContent().empty() && run.GetDenominatorContent().empty())
        return SUCCESS;

    DRange2d denominatorRange;
    POSTCONDITION(SUCCESS == layoutRun.GetSubRange(denominatorRange, AnnotationLayoutRun::SubRange::FractionDenominator), ERROR);
    
    DRange2d numeratorRange;
    POSTCONDITION(SUCCESS == layoutRun.GetSubRange(numeratorRange, AnnotationLayoutRun::SubRange::FractionNumerator), ERROR);
    
    AnnotationTextStylePtr effectiveStyle = run.CreateEffectiveStyle();
    
    DPoint2d fontSize = DPoint2d::From(effectiveStyle->GetHeight() * effectiveStyle->GetWidthFactor(), effectiveStyle->GetHeight());
    fontSize.Scale(effectiveStyle->GetStackedFractionScale());
    
    DVec3d numeratorOffset = DVec3d::From(numeratorRange.low.x, numeratorRange.low.y);
    DVec3d denominatorOffset = DVec3d::From(denominatorRange.low.x, denominatorRange.low.y);

    switch (effectiveStyle->GetStackedFractionType())
        {
        case AnnotationStackedFractionType::HorizontalBar:
            {
            double fractionWidth = std::max(numeratorRange.XLength(), denominatorRange.XLength());
            DPoint3d barPts[2];
            barPts[0] = { 0.0, (1.25 * denominatorRange.YLength()), 0.0 };
            barPts[1] = { fractionWidth, barPts[0].y, 0.0 };
            
            context.DrawStyledLineString3d(_countof(barPts), barPts, NULL);
            
            break;
            }
        
        case AnnotationStackedFractionType::DiagonalBar:
            {
            // This positioning is carried over from DgnV8... I'm not aware of any specific complaints, so let's stick with it for now.
            DPoint3d barPts [2];
            barPts[0] = { denominatorRange.low.x - (fontSize.x / 2.0), denominatorRange.low.y + (fontSize.y * (1.0 / 3.0)), 0.0 };
            barPts[1] = { barPts[0].x + fontSize.x, barPts[0].y + (fontSize.y * 1.5), 0.0 };

            context.DrawStyledLineString3d(_countof(barPts), barPts, NULL);

            break;
            }

        default:
            BeAssert(false); // Unknown / unexpected AnnotationStackedFractionType.
            break;
        }
    
    TextStringProperties tsProps = textStringPropertiesFromAnnotationRun(run);

    if (!run.GetNumeratorContent().empty())
        {
        TextString tsNumerator(WString(run.GetNumeratorContent().c_str(), BentleyCharEncoding::Utf8).c_str(), &numeratorOffset, NULL, tsProps);
        tsNumerator.SetFontSize(fontSize);

#ifdef MERGE_0501_06_JEFF
        tsNumerator.Draw(context);
#endif
        }
    
    if (!run.GetDenominatorContent().empty())
        {
        TextString tsDenominator(WString(run.GetDenominatorContent().c_str(), BentleyCharEncoding::Utf8).c_str(), &denominatorOffset, NULL, tsProps);
        tsDenominator.SetFontSize(fontSize);

#ifdef MERGE_0501_06_JEFF
        tsDenominator.Draw(context);
#endif
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockDraw::DrawLineBreakRun(AnnotationLayoutRunCR, ViewContextR) const
    {
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockDraw::Draw(ViewContextR context) const
    {
    BentleyStatus status = SUCCESS;

    for (auto const& line : m_layout->GetLines())
        {
        Transform lineTrans;
        lineTrans.InitIdentity();
        lineTrans.SetTranslation(line->GetOffsetFromDocument());

        context.PushTransform(lineTrans);

        for (auto const& run : line->GetRuns())
            {
            Transform runTrans;
            runTrans.InitIdentity();
            runTrans.SetTranslation(run->GetOffsetFromLine());

            context.PushTransform(runTrans);
            
            BentleyStatus runStatus = SUCCESS;

            switch (run->GetSeedRun().GetType())
                {
                case AnnotationRunType::Text: runStatus = DrawTextRun(*run, context); break;
                case AnnotationRunType::Fraction: runStatus = DrawFractionRun(*run, context); break;
                case AnnotationRunType::LineBreak: runStatus = DrawLineBreakRun(*run, context); break;

                default:
                    BeAssert(false); // Unknown/unexpected AnnotationRunType.
                    break;
                }

            if (SUCCESS != runStatus)
                status = ERROR;

            context.PopTransformClip(); // runTrans
            }

        context.PopTransformClip(); // lineTrans
        }

    return status;
    }
