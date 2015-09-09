/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/TextAnnotationElement.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatform/DgnCore/Annotations/TextAnnotationElement.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationPersistence.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(TextAnnotation);
}

namespace dgn_AspectHandler
{
HANDLER_DEFINE_MEMBERS(TextAnnotationData);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

namespace
{
//=======================================================================================
// We want to re-use TextAnnotationDraw, which requires a context, so use an IElementGraphicsProcessor to listen to context events and emit to an ElementGeometryBuilder.
// Note that this is tightly coupled with TextAnnotationDraw (and AnnotationFrameDraw, AnnotationLeaderDraw, and AnnotationTextBlockDraw), which uses a subset of draw commands.
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct TextAnnotationGraphicsProcessor : IElementGraphicsProcessor
    {
    TextAnnotationCR m_annotation;
    DgnCategoryId m_categoryId;
    ElementGeometryBuilderR m_builder;
    Transform m_transform;

    TextAnnotationGraphicsProcessor(TextAnnotationCR annotation, DgnCategoryId categoryId, ElementGeometryBuilderR builder) :
        m_annotation(annotation), m_categoryId(categoryId), m_builder(builder), m_transform(Transform::FromIdentity()) {}

    virtual void _AnnounceTransform(TransformCP transform) override { if (nullptr != transform) { m_transform = *transform; } else { m_transform.InitIdentity(); } }
    virtual void _AnnounceElemDisplayParams(ElemDisplayParamsCR params) override { m_builder.Append(params); }
    virtual BentleyStatus _ProcessTextString(TextStringCR) override;
    virtual BentleyStatus _ProcessCurveVector(CurveVectorCR, bool isFilled) override;
    virtual void _OutputGraphics(ViewContextR) override;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationGraphicsProcessor::_ProcessTextString(TextStringCR text)
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
BentleyStatus TextAnnotationGraphicsProcessor::_ProcessCurveVector(CurveVectorCR curves, bool isFilled)
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
void TextAnnotationGraphicsProcessor::_OutputGraphics(ViewContextR context)
    {
    context.GetCurrentDisplayParams().SetCategoryId(m_categoryId);

    TextAnnotationDraw annotationDraw(m_annotation);
    annotationDraw.Draw(context);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
DgnDbStatus TextAnnotationDataAspect::_UpdateProperties(DgnElementCR el)
    {
    // T_Super::_UpdateProperties is pure; it is a link error to call super, so don't.
    
    bvector<Byte> annotationBlob;
    if (m_annotation.IsValid())
        {
        if (SUCCESS != TextAnnotationPersistence::EncodeAsFlatBuf(annotationBlob, *m_annotation))
            return DgnDbStatus::WriteError;
        }

    CachedECSqlStatementPtr update = el.GetDgnDb().GetPreparedECSqlStatement("UPDATE " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationDataAspect) " SET TextAnnotation=? WHERE ECInstanceId=?");
    if (!update.IsValid())
        return DgnDbStatus::WriteError;

    if (annotationBlob.empty())
        update->BindNull(1);
    else
        update->BindBinary(1, &annotationBlob[0], (int)annotationBlob.size(), IECSqlBinder::MakeCopy::No);

    update->BindId(2, el.GetElementId());

    if (ECSqlStepStatus::Done != update->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
DgnDbStatus TextAnnotationDataAspect::_LoadProperties(DgnElementCR el)
    {
    // T_Super::_LoadProperties is pure; it is a link error to call super, so don't.
    
    CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement("SELECT TextAnnotation FROM " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationDataAspect) " WHERE ECInstanceId=?");
    if (!select.IsValid())
        return DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if ((ECSqlStepStatus::HasRow != select->Step()) || select->IsValueNull(0))
        {
        m_annotation = nullptr;
        return DgnDbStatus::Success;
        }

    int dataSize = 0;
    ByteCP data = (ByteCP)select->GetValueBinary(0, &dataSize);
    if ((0 == dataSize) || (nullptr == data))
        {
        m_annotation = nullptr;
        return DgnDbStatus::Success;
        }

    TextAnnotationPtr annotation = TextAnnotation::Create(el.GetDgnDb());
    if (SUCCESS != TextAnnotationPersistence::DecodeFromFlatBuf(*annotation, data, (size_t)dataSize))
        return DgnDbStatus::ReadError;

    m_annotation = annotation;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
void TextAnnotationDataAspect::_AppendGeometry(ElementGeometryBuilderR builder, TextAnnotationElementR el) const
    {
    if (!m_annotation.IsValid())
        return;
    
    TextAnnotationGraphicsProcessor annotationGraphics(*m_annotation, el.GetCategoryId(), builder);
    ElementGraphicsOutput::Process(annotationGraphics, el.GetDgnDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
TextAnnotationDataAspectR TextAnnotationElement::GetDataAspectR()
    {
    TextAnnotationDataAspectP aspect = TextAnnotationDataAspect::GetP(*this);
    if (nullptr == aspect)
        {
        aspect = new TextAnnotationDataAspect();
        TextAnnotationDataAspect::SetAspect(*this, *aspect);
        }

    return *aspect;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
void TextAnnotationElement::_UpdateGeomStream()
    {
    TextAnnotationDataAspectCP dataAspect = GetDataAspectCP();
    if (nullptr == dataAspect)
        return;
    
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*GetModel(), m_categoryId, m_placement.GetOrigin(), m_placement.GetAngle());
    dataAspect->_AppendGeometry(*builder, *this);
    builder->SetGeomStreamAndPlacement(*this);
    }
