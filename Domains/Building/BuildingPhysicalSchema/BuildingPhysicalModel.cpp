/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BuildingDomainInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace BuildingPhysical
	{

	HANDLER_DEFINE_MEMBERS(BuildingTypeDefinitionModelHandler)
	HANDLER_DEFINE_MEMBERS(BuildingPhysicalModelHandler)


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	BuildingPhysicalModelPtr BuildingPhysicalModel::Create(Dgn::PhysicalPartitionCR partition)
		{
		Dgn::DgnDbR db = partition.GetDgnDb();
		Dgn::DgnElementId modeledElementId = partition.GetElementId();
		Dgn::DgnClassId classId = db.Domains().GetClassId(BuildingPhysicalModelHandler::GetHandler());

		Dgn::DgnModelPtr model = BuildingPhysicalModelHandler::GetHandler().Create(Dgn::DgnModel::CreateParams(db, classId, modeledElementId));
		if (!model.IsValid())
			return nullptr;

		// Insert the new model into the DgnDb
		if (Dgn::DgnDbStatus::Success != model->Insert())
			return nullptr;


		return dynamic_cast<BuildingPhysicalModelP>(model.get());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnDbStatus BuildingPhysicalModel::_OnInsertElement(Dgn::DgnElementR element)
		{
	  //  if (nullptr == dynamic_cast<ArchitecturalBaseElementCP>(&element))
	  //      return DgnDbStatus::WrongElement;

		return T_Super::_OnInsertElement(element);
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	BuildingTypeDefinitionModelPtr  BuildingTypeDefinitionModel::Create(Dgn::DefinitionPartitionCR partition)
		{
		Dgn::DgnDbR db = partition.GetDgnDb();
		Dgn::DgnElementId modeledElementId = partition.GetElementId();
		Dgn::DgnClassId classId = db.Domains().GetClassId(BuildingTypeDefinitionModelHandler::GetHandler());

		Dgn::DgnModelPtr model = BuildingTypeDefinitionModelHandler::GetHandler().Create(Dgn::DgnModel::CreateParams(db, classId, modeledElementId));
		if (!model.IsValid())
			return nullptr;

		// Insert the new model into the DgnDb
		if (Dgn::DgnDbStatus::Success != model->Insert())
			return nullptr;

		return dynamic_cast<BuildingTypeDefinitionModelP>(model.get());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnDbStatus BuildingTypeDefinitionModel::_OnInsertElement(Dgn::DgnElementR element)
		{
	    //  if (nullptr == dynamic_cast<DoorTypeCP>(&element))
	    //      return DgnDbStatus::WrongElement;

		return T_Super::_OnInsertElement(element);
		}

	} // End BuildingPhysical namespace

END_BENTLEY_NAMESPACE


