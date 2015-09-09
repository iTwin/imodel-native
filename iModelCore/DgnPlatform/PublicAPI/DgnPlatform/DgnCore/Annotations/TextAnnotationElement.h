/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/TextAnnotationElement.h $
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

DGNPLATFORM_TYPEDEFS(TextAnnotationDataAspect);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationDataAspect);
DGNPLATFORM_TYPEDEFS(TextAnnotationElement);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationElement);

#define DGN_CLASSNAME_TextAnnotationDataAspect "TextAnnotationDataAspect"
#define DGN_CLASSNAME_TextAnnotationElement "TextAnnotationElement"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationDataAspect : DgnElement::UniqueAspect
{
    DGNASPECT_DECLARE_MEMBERS(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationDataAspect, UniqueAspect);

protected:
    TextAnnotationPtr m_annotation;
    
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateProperties(DgnElementCR) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LoadProperties(DgnElementCR) override;

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationDataAspect); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static TextAnnotationDataAspectCP GetCP(DgnElementCR el) { return UniqueAspect::Get<TextAnnotationDataAspect>(el, *QueryECClass(el.GetDgnDb())); }
    static TextAnnotationDataAspectP GetP(DgnElementR el) { return UniqueAspect::GetP<TextAnnotationDataAspect>(el, *QueryECClass(el.GetDgnDb())); }

    TextAnnotationCP GetAnnotation() const { return m_annotation.get(); }
    void SetAnnotation(TextAnnotationCP value) { m_annotation = value ? value->Clone() : nullptr; }
    DGNPLATFORM_EXPORT virtual void _AppendGeometry(ElementGeometryBuilderR, TextAnnotationElementR) const;
};

namespace dgn_AspectHandler
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     09/2015
    //=======================================================================================
    struct TextAnnotationData : Aspect
    {
        DOMAINHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationDataAspect, TextAnnotationData, Aspect, DGNPLATFORM_EXPORT);
    
    protected:
        virtual RefCountedPtr<DgnElement::Aspect> _CreateInstance() override { return new TextAnnotationDataAspect(); }
    };
}

//=======================================================================================
//! Implementation of TextAnnotationElement element.
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationElement : DrawingElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationElement, DrawingElement);

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR) override;
    virtual bool _DrawHit(HitDetailCR, ViewContextR) const override { return false; } // Don't flash text box...
    virtual SnapStatus _OnSnap(SnapContextR context) const override { return context.DoTextSnap(); } // Default snap using text box...
    DGNPLATFORM_EXPORT virtual void _UpdateGeomStream();
    TextAnnotationDataAspectCP GetDataAspectCP() const { return TextAnnotationDataAspect::GetCP(*this); }
    DGNPLATFORM_EXPORT TextAnnotationDataAspectR GetDataAspectR();

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationElement); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static TextAnnotationElementCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<TextAnnotationElement>(id); }
    static TextAnnotationElementPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<TextAnnotationElement>(id); }

    explicit TextAnnotationElement(CreateParams const& params) : T_Super(params) {}
    
    TextAnnotationCP GetAnnotation() const { TextAnnotationDataAspectCP dataAspect = GetDataAspectCP(); return dataAspect ? dataAspect->GetAnnotation() : nullptr; }
    void SetAnnotation(TextAnnotationCP value) { GetDataAspectR().SetAnnotation(value); _UpdateGeomStream(); }
    
    TextAnnotationElementCPtr Insert() { return GetDgnDb().Elements().Insert<TextAnnotationElement>(*this); }
    TextAnnotationElementCPtr Update() { return GetDgnDb().Elements().Update<TextAnnotationElement>(*this); }
    DgnDbStatus Delete() { return GetDgnDb().Elements().Delete(*this); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for TextAnnotationElement.
    // @bsiclass                                                    Jeff.Marker     09/2015
    //=======================================================================================
    struct TextAnnotation : Drawing
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationElement, TextAnnotationElement, TextAnnotation, Drawing, DGNPLATFORM_EXPORT);
    };
}

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
