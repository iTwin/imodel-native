/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "BuildingPhysicalDefinitions.h"

BEGIN_BENTLEY_NAMESPACE

namespace BuildingPhysical
	{

	//=======================================================================================
	//! The DgnDomain for the Building Physical schema.
	//! @ingroup GROUP_BuildingPhyscal
	//=======================================================================================
	struct BuildingPhysicalDomain : Dgn::DgnDomain
		{
		DOMAIN_DECLARE_MEMBERS(BuildingPhysicalDomain, BUILDING_PHYSICAL_EXPORT)
		protected:
			WCharCP _GetSchemaRelativePath() const override { return BENTLEY_BUILDING_PHYSICAL_SCHEMA_PATH; }
			virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
			virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

		public:
			BuildingPhysicalDomain();
			BUILDING_PHYSICAL_EXPORT static Dgn::CodeSpecId QueryBuildingPhysicalCodeSpecId(Dgn::DgnDbCR dgndb);
			BUILDING_PHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
		};

	} // End BuildingPhysical namespace

END_BENTLEY_NAMESPACE

