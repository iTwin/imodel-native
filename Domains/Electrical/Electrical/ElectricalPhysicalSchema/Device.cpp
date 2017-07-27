/*--------------------------------------------------------------------------------------+
|
|     $Source: Electrical/ElectricalPhysicalSchema/Device.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ElectricalDomainInternal.h"

BEGIN_BENTLEY_NAMESPACE

namespace ElectricalPhysical
{

	    HANDLER_DEFINE_MEMBERS(DeviceHandler)
		HANDLER_DEFINE_MEMBERS(DeviceTypeHandler)
		HANDLER_DEFINE_MEMBERS(ElectricalBaseElementHandler)


		//---------------------------------------------------------------------------------------
		// @bsimethod                                   Bentley.Systems
		//---------------------------------------------------------------------------------------

		ElectricalBaseElementPtr ElectricalBaseElement::Create(Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelR model)
	{

		Dgn::DgnDbR db = model.GetDgnDb();
		Dgn::DgnModelId modelId = model.GetModelId();

		// Find the class

		ECN::ECClassCP ElectricalClass = model.GetDgnDb().GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

		if (nullptr == ElectricalClass)
			return nullptr;

		ECN::ECClassId classId = ElectricalClass->GetId();

		Dgn::ElementHandlerP elmHandler = Dgn::dgn_ElementHandler::Element::FindHandler(db, classId);
		if (NULL == elmHandler)
			return nullptr;


		Dgn::DgnCategoryId categoryId = ElectricalPhysicalCategory::QueryElectricalPhysicalCategoryId(db, className.c_str());

		Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

		//  ElectricalPhysical::ElectricalBaseElementHandler handler = ElectricalPhysical::ElectricalBaseElementHandler::GetHandler();

		Dgn::DgnElementPtr element = elmHandler->Create(params);

		ElectricalBaseElementPtr ElectricalElement = dynamic_pointer_cast<ElectricalBaseElement>(element);

		auto geomSource = ElectricalElement->ToGeometrySourceP();

		if (nullptr == geomSource)
			return nullptr;

		geomSource->SetCategoryId(categoryId);

		return ElectricalElement;

	}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	ECN::IECInstancePtr ElectricalBaseElement::AddAspect(Dgn::PhysicalModelR model, ElectricalBaseElementPtr element, Utf8StringCR className)
	{

		// Find the class

		ECN::ECClassCP aspectClassP = model.GetDgnDb().GetClassLocater().LocateClass(BENTLEY_ELECTRICAL_PHYSICAL_SCHEMA_NAME, className.c_str());

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
	DevicePtr Device::Create(Dgn::PhysicalModelR model)
	{
		Dgn::DgnDbR db = model.GetDgnDb();
		Dgn::DgnModelId modelId = model.GetModelId();
		Dgn::DgnCategoryId categoryId = ElectricalPhysicalCategory::QueryElectricalPhysicalDeviceCategoryId(db);
		Dgn::DgnClassId classId = db.Domains().GetClassId(DeviceHandler::GetHandler());

		DevicePtr device = new Device(CreateParams(db, modelId, classId, categoryId));

		return device;
	}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	DeviceTypePtr DeviceType::Create(Dgn::DefinitionModelR model)
	{
		Dgn::DgnDbR db = model.GetDgnDb();
		Dgn::DgnModelId modelId = model.GetModelId();
		Dgn::DgnClassId classId = db.Domains().GetClassId(DeviceTypeHandler::GetHandler());

		DeviceTypePtr element = new DeviceType(CreateParams(db, modelId, classId));
		if (!element.IsValid())
			return nullptr;

		return element;
	}
} // End ElectricalPhysical namespace


END_BENTLEY_NAMESPACE

