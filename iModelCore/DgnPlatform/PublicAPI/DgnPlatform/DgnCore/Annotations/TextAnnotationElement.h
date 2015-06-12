//-------------------------------------------------------------------------------------- 
//     $Source:  
//  $Copyright:  
//-------------------------------------------------------------------------------------- 
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

    DgnModelStatus UpdatePropertiesInDb();
    void UpdateGeometryRepresentation();

protected:
    explicit PhysicalTextAnnotationElement(CreateParams const& params) : T_Super(params) {}
    PhysicalTextAnnotationElement(CreateParams const& params, TextAnnotationCR seedAnnotation) : T_Super(params) { SetAnnotation(seedAnnotation); }
    DGNPLATFORM_EXPORT virtual DgnModelStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT virtual DgnModelStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT virtual DgnModelStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT virtual DgnModelStatus _CopyFrom(DgnElementCR) override;

public:
    static PhysicalTextAnnotationElementPtr Create(CreateParams const& params) { return new PhysicalTextAnnotationElement(params); }
    static PhysicalTextAnnotationElementPtr Create(CreateParams const& params, TextAnnotationCR seedAnnotation) { return new PhysicalTextAnnotationElement(params, seedAnnotation); }

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalTextAnnotationElement)); }
    TextAnnotationCP GetAnnotation() const { return m_annotation.get(); }
    DGNPLATFORM_EXPORT BentleyStatus SetAnnotation(TextAnnotationCR);

    static PhysicalTextAnnotationElementCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<PhysicalTextAnnotationElement>(id); }
    static PhysicalTextAnnotationElementPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<PhysicalTextAnnotationElement>(id); }
    PhysicalTextAnnotationElementCPtr Insert() { return GetDgnDb().Elements().Insert<PhysicalTextAnnotationElement>(*this); }
    PhysicalTextAnnotationElementCPtr Update() { return GetDgnDb().Elements().Update<PhysicalTextAnnotationElement>(*this); }
    DgnModelStatus Delete() { return GetDgnDb().Elements().Delete(*this); }
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2015
//=======================================================================================
struct PhysicalTextAnnotationElementHandler : ElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_PhysicalTextAnnotationElement, PhysicalTextAnnotationElement, PhysicalTextAnnotationElementHandler, ElementHandler, DGNPLATFORM_EXPORT);
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
