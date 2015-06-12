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
#ifdef MERGE_0501_06_JEFF
static void recookDisplayParams(ViewContextR context, ElemDisplayParamsR displayParams)
    {
    DgnModelP model = context.GetCurrentModel();
    PRECONDITION(NULL != model, );

    displayParams.SetElementColorFlags(model);
    displayParams.SetFillColorFlags(model);

    context.ResolveEffectiveDisplayParams(displayParams, NULL);
    context.CookDisplayParams(0);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void setStrokeSymbology(ViewContextR context, ColorDef color, int32_t style, uint32_t weight)
    {
#ifdef MERGE_0501_06_JEFF
    ElemDisplayParamsP displayParams = context.GetCurrentDisplayParams();
    PRECONDITION(NULL != displayParams, );

    displayParams->m_symbology.color = color;
    displayParams->m_symbology.style = style;
    displayParams->m_symbology.weight = weight;
    displayParams->m_fillDisplay = FillDisplay::Never;

    recookDisplayParams(context, *displayParams);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void setFillSymbology(ViewContextR context, ColorDef color, double transparency)
    {
#ifdef MERGE_0501_06_JEFF
    ElemDisplayParamsP displayParams = context.GetCurrentDisplayParams();
    PRECONDITION(NULL != displayParams, );

    displayParams->m_fillColor = color;
    displayParams->m_transparency = transparency;
    displayParams->m_fillDisplay = FillDisplay::Blanking;

    recookDisplayParams(context, *displayParams);
#endif
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
        setStrokeSymbology(context, frameStyle->GetStrokeColor(), frameStyle->GetStrokeStyle(), frameStyle->GetStrokeWeight());
        frameGeometry->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        output.DrawCurveVector(*frameGeometry, false);
        }

    if (frameStyle->IsFillEnabled())
        {
        setFillSymbology(context, frameStyle->GetFillColor(), frameStyle->GetFillTransparency());
        frameGeometry->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
        output.DrawCurveVector(*frameGeometry, true);
        }

    return SUCCESS;
    }
