/*--------------------------------------------------------------------------------------+
|
|     $Source: ElectricalPhysicalSchema/PublicAPI/ElectricalPhysical/Device.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ElectricalPhysicalDomain.h"

BEGIN_BENTLEY_NAMESPACE

namespace ElectricalPhysical 
	{

	//=======================================================================================
	//@bsimethod                                   Bentley.Systems
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE Device : Dgn::PhysicalElement
		{
		DGNELEMENT_DECLARE_MEMBERS(EP_CLASS_Device, Dgn::PhysicalElement);
		friend struct DeviceHandler;

		protected:
			explicit Device(CreateParams const& params) : T_Super(params) {}
		

		public:
			ELECTRICAL_PHYSICAL_EXPORT static DevicePtr                             Create(Dgn::PhysicalModelR);
			static ECN::IECInstancePtr                                              AddAspect(Dgn::PhysicalModelR model, DevicePtr element, Utf8StringCR className);
			ELECTRICAL_PHYSICAL_EXPORT static ECN::IECInstancePtr                   AddClassificationAspect (Dgn::PhysicalModelR model, DevicePtr element) {return AddAspect( model, element, EC_CLASS_Classification); }
			ELECTRICAL_PHYSICAL_EXPORT static ECN::IECInstancePtr                   AddManufacturerAspect(Dgn::PhysicalModelR model, DevicePtr element) { return AddAspect(model, element, EC_CLASS_Manufacturer); }
		};


	//=======================================================================================
	//@bsimethod                                   Bentley.Systems
	//=======================================================================================
	struct EXPORT_VTABLE_ATTRIBUTE DeviceHandler : Dgn::dgn_ElementHandler::Physical
		{
		ELEMENTHANDLER_DECLARE_MEMBERS(EP_CLASS_Device, Device, DeviceHandler, Dgn::dgn_ElementHandler::Physical, ELECTRICAL_PHYSICAL_EXPORT)
		};

}
END_BENTLEY_NAMESPACE