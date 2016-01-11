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
    m_documentTransform.InitIdentity();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void TextAnnotationDraw::CopyFrom(TextAnnotationDrawCR rhs)
    {
    m_annotation = rhs.m_annotation;
    m_documentTransform = rhs.m_documentTransform;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
Render::GraphicPtr TextAnnotationDraw::Draw(ViewContextR context, GeometryParamsR geomParams) const
    {
    //.............................................................................................
    if (nullptr == m_annotation->GetTextCP())
        return nullptr;

    Render::GraphicPtr graphic = context.CreateGraphic(Graphic::CreateParams(context.GetViewport(), m_documentTransform));

    AnnotationTextBlockLayout textLayout(*m_annotation->GetTextCP());
    AnnotationTextBlockDraw textDraw(textLayout);

    textDraw.Draw(*graphic, context, geomParams);

    //.............................................................................................
    if (nullptr != m_annotation->GetFrameCP())
        {
        AnnotationFrameLayout frameLayout(*m_annotation->GetFrameCP(), textLayout);
        AnnotationFrameDraw frameDraw(frameLayout);
    
        frameDraw.Draw(*graphic, context, geomParams);

        //.............................................................................................
        if (!m_annotation->GetLeaders().empty())
            {    
            Transform   invDocTrans;

            // NEEDSWORK: Probably shouldn't assume that m_documentTransform == element's placement...
            invDocTrans.InverseOf(m_documentTransform); // Don't want sub-graphic relative to main graphic...

            Render::GraphicPtr subGraphic = graphic->CreateSubGraphic(invDocTrans);

            for (auto const& leader : m_annotation->GetLeaders())
                {
                AnnotationLeaderLayout leaderLayout(*leader, frameLayout);
                leaderLayout.SetFrameTransform(m_documentTransform);

                AnnotationLeaderDraw leaderDraw(leaderLayout);
        
                leaderDraw.Draw(*graphic, context, geomParams);
                }

            subGraphic->Close();

            // NOTE: Need to cook GeometryParams to get GraphicParams, but we don't want to activate...
            GraphicParams graphicParams;

            context.CookGeometryParams(geomParams, graphicParams);
            graphic->AddSubGraphic(*subGraphic, invDocTrans, graphicParams);
            }
        }

    graphic->Close();

    return graphic;
    }
