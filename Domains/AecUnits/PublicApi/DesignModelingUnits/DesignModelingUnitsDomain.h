/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DesignModelingUnits/DesignModelingUnitsDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "DesignModelingUnitsDefinitions.h"


BEGIN_BENTLEY_NAMESPACE

namespace DesignModelingUnits
	{

	//=======================================================================================
	//! The DgnDomain for the DesignModelingUnits schema.
	//! @ingroup GROUP_DesignModelingUnits
	//=======================================================================================
	struct DesignModelingUnitsDomain : Dgn::DgnDomain
		{
		DOMAIN_DECLARE_MEMBERS(DesignModelingUnitsDomain, DESIGN_MODELING_UNITS_EXPORT)
		protected:
			WCharCP _GetSchemaRelativePath() const override { return BENTLEY_DESIGN_MODELING_UNITS_SCHEMA_PATH; }
			virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
			virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

		public:
			DesignModelingUnitsDomain ();
			DESIGN_MODELING_UNITS_EXPORT static Dgn::CodeSpecId QueryDesignModelingUnitsCodeSpecId(Dgn::DgnDbCR dgndb);
		        DESIGN_MODELING_UNITS_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);

		};

	} // End DesignModelingUnitsnamespace

END_BENTLEY_NAMESPACE

