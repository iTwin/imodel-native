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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
void PhysicalTextAnnotationElement::UpdateGeometryRepresentation()
    {
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
