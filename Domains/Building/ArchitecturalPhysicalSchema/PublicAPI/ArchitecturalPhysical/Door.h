/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/Door.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ArchitecturalPhysical\ArchitecturalPhysicalApi.h>
#include <BuildingPhysical\BuildingPhysicalApi.h>
#include <BuildingCommon\BuildingCommonApi.h>

BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArchitecturalBaseElement : Dgn::PhysicalElement
    {
    //    DEFINE_T_SUPER(Dgn::PhysicalElement)
    DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_ArchitecturalBaseElement, Dgn::PhysicalElement);
    friend struct ArchitecturalBaseElementHandler;

    protected:
        explicit ArchitecturalBaseElement(CreateParams const& params) : T_Super(params) {}

        static ECN::IECInstancePtr                   AddAspect(BuildingPhysical::BuildingPhysicalModelR model, ArchitecturalBaseElementPtr element, Utf8StringCR className);

        //BUILDING_PHYSICAL_DOMAIN_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Dgn::ElementECPropertyAccessor&, ECN::ECValueCR value, Dgn::PropertyArrayIndex const&) override;

        // static Dgn::Render::GeometryParams GetCasingMaterialParams(Dgn::DgnDbR, Dgn::DgnCategoryId, CasingMaterialType);
        // static Dgn::Render::GeometryParams GetMagnetMaterialParams(Dgn::DgnDbR, Dgn::DgnCategoryId);

    public:
        ARCHITECTURAL_PHYSICAL_EXPORT static ArchitecturalPhysical::ArchitecturalBaseElementPtr           Create(Utf8StringCR,  BuildingPhysical::BuildingPhysicalModelR);
        ARCHITECTURAL_PHYSICAL_EXPORT static ArchitecturalBaseElementPtr           Create(CreateParams const& params);
        ARCHITECTURAL_PHYSICAL_EXPORT static ECN::IECInstancePtr                   AddClassificationAspect (BuildingPhysical::BuildingPhysicalModelR model, ArchitecturalBaseElementPtr element) {return AddAspect( model, element, BC_CLASS_Classification); }
        ARCHITECTURAL_PHYSICAL_EXPORT static ECN::IECInstancePtr                   AddManufacturerAspect(BuildingPhysical::BuildingPhysicalModelR model, ArchitecturalBaseElementPtr element) { return AddAspect(model, element, BC_CLASS_Manufacturer); }

        template <class T> static RefCountedPtr<T>                                 QueryById  (BuildingPhysical::BuildingPhysicalModelCR model, Dgn::DgnElementId id) { Dgn::DgnDbR    db = model.GetDgnDb(); return db.Elements().GetForEdit<T>(id);}
        template <class T> static RefCountedPtr<T>                                 QueryByCode(BuildingPhysical::BuildingPhysicalModelCR model, Dgn::DgnCodeCR code) { Dgn::DgnDbR  db = model.GetDgnDb(); return QueryById<T>(model, db.Elements().QueryElementIdByCode(code));}
        template <class T> static RefCountedPtr<T>                                 QueryByCodeValue(BuildingPhysical::BuildingPhysicalModelCR model, Utf8StringCR codeValue) { Dgn::DgnCode code = CreateCode(model, codeValue); return QueryByCode<T>(model, code); }
        
        static  Dgn::DgnCode                                                       CreateCode (BuildingPhysical::BuildingPhysicalModelCR model, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode (BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, model, codeValue); }
        
        //BUILDING_PHYSICAL_DOMAIN_EXPORT static CasingMaterialType ParseCasingMaterial(Utf8CP);

        //! Move the placement for this element by adding the specified vector to its current placement
        //  BUILDING_PHYSICAL_DOMAIN_EXPORT void MoveRelative(DVec3dCR);
    };

//=======================================================================================
//! The ElementHandler for SmallSquareTile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArchitecturalBaseElementHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_ArchitecturalBaseElement, ArchitecturalBaseElement, ArchitecturalBaseElementHandler, Dgn::dgn_ElementHandler::Physical, ARCHITECTURAL_PHYSICAL_EXPORT)
    };


//=======================================================================================
//! The SmallSquare tile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Door : ArchitecturalBaseElement
    {
    DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_Door, ArchitecturalBaseElement);
    friend struct DoorHandler;

    protected:
        explicit Door(CreateParams const& params) : T_Super(params) {}

    public:
        ARCHITECTURAL_PHYSICAL_EXPORT static DoorPtr Create(BuildingPhysical::BuildingPhysicalModelR);
   //     ARCHITECTURAL_PHYSICAL_EXPORT static DoorPtr QueryById(BuildingPhysical::BuildingPhysicalModelCR, Dgn::DgnElementId );
        //ARCHITECTURAL_PHYSICAL_EXPORT static DoorPtr QueryByCodeValue(BuildingPhysical::BuildingPhysicalModelCR, Dgn::DgnCodeCR code);
    };

//__PUBLISH_EXTRACT_END__
//__PUBLISH_EXTRACT_START__ ToyTilePhysicalElement_DeclareHandler.sampleCode
//=======================================================================================
//! The ElementHandler for SmallSquareTile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoorHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_Door, Door, DoorHandler, ArchitecturalBaseElementHandler, ARCHITECTURAL_PHYSICAL_EXPORT)
    };


//=======================================================================================
//! The SmallSquare tile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoorType : Dgn::PhysicalType
    {
    DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_DoorType, Dgn::PhysicalType);
    friend struct DoorTypeHandler;

    protected:
        explicit DoorType(CreateParams const& params) : T_Super(params) {}

    public:
        ARCHITECTURAL_PHYSICAL_EXPORT static DoorTypePtr Create(BuildingPhysical::BuildingTypeDefinitionModelR);
    };

//=======================================================================================
//! The ElementHandler for SmallSquareTile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoorTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_DoorType, DoorType, DoorTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, ARCHITECTURAL_PHYSICAL_EXPORT)
    };

END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE





