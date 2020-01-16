/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BridgeStructuralPhysicalInternal.h"
#include <BridgeStructuralPhysical/BridgeSubstructure.h>
#include <BridgeStructuralPhysical/Bridge.h>
#include <BridgeStructuralPhysical/BridgeCategory.h>
//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  06/2018
//---------------------------------------------------------------------------------------

SubstructureElement::SubstructureElement(
    Dgn::PhysicalElementR element,
    ILinearlyLocatedSingleAt::CreateAtParams const& atParams):
	T_Super(element), ILinearlyLocatedSingleAt(atParams)
	{
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  06/2018
//---------------------------------------------------------------------------------------
GenericSubstructureElementPtr GenericSubstructureElement::Create(Dgn::PhysicalElement::CreateParams createParams,
	ILinearlyLocatedSingleAt::CreateAtParams const& atParams, double skew)
    {
	auto retVal = new GenericSubstructureElement(*Create(createParams.m_dgndb, createParams), atParams);
    retVal->SetSkew(skew);
    return retVal;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  06/2018
//---------------------------------------------------------------------------------------
GenericSubstructureElement::GenericSubstructureElement(Dgn::PhysicalElementCR element):
	T_Super(element)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  06/2018
//---------------------------------------------------------------------------------------

SubstructureElement::SubstructureElement(Dgn::PhysicalElementCR element):
    T_Super(element)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  06/2018
//---------------------------------------------------------------------------------------
GenericSubstructureElement::GenericSubstructureElement(
    Dgn::PhysicalElementR element,
	ILinearlyLocatedSingleAt::CreateAtParams const& atParams) :
	T_Super(element, atParams)
	{	
    _SetLinearElement(atParams.m_linearElementCPtr->GetElementId());
    _AddLinearlyReferencedLocation(*_GetUnpersistedAtLocation());
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
GenericSubstructureElementCPtr GenericSubstructureElement::Insert(DgnDbStatus* stat)
    {
    GenericSubstructureElementCPtr retCPtr = new GenericSubstructureElement(*getP()->GetDgnDb().Elements().Insert<PhysicalElement>(*getP(), stat));

    DgnDbStatus status = Dgn::DgnDbStatus::Success;
    if (retCPtr.IsNull() ||
        DgnDbStatus::Success != (status = _InsertLinearElementRelationship()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
GenericSubstructureElementCPtr GenericSubstructureElement::Update(DgnDbStatus* stat)
    {
    GenericSubstructureElementCPtr retCPtr = new GenericSubstructureElement(*getP()->GetDgnDb().Elements().Update<PhysicalElement>(*getP(), stat));

    DgnDbStatus status = Dgn::DgnDbStatus::Success;
    if (retCPtr.IsNull() ||
        DgnDbStatus::Success != (status = _UpdateLinearElementRelationship()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retCPtr;
    }