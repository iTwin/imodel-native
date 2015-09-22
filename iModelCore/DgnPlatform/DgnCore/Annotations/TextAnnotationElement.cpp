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
HANDLER_DEFINE_MEMBERS(PhysicalTextAnnotation);
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
DgnDbStatus TextAnnotationItem::_UpdateProperties(DgnElementCR el)
    {
    // T_Super::_UpdateProperties is pure; it is a link error to call super, so don't.
    
    bvector<Byte> annotationBlob;
    if (m_annotation.IsValid())
        {
        if (SUCCESS != TextAnnotationPersistence::EncodeAsFlatBuf(annotationBlob, *m_annotation))
            return DgnDbStatus::WriteError;
        }

    CachedECSqlStatementPtr update = el.GetDgnDb().GetPreparedECSqlStatement("UPDATE " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationItem) " SET TextAnnotation=? WHERE ECInstanceId=?");
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
DgnDbStatus TextAnnotationItem::_LoadProperties(DgnElementCR el)
    {
    // T_Super::_LoadProperties is pure; it is a link error to call super, so don't.
    
    CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement("SELECT TextAnnotation FROM " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationItem) " WHERE ECInstanceId=?");
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
DgnDbStatus TextAnnotationItem::_GenerateElementGeometry(GeometricElementR el, GenerateReason reason)
    {
    if (!m_annotation.IsValid())
        return DgnDbStatus::Success;

    ElementGeometryBuilderPtr builder;
    DgnElement3dCP elem3d = el.ToElement3d();
    if (nullptr != elem3d)
        builder = ElementGeometryBuilder::Create(*el.GetModel(), el.GetCategoryId(), elem3d->GetPlacement().GetOrigin(), elem3d->GetPlacement().GetAngles());
    else
        {
        DgnElement2dCP elem2d = el.ToElement2d();
        BeAssert(nullptr != elem2d);
        builder = ElementGeometryBuilder::Create(*el.GetModel(), el.GetCategoryId(), elem2d->GetPlacement().GetOrigin(), elem2d->GetPlacement().GetAngle());
        }
    
    TextAnnotationGraphicsProcessor annotationGraphics(*m_annotation, el.GetCategoryId(), *builder);
    ElementGraphicsOutput::Process(annotationGraphics, el.GetDgnDb());

    builder->SetGeomStreamAndPlacement(el);
    
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
static TextAnnotationItemR getItemR(DgnElementR el)
    {
    TextAnnotationItemP item = TextAnnotationItem::GetP(el);
    if (nullptr == item)
        {
        item = new TextAnnotationItem();
        TextAnnotationItem::SetItem(el, *item);
        }

    return *item;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
TextAnnotationItemR TextAnnotationElement::GetItemR() { return getItemR(*this); }
TextAnnotationItemR PhysicalTextAnnotationElement::GetItemR() { return getItemR(*this); }
