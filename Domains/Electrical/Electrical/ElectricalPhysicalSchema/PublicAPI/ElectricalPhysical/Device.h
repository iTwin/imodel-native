/*--------------------------------------------------------------------------------------+
|
|     $Source: Electrical/ElectricalPhysicalSchema/PublicAPI/ElectricalPhysical/Device.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ElectricalPhysicalDomain.h"

BEGIN_BENTLEY_NAMESPACE

namespace ElectricalPhysical {

	//=======================================================================================
	// @bsiclass                                    BentleySystems
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE ElectricalBaseElement : Dgn::PhysicalElement
	{
		//    DEFINE_T_SUPER(Dgn::PhysicalElement)
		DGNELEMENT_DECLARE_MEMBERS(EP_CLASS_ElectricalBaseElement, Dgn::PhysicalElement);
		friend struct ElectricalBaseElementHandler;

	protected:
		explicit ElectricalBaseElement(CreateParams const& params) : T_Super(params) {}

		static ECN::IECInstancePtr                   AddAspect(Dgn::PhysicalModelR model, ElectricalBaseElementPtr element, Utf8StringCR className);

		//BUILDING_PHYSICAL_DOMAIN_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Dgn::ElementECPropertyAccessor&, ECN::ECValueCR value, Dgn::PropertyArrayIndex const&) override;

		// static Dgn::Render::GeometryParams GetCasingMaterialParams(Dgn::DgnDbR, Dgn::DgnCategoryId, CasingMaterialType);
		// static Dgn::Render::GeometryParams GetMagnetMaterialParams(Dgn::DgnDbR, Dgn::DgnCategoryId);

	public:
		ELECTRICAL_PHYSICAL_EXPORT static ElectricalPhysical::ElectricalBaseElementPtr           Create(Utf8StringCR, Utf8StringCR, Dgn::PhysicalModelR);
		ELECTRICAL_PHYSICAL_EXPORT static ElectricalBaseElementPtr           Create(CreateParams const& params);
		// ELECTRICAL_PHYSICAL_EXPORT static ECN::IECInstancePtr                   AddClassificationAspect (Dgn::PhysicalModelR model, ElectricalBaseElementPtr element) {return AddAspect( model, element, BC_CLASS_Classification); }
		// ELECTRICAL_PHYSICAL_EXPORT static ECN::IECInstancePtr                   AddManufacturerAspect(Dgn::PhysicalModelR model, ElectricalBaseElementPtr element) { return AddAspect(model, element, BC_CLASS_Manufacturer); }

		template <class T> static RefCountedPtr<T>                                 QueryById(Dgn::PhysicalModelCR model, Dgn::DgnElementId id) { Dgn::DgnDbR    db = model.GetDgnDb(); return db.Elements().GetForEdit<T>(id); }
		template <class T> static RefCountedPtr<T>                                 QueryByCode(Dgn::PhysicalModelCR model, Dgn::DgnCodeCR code) { Dgn::DgnDbR  db = model.GetDgnDb(); return QueryById<T>(model, db.Elements().QueryElementIdByCode(code)); }
		template <class T> static RefCountedPtr<T>                                 QueryByCodeValue(Dgn::PhysicalModelCR model, Utf8StringCR codeValue) { Dgn::DgnCode code = CreateCode(model, codeValue); return QueryByCode<T>(model, code); }

		static  Dgn::DgnCode                                                       CreateCode(Dgn::PhysicalModelCR model, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(BENTLEY_ELECTRICAL_PHYSICAL_AUTHORITY, model, codeValue); }

		//BUILDING_PHYSICAL_DOMAIN_EXPORT static CasingMaterialType ParseCasingMaterial(Utf8CP);

		//! Move the placement for this element by adding the specified vector to its current placement
		//  BUILDING_PHYSICAL_DOMAIN_EXPORT void MoveRelative(DVec3dCR);
	};

	//=======================================================================================
	//! The ElementHandler for SmallSquareTile
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE ElectricalBaseElementHandler : Dgn::dgn_ElementHandler::Physical
	{
		ELEMENTHANDLER_DECLARE_MEMBERS(EP_CLASS_ElectricalBaseElement, ElectricalBaseElement, ElectricalBaseElementHandler, Dgn::dgn_ElementHandler::Physical, ELECTRICAL_PHYSICAL_EXPORT)
	};


	//=======================================================================================
	//! The SmallSquare tile
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE Device : ElectricalBaseElement
	{
		DGNELEMENT_DECLARE_MEMBERS(EP_CLASS_Device, ElectricalBaseElement);
		friend struct DeviceHandler;

	protected:
		explicit Device(CreateParams const& params) : T_Super(params) {}

	public:
		ELECTRICAL_PHYSICAL_EXPORT static DevicePtr Create(Dgn::PhysicalModelR);
		//     ELECTRICAL_PHYSICAL_EXPORT static DevicePtr QueryById(BuildingPhysical::BuildingPhysicalModelCR, Dgn::DgnElementId );
		//ELECTRICAL_PHYSICAL_EXPORT static DevicePtr QueryByCodeValue(BuildingPhysical::BuildingPhysicalModelCR, Dgn::DgnCodeCR code);
	};

	//__PUBLISH_EXTRACT_END__
	//__PUBLISH_EXTRACT_START__ ToyTilePhysicalElement_DeclareHandler.sampleCode
	//=======================================================================================
	//! The ElementHandler for SmallSquareTile
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE DeviceHandler : Dgn::dgn_ElementHandler::Physical
	{
		ELEMENTHANDLER_DECLARE_MEMBERS(EP_CLASS_Device, Device, DeviceHandler, ElectricalBaseElementHandler, ELECTRICAL_PHYSICAL_EXPORT)
	};


	//=======================================================================================
	//! The SmallSquare tile
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE DeviceType : Dgn::PhysicalType
	{
		DGNELEMENT_DECLARE_MEMBERS(EP_CLASS_DeviceType, Dgn::PhysicalType);
		friend struct DeviceTypeHandler;

	protected:
		explicit DeviceType(CreateParams const& params) : T_Super(params) {}

	public:
		ELECTRICAL_PHYSICAL_EXPORT static DeviceTypePtr Create(Dgn::DefinitionModelR);
	};

	//=======================================================================================
	//! The ElementHandler for SmallSquareTile
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE DeviceTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
	{
		ELEMENTHANDLER_DECLARE_MEMBERS(EP_CLASS_DeviceType, DeviceType, DeviceTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, ELECTRICAL_PHYSICAL_EXPORT)
	};

}
END_BENTLEY_NAMESPACE