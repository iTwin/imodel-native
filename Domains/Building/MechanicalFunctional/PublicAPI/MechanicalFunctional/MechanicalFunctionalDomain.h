/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "MechanicalFunctionalDefinitions.h"

BEGIN_BENTLEY_NAMESPACE

namespace MechanicalFunctional
	{

	//=======================================================================================
	//! The DgnDomain for the Architectural Physical schema.
	//! @ingroup GROUP_ArchitecturalPhyscal
	//=======================================================================================
	struct MechanicalFunctionalDomain : Dgn::DgnDomain
		{
		DOMAIN_DECLARE_MEMBERS(MechanicalFunctionalDomain, MECHANICAL_FUNCTIONAL_EXPORT)
		protected:
			WCharCP _GetSchemaRelativePath() const override { return BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA_PATH; }
			virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
			virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
			static void InsertDomainCodeSpecs(Dgn::DgnDbR db);


		public:
			MechanicalFunctionalDomain();
			BUILDING_COMMON_EXPORT static Dgn::CodeSpecId QueryMechanicalFunctionalCodeSpecId(Dgn::DgnDbCR dgndb);
			BUILDING_COMMON_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Dgn::DgnModelCR scopeModel, Utf8StringCR value);
			BUILDING_COMMON_EXPORT static ECN::IECInstancePtr MechanicalFunctionalDomain::AddAspect(Dgn::PhysicalModelR model, Dgn::PhysicalElementPtr element, Utf8StringCR className);

		};

	} // End MechanicalFunctional namespace

END_BENTLEY_NAMESPACE

