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
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationDraw::Draw(Render::GraphicR graphic, ViewContextR context, GeometryParamsR geomParams) const
    {
    //.............................................................................................
    if (nullptr == m_annotation->GetTextCP())
        return SUCCESS;

    AnnotationTextBlockLayout textLayout(*m_annotation->GetTextCP());
    AnnotationTextBlockDraw textDraw(textLayout);

    textDraw.SetDocumentTransform(m_annotation->GetDocumentTransform());
    BentleyStatus status = textDraw.Draw(graphic, context, geomParams);

    //.............................................................................................
    if (nullptr == m_annotation->GetFrameCP())
        return status;

    AnnotationFrameLayout frameLayout(*m_annotation->GetFrameCP(), textLayout);
    AnnotationFrameDraw frameDraw(frameLayout);
    
    frameDraw.SetDocumentTransform(m_annotation->GetDocumentTransform());
    status = frameDraw.Draw(graphic, context, geomParams);

    //.............................................................................................
    if (m_annotation->GetLeaders().empty())
        return status;

    for (auto const& leader : m_annotation->GetLeaders())
        {
        AnnotationLeaderLayout leaderLayout(*leader, frameLayout);
        leaderLayout.SetFrameTransform(m_annotation->GetDocumentTransform());

        AnnotationLeaderDraw leaderDraw(leaderLayout);
        status = leaderDraw.Draw(graphic, context, geomParams);
        }

    return status;
    }
