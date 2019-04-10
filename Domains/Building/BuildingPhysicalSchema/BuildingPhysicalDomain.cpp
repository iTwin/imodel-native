/*--------------------------------------------------------------------------------------+
|
|     $Source: BuildingPhysicalSchema/BuildingPhysicalDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BuildingDomainInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace BuildingPhysical
	{

	DOMAIN_DEFINE_MEMBERS(BuildingPhysicalDomain)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	BuildingPhysicalDomain::BuildingPhysicalDomain() : DgnDomain(BENTLEY_BUILDING_PHYSICAL_SCHEMA_NAME, "Bentley Building Physical Domain", 1)
		{
		RegisterHandler(BuildingPhysicalModelHandler::GetHandler());
		RegisterHandler(BuildingTypeDefinitionModelHandler::GetHandler());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void BuildingPhysicalDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
		{
		Dgn::DgnSubCategory::Appearance defaultApperance;
		defaultApperance.SetInvisible(false);
	    }

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::CodeSpecId  BuildingPhysicalDomain::QueryBuildingPhysicalCodeSpecId(Dgn::DgnDbCR dgndb)
		{
		Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_BUILDING_PHYSICAL_AUTHORITY);
		BeAssert(codeSpecId.IsValid());
		return codeSpecId;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCode BuildingPhysicalDomain::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
		{
		return Dgn::CodeSpec::CreateCode(dgndb, BENTLEY_BUILDING_PHYSICAL_AUTHORITY, value);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void BuildingPhysicalDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
		{
		}

	} // End BuildingPhysical namespace

END_BENTLEY_NAMESPACE
