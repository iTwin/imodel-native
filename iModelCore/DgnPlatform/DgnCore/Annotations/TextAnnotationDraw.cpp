//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/TextAnnotationDraw.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TextAnnotationDraw::TextAnnotationDraw(TextAnnotationCR annotation) :
    T_Super()
    {
    m_annotation = &annotation;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void TextAnnotationDraw::CopyFrom(TextAnnotationDrawCR rhs)
    {
    m_annotation = rhs.m_annotation;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    03/2016
//---------------------------------------------------------------------------------------
static Transform    computeDocumentTransform (AnnotationTextBlockLayoutCR layout, TextAnnotationCR annotation)
    {
    DPoint3d                    origin      = annotation.GetOrigin();
    YawPitchRollAngles          orientation = annotation.GetOrientation();
    TextAnnotation::AnchorPoint anchorPoint = annotation.GetAnchorPoint();

    RotMatrix   matrix = orientation.ToRotMatrix();

    if (TextAnnotation::AnchorPoint::LeftTop == anchorPoint)
        return Transform::From (matrix, origin);

    DRange2d    textRange = layout.GetLayoutRange();

    switch (((int) anchorPoint - 1) / 3)
        {
        case 1: origin.x -= textRange.XLength() / 2.0; break; // Center
        case 2: origin.x -= textRange.XLength();       break; // Right
        }

    switch (((int) anchorPoint - 1) % 3)
        {
        case 1: origin.y += textRange.YLength() / 2.0; break; // Middle
        case 2: origin.y += textRange.YLength();       break; // Bottom
        }

    return Transform::From (matrix, origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationDraw::Draw(Render::GraphicBuilderR graphic, ViewContextR context, GeometryParamsR geomParams) const
    {
    //.............................................................................................
    if (nullptr == m_annotation->GetTextCP())
        return SUCCESS;

    AnnotationTextBlockLayout textLayout(*m_annotation->GetTextCP());
    AnnotationTextBlockDraw textDraw(textLayout);
    Transform documentTransform = computeDocumentTransform (textLayout, *m_annotation);

    textDraw.SetDocumentTransform(documentTransform);
    BentleyStatus status = textDraw.Draw(graphic, context, geomParams);

    //.............................................................................................
    if (nullptr == m_annotation->GetFrameCP())
        return status;

    AnnotationFrameLayout frameLayout(*m_annotation->GetFrameCP(), textLayout);
    AnnotationFrameDraw frameDraw(frameLayout);
    
    frameDraw.SetDocumentTransform(documentTransform);
    status = frameDraw.Draw(graphic, context, geomParams);

    //.............................................................................................
    if (m_annotation->GetLeaders().empty())
        return status;

    for (auto const& leader : m_annotation->GetLeaders())
        {
        AnnotationLeaderLayout leaderLayout(*leader, frameLayout);
        leaderLayout.SetFrameTransform(documentTransform);

        AnnotationLeaderDraw leaderDraw(leaderLayout);
        status = leaderDraw.Draw(graphic, context, geomParams);
        }

    return status;
    }
