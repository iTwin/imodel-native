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
    HANDLER_DEFINE_MEMBERS(TextAnnotationHandler);
    HANDLER_DEFINE_MEMBERS(PhysicalTextAnnotationHandler);
}

namespace dgn_AspectHandler
{
    HANDLER_DEFINE_MEMBERS(TextAnnotationItemHandler);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

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

    if (BE_SQLITE_DONE != update->Step())
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
#ifdef WIP_ELEMENT_ITEM // *** pending redesign
DgnDbStatus TextAnnotationItem::_GenerateElementGeometry(GeometricElementR el, GenerateReason reason)
#else
DgnDbStatus TextAnnotationItem::GenerateElementGeometry(GeometricElementR el, GenerateReason reason)
#endif
    {
    // To allow DgnV8 conversion to create first-class text elements, but provide custom WYSIWYG geometry.
    if (m_isGeometrySuppressed)
        return DgnDbStatus::Success;
    
    if (!m_annotation.IsValid())
        return DgnDbStatus::Success;

    ElementGeometryBuilderPtr builder;
    DgnElement3dCP elem3d = el.ToElement3d();
    if (nullptr != elem3d)
        {
        builder = ElementGeometryBuilder::Create(*el.GetModel(), el.GetCategoryId(), elem3d->GetPlacement().GetOrigin(), elem3d->GetPlacement().GetAngles());
        }
    else
        {
        DgnElement2dCP elem2d = el.ToElement2d();
        BeAssert(nullptr != elem2d);
        builder = ElementGeometryBuilder::Create(*el.GetModel(), el.GetCategoryId(), elem2d->GetPlacement().GetOrigin(), elem2d->GetPlacement().GetAngle());
        }
    
    builder->Append(*m_annotation);
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
        TextAnnotationItem::SetAspect(el, *item); // *** WIP_ELEMENT_ITEM 
        }

    return *item;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
TextAnnotationItemR TextAnnotationElement::GetItemR() { return getItemR(*this); }
TextAnnotationItemR PhysicalTextAnnotationElement::GetItemR() { return getItemR(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2015
//---------------------------------------------------------------------------------------
TextAnnotationItemCP TextAnnotationItem::GetCP(DgnElementCR el) // *** WIP_ELEMENT_ITEM 
    {
    ECN::ECClassCP ecclass = QueryECClass(el.GetDgnDb());
    if (nullptr == ecclass)
        return nullptr;
    return UniqueAspect::Get<TextAnnotationItem>(el, *ecclass); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2015
//---------------------------------------------------------------------------------------
TextAnnotationItemP TextAnnotationItem::GetP(DgnElementR el) // *** WIP_ELEMENT_ITEM 
    {
    ECN::ECClassCP ecclass = QueryECClass(el.GetDgnDb());
    if (nullptr == ecclass)
        return nullptr;
    return UniqueAspect::GetP<TextAnnotationItem>(el, *ecclass);
    }
