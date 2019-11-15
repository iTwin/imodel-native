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
	_AddLinearlyReferencedLocation(*_GetUnpersistedAtLocation());
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  06/2018
//---------------------------------------------------------------------------------------
GenericSubstructureElementPtr GenericSubstructureElement::Create(Dgn::PhysicalElement::CreateParams createParams,
	ILinearlyLocatedSingleAt::CreateAtParams const& atParams, double skew)
    {
	auto retVal = new GenericSubstructureElement(*Create(createParams.m_dgndb, createParams), atParams);
    retVal->SetSkew(skew);
	retVal->_SetLinearElement(atParams.m_linearElementCPtr->GetElementId());
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
	
	}