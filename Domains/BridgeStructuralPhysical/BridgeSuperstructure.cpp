/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BridgeStructuralPhysicalInternal.h"
#include <BridgeStructuralPhysical/BridgeSuperstructure.h>
#include <BridgeStructuralPhysical/Bridge.h>
#include <BridgeStructuralPhysical/BridgeSubstructure.h>
#include <BridgeStructuralPhysical/BridgeCategory.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  06/2018
//---------------------------------------------------------------------------------------
SuperstructureElement::SuperstructureElement(PhysicalElementCR element):
    T_Super(element)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  06/2018
//---------------------------------------------------------------------------------------
SuperstructureElement::SuperstructureElement(PhysicalElementR element, ILinearlyLocatedSingleAt::CreateAtParams const& atParams)
	: T_Super(element), ILinearlyLocatedSingleAt(atParams)
	{
	}
//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  07/2018
//---------------------------------------------------------------------------------------
GenericSuperstructureElement::GenericSuperstructureElement(PhysicalElementCR element) :
	SuperstructureElement(element)
	{
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  07/2018
//---------------------------------------------------------------------------------------
GenericSuperstructureElement::GenericSuperstructureElement(PhysicalElementR element, ILinearlyLocatedSingleAt::CreateAtParams const& atParams)
	: SuperstructureElement(element, atParams)
	{
    _SetLinearElement(atParams.m_linearElementCPtr->GetElementId());
    _AddLinearlyReferencedLocation(*_GetUnpersistedAtLocation());
	}
//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  07/2018
//---------------------------------------------------------------------------------------
GenericSuperstructureElementPtr GenericSuperstructureElement::Create(PhysicalElement::CreateParams createParams,
	ILinearlyLocatedSingleAt::CreateAtParams const& atParams)
	{
	return new GenericSuperstructureElement(*Create(createParams.m_dgndb, createParams), atParams);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
GenericSuperstructureElementCPtr GenericSuperstructureElement::Insert(DgnDbStatus* stat)
    {
    GenericSuperstructureElementCPtr retCPtr = new GenericSuperstructureElement(*getP()->GetDgnDb().Elements().Insert<PhysicalElement>(*getP(), stat));

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
GenericSuperstructureElementCPtr GenericSuperstructureElement::Update(DgnDbStatus* stat)
    {
    GenericSuperstructureElementCPtr retCPtr = new GenericSuperstructureElement(*getP()->GetDgnDb().Elements().Update<PhysicalElement>(*getP(), stat));

    DgnDbStatus status = Dgn::DgnDbStatus::Success;
    if (retCPtr.IsNull() ||
        DgnDbStatus::Success != (status = _UpdateLinearElementRelationship()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retCPtr;
    }