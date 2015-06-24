/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/TextAnnotationElement.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include "TextAnnotation.h"
#include <DgnPlatform/DgnCore/DgnElement.h>
#include <DgnPlatform/DgnCore/ElementHandler.h>

DGNPLATFORM_TYPEDEFS(PhysicalTextAnnotationElement);
DGNPLATFORM_REF_COUNTED_PTR(PhysicalTextAnnotationElement);

#define DGN_CLASSNAME_PhysicalTextAnnotationElement "PhysicalTextAnnotationElement"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalTextAnnotationElement : PhysicalElement
{
private:
    DEFINE_T_SUPER(PhysicalElement);
    friend struct PhysicalTextAnnotationElementHandler;
    
    TextAnnotationPtr m_annotation;

    DgnDbStatus UpdatePropertiesInDb();
    void UpdateGeometryRepresentation();

protected:
    PhysicalTextAnnotationElement(CreateParams const& params, TextAnnotationCR seedAnnotation) : T_Super(params) { SetAnnotation(seedAnnotation); }
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR) override;

public:
    explicit PhysicalTextAnnotationElement(CreateParams const& params) : T_Super(params) {}
    static PhysicalTextAnnotationElementPtr Create(CreateParams const& params) { return new PhysicalTextAnnotationElement(params); }
    static PhysicalTextAnnotationElementPtr Create(CreateParams const& params, TextAnnotationCR seedAnnotation) { return new PhysicalTextAnnotationElement(params, seedAnnotation); }

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalTextAnnotationElement)); }
    TextAnnotationCP GetAnnotation() const { return m_annotation.get(); }
    DGNPLATFORM_EXPORT BentleyStatus SetAnnotation(TextAnnotationCR);

    static PhysicalTextAnnotationElementCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<PhysicalTextAnnotationElement>(id); }
    static PhysicalTextAnnotationElementPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<PhysicalTextAnnotationElement>(id); }
    PhysicalTextAnnotationElementCPtr Insert() { return GetDgnDb().Elements().Insert<PhysicalTextAnnotationElement>(*this); }
    PhysicalTextAnnotationElementCPtr Update() { return GetDgnDb().Elements().Update<PhysicalTextAnnotationElement>(*this); }
    DgnDbStatus Delete() { return GetDgnDb().Elements().Delete(*this); }
};

namespace dgn_ElementHandler
{
//=======================================================================================
//! The ElementHandler for PhysicalTextAnnotationElement
// @bsiclass                                                    Jeff.Marker     06/2015
//=======================================================================================
struct PhysicalText : Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_PhysicalTextAnnotationElement, PhysicalTextAnnotationElement, PhysicalText, Element, DGNPLATFORM_EXPORT);
};
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
