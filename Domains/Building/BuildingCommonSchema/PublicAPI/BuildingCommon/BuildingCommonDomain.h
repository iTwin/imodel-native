/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "BuildingCommonDefinitions.h"

BEGIN_BENTLEY_NAMESPACE

namespace BuildingCommon
	{

	//=======================================================================================
	//! The DgnDomain for the Architectural Physical schema.
	//! @ingroup GROUP_ArchitecturalPhyscal
	//=======================================================================================
	struct BuildingCommonDomain : Dgn::DgnDomain
		{
		DOMAIN_DECLARE_MEMBERS(BuildingCommonDomain, BUILDING_COMMON_EXPORT)
		protected:
			WCharCP _GetSchemaRelativePath() const override { return BENTLEY_BUILDING_COMMON_SCHEMA_PATH; }
			virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
			virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

		public:
			BuildingCommonDomain();
			BUILDING_COMMON_EXPORT static Dgn::CodeSpecId QueryBuildingCommonCodeSpecId(Dgn::DgnDbCR dgndb);
			BUILDING_COMMON_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
			BUILDING_COMMON_EXPORT static ECN::IECInstancePtr BuildingCommonDomain::AddAspect(Dgn::PhysicalModelR model, Dgn::PhysicalElementPtr element, Utf8StringCR className);

		};

	} // End BuildingCommon namespace

END_BENTLEY_NAMESPACE

