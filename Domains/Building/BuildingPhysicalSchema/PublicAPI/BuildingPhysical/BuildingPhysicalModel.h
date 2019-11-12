/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "BuildingPhysicalDefinitions.h"

BEGIN_BENTLEY_NAMESPACE

namespace BuildingPhysical
	{
	//=======================================================================================
	//! The PhysicalModel that only contains BuildingPhysicalElements
	//=======================================================================================
	struct BuildingPhysicalModel : Dgn::PhysicalModel
		{
		// Declare BuildingPhysicalModelHandler as a friend so that it can access the protected BuildingPhysicalModel constructor
		friend struct BuildingPhysicalModelHandler;

		// The DGNMODEL_DECLARE_MEMBERS macro automates some required configuration:
		// Parameter 1) BP_CLASS_BuildingPhysicalModel - the name of the ECEntityClass in the ecschema.xml file
		// Parameter 2) Dgn::PhysicalModel - the superclass of BuildingPhysicalModel
		DGNMODEL_DECLARE_MEMBERS(BP_CLASS_BuildingPhysicalModel, Dgn::PhysicalModel);

		protected:
			//! BuildingPhysicalModel wants custom behavior when elements are inserted
			BUILDING_PHYSICAL_EXPORT virtual Dgn::DgnDbStatus _OnInsertElement(Dgn::DgnElementR) override;

			//! BuildingPhysicalModel constructor used by BuildingPhysicalModelHandler
			explicit BuildingPhysicalModel(CreateParams const& params) : T_Super(params) {}

		public:
			//! Uses to create (in memory) a new BuildingPhysicalModel
			BUILDING_PHYSICAL_EXPORT static BuildingPhysicalModelPtr Create(Dgn::PhysicalPartitionCR);
		};


	//=======================================================================================
	//! The ModelHandler for BuildingPhysicalModel
	//=======================================================================================
	struct BuildingPhysicalModelHandler : Dgn::dgn_ModelHandler::Physical
		{
		// The MODELHANDLER_DECLARE_MEMBERS macro automates some required configuration:
		// Parameter 1) BP_CLASS_BuildingPhysicalModel - the name of the ECEntityClass in the ecschema.xml file
		// Parameter 2) BuildingPhysicalModel - the DgnModel subclass which provides the custom behavior for the ECEntityClass
		// Parameter 3) BuildingPhysicalModelHandler - the factory class that will create instances of BuildingPhysicalModel
		// Parameter 4) Dgn::dgn_ModelHandler::Physical - the superclass of BuildingPhysicalModelHandler                                                    
		MODELHANDLER_DECLARE_MEMBERS(BP_CLASS_BuildingPhysicalModel, BuildingPhysicalModel, BuildingPhysicalModelHandler, Dgn::dgn_ModelHandler::Physical, BUILDING_PHYSICAL_EXPORT)
		};


	struct BuildingTypeDefinitionModel : Dgn::DefinitionModel
		{
		// Declare BuildingTypeDefinitionModelHandler as a friend so that it can access the protected BuildingTypeDefinitionModel constructor
		friend struct BuildingTypeDefinitionModelHandler;

		// The DGNMODEL_DECLARE_MEMBERS macro automates some required configuration:
		// Parameter 1) BP_CLASS_BuildingTypeDefinitionModel - the name of the ECEntityClass in the ecschema.xml file
		// Parameter 2) Dgn::DefinitionModel - the superclass of BuildingTypeDefinitionModel
		DGNMODEL_DECLARE_MEMBERS(BP_CLASS_BuildingTypeDefinitionModel, Dgn::DefinitionModel);

		protected:
			//! BuildingTypeDefinitionModel wants custom behavior when elements are inserted
			BUILDING_PHYSICAL_EXPORT virtual Dgn::DgnDbStatus _OnInsertElement(Dgn::DgnElementR) override;

			//! BuildingTypeDefinitionModel constructor used by BuildingTypeDefinitionModelHandler
			explicit BuildingTypeDefinitionModel(CreateParams const& params) : T_Super(params) {}

		public:
			//! Uses to create (in memory) a new BuildingTypeDefinitionModel
			BUILDING_PHYSICAL_EXPORT static BuildingTypeDefinitionModelPtr Create(Dgn::DefinitionPartitionCR);
		};

	//=======================================================================================
	//! The ModelHandler for BuildingTypeDefinitionModel
	//=======================================================================================
	struct BuildingTypeDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
		{
		// The MODELHANDLER_DECLARE_MEMBERS macro automates some required configuration:
		// Parameter 1) BP_CLASS_BuildingTypeDefinitionModel - the name of the ECEntityClass in the ecschema.xml file
		// Parameter 2) BuildingTypeDefinitionModel - the DgnModel subclass which provides the custom behavior for the ECEntityClass
		// Parameter 3) TBuildingTypeDefinitionModelHandler - the factory class that will create instances of BuildingTypeDefinitionModel
		// Parameter 4) Dgn::dgn_ModelHandler::Definition - the superclass of BuildingTypeDefinitionModelHandler
		MODELHANDLER_DECLARE_MEMBERS(BP_CLASS_BuildingTypeDefinitionModel, BuildingTypeDefinitionModel, BuildingTypeDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, BUILDING_PHYSICAL_EXPORT)
		};

	} // End BuildingPhysical namespace

END_BENTLEY_NAMESPACE

