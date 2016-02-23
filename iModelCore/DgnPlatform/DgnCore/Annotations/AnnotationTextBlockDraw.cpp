/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/AnnotationTextBlockDraw.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatform/TextStyleInterop.h>

USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextBlockDraw::AnnotationTextBlockDraw(AnnotationTextBlockLayoutCR layout) :
    T_Super()
    {
    m_layout = &layout;
    m_documentTransform.InitIdentity();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextBlockDraw::CopyFrom(AnnotationTextBlockDrawCR rhs)
    {
    m_layout = rhs.m_layout;
    m_documentTransform = rhs.m_documentTransform;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static void adjustForSubOrSuperScript(TextStringR ts, AnnotationTextRunCR textRun)
    {
    // Completely ignore if neither or both.
    if (AnnotationTextRunSubSuperScript::Neither == textRun.GetSubSuperScript())
        return;

    AnnotationTextStylePtr style = textRun.CreateEffectiveStyle();

    double offsetFactor = (textRun.IsSubScript() ? style->GetSubScriptOffsetFactor() : style->GetSuperScriptOffsetFactor());
    double scale = (textRun.IsSubScript() ? style->GetSubScriptScale() : style->GetSuperScriptScale());
    
    double seedFontHeight = ts.GetStyle().GetHeight();
    DPoint3d seedLowerLeft = ts.GetOrigin();
        
    DPoint3d lowerLeft = seedLowerLeft;
    lowerLeft.y += (offsetFactor * seedFontHeight);

    ts.GetStyleR().SetHeight(seedFontHeight * scale);
    ts.SetOrigin(lowerLeft);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockDraw::DrawTextRun(AnnotationLayoutRunCR layoutRun, Render::GraphicR graphic, ViewContextR context, GeometryParamsR geomParams, TransformCR transform) const
    {
    AnnotationTextRunCR run = (AnnotationTextRunCR)layoutRun.GetSeedRun();
    
    if (run.GetContent().empty())
        return SUCCESS;
    
    TextString ts;
    
    Utf8String text = run.GetContent().substr(layoutRun.GetCharOffset(), layoutRun.GetNumChars());
    ts.SetText(text.c_str());
    
    AnnotationTextStylePtr effectiveStyle = run.CreateEffectiveStyle();
    TextStyleInterop::AnnotationToTextString(ts.GetStyleR(), *effectiveStyle);
    
    adjustForSubOrSuperScript(ts, run);

    if (!transform.IsIdentity())
        ts.ApplyTransform(transform);
    
    geomParams.ResetAppearance();

    switch (effectiveStyle->GetColorType())
        {
        case AnnotationColorType::ByCategory: /* don't override */break;
        case AnnotationColorType::RGBA: geomParams.SetLineColor(effectiveStyle->GetColorValue()); break;
        case AnnotationColorType::ViewBackground: BeAssert(false) /* unsupported */; break;
        default: BeAssert(false) /* unknown */; break;
        }
    
    context.CookGeometryParams(geomParams, graphic);
    graphic.AddTextString(ts);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockDraw::DrawFractionRun(AnnotationLayoutRunCR layoutRun, Render::GraphicR graphic, ViewContextR context, GeometryParamsR geomParams, TransformCR transform) const
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
            
            transform.Multiply(barPts, barPts, 2);
            graphic.AddLineString(2, barPts);
            break;
            }
        
        case AnnotationStackedFractionType::DiagonalBar:
            {
            // This positioning is carried over from DgnV8... I'm not aware of any specific complaints, so let's stick with it for now.
            DPoint3d barPts [2];
            barPts[0] = { denominatorRange.low.x - (fontSize.x / 2.0), denominatorRange.low.y + (fontSize.y * (1.0 / 3.0)), 0.0 };
            barPts[1] = { barPts[0].x + fontSize.x, barPts[0].y + (fontSize.y * 1.5), 0.0 };

            transform.Multiply(barPts, barPts, 2);
            graphic.AddLineString(2, barPts);
            break;
            }

        default:
            BeAssert(false); // Unknown / unexpected AnnotationStackedFractionType.
            break;
        }
    
    if (!run.GetNumeratorContent().empty())
        {
        TextString tsNumerator;
        tsNumerator.SetText(run.GetNumeratorContent().c_str());
        tsNumerator.SetOrigin(numeratorOffset);
        TextStyleInterop::AnnotationToTextString(tsNumerator.GetStyleR(), *effectiveStyle);
        tsNumerator.GetStyleR().SetSize(fontSize);

        if (!transform.IsIdentity())
            tsNumerator.ApplyTransform(transform);

        graphic.AddTextString(tsNumerator);
        }
    
    if (!run.GetDenominatorContent().empty())
        {
        TextString tsDenominator;
        tsDenominator.SetText(run.GetDenominatorContent().c_str());
        tsDenominator.SetOrigin(denominatorOffset);
        TextStyleInterop::AnnotationToTextString(tsDenominator.GetStyleR(), *effectiveStyle);
        tsDenominator.GetStyleR().SetSize(fontSize);

        if (!transform.IsIdentity())
            tsDenominator.ApplyTransform(transform);

        graphic.AddTextString(tsDenominator);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockDraw::DrawLineBreakRun(AnnotationLayoutRunCR, Render::GraphicR, ViewContextR, GeometryParamsR, TransformCR) const
    {
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextBlockDraw::Draw(Render::GraphicR graphic, ViewContextR context, GeometryParamsR geomParams) const
    {
    BentleyStatus status = SUCCESS;

    for (auto const& line : m_layout->GetLines())
        {
        Transform lineTrans = Transform::From(DPoint3d::From(line->GetOffsetFromDocument()));

        for (auto const& run : line->GetRuns())
            {
            Transform runTrans = Transform::From(DPoint3d::From(run->GetOffsetFromLine()));
            Transform compoundTrans = Transform::FromProduct(m_documentTransform, runTrans, lineTrans);
            
            BentleyStatus runStatus = SUCCESS;

            switch (run->GetSeedRun().GetType())
                {
                case AnnotationRunType::Text: runStatus = DrawTextRun(*run, graphic, context, geomParams, compoundTrans); break;
                case AnnotationRunType::Fraction: runStatus = DrawFractionRun(*run, graphic, context, geomParams, compoundTrans); break;
                case AnnotationRunType::LineBreak: runStatus = DrawLineBreakRun(*run, graphic, context, geomParams, compoundTrans); break;

                default:
                    BeAssert(false); // Unknown/unexpected AnnotationRunType.
                    break;
                }

            if (SUCCESS != runStatus)
                status = ERROR;
            }
        }

    return status;
    }
