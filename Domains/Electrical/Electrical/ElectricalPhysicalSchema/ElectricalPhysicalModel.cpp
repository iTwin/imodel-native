/*--------------------------------------------------------------------------------------+
|
|     $Source: Electrical/ElectricalPhysicalSchema/ElectricalPhysicalModel.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ElectricalDomainInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace ElectricalPhysical
	{

	HANDLER_DEFINE_MEMBERS(ElectricalTypeDefinitionModelHandler)
	HANDLER_DEFINE_MEMBERS(ElectricalPhysicalModelHandler)


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
		ElectricalPhysicalModelPtr ElectricalPhysicalModel::Create(Dgn::PhysicalPartitionCR partition)
		{
		Dgn::DgnDbR db = partition.GetDgnDb();
		Dgn::DgnElementId modeledElementId = partition.GetElementId();
		Dgn::DgnClassId classId = db.Domains().GetClassId(ElectricalPhysicalModelHandler::GetHandler());

		Dgn::DgnModelPtr model = ElectricalPhysicalModelHandler::GetHandler().Create(Dgn::DgnModel::CreateParams(db, classId, modeledElementId));
		if (!model.IsValid())
			return nullptr;

		// Insert the new model into the DgnDb
		if (Dgn::DgnDbStatus::Success != model->Insert())
			return nullptr;


		return dynamic_cast<ElectricalPhysicalModelP>(model.get());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnDbStatus ElectricalPhysicalModel::_OnInsertElement(Dgn::DgnElementR element)
		{
	  //  if (nullptr == dynamic_cast<ArchitecturalBaseElementCP>(&element))
	  //      return DgnDbStatus::WrongElement;

		return T_Super::_OnInsertElement(element);
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	ElectricalTypeDefinitionModelPtr  ElectricalTypeDefinitionModel::Create(Dgn::DefinitionPartitionCR partition)
		{
		Dgn::DgnDbR db = partition.GetDgnDb();
		Dgn::DgnElementId modeledElementId = partition.GetElementId();
		Dgn::DgnClassId classId = db.Domains().GetClassId(ElectricalTypeDefinitionModelHandler::GetHandler());

		Dgn::DgnModelPtr model = ElectricalTypeDefinitionModelHandler::GetHandler().Create(Dgn::DgnModel::CreateParams(db, classId, modeledElementId));
		if (!model.IsValid())
			return nullptr;

		// Insert the new model into the DgnDb
		if (Dgn::DgnDbStatus::Success != model->Insert())
			return nullptr;

		return dynamic_cast<ElectricalTypeDefinitionModelP>(model.get());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnDbStatus ElectricalTypeDefinitionModel::_OnInsertElement(Dgn::DgnElementR element)
		{
	    //  if (nullptr == dynamic_cast<DoorTypeCP>(&element))
	    //      return DgnDbStatus::WrongElement;

		return T_Super::_OnInsertElement(element);
		}

	} // End ElectricalPhysical namespace

END_BENTLEY_NAMESPACE


