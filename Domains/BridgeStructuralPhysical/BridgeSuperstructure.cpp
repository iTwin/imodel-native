/*--------------------------------------------------------------------------------------+
|
|     $Source$
|
|  $Copyright$
|
+--------------------------------------------------------------------------------------*/
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
SuperstructureElement::SuperstructureElement(PhysicalElementR element, DistanceExpression distanceExpr)
	: T_Super(element)
	{
	auto atLocationPtr = LinearlyReferencedAtLocation::Create(distanceExpr);
	_AddLinearlyReferencedLocation(*atLocationPtr);
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
GenericSuperstructureElement::GenericSuperstructureElement(PhysicalElementR element, DistanceExpression distanceExpr)
	: SuperstructureElement(element, distanceExpr)
	{
	}
//---------------------------------------------------------------------------------------
// @bsimethod                                                       Nick.Purcell  07/2018
//---------------------------------------------------------------------------------------
GenericSuperstructureElementPtr GenericSuperstructureElement::Create(PhysicalElement::CreateParams createParams,
	ILinearElementCR linearElement, DistanceExpression distanceExpr)
	{
	GenericSuperstructureElementPtr retVal = 
		new GenericSuperstructureElement(*Create(createParams.m_dgndb, createParams), distanceExpr);
	retVal->_SetLinearElement(linearElement.ToElement().GetElementId());
	return retVal;
	}

