/*--------------------------------------------------------------------------------------+
|
|     $Source$
|
|  $Copyright$
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "BridgeStructuralPhysical.h"

BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass Abstract Base class for physical elements describing superstructure elements of a bridge.    
// Nick.Purcell  07/2018
//---------------------------------------------------------------------------------------
struct EXPORT_VTABLE_ATTRIBUTE SuperstructureElement : LinearReferencing::LinearPhysicalElement, LinearReferencing::ILinearlyLocatedSingleAt
    {
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(LinearReferencing::LinearPhysicalElement, Dgn::PhysicalElement)

    protected:
        //! @private
        BRIDGESTRUCTURALPHYSICAL_EXPORT explicit SuperstructureElement(Dgn::PhysicalElementCR element);
        BRIDGESTRUCTURALPHYSICAL_EXPORT explicit SuperstructureElement(Dgn::PhysicalElementR element) : T_Super(element) {}
		BRIDGESTRUCTURALPHYSICAL_EXPORT explicit SuperstructureElement(Dgn::PhysicalElementR element, LinearReferencing::DistanceExpression distanceExpr);
		virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *get(); }
        Dgn::DgnElementId GetFromBridgeSupportId() const { return get()->GetPropertyValueId<Dgn::DgnElementId>(BBP_PROP_BridgeSuperstructure_FromSupport); }
        Dgn::DgnElementId GetToBridgeSupportId() const { return get()->GetPropertyValueId<Dgn::DgnElementId>(BBP_PROP_BridgeSuperstructure_ToSupport); }
    public:
        DECLARE_BRIDGESTRUCTURALPHYSICAL_QUERYCLASS_METHODS(SuperstructureElement)
        DECLARE_BRIDGESTRUCTURALPHYSICAL_ELEMENT_BASE_METHODS(SuperstructureElement, Dgn::PhysicalElement)

    }; // SuperstructureElement

//---------------------------------------------------------------------------------------
// @bsiclass Generic Base class for physical elements describing substructure elements of a bridge.    
// Nick.Purcell  07/2018
//---------------------------------------------------------------------------------------
struct EXPORT_VTABLE_ATTRIBUTE GenericSuperstructureElement : SuperstructureElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(SuperstructureElement, Dgn::PhysicalElement)

protected:
	//! @private
	BRIDGESTRUCTURALPHYSICAL_EXPORT explicit GenericSuperstructureElement(Dgn::PhysicalElementCR element);
    BRIDGESTRUCTURALPHYSICAL_EXPORT explicit GenericSuperstructureElement(Dgn::PhysicalElementR element) : T_Super(element) {}
	BRIDGESTRUCTURALPHYSICAL_EXPORT explicit GenericSuperstructureElement(Dgn::PhysicalElementR element, LinearReferencing::DistanceExpression distanceExpr);
public:
	DECLARE_BRIDGESTRUCTURALPHYSICAL_QUERYCLASS_METHODS(GenericSuperstructureElement)
	DECLARE_BRIDGESTRUCTURALPHYSICAL_ELEMENT_BASE_METHODS(GenericSuperstructureElement, Dgn::PhysicalElement)
	BRIDGESTRUCTURALPHYSICAL_EXPORT static GenericSuperstructureElementPtr Create(Dgn::PhysicalElement::CreateParams createParams, LinearReferencing::ILinearElementCR linearElement, LinearReferencing::DistanceExpression distanceExpr);
}; //GenericSuperstructureElement

END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE