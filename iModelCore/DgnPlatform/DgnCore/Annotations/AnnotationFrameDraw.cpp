//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationFrameDraw.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

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
static void setStrokeSymbology(ViewContextR context, bool isColorByCategory, ColorDef color, uint32_t weight)
    {
    ElemDisplayParamsP displayParams = context.GetCurrentDisplayParams();
    PRECONDITION(NULL != displayParams, );

    displayParams->ResetAppearance();

    displayParams->SetWeight(weight);
    displayParams->SetFillDisplay(FillDisplay::Never);

    if (!isColorByCategory)
        displayParams->SetLineColor(color);

    context.CookDisplayParams();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void setFillSymbology(ViewContextR context, bool isColorByCategory, ColorDef color, double transparency)
    {
    ElemDisplayParamsP displayParams = context.GetCurrentDisplayParams();
    PRECONDITION(NULL != displayParams, );

    displayParams->ResetAppearance();

    displayParams->SetTransparency(transparency);
    displayParams->SetFillDisplay(FillDisplay::Blanking);

    if (!isColorByCategory)
        displayParams->SetFillColor(color);

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
    
    IDrawGeomR output = context.GetIDrawGeom();

    ElemDisplayParamsP displayParams = context.GetCurrentDisplayParams();
    PRECONDITION(NULL != displayParams, ERROR);

    // We have to copy so that we can call SetBoundaryType to ensure we get a line string for stroke vs. a surface for fill.
    CurveVectorPtr frameGeometry = m_frameLayout->GetFrameGeometry().Clone();
    
    if (frameStyle->IsStrokeEnabled())
        {
        setStrokeSymbology(context, frameStyle->IsStrokeColorByCategory(), frameStyle->GetStrokeColor(), frameStyle->GetStrokeWeight());
        frameGeometry->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        output.DrawCurveVector(*frameGeometry, false);
        }

    if (frameStyle->IsFillEnabled())
        {
        setFillSymbology(context, frameStyle->IsFillColorByCategory(), frameStyle->GetFillColor(), frameStyle->GetFillTransparency());
        frameGeometry->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
        output.DrawCurveVector(*frameGeometry, true);
        }

    return SUCCESS;
    }
