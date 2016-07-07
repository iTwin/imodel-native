/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Annotations/TextAnnotationElement.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include "Annotations.h"
#include "../DgnElement.h"
#include "../ElementHandler.h"
#include "../SnapContext.h"

DGNPLATFORM_TYPEDEFS(TextAnnotationData);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationData);
DGNPLATFORM_TYPEDEFS(TextAnnotation2d);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotation2d);
DGNPLATFORM_TYPEDEFS(TextAnnotation3d);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotation3d);

#define DGN_CLASSNAME_TextAnnotationData "TextAnnotationData"
#define DGN_CLASSNAME_TextAnnotation2d "TextAnnotation2d"
#define DGN_CLASSNAME_TextAnnotation3d "TextAnnotation3d"

BEGIN_BENTLEY_DGN_NAMESPACE

//__PUBLISH_SECTION_END__
// So we can friend ConvertV8TextToDgnDbExtension within TextAnnotationData.
namespace DgnDbSync { namespace DgnV8 { struct ConvertV8TextToDgnDbExtension; } }
//__PUBLISH_SECTION_START__

//=======================================================================================
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationData : DgnElement::UniqueAspect
{
    DGNASPECT_DECLARE_MEMBERS(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationData, DgnElement::UniqueAspect);

private:
//__PUBLISH_SECTION_END__
    // To allow DgnV8 conversion to create first-class text elements, but provide custom WYSIWYG geometry.
    friend struct DgnDbSync::DgnV8::ConvertV8TextToDgnDbExtension;
//__PUBLISH_SECTION_START__

    bool m_isGeometrySuppressed;

protected:
    TextAnnotationPtr m_annotation;
    
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateProperties(DgnElementCR) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LoadProperties(DgnElementCR) override;
    
public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationData); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); }
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
    // @bsiclass                                                    Jeff.Marker     09/2015
    //=======================================================================================
    struct TextAnnotationDataHandler : Aspect
    {
        DOMAINHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationData, TextAnnotationDataHandler, Aspect, DGNPLATFORM_EXPORT);
        RefCountedPtr<DgnElement::Aspect> _CreateInstance() override { return new TextAnnotationData(); }
    };
}

//=======================================================================================
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotation2d : AnnotationElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotation2d, AnnotationElement2d);

protected:
    virtual Render::GraphicPtr _StrokeHit(ViewContextR, HitDetailCR) const {return nullptr;} // Don't flash text box...
    virtual SnapStatus _OnSnap(SnapContextR context) const override { return context.DoTextSnap(); } // Default snap using text box...
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR originalElment) override;
    DGNPLATFORM_EXPORT DgnElementPtr virtual _Clone(DgnDbStatus* status=nullptr, DgnElement::CreateParams const* params=nullptr) const;
    DGNPLATFORM_EXPORT DgnElementPtr virtual _CloneForImport(DgnDbStatus*, DgnModelR destModel, DgnImportContext&) const;
    TextAnnotationDataCP GetItemCP() const { return TextAnnotationData::GetCP(*this); }
    DGNPLATFORM_EXPORT TextAnnotationDataR GetItemR();

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotation2d); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static TextAnnotation2dCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<TextAnnotation2d>(id); }
    static TextAnnotation2dPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<TextAnnotation2d>(id); }

    explicit TextAnnotation2d(CreateParams const& params) : T_Super(params) {}
    static TextAnnotation2dPtr Create(CreateParams const& params) { return new TextAnnotation2d(params); }

    TextAnnotationCP GetAnnotation() const { TextAnnotationDataCP item = GetItemCP(); return item ? item->GetAnnotation() : nullptr; }
    void SetAnnotation(TextAnnotationCP value) { GetItemR().SetAnnotation(value); }
    
    TextAnnotation2dCPtr Insert() { return GetDgnDb().Elements().Insert<TextAnnotation2d>(*this); }
    TextAnnotation2dCPtr Update() { return GetDgnDb().Elements().Update<TextAnnotation2d>(*this); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     09/2015
    //=======================================================================================
    struct TextAnnotation2dHandler : Annotation2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotation2d, TextAnnotation2d, TextAnnotation2dHandler, Annotation2d, DGNPLATFORM_EXPORT);
    };
}

//=======================================================================================
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotation3d : GraphicalElement3d
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotation3d, GraphicalElement3d);

protected:
    virtual Render::GraphicPtr _StrokeHit(ViewContextR, HitDetailCR) const {return nullptr;} // Don't flash text box...
    virtual SnapStatus _OnSnap(SnapContextR context) const override { return context.DoTextSnap(); } // Default snap using text box...
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR originalElment) override;
    DGNPLATFORM_EXPORT DgnElementPtr virtual _Clone(DgnDbStatus* status=nullptr, DgnElement::CreateParams const* params=nullptr) const;
    DGNPLATFORM_EXPORT DgnElementPtr virtual _CloneForImport(DgnDbStatus*, DgnModelR destModel, DgnImportContext&) const;
    TextAnnotationDataCP GetItemCP() const { return TextAnnotationData::GetCP(*this); }
    DGNPLATFORM_EXPORT TextAnnotationDataR GetItemR();

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotation3d); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static TextAnnotation3dCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<TextAnnotation3d>(id); }
    static TextAnnotation3dPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<TextAnnotation3d>(id); }

    explicit TextAnnotation3d(CreateParams const& params) : T_Super(params) {}
    static TextAnnotation3dPtr Create(CreateParams const& params) { return new TextAnnotation3d(params); }
    
    TextAnnotationCP GetAnnotation() const { TextAnnotationDataCP item = GetItemCP(); return item ? item->GetAnnotation() : nullptr; }
    void SetAnnotation(TextAnnotationCP value) { GetItemR().SetAnnotation(value); }
    
    TextAnnotation3dCPtr Insert() { return GetDgnDb().Elements().Insert<TextAnnotation3d>(*this); }
    TextAnnotation3dCPtr Update() { return GetDgnDb().Elements().Update<TextAnnotation3d>(*this); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     09/2015
    //=======================================================================================
    struct TextAnnotation3dHandler : Geometric3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotation3d, TextAnnotation3d, TextAnnotation3dHandler, Geometric3d, DGNPLATFORM_EXPORT);
    };
}

END_BENTLEY_DGN_NAMESPACE
