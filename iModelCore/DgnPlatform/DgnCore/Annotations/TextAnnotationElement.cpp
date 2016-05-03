/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/TextAnnotationElement.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatform/Annotations/TextAnnotationElement.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationPersistence.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(TextAnnotation2dHandler);
    HANDLER_DEFINE_MEMBERS(TextAnnotation3dHandler);
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
void TextAnnotationData::GenerateGeometricPrimitive(GeometrySourceR source, GenerateReason reason) const
    {
    // To allow DgnV8 conversion to create first-class text elements, but provide custom WYSIWYG geometry.
    if (m_isGeometrySuppressed)
        return;
    
    if (!m_annotation.IsValid())
        return;

    GeometryBuilderPtr builder = GeometryBuilder::Create(source);
    
    builder->Append(*m_annotation);
    builder->SetGeometryStreamAndPlacement(source);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2016
//---------------------------------------------------------------------------------------
void TextAnnotationData::CopyFrom(TextAnnotationData const& rhs)
    {
    SetAnnotation(rhs.m_annotation.get());
    m_isGeometrySuppressed = rhs.m_isGeometrySuppressed;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2015
//---------------------------------------------------------------------------------------
void TextAnnotationData::RemapIds(DgnImportContext& context)
    {
    if (!m_annotation.IsValid())
        return;
    
    AnnotationTextBlockP text = m_annotation->GetTextP();
    if (nullptr == text)
        return;

    for (AnnotationParagraphPtr paragraph : text->GetParagraphs())
    for (AnnotationRunBasePtr run : paragraph->GetRuns())
        {
        if (!run->GetStyleOverrides().HasProperty(AnnotationTextStyleProperty::FontId))
            continue;
        
        DgnFontId srcFontId = DgnFontId((uint64_t)run->GetStyleOverrides().GetIntegerProperty(AnnotationTextStyleProperty::FontId));
        DgnFontId dstFontId = context.RemapFont(srcFontId);
        
        run->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::FontId, (int64_t)dstFontId.GetValue());
        }
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
TextAnnotationDataR TextAnnotation2d::GetItemR() { return getItemR(*this); }
TextAnnotationDataR TextAnnotation3d::GetItemR() { return getItemR(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2015
//---------------------------------------------------------------------------------------
static DgnDbStatus updateGeometryOnChange(DgnDbStatus superStatus, DgnElementR el, TextAnnotationDataCP item, DgnElement::UniqueAspect::GenerateReason reason)
    {
    if (DgnDbStatus::Success != superStatus)
        return superStatus;
    
    if (nullptr != item && nullptr != el.ToGeometrySourceP())
        item->GenerateGeometricPrimitive(*el.ToGeometrySourceP(), reason);

    return DgnDbStatus::Success;
    }
DgnDbStatus TextAnnotation2d::_OnInsert() { return updateGeometryOnChange(T_Super::_OnInsert(), *this, GetItemCP(), DgnElement::UniqueAspect::GenerateReason::Insert); }
DgnDbStatus TextAnnotation3d::_OnInsert() { return updateGeometryOnChange(T_Super::_OnInsert(), *this, GetItemCP(), DgnElement::UniqueAspect::GenerateReason::Insert); }
DgnDbStatus TextAnnotation2d::_OnUpdate(DgnElementCR el) { return updateGeometryOnChange(T_Super::_OnUpdate(el), *this, GetItemCP(), DgnElement::UniqueAspect::GenerateReason::Update); }
DgnDbStatus TextAnnotation3d::_OnUpdate(DgnElementCR el) { return updateGeometryOnChange(T_Super::_OnUpdate(el), *this, GetItemCP(), DgnElement::UniqueAspect::GenerateReason::Update); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2016
//---------------------------------------------------------------------------------------
static TextAnnotationDataP cloneData(DgnElementCR oldElem, DgnElementR newElem)
    {
    TextAnnotationDataCP oldData = TextAnnotationData::GetCP(oldElem);
    if (nullptr == oldData)
        return nullptr;

    TextAnnotationDataP newData = new TextAnnotationData();
    newData->CopyFrom(*oldData);
    TextAnnotationData::SetAspect(newElem, *newData);

    return newData;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2016
//---------------------------------------------------------------------------------------
DgnElementPtr TextAnnotation2d::_Clone(DgnDbStatus* status, DgnElement::CreateParams const* params) const
    {
    DgnDbStatus _status;
    if (nullptr == status)
        status = &_status;

    DgnElementPtr newElem = T_Super::_Clone(status, params);
    if (!newElem.IsValid() || (DgnDbStatus::Success != *status))
        return newElem;
    
    cloneData(*this, *newElem);

    return newElem;
    }
DgnElementPtr TextAnnotation3d::_Clone(DgnDbStatus* status, DgnElement::CreateParams const* params) const
    {
    DgnDbStatus _status;
    if (nullptr == status)
        status = &_status;

    DgnElementPtr newElem = T_Super::_Clone(status, params);
    if (!newElem.IsValid() || (DgnDbStatus::Success != *status))
        return newElem;
    
    cloneData(*this, *newElem);

    return newElem;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2016
//---------------------------------------------------------------------------------------
DgnElementPtr TextAnnotation2d::_CloneForImport(DgnDbStatus* status, DgnModelR destModel, DgnImportContext& context) const
    {
    DgnDbStatus _status;
    if (nullptr == status)
        status = &_status;

    DgnElementPtr newElem = T_Super::_CloneForImport(status, destModel, context);
    if (!newElem.IsValid() || (DgnDbStatus::Success != *status))
        return newElem;
    
    TextAnnotationDataP newData = cloneData(*this, *newElem);
    if ((nullptr != newData) && context.IsBetweenDbs())
        newData->RemapIds(context);

    return newElem;
    }
DgnElementPtr TextAnnotation3d::_CloneForImport(DgnDbStatus* status, DgnModelR destModel, DgnImportContext& context) const
    {
    DgnDbStatus _status;
    if (nullptr == status)
        status = &_status;

    DgnElementPtr newElem = T_Super::_CloneForImport(status, destModel, context);
    if (!newElem.IsValid() || (DgnDbStatus::Success != *status))
        return newElem;
    
    TextAnnotationDataP newData = cloneData(*this, *newElem);
    if ((nullptr != newData) && context.IsBetweenDbs())
        newData->RemapIds(context);

    return newElem;
    }
