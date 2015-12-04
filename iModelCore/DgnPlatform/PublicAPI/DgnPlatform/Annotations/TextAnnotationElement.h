/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Annotations/TextAnnotationElement.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
DGNPLATFORM_TYPEDEFS(TextAnnotationElement);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationElement);
DGNPLATFORM_TYPEDEFS(PhysicalTextAnnotationElement);
DGNPLATFORM_REF_COUNTED_PTR(PhysicalTextAnnotationElement);

#define DGN_CLASSNAME_TextAnnotationData "TextAnnotationData"
#define DGN_CLASSNAME_TextAnnotationElement "TextAnnotationElement"
#define DGN_CLASSNAME_PhysicalTextAnnotationElement "PhysicalTextAnnotationElement"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
// So we can friend ConvertV8TextToDgnDbExtension within TextAnnotationData.
namespace DgnDbSync { namespace DgnV8 { struct ConvertV8TextToDgnDbExtension; } }
//__PUBLISH_SECTION_START__

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationData : DgnElement::UniqueAspect
{
    DGNASPECT_DECLARE_MEMBERS(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationData, DgnElement::UniqueAspect);

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
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationData); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static TextAnnotationDataCP GetCP(DgnElementCR el) { return UniqueAspect::Get<TextAnnotationData>(el, *QueryECClass(el.GetDgnDb())); }
    static TextAnnotationDataP GetP(DgnElementR el) { return UniqueAspect::GetP<TextAnnotationData>(el, *QueryECClass(el.GetDgnDb())); }

    TextAnnotationData() : m_isGeometrySuppressed(false) {}
    TextAnnotationCP GetAnnotation() const { return m_annotation.get(); }
    void SetAnnotation(TextAnnotationCP value) { m_annotation = value ? value->Clone() : nullptr; }
    DGNPLATFORM_EXPORT void GenerateElementGeometry(GeometrySourceR, GenerateReason) const;
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
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationElement : AnnotationElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationElement, AnnotationElement);

protected:
    virtual bool _DrawHit(HitDetailCR, ViewContextR) const override { return false; } // Don't flash text box...
    virtual SnapStatus _OnSnap(SnapContextR context) const override { return context.DoTextSnap(); } // Default snap using text box...
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR originalElment) override;
    TextAnnotationDataCP GetItemCP() const { return TextAnnotationData::GetCP(*this); }
    DGNPLATFORM_EXPORT TextAnnotationDataR GetItemR();

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationElement); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static TextAnnotationElementCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<TextAnnotationElement>(id); }
    static TextAnnotationElementPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<TextAnnotationElement>(id); }

    explicit TextAnnotationElement(CreateParams const& params) : T_Super(params) {}
    static TextAnnotationElementPtr Create(CreateParams const& params) { return new TextAnnotationElement(params); }

    TextAnnotationCP GetAnnotation() const { TextAnnotationDataCP item = GetItemCP(); return item ? item->GetAnnotation() : nullptr; }
    void SetAnnotation(TextAnnotationCP value) { GetItemR().SetAnnotation(value); }
    
    TextAnnotationElementCPtr Insert() { return GetDgnDb().Elements().Insert<TextAnnotationElement>(*this); }
    TextAnnotationElementCPtr Update() { return GetDgnDb().Elements().Update<TextAnnotationElement>(*this); }
    DgnDbStatus Delete() { return GetDgnDb().Elements().Delete(*this); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     09/2015
    //=======================================================================================
    struct TextAnnotationHandler : Annotation
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationElement, TextAnnotationElement, TextAnnotationHandler, Annotation, DGNPLATFORM_EXPORT);
    };
}

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalTextAnnotationElement : PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_PhysicalTextAnnotationElement, PhysicalElement);

protected:
    virtual bool _DrawHit(HitDetailCR, ViewContextR) const override { return false; } // Don't flash text box...
    virtual SnapStatus _OnSnap(SnapContextR context) const override { return context.DoTextSnap(); } // Default snap using text box...
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR originalElment) override;
    TextAnnotationDataCP GetItemCP() const { return TextAnnotationData::GetCP(*this); }
    DGNPLATFORM_EXPORT TextAnnotationDataR GetItemR();

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalTextAnnotationElement); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static PhysicalTextAnnotationElementCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<PhysicalTextAnnotationElement>(id); }
    static PhysicalTextAnnotationElementPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<PhysicalTextAnnotationElement>(id); }

    explicit PhysicalTextAnnotationElement(CreateParams const& params) : T_Super(params) {}
    static PhysicalTextAnnotationElementPtr Create(CreateParams const& params) { return new PhysicalTextAnnotationElement(params); }
    
    TextAnnotationCP GetAnnotation() const { TextAnnotationDataCP item = GetItemCP(); return item ? item->GetAnnotation() : nullptr; }
    void SetAnnotation(TextAnnotationCP value) { GetItemR().SetAnnotation(value); }
    
    PhysicalTextAnnotationElementCPtr Insert() { return GetDgnDb().Elements().Insert<PhysicalTextAnnotationElement>(*this); }
    PhysicalTextAnnotationElementCPtr Update() { return GetDgnDb().Elements().Update<PhysicalTextAnnotationElement>(*this); }
    DgnDbStatus Delete() { return GetDgnDb().Elements().Delete(*this); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     09/2015
    //=======================================================================================
    struct PhysicalTextAnnotationHandler : Physical
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_PhysicalTextAnnotationElement, PhysicalTextAnnotationElement, PhysicalTextAnnotationHandler, Physical, DGNPLATFORM_EXPORT);
    };
}

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
