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

DGNPLATFORM_TYPEDEFS(TextAnnotationElement);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationElement);

#define DGN_CLASSNAME_TextAnnotationElement "TextAnnotationElement"
#define DGN_CLASSNAME_TextAnnotationDataAspect "TextAnnotationDataAspect"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! Implementation of TextAnnotationElement element.
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationElement : DrawingElement
{
private:
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationElement, DrawingElement);

protected:
    TextAnnotationPtr m_annotation;
    
    DGNPLATFORM_EXPORT virtual void _UpdateGeomStream();
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR) override;
    virtual bool _DrawHit(HitDetailCR, ViewContextR) const override { return false; } // Don't flash text box...
    virtual SnapStatus _OnSnap(SnapContextR context) const override { return context.DoTextSnap(); } // Default snap using text box...

public:
    explicit TextAnnotationElement(CreateParams const& params) : T_Super(params) {}
    static TextAnnotationElementPtr Create(CreateParams const& params) { return new TextAnnotationElement(params); }
    
    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationElement)); }
    TextAnnotationCP GetAnnotation() const { return m_annotation.get(); }
    void SetAnnotation(TextAnnotationCP value) { m_annotation = value ? value->Clone() : nullptr; _UpdateGeomStream(); }
    
    static TextAnnotationElementCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<TextAnnotationElement>(id); }
    static TextAnnotationElementPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<TextAnnotationElement>(id); }
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
