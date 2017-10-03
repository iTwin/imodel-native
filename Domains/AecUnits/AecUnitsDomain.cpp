/*--------------------------------------------------------------------------------------+
|
|     $Source: AecUnitsDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "AecUnitsInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace AecUnits
	{

	DOMAIN_DEFINE_MEMBERS(AecUnitsDomain)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	AecUnitsDomain::AecUnitsDomain() : DgnDomain(BENTLEY_AEC_UNITS_SCHEMA_NAME, "Aec Units Schema", 1)
		{
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void AecUnitsDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
		{
		Dgn::DgnSubCategory::Appearance defaultApperance;
		defaultApperance.SetInvisible(false);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::CodeSpecId  AecUnitsDomain::QueryAecUnitsCodeSpecId(Dgn::DgnDbCR dgndb)
		{
		Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_AEC_UNITS_AUTHORITY);
		BeAssert(codeSpecId.IsValid());
		return codeSpecId;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCode AecUnitsDomain::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
		{
		return Dgn::CodeSpec::CreateCode(dgndb, BENTLEY_AEC_UNITS_AUTHORITY, value);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void AecUnitsDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
		{
		}

	} // End AecUnitsDomain namespace

END_BENTLEY_NAMESPACE
