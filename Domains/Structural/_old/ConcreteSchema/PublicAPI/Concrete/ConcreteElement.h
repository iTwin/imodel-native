/*--------------------------------------------------------------------------------------+
|
|     $Source: _old/ConcreteSchema/PublicAPI/Concrete/ConcreteElement.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Concrete\ConcreteApi.h>

BEGIN_BENTLEY_CONCRETE_NAMESPACE


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ConcreteElement : Dgn::PhysicalElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CONCRETE_CLASS_ConcreteElement, Dgn::PhysicalElement);
    //friend struct ConcreteElementHandler;

    protected:
        explicit ConcreteElement(CreateParams const& params) : T_Super(params) {}

        //static ECN::IECInstancePtr                   AddAspect(BuildingPhysical::BuildingPhysicalModelR model, ArchitecturalBaseElementPtr element, Utf8StringCR className);

    //public:
        //CONCRETE_EXPORT static Concrete::ConcreteElementPtr                        Create(Utf8StringCR, BuildingPhysical::BuildingPhysicalModelR);
        //CONCRETE_EXPORT static ConcreteElementPtr                                  Create(CreateParams const& params);
        //ARCHITECTURAL_PHYSICAL_EXPORT static ECN::IECInstancePtr                   AddClassificationAspect (BuildingPhysical::BuildingPhysicalModelR model, ArchitecturalBaseElementPtr element) {return AddAspect( model, element, BC_CLASS_Classification); }
        //ARCHITECTURAL_PHYSICAL_EXPORT static ECN::IECInstancePtr                   AddManufacturerAspect(BuildingPhysical::BuildingPhysicalModelR model, ArchitecturalBaseElementPtr element) { return AddAspect(model, element, BC_CLASS_Manufacturer); }

        //template <class T> static RefCountedPtr<T>                                 QueryById  (BuildingPhysical::BuildingPhysicalModelCR model, Dgn::DgnElementId id) { Dgn::DgnDbR    db = model.GetDgnDb(); return db.Elements().GetForEdit<T>(id);}
        //template <class T> static RefCountedPtr<T>                                 QueryByCode(BuildingPhysical::BuildingPhysicalModelCR model, Dgn::DgnCodeCR code) { Dgn::DgnDbR  db = model.GetDgnDb(); return QueryById<T>(model, db.Elements().QueryElementIdByCode(code));}
        //template <class T> static RefCountedPtr<T>                                 QueryByCodeValue(BuildingPhysical::BuildingPhysicalModelCR model, Utf8StringCR codeValue) { Dgn::DgnCode code = CreateCode(model, codeValue); return QueryByCode<T>(model, code); }

        //static  Dgn::DgnCode                                                       CreateCode (BuildingPhysical::BuildingPhysicalModelCR model, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode (BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, model, codeValue); }
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
//struct EXPORT_VTABLE_ATTRIBUTE ConcreteElementHandler : Dgn::dgn_ElementHandler::Physical
//    {
//    ELEMENTHANDLER_DECLARE_MEMBERS(CONCRETE_CLASS_ConcreteElement, ConcreteElement, ConcreteElementHandler, Dgn::dgn_ElementHandler::Physical, CONCRETE_EXPORT)
//    };


END_BENTLEY_CONCRETE_NAMESPACE
