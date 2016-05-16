/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/AnnotationFrameDraw.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrameDraw::AnnotationFrameDraw(AnnotationFrameLayoutCR frameLayout) :
    T_Super()
    {
    m_frameLayout = &frameLayout;
    m_documentTransform.InitIdentity();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationFrameDraw::CopyFrom(AnnotationFrameDrawCR rhs)
    {
    m_frameLayout = rhs.m_frameLayout;
    m_documentTransform = rhs.m_documentTransform;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void setStrokeSymbology(Render::GraphicBuilderR graphic, ViewContextR context, GeometryParamsR geomParams, AnnotationColorType colorType, ColorDef colorValue, uint32_t weight)
    {
    geomParams.ResetAppearance();
    geomParams.SetWeight(weight);
    geomParams.SetFillDisplay(FillDisplay::Never);
    geomParams.SetTransparency(0.0);
    
    switch (colorType)
        {
        case AnnotationColorType::ByCategory: /* don't override */break;
        case AnnotationColorType::RGBA: geomParams.SetLineColor(colorValue); break;
        case AnnotationColorType::ViewBackground: BeAssert(false) /* unsupported */; break;
        default: BeAssert(false) /* unknown */; break;
        }

    context.CookGeometryParams(geomParams, graphic);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void setFillSymbology(Render::GraphicBuilderR graphic, ViewContextR context, GeometryParamsR geomParams, AnnotationColorType colorType, ColorDef colorValue, double transparency)
    {
    geomParams.ResetAppearance();
    geomParams.SetTransparency(transparency);
    geomParams.SetFillDisplay(FillDisplay::Blanking);

    switch (colorType)
        {
        case AnnotationColorType::ByCategory: /* don't override */break;
        case AnnotationColorType::RGBA: geomParams.SetFillColor(colorValue); break;
        case AnnotationColorType::ViewBackground: geomParams.SetFillColorToViewBackground(); break;
        default: BeAssert(false) /* unknown */; break;
        }

    context.CookGeometryParams(geomParams, graphic);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationFrameDraw::Draw(Render::GraphicBuilderR graphic, ViewContextR context, GeometryParamsR geomParams) const
    {
    AnnotationFrameStylePtr frameStyle = m_frameLayout->GetFrame().CreateEffectiveStyle();
    
    // Invisible?
    if (AnnotationFrameType::InvisibleBox == frameStyle->GetType())
        return SUCCESS;

    // Both fill and stroke disabled?
    if (!frameStyle->IsStrokeEnabled() && !frameStyle->IsFillEnabled())
        return SUCCESS;
    
    // We have to copy so that we can call SetBoundaryType to ensure we get a line string for stroke vs. a surface for fill.
    CurveVectorPtr frameGeometry = m_frameLayout->GetFrameGeometry().Clone();

    frameGeometry->TransformInPlace(m_documentTransform);
    
    if (frameStyle->IsStrokeEnabled())
        {
        setStrokeSymbology(graphic, context, geomParams, frameStyle->GetStrokeColorType(), frameStyle->GetStrokeColorValue(), frameStyle->GetStrokeWeight());
        frameGeometry->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        graphic.AddCurveVector(*frameGeometry, false);
        }

    if (frameStyle->IsFillEnabled() && (frameStyle->GetFillTransparency() < 1.0))
        {
        setFillSymbology(graphic, context, geomParams, frameStyle->GetFillColorType(), frameStyle->GetFillColorValue(), frameStyle->GetFillTransparency());
        frameGeometry->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
        graphic.AddCurveVector(*frameGeometry, true);
        }

    return SUCCESS;
    }
