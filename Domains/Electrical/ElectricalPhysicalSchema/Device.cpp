/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ElectricalDomainInternal.h"

BEGIN_BENTLEY_NAMESPACE

namespace ElectricalPhysical
	{

	HANDLER_DEFINE_MEMBERS(DeviceHandler)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	ECN::IECInstancePtr Device::AddAspect(Dgn::PhysicalModelR model, DevicePtr element, Utf8StringCR className)
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
		if (!device.IsValid())
			return nullptr;

		return device;
		} 

	} // End ElectricalPhysical namespace


END_BENTLEY_NAMESPACE

