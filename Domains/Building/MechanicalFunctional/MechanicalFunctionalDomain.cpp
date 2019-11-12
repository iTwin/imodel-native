/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BuildingDomainInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace MechanicalFunctional
	{

	DOMAIN_DEFINE_MEMBERS(MechanicalFunctionalDomain)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	MechanicalFunctionalDomain::MechanicalFunctionalDomain() : DgnDomain(BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA_NAME, "Bentley Mechanical Functional Domain", 1)
		{
		//RegisterHandler(RadialDistortionHandler::GetHandler());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::CodeSpecId  MechanicalFunctionalDomain::QueryMechanicalFunctionalCodeSpecId(Dgn::DgnDbCR dgndb)
		{
		Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_MECHANICAL_FUNCTIONAL_AUTHORITY);
		BeAssert(codeSpecId.IsValid());
		return codeSpecId;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCode MechanicalFunctionalDomain::CreateCode(Dgn::DgnDbR dgndb, Dgn::DgnModelCR scopeModel, Utf8StringCR value)
		{
		return Dgn::CodeSpec::CreateCode(BENTLEY_MECHANICAL_FUNCTIONAL_AUTHORITY, scopeModel, value);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	void MechanicalFunctionalDomain::InsertDomainCodeSpecs(Dgn::DgnDbR db)
		{
		Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, BENTLEY_MECHANICAL_FUNCTIONAL_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
		if (codeSpec.IsValid())
			codeSpec->Insert();
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	void MechanicalFunctionalDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
		{
		InsertDomainCodeSpecs(dgndb);
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void MechanicalFunctionalDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
		{
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	ECN::IECInstancePtr MechanicalFunctionalDomain::AddAspect(Dgn::PhysicalModelR model, Dgn::PhysicalElementPtr element, Utf8StringCR className)
		{

		// Find the class

		ECN::ECClassCP aspectClassP = model.GetDgnDb().GetClassLocater().LocateClass(BENTLEY_MECHANICAL_FUNCTIONAL_SCHEMA_NAME, className.c_str());

		if (nullptr == aspectClassP)
			return nullptr;

		// If the element is already persisted and has the Aspect class, you can't add another

		if (element->GetElementId().IsValid())
			{
			ECN::IECInstanceCP instance = Dgn::DgnElement::GenericUniqueAspect::GetAspect(*element, *aspectClassP);

			if (nullptr != instance)
				return nullptr;
			}

		ECN::StandaloneECEnablerPtr enabler = aspectClassP->GetDefaultStandaloneEnabler();

		if (!enabler.IsValid())
			return nullptr;

		ECN::IECInstancePtr instance = enabler->CreateInstance().get();
		if (!instance.IsValid())
			return nullptr;

		Dgn::DgnDbStatus status = Dgn::DgnElement::GenericUniqueAspect::SetAspect(*element, *instance);

		if (Dgn::DgnDbStatus::Success != status)
			return nullptr;

		return instance;
		}

	} // End MechanicalFunctional namespace

END_BENTLEY_NAMESPACE
