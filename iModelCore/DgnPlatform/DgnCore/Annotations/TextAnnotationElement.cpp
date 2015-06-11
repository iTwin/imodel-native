//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/TextAnnotationElement.cpp $ 
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $ 
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatform/DgnCore/Annotations/TextAnnotationElement.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationPersistence.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

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
DgnModelStatus PhysicalTextAnnotationElement::UpdatePropertiesInDb()
    {
    bvector<Byte> annotationBlob;
    if (m_annotation.IsValid())
        {
        if (SUCCESS != TextAnnotationPersistence::EncodeAsFlatBuf(annotationBlob, *m_annotation))
            {
            DGNCORELOG->error("PhysicalTextAnnotationElement::UpdatePropertiesInDb - TextAnnotation serialization failed.");
            return DGNMODEL_STATUS_ElementWriteError;
            }
        }
    
    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("UPDATE " DGN_SCHEMA(DGN_CLASSNAME_PhysicalTextAnnotationElement) " SET TextAnnotationBlob=? WHERE ECInstanceId=?");
    if (!statement.IsValid())
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::UpdatePropertiesInDb - Update ECSql statement failed to prepare.");
        return DGNMODEL_STATUS_ElementWriteError;
        }

    if (annotationBlob.empty())
        statement->BindNull(1);
    else
        statement->BindBinary(1, &annotationBlob[0], (int)annotationBlob.size(), IECSqlBinder::MakeCopy::No);

    statement->BindId(2, GetElementId());

    if (ECSqlStepStatus::Done != statement->Step())
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::UpdatePropertiesInDb - Update ECSql statement failed to step.");
        return DGNMODEL_STATUS_ElementWriteError;
        }

    return DGNMODEL_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnModelStatus PhysicalTextAnnotationElement::_InsertInDb()
    {
    DgnModelStatus status = T_Super::_InsertInDb();
    if (DGNMODEL_STATUS_Success != status)
        return status;

    return UpdatePropertiesInDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnModelStatus PhysicalTextAnnotationElement::_UpdateInDb()
    {
    DgnModelStatus status = T_Super::_UpdateInDb();
    if (DGNMODEL_STATUS_Success != status)
        return status;

    return UpdatePropertiesInDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnModelStatus PhysicalTextAnnotationElement::_LoadFromDb()
    {
    DgnModelStatus status = T_Super::_LoadFromDb();
    if (DGNMODEL_STATUS_Success != status)
        return status;
    
    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TextAnnotationBlob FROM " DGN_SCHEMA(DGN_CLASSNAME_PhysicalTextAnnotationElement) " WHERE ECInstanceId=?");
    if (!statement.IsValid())
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::_LoadFromDb - Select ECSql statement failed to prepare.");
        return DGNMODEL_STATUS_ElementReadError;
        }

    statement->BindId(1, GetElementId());
    
    // Should always be able to find this row by-ID, even if data is null...
    if (ECSqlStepStatus::HasRow != statement->Step())
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::_LoadFromDb - Select ECSql statement failed to step.");
        return DGNMODEL_STATUS_ElementReadError;
        }

    // An annotation element with no annotation is pretty meaningless, but not strictly a read error either...
    if (statement->IsValueNull(0))
        {
        m_annotation = nullptr;
        return DGNMODEL_STATUS_Success;
        }

    int dataSize = 0;
    ByteCP data = (ByteCP)statement->GetValueBinary(0, &dataSize);
    if ((0 == dataSize) || (nullptr == data))
        return DGNMODEL_STATUS_Success;

    // Was an annotation never provided? Seed one so we can fill it in from the DB.
    if (!m_annotation.IsValid())
        m_annotation = TextAnnotation::Create(GetDgnDb());
    
    if (SUCCESS != TextAnnotationPersistence::DecodeFromFlatBuf(*m_annotation, data, (size_t)dataSize))
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::_LoadFromDb - TextAnnotation deserialization failed.");
        return DGNMODEL_STATUS_ElementReadError;
        }

    return DGNMODEL_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnModelStatus PhysicalTextAnnotationElement::_CopyFrom(DgnElementCR rhsElement)
    {
    DgnModelStatus status = T_Super::_CopyFrom(rhsElement);
    if (DGNMODEL_STATUS_Success != status)
        return status;

    PhysicalTextAnnotationElementCP rhs = dynamic_cast<PhysicalTextAnnotationElementCP>(&rhsElement);
    if (nullptr == rhs)
        {
        DGNCORELOG->error("PhysicalTextAnnotationElement::_CopyFrom - Other element must be a PhysicalTextAnnotationElement.");
        return DGNMODEL_STATUS_WrongElement;
        }

    m_annotation = (rhs->m_annotation.IsValid() ? rhs->m_annotation->Clone() : nullptr);

    return DGNMODEL_STATUS_Success;
    }
