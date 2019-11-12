/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ElectricalPhysicalDefinitions.h"

BEGIN_BENTLEY_NAMESPACE

namespace ElectricalPhysical
	{
	//=======================================================================================
	//! The PhysicalModel that only contains ElectricalPhysicalElements
	//=======================================================================================
	struct ElectricalPhysicalModel : Dgn::PhysicalModel
		{
		// Declare ElectricalPhysicalModelHandler as a friend so that it can access the protected ElectricalPhysicalModel constructor
		friend struct ElectricalPhysicalModelHandler;

		// The DGNMODEL_DECLARE_MEMBERS macro automates some required configuration:
		// Parameter 1) EP_CLASS_ElectricalPhysicalModel - the name of the ECEntityClass in the ecschema.xml file
		// Parameter 2) Dgn::PhysicalModel - the superclass of ElectricalPhysicalModel
		DGNMODEL_DECLARE_MEMBERS(EP_CLASS_ElectricalPhysicalModel, Dgn::PhysicalModel);

		protected:
			//! ElectricalPhysicalModel wants custom behavior when elements are inserted
			ELECTRICAL_PHYSICAL_EXPORT virtual Dgn::DgnDbStatus _OnInsertElement(Dgn::DgnElementR) override;

			//! ElectricalPhysicalModel constructor used by ElectricalPhysicalModelHandler
			explicit ElectricalPhysicalModel(CreateParams const& params) : T_Super(params) {}

		public:
			//! Uses to create (in memory) a new ElectricalPhysicalModel
			ELECTRICAL_PHYSICAL_EXPORT static ElectricalPhysicalModelPtr Create(Dgn::PhysicalPartitionCR);
		};


	//=======================================================================================
	//! The ModelHandler for ElectricalPhysicalModel
	//=======================================================================================
	struct ElectricalPhysicalModelHandler : Dgn::dgn_ModelHandler::Physical
		{
		// The MODELHANDLER_DECLARE_MEMBERS macro automates some required configuration:
		// Parameter 1) EP_CLASS_ElectricalPhysicalModel - the name of the ECEntityClass in the ecschema.xml file
		// Parameter 2) ElectricalPhysicalModel - the DgnModel subclass which provides the custom behavior for the ECEntityClass
		// Parameter 3) ElectricalPhysicalModelHandler - the factory class that will create instances of ElectricalPhysicalModel
		// Parameter 4) Dgn::dgn_ModelHandler::Physical - the superclass of ElectricalPhysicalModelHandler                                                    
		MODELHANDLER_DECLARE_MEMBERS(EP_CLASS_ElectricalPhysicalModel, ElectricalPhysicalModel, ElectricalPhysicalModelHandler, Dgn::dgn_ModelHandler::Physical, ELECTRICAL_PHYSICAL_EXPORT)
		};


	struct ElectricalTypeDefinitionModel : Dgn::DefinitionModel
		{
		// Declare ElectricalTypeDefinitionModelHandler as a friend so that it can access the protected ElectricalTypeDefinitionModel constructor
		friend struct ElectricalTypeDefinitionModelHandler;

		// The DGNMODEL_DECLARE_MEMBERS macro automates some required configuration:
		// Parameter 1) EP_CLASS_ElectricalTypeDefinitionModel - the name of the ECEntityClass in the ecschema.xml file
		// Parameter 2) Dgn::DefinitionModel - the superclass of ElectricalTypeDefinitionModel
		DGNMODEL_DECLARE_MEMBERS(EP_CLASS_ElectricalTypeDefinitionModel, Dgn::DefinitionModel);

		protected:
			//! ElectricalTypeDefinitionModel wants custom behavior when elements are inserted
			ELECTRICAL_PHYSICAL_EXPORT virtual Dgn::DgnDbStatus _OnInsertElement(Dgn::DgnElementR) override;

			//! ElectricalTypeDefinitionModel constructor used by ElectricalTypeDefinitionModelHandler
			explicit ElectricalTypeDefinitionModel(CreateParams const& params) : T_Super(params) {}

		public:
			//! Uses to create (in memory) a new ElectricalTypeDefinitionModel
			ELECTRICAL_PHYSICAL_EXPORT static ElectricalTypeDefinitionModelPtr Create(Dgn::DefinitionPartitionCR);
		};

	//=======================================================================================
	//! The ModelHandler for ElectricalTypeDefinitionModel
	//=======================================================================================
	struct ElectricalTypeDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
		{
		// The MODELHANDLER_DECLARE_MEMBERS macro automates some required configuration:
		// Parameter 1) EP_CLASS_ElectricalTypeDefinitionModel - the name of the ECEntityClass in the ecschema.xml file
		// Parameter 2) ElectricalTypeDefinitionModel - the DgnModel subclass which provides the custom behavior for the ECEntityClass
		// Parameter 3) TElectricalTypeDefinitionModelHandler - the factory class that will create instances of ElectricalTypeDefinitionModel
		// Parameter 4) Dgn::dgn_ModelHandler::Definition - the superclass of ElectricalTypeDefinitionModelHandler
		MODELHANDLER_DECLARE_MEMBERS(EP_CLASS_ElectricalTypeDefinitionModel, ElectricalTypeDefinitionModel, ElectricalTypeDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, ELECTRICAL_PHYSICAL_EXPORT)
		};

	} // End ElectricalPhysical namespace

END_BENTLEY_NAMESPACE

