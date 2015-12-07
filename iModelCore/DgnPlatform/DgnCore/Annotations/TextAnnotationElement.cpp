/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/TextAnnotationElement.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatform/Annotations/TextAnnotationElement.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationPersistence.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(TextAnnotationHandler);
    HANDLER_DEFINE_MEMBERS(PhysicalTextAnnotationHandler);
}

namespace dgn_AspectHandler
{
    HANDLER_DEFINE_MEMBERS(TextAnnotationDataHandler);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
DgnDbStatus TextAnnotationData::_UpdateProperties(DgnElementCR el)
    {
    // T_Super::_UpdateProperties is pure; it is a link error to call super, so don't.
    
    bvector<Byte> annotationBlob;
    if (m_annotation.IsValid())
        {
        if (SUCCESS != TextAnnotationPersistence::EncodeAsFlatBuf(annotationBlob, *m_annotation))
            return DgnDbStatus::WriteError;
        }

    CachedECSqlStatementPtr update = el.GetDgnDb().GetPreparedECSqlStatement("UPDATE " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationData) " SET TextAnnotation=? WHERE ECInstanceId=?");
    if (!update.IsValid())
        return DgnDbStatus::WriteError;

    if (annotationBlob.empty())
        update->BindNull(1);
    else
        update->BindBinary(1, &annotationBlob[0], (int)annotationBlob.size(), IECSqlBinder::MakeCopy::No);

    update->BindId(2, el.GetElementId());

    if (BE_SQLITE_DONE != update->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
DgnDbStatus TextAnnotationData::_LoadProperties(DgnElementCR el)
    {
    // T_Super::_LoadProperties is pure; it is a link error to call super, so don't.
    
    CachedECSqlStatementPtr select = el.GetDgnDb().GetPreparedECSqlStatement("SELECT TextAnnotation FROM " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationData) " WHERE ECInstanceId=?");
    if (!select.IsValid())
        return DgnDbStatus::ReadError;

    select->BindId(1, el.GetElementId());

    if ((BE_SQLITE_ROW != select->Step()) || select->IsValueNull(0))
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
void TextAnnotationData::GenerateElementGeometry(GeometrySourceR source, GenerateReason reason) const
    {
    // To allow DgnV8 conversion to create first-class text elements, but provide custom WYSIWYG geometry.
    if (m_isGeometrySuppressed)
        return;
    
    if (!m_annotation.IsValid())
        return;

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(source);
    
    builder->Append(*m_annotation);
    builder->SetGeomStreamAndPlacement(source);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
static TextAnnotationDataR getItemR(DgnElementR el)
    {
    TextAnnotationDataP item = TextAnnotationData::GetP(el);
    if (nullptr == item)
        {
        item = new TextAnnotationData();
        TextAnnotationData::SetAspect(el, *item);
        }

    return *item;
    }
TextAnnotationDataR TextAnnotationElement::GetItemR() { return getItemR(*this); }
TextAnnotationDataR PhysicalTextAnnotationElement::GetItemR() { return getItemR(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2015
//---------------------------------------------------------------------------------------
static DgnDbStatus updateGeometryOnChange(DgnDbStatus superStatus, DgnElementR el, TextAnnotationDataCP item, DgnElement::UniqueAspect::GenerateReason reason)
    {
    if (DgnDbStatus::Success != superStatus)
        return superStatus;
    
    if (nullptr != item && nullptr != el.ToGeometrySourceP())
        item->GenerateElementGeometry(*el.ToGeometrySourceP(), reason);

    return DgnDbStatus::Success;
    }
DgnDbStatus TextAnnotationElement::_OnInsert() { return updateGeometryOnChange(T_Super::_OnInsert(), *this, GetItemCP(), DgnElement::UniqueAspect::GenerateReason::Insert); }
DgnDbStatus PhysicalTextAnnotationElement::_OnInsert() { return updateGeometryOnChange(T_Super::_OnInsert(), *this, GetItemCP(), DgnElement::UniqueAspect::GenerateReason::Insert); }
DgnDbStatus TextAnnotationElement::_OnUpdate(DgnElementCR el) { return updateGeometryOnChange(T_Super::_OnUpdate(el), *this, GetItemCP(), DgnElement::UniqueAspect::GenerateReason::Update); }
DgnDbStatus PhysicalTextAnnotationElement::_OnUpdate(DgnElementCR el) { return updateGeometryOnChange(T_Super::_OnUpdate(el), *this, GetItemCP(), DgnElement::UniqueAspect::GenerateReason::Update); }
