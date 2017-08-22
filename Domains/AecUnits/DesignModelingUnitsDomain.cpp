/*--------------------------------------------------------------------------------------+
|
|     $Source: DesignModelingUnitsDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DesignModelingUnitsInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace DesignModelingUnits
	{

	DOMAIN_DEFINE_MEMBERS(DesignModelingUnitsDomain)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	DesignModelingUnitsDomain::DesignModelingUnitsDomain() : DgnDomain(BENTLEY_DESIGN_MODELING_UNITS_SCHEMA_NAME, "Design Modeling Units Schema", 1)
		{
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void DesignModelingUnitsDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
		{
		Dgn::DgnSubCategory::Appearance defaultApperance;
		defaultApperance.SetInvisible(false);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::CodeSpecId  DesignModelingUnitsDomain::QueryDesignModelingUnitsCodeSpecId(Dgn::DgnDbCR dgndb)
		{
		Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_DESIGN_MODELING_UNITS_AUTHORITY);
		BeAssert(codeSpecId.IsValid());
		return codeSpecId;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCode DesignModelingUnitsDomain::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
		{
		return Dgn::CodeSpec::CreateCode(dgndb, BENTLEY_DESIGN_MODELING_UNITS_AUTHORITY, value);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void DesignModelingUnitsDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
		{
		}

	} // End DesignModelingUnits namespace

END_BENTLEY_NAMESPACE
