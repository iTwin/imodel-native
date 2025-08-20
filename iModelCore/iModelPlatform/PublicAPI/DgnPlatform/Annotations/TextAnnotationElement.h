/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#include "Annotations.h"
#include "../DgnElement.h"
#include "../ElementHandler.h"

DGNPLATFORM_TYPEDEFS(TextAnnotationData);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationData);
DGNPLATFORM_TYPEDEFS(TextAnnotation2d);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotation2d);
DGNPLATFORM_TYPEDEFS(TextAnnotation3d);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotation3d);

#define BIS_CLASS_TextAnnotationData "TextAnnotationData"
#define BIS_CLASS_TextAnnotation2d "TextAnnotation2d"
#define BIS_CLASS_TextAnnotation3d "TextAnnotation3d"

BEGIN_BENTLEY_DGN_NAMESPACE

// So we can friend ConvertV8TextToDgnDbExtension within TextAnnotationData.
namespace DgnDbSync { namespace DgnV8 { struct ConvertV8TextToDgnDbExtension; } }

//=======================================================================================
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationData : DgnElement::UniqueAspect
{
    DGNASPECT_DECLARE_MEMBERS(BIS_ECSCHEMA_NAME, BIS_CLASS_TextAnnotationData, DgnElement::UniqueAspect);

private:
    // To allow DgnV8 conversion to create first-class text elements, but provide custom WYSIWYG geometry.
    friend struct DgnDbSync::DgnV8::ConvertV8TextToDgnDbExtension;
    // Similarly for the remap cleanup logic to leave DgnV8 visual fidelity alone.
    friend struct RemapXSA;

    bool m_isGeometrySuppressed;

protected:
    TextAnnotationPtr m_annotation;

    DGNPLATFORM_EXPORT DgnDbStatus _UpdateProperties(DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
    DGNPLATFORM_EXPORT DgnDbStatus _LoadProperties(DgnElementCR) override;
    DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, PropertyArrayIndex const&) const override {return DgnDbStatus::NotEnabled;}
    DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, PropertyArrayIndex const&) override {return DgnDbStatus::NotEnabled;}

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_TextAnnotationData); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static TextAnnotationDataCP GetCP(DgnElementCR el) { return UniqueAspect::Get<TextAnnotationData>(el, *QueryECClass(el.GetDgnDb())); }
    static TextAnnotationDataP GetP(DgnElementR el) { return UniqueAspect::GetP<TextAnnotationData>(el, *QueryECClass(el.GetDgnDb())); }

    TextAnnotationData() : m_isGeometrySuppressed(false) {}
    TextAnnotationCP GetAnnotation() const { return m_annotation.get(); }
    void SetAnnotation(TextAnnotationCP value) { m_annotation = value ? value->Clone() : nullptr; }
    DGNPLATFORM_EXPORT void GenerateGeometricPrimitive(GeometrySourceR, GenerateReason) const;
    DGNPLATFORM_EXPORT void CopyFrom(TextAnnotationData const&);
    DGNPLATFORM_EXPORT void RemapIds(DgnImportContext&);
};

namespace dgn_AspectHandler
{
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct TextAnnotationDataHandler : Aspect
    {
        DOMAINHANDLER_DECLARE_MEMBERS(BIS_CLASS_TextAnnotationData, TextAnnotationDataHandler, Aspect, DGNPLATFORM_EXPORT);
        RefCountedPtr<DgnElement::Aspect> _CreateInstance() override { return new TextAnnotationData(); }
    };
}

//=======================================================================================
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotation2d : AnnotationElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_TextAnnotation2d, AnnotationElement2d);

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnUpdate(DgnElementCR originalElment) override;
    DGNPLATFORM_EXPORT DgnElementPtr _Clone(DgnDbStatus* status=nullptr, DgnElement::CreateParams const* params=nullptr) const override;
    DGNPLATFORM_EXPORT DgnElementPtr _CloneForImport(DgnDbStatus*, DgnModelR destModel, DgnImportContext&) const override;
    TextAnnotationDataCP GetItemCP() const { return TextAnnotationData::GetCP(*this); }
    DGNPLATFORM_EXPORT TextAnnotationDataR GetItemR();

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_TextAnnotation2d); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static TextAnnotation2dCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<TextAnnotation2d>(id); }
    static TextAnnotation2dPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<TextAnnotation2d>(id); }

    explicit TextAnnotation2d(CreateParams const& params) : T_Super(params) {}
    static TextAnnotation2dPtr Create(CreateParams const& params) { return new TextAnnotation2d(params); }

    TextAnnotationCP GetAnnotation() const { TextAnnotationDataCP item = GetItemCP(); return item ? item->GetAnnotation() : nullptr; }
    void SetAnnotation(TextAnnotationCP value) { GetItemR().SetAnnotation(value); }

    TextAnnotation2dCPtr Insert() { return GetDgnDb().Elements().Insert<TextAnnotation2d>(*this); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct TextAnnotation2dHandler : Annotation2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_TextAnnotation2d, TextAnnotation2d, TextAnnotation2dHandler, Annotation2d, DGNPLATFORM_EXPORT);
    };
}

//=======================================================================================
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotation3d : GraphicalElement3d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_TextAnnotation3d, GraphicalElement3d);

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnUpdate(DgnElementCR originalElment) override;
    DGNPLATFORM_EXPORT DgnElementPtr _Clone(DgnDbStatus* status=nullptr, DgnElement::CreateParams const* params=nullptr) const override;
    DGNPLATFORM_EXPORT DgnElementPtr _CloneForImport(DgnDbStatus*, DgnModelR destModel, DgnImportContext&) const override;
    TextAnnotationDataCP GetItemCP() const { return TextAnnotationData::GetCP(*this); }
    DGNPLATFORM_EXPORT TextAnnotationDataR GetItemR();

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_TextAnnotation3d); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static TextAnnotation3dCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<TextAnnotation3d>(id); }
    static TextAnnotation3dPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<TextAnnotation3d>(id); }

    explicit TextAnnotation3d(CreateParams const& params) : T_Super(params) {}
    static TextAnnotation3dPtr Create(CreateParams const& params) { return new TextAnnotation3d(params); }

    TextAnnotationCP GetAnnotation() const { TextAnnotationDataCP item = GetItemCP(); return item ? item->GetAnnotation() : nullptr; }
    void SetAnnotation(TextAnnotationCP value) { GetItemR().SetAnnotation(value); }

    TextAnnotation3dCPtr Insert() { return GetDgnDb().Elements().Insert<TextAnnotation3d>(*this); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct TextAnnotation3dHandler : Geometric3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_TextAnnotation3d, TextAnnotation3d, TextAnnotation3dHandler, Geometric3d, DGNPLATFORM_EXPORT);
    };
}

END_BENTLEY_DGN_NAMESPACE
