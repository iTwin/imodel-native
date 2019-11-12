/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "Door.h"

BEGIN_BENTLEY_NAMESPACE

namespace ArchitecturalPhysical
	{


	//=======================================================================================
	//! The Window
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE Window : ArchitecturalBaseElement
		{
		DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_Window, ArchitecturalBaseElement);
		friend struct WindowHandler;

		protected:
			explicit Window(CreateParams const& params) : T_Super(params) {}

		public:
			ARCHITECTURAL_PHYSICAL_EXPORT static WindowPtr Create(Dgn::PhysicalModelR);
		};

	//=======================================================================================
	//! The ElementHandler for Window
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE WindowHandler : Dgn::dgn_ElementHandler::Physical
		{
		ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_Window, Window, WindowHandler, ArchitecturalBaseElementHandler, ARCHITECTURAL_PHYSICAL_EXPORT)
		};


	//=======================================================================================
	//! The WindowType tile
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE WindowType : Dgn::PhysicalType
		{
		DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_WindowType, Dgn::PhysicalType);
		friend struct WindowTypeHandler;

		protected:
			explicit WindowType(CreateParams const& params) : T_Super(params) {}

		public:
			ARCHITECTURAL_PHYSICAL_EXPORT static WindowTypePtr Create(Dgn::DefinitionModelR);
		};

	//=======================================================================================
	//! The ElementHandler for WindowType
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE WindowTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
		{
		ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_WindowType, WindowType, WindowTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, ARCHITECTURAL_PHYSICAL_EXPORT)
		};

	} // End ArchitecturalPhysical namespace

END_BENTLEY_NAMESPACE





