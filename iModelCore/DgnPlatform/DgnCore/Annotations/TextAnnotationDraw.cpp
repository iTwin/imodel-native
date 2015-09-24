//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/TextAnnotationDraw.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationDraw::DrawToElementGeometry::_ProcessTextString(TextStringCR text)
    {
    if (m_transform.IsIdentity())
        {
        m_builder.Append(text);
        }
    else
        {
        TextString transformedText(text);
        transformedText.ApplyTransform(m_transform);
        m_builder.Append(transformedText);
        }

    return SUCCESS; // SUCCESS means handled
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationDraw::DrawToElementGeometry::_ProcessCurveVector(CurveVectorCR curves, bool isFilled)
    {
    if (m_transform.IsIdentity())
        {
        m_builder.Append(curves);
        }
    else
        {
        CurveVector transformedCurves(curves);
        transformedCurves.TransformInPlace(m_transform);
        m_builder.Append(transformedCurves);
        }

    return SUCCESS; // SUCCESS means handled
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
void TextAnnotationDraw::DrawToElementGeometry::_OutputGraphics(ViewContextR context)
    {
    context.GetCurrentDisplayParams().SetCategoryId(m_categoryId);

    m_annotationDraw.Draw(context);
    }

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

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct PopTransformClipOnDestruct
{
private:
    bool m_isCancelled;
    ViewContextR m_context;

    void Pop() { if (!m_isCancelled) { m_context.PopTransformClip(); } }

public:
    explicit PopTransformClipOnDestruct(ViewContextR context) : m_isCancelled(false), m_context(context) {}
    ~PopTransformClipOnDestruct() { Pop(); }
    void CallThenCancel() { Pop(); Cancel(); }
    void Cancel() { m_isCancelled = true; }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationDraw::Draw(ViewContextR context) const
    {
    BentleyStatus status = SUCCESS;
    
    //.............................................................................................
    if (NULL == m_annotation->GetTextCP())
        return status;

    context.PushTransform(m_documentTransform);
    PopTransformClipOnDestruct autoPopDocumentTransform(context);

    AnnotationTextBlockLayout textLayout(*m_annotation->GetTextCP());
    AnnotationTextBlockDraw textDraw(textLayout);

    if (SUCCESS != textDraw.Draw(context))
        status = ERROR;

    //.............................................................................................
    if (NULL == m_annotation->GetFrameCP())
        return status;
        
    AnnotationFrameLayout frameLayout(*m_annotation->GetFrameCP(), textLayout);
    AnnotationFrameDraw frameDraw(frameLayout);
    
    if (SUCCESS != frameDraw.Draw(context))
        status = ERROR;
    
    //.............................................................................................
    if (m_annotation->GetLeaders().empty())
        return status;
    
    autoPopDocumentTransform.CallThenCancel();

    for (auto const& leader : m_annotation->GetLeaders())
        {
        AnnotationLeaderLayout leaderLayout(*leader, frameLayout);
        leaderLayout.SetFrameTransform(m_documentTransform);

        AnnotationLeaderDraw leaderDraw(leaderLayout);
        
        if (SUCCESS != leaderDraw.Draw(context))
            status = ERROR;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationDraw::Draw(ElementGeometryBuilderR builder, DgnDbR db, DgnCategoryId categoryId) const
    {
    TextAnnotationDraw::DrawToElementGeometry annotationDrawToGeometry(*this, builder, categoryId);
    ElementGraphicsOutput::Process(annotationDrawToGeometry, db);

    return SUCCESS;
    }
