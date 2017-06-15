/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/Door.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArchitecturalPhysicalSchemaInternal.h"
#include <BuildingPhysical/BuildingPhysicalApi.h>


BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(DoorHandler)
HANDLER_DEFINE_MEMBERS(DoorTypeHandler)
HANDLER_DEFINE_MEMBERS(ArchitecturalBaseElementHandler)


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

ArchitecturalPhysical::ArchitecturalBaseElementPtr ArchitecturalBaseElement::Create(Utf8StringCR className, BuildingPhysical::BuildingPhysicalModelR model)
    {

    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();

    // Find the class

    ECN::ECClassCP buildingClass = model.GetDgnDb().GetClassLocater().LocateClass(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, className.c_str());

    if (nullptr == buildingClass)
        return nullptr;

    ECN::ECClassId classId = buildingClass->GetId();

    Dgn::ElementHandlerP elmHandler = dgn_ElementHandler::Element::FindHandler(db, classId);
    if (NULL == elmHandler)
        return nullptr;


    DgnCategoryId categoryId = ArchitecturalPhysicalCategory::QueryBuildingPhysicalCategoryId(db, className.c_str());

    Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

  //  ArchitecturalPhysical::ArchitecturalBaseElementHandler handler = ArchitecturalPhysical::ArchitecturalBaseElementHandler::GetHandler();

    Dgn::DgnElementPtr element = elmHandler->Create(params);

    ArchitecturalPhysical::ArchitecturalBaseElementPtr buildingElement = dynamic_pointer_cast<ArchitecturalPhysical::ArchitecturalBaseElement>(element);

    auto geomSource = buildingElement->ToGeometrySourceP();

    if (nullptr == geomSource)
        return nullptr;

    geomSource->SetCategoryId(categoryId);

    return buildingElement;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
ECN::IECInstancePtr ArchitecturalBaseElement::AddAspect(BuildingPhysical::BuildingPhysicalModelR model, ArchitecturalBaseElementPtr element, Utf8StringCR className)
    {

    // Find the class

    ECN::ECClassCP aspectClassP = model.GetDgnDb().GetClassLocater().LocateClass(BENTLEY_BUILDING_COMMON_SCHEMA_NAME, className.c_str());

    if (nullptr == aspectClassP)
        return nullptr;

    // If the element is already persisted and has the Aspect class, you can't add another

    if (element->GetElementId().IsValid())
        {
        ECN::IECInstanceCP instance = DgnElement::GenericUniqueAspect::GetAspect(*element, *aspectClassP);

        if (nullptr != instance)
            return nullptr;
        }

    ECN::StandaloneECEnablerPtr enabler = aspectClassP->GetDefaultStandaloneEnabler();

    if (!enabler.IsValid())
        return nullptr;

    ECN::IECInstancePtr instance = enabler->CreateInstance().get();
    if (!instance.IsValid())
        return nullptr;

    Dgn::DgnDbStatus status = DgnElement::GenericUniqueAspect::SetAspect(*element, *instance);

    if (Dgn::DgnDbStatus::Success != status)
        return nullptr;

    return instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DoorPtr Door::Create(BuildingPhysical::BuildingPhysicalModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    DgnCategoryId categoryId = ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorCategoryId(db);
    DgnClassId classId = db.Domains().GetClassId(DoorHandler::GetHandler());

    DoorPtr door = new Door(CreateParams(db, modelId, classId, categoryId));

    return door;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DoorTypePtr DoorType::Create(BuildingPhysical::BuildingTypeDefinitionModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    DgnClassId classId = db.Domains().GetClassId(DoorTypeHandler::GetHandler());

    DoorTypePtr element = new DoorType(CreateParams(db, modelId, classId));
    if (!element.IsValid())
        return nullptr;

    return element;
    }



END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

