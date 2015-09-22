/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/AnnotationFrameDraw.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrameDraw::AnnotationFrameDraw(AnnotationFrameLayoutCR frameLayout) :
    T_Super()
    {
    m_frameLayout = &frameLayout;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationFrameDraw::CopyFrom(AnnotationFrameDrawCR rhs)
    {
    m_frameLayout = rhs.m_frameLayout;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void setStrokeSymbology(ViewContextR context, AnnotationColorType colorType, ColorDef colorValue, uint32_t weight)
    {
    ElemDisplayParamsR displayParams = context.GetCurrentDisplayParams();

    displayParams.ResetAppearance();

    displayParams.SetWeight(weight);
    displayParams.SetFillDisplay(FillDisplay::Never);

    switch (colorType)
        {
        case AnnotationColorType::ByCategory: /* don't override */break;
        case AnnotationColorType::RGBA: displayParams.SetLineColor(colorValue); break;
        case AnnotationColorType::ViewBackground: BeAssert(false) /* unsupported */; break;
        default: BeAssert(false) /* unknown */; break;
        }

    context.CookDisplayParams();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void setFillSymbology(ViewContextR context, AnnotationColorType colorType, ColorDef colorValue, double transparency)
    {
    ElemDisplayParamsR displayParams = context.GetCurrentDisplayParams();

    displayParams.ResetAppearance();

    displayParams.SetTransparency(transparency);
    displayParams.SetFillDisplay(FillDisplay::Blanking);

    switch (colorType)
        {
        case AnnotationColorType::ByCategory: /* don't override */break;
        case AnnotationColorType::RGBA: displayParams.SetFillColor(colorValue); break;
        case AnnotationColorType::ViewBackground: displayParams.SetFillColorToViewBackground(); break;
        default: BeAssert(false) /* unknown */; break;
        }

    context.CookDisplayParams();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationFrameDraw::Draw(ViewContextR context) const
    {
    AnnotationFrameStylePtr frameStyle = m_frameLayout->GetFrame().CreateEffectiveStyle();
    
    // Invisible?
    if (AnnotationFrameType::InvisibleBox == frameStyle->GetType())
        return SUCCESS;

    // Both fill and stroke disabled?
    if (!frameStyle->IsStrokeEnabled() && !frameStyle->IsFillEnabled())
        return SUCCESS;
    
    GeomDrawR output = context.GetIDrawGeom();

    // We have to copy so that we can call SetBoundaryType to ensure we get a line string for stroke vs. a surface for fill.
    CurveVectorPtr frameGeometry = m_frameLayout->GetFrameGeometry().Clone();
    
    if (frameStyle->IsStrokeEnabled())
        {
        setStrokeSymbology(context, frameStyle->GetStrokeColorType(), frameStyle->GetStrokeColorValue(), frameStyle->GetStrokeWeight());
        frameGeometry->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        output.DrawCurveVector(*frameGeometry, false);
        }

    if (frameStyle->IsFillEnabled())
        {
        setFillSymbology(context, frameStyle->GetFillColorType(), frameStyle->GetFillColorValue(), frameStyle->GetFillTransparency());
        frameGeometry->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
        output.DrawCurveVector(*frameGeometry, true);
        }

    return SUCCESS;
    }
