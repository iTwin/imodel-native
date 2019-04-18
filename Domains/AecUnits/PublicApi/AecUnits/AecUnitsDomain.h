/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "AecUnitsDefinitions.h"


BEGIN_BENTLEY_NAMESPACE

namespace AecUnits
	{

	//=======================================================================================
	//! The DgnDomain for the AecUnits schema.
	//! @ingroup GROUP_AecUnits
	//=======================================================================================
	struct AecUnitsDomain : Dgn::DgnDomain
		{
		DOMAIN_DECLARE_MEMBERS(AecUnitsDomain, AEC_UNITS_EXPORT)
		protected:
			WCharCP _GetSchemaRelativePath() const override { return BENTLEY_AEC_UNITS_SCHEMA_PATH; }
			virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
			virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

		public:
			AecUnitsDomain ();
			AEC_UNITS_EXPORT static Dgn::CodeSpecId QueryAecUnitsCodeSpecId(Dgn::DgnDbCR dgndb);
		        AEC_UNITS_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);

		};

	} // End AecUnitsnamespace

END_BENTLEY_NAMESPACE

