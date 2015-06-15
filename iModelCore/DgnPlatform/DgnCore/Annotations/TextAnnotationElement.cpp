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

HANDLER_DEFINE_MEMBERS(PhysicalTextAnnotationElementHandler);

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
BentleyStatus PhysicalTextAnnotationElement::SetAnnotation(TextAnnotationCR value)
    {
    if (&value.GetDbR() != &GetDgnDb())
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::SetText - New value must be in the same database.");
        return ERROR;
        }
    
    m_annotation = value.Clone();
    UpdateGeometryRepresentation();

    return SUCCESS;
    }

//=======================================================================================
// TextAnnotationDraw prefers a context, so use an IElementGraphicsProcessor to listen to context events and emit to an ElementGeometryBuilder.
// Note that this is tightly coupled with TextAnnotationDraw (and AnnotationFrameDraw, AnnotationLeaderDraw, and AnnotationTextBlockDraw), which uses a subset of draw commands.
// @bsiclass                                                    Jeff.Marker     06/2015
//=======================================================================================
struct TextAnnotationGraphicsProcessor : IElementGraphicsProcessor
{
    TextAnnotationCR m_annotation;
    DgnCategoryId m_categoryId;
    ElementGeometryBuilderR m_builder;
    Transform m_transform;

    TextAnnotationGraphicsProcessor(TextAnnotationCR annotation, DgnCategoryId categoryId, ElementGeometryBuilderR builder) :
        m_annotation(annotation), m_categoryId(categoryId), m_builder(builder), m_transform(Transform::FromIdentity()) {}

    virtual void _AnnounceTransform(TransformCP transform) override { (nullptr != transform) ? m_transform = *transform : m_transform.InitIdentity(); }
    virtual void _AnnounceElemDisplayParams(ElemDisplayParamsCR params) override { m_builder.Append(params); }
    virtual BentleyStatus _ProcessTextString(TextStringCR) override;
    virtual BentleyStatus _ProcessCurveVector(CurveVectorCR, bool isFilled) override;
    virtual void _OutputGraphics(ViewContextR) override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
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
// @bsimethod                                                   Jeff.Marker     06/2015
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
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
void TextAnnotationGraphicsProcessor::_OutputGraphics(ViewContextR context)
    {
    context.GetCurrentDisplayParams()->SetCategoryId(m_categoryId);
    
    TextAnnotationDraw annotationDraw(m_annotation);
    annotationDraw.Draw(context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
void PhysicalTextAnnotationElement::UpdateGeometryRepresentation()
    {
    if (!m_annotation.IsValid())
        return;
    
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(m_dgnModel, m_categoryId, m_placement.GetOrigin(), m_placement.GetAngles());
    TextAnnotationGraphicsProcessor annotationGraphics(*m_annotation, m_categoryId, *builder);
    ElementGraphicsOutput::Process(annotationGraphics, m_dgnModel.GetDgnDb());

    builder->SetGeomStreamAndPlacement(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnDbStatus PhysicalTextAnnotationElement::UpdatePropertiesInDb()
    {
    bvector<Byte> annotationBlob;
    if (m_annotation.IsValid())
        {
        if (SUCCESS != TextAnnotationPersistence::EncodeAsFlatBuf(annotationBlob, *m_annotation))
            {
            DGNCORELOG->error("PhysicalTextAnnotationElement::UpdatePropertiesInDb - TextAnnotation serialization failed.");
            return DgnDbStatus::ElementWriteError;
            }
        }
    
    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("UPDATE " DGN_SCHEMA(DGN_CLASSNAME_PhysicalTextAnnotationElement) " SET TextAnnotationBlob=? WHERE ECInstanceId=?");
    if (!statement.IsValid())
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::UpdatePropertiesInDb - Update ECSql statement failed to prepare.");
        return DgnDbStatus::ElementWriteError;
        }

    if (annotationBlob.empty())
        statement->BindNull(1);
    else
        statement->BindBinary(1, &annotationBlob[0], (int)annotationBlob.size(), IECSqlBinder::MakeCopy::No);

    statement->BindId(2, GetElementId());

    if (ECSqlStepStatus::Done != statement->Step())
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::UpdatePropertiesInDb - Update ECSql statement failed to step.");
        return DgnDbStatus::ElementWriteError;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnDbStatus PhysicalTextAnnotationElement::_InsertInDb()
    {
    DgnDbStatus status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return UpdatePropertiesInDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnDbStatus PhysicalTextAnnotationElement::_UpdateInDb()
    {
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return UpdatePropertiesInDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnDbStatus PhysicalTextAnnotationElement::_LoadFromDb()
    {
    DgnDbStatus status = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != status)
        return status;
    
    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TextAnnotationBlob FROM " DGN_SCHEMA(DGN_CLASSNAME_PhysicalTextAnnotationElement) " WHERE ECInstanceId=?");
    if (!statement.IsValid())
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::_LoadFromDb - Select ECSql statement failed to prepare.");
        return DgnDbStatus::ElementReadError;
        }

    statement->BindId(1, GetElementId());
    
    // Should always be able to find this row by-ID, even if data is null...
    if (ECSqlStepStatus::HasRow != statement->Step())
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::_LoadFromDb - Select ECSql statement failed to step.");
        return DgnDbStatus::ElementReadError;
        }

    // An annotation element with no annotation is pretty meaningless, but not strictly a read error either...
    if (statement->IsValueNull(0))
        {
        m_annotation = nullptr;
        return DgnDbStatus::Success;
        }

    int dataSize = 0;
    ByteCP data = (ByteCP)statement->GetValueBinary(0, &dataSize);
    if ((0 == dataSize) || (nullptr == data))
        return DgnDbStatus::Success;

    // Was an annotation never provided? Seed one so we can fill it in from the DB.
    if (!m_annotation.IsValid())
        m_annotation = TextAnnotation::Create(GetDgnDb());
    
    if (SUCCESS != TextAnnotationPersistence::DecodeFromFlatBuf(*m_annotation, data, (size_t)dataSize))
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::_LoadFromDb - TextAnnotation deserialization failed.");
        return DgnDbStatus::ElementReadError;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
void PhysicalTextAnnotationElement::_CopyFrom(DgnElementCR rhsElement)
    {
    T_Super::_CopyFrom(rhsElement);

    PhysicalTextAnnotationElementCP rhs = dynamic_cast<PhysicalTextAnnotationElementCP>(&rhsElement);
    if (nullptr == rhs)
        return;

    m_annotation = (rhs->m_annotation.IsValid() ? rhs->m_annotation->Clone() : nullptr);
    }
