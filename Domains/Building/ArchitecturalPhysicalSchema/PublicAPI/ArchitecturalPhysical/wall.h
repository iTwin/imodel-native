/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/wall.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Door.h"

BEGIN_BENTLEY_NAMESPACE

namespace ArchitecturalPhysical 
	{

	//=======================================================================================
	//! The Wall
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE Wall : ArchitecturalBaseElement
		{
		DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_Wall, ArchitecturalBaseElement);
		friend struct WallHandler;

		protected:
			explicit Wall(CreateParams const& params) : T_Super(params) {}

		public:
			ARCHITECTURAL_PHYSICAL_EXPORT static WallPtr Create(Dgn::PhysicalModelR);
		};

	//=======================================================================================
	//! The ElementHandler for Wall
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE WallHandler : Dgn::dgn_ElementHandler::Physical
		{
		ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_Wall, Wall, WallHandler, ArchitecturalBaseElementHandler, ARCHITECTURAL_PHYSICAL_EXPORT)
		};


	//=======================================================================================
	//! The WallType 
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE WallType : Dgn::PhysicalType
		{
		DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_WallType, Dgn::PhysicalType);
		friend struct WallTypeHandler;

		protected:
			explicit WallType(CreateParams const& params) : T_Super(params) {}

		public:
			ARCHITECTURAL_PHYSICAL_EXPORT static WallTypePtr Create(Dgn::DefinitionModelR);
		};

	//=======================================================================================
	//! The ElementHandler for Wall
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE WallTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
		{
		ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_WallType, WallType, WallTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, ARCHITECTURAL_PHYSICAL_EXPORT)
		};

	} // End ArchitecturalPhysical namespace

END_BENTLEY_NAMESPACE






