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
// @bsiclass Abstract Base class for physical elements describing substructures of a bridge.    
// Nick.Purcell  07/2018
//---------------------------------------------------------------------------------------
/* IMPLEMENT SKEW METHODS*/
struct EXPORT_VTABLE_ATTRIBUTE SubstructureElement : GeometricElementWrapper<Dgn::PhysicalElement>, LinearReferencing::ILinearlyLocatedSingleAt
    {
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::PhysicalElement)

    protected:
        //! @private
        BRIDGESTRUCTURALPHYSICAL_EXPORT explicit SubstructureElement(Dgn::PhysicalElementCR element);
        BRIDGESTRUCTURALPHYSICAL_EXPORT explicit SubstructureElement(Dgn::PhysicalElementR element) : T_Super(element) {}
		BRIDGESTRUCTURALPHYSICAL_EXPORT explicit SubstructureElement(Dgn::PhysicalElementR element,
			ILinearlyLocatedSingleAt::CreateAtParams const& atParams);
		virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *get(); }
    public:
		DECLARE_BRIDGESTRUCTURALPHYSICAL_QUERYCLASS_METHODS(SubstructureElement)
        DECLARE_BRIDGESTRUCTURALPHYSICAL_ELEMENT_BASE_METHODS(SubstructureElement, Dgn::PhysicalElement)

        double GetSkew() const { return get()->GetPropertyValueDouble("Skew"); }
	    void SetSkew(double skew) { getP()->SetPropertyValue("Skew", skew); }
    }; // SubstructureElement

//---------------------------------------------------------------------------------------
// @bsiclass Generic Base class for physical elements describing substructure elements of a bridge.    
// Nick.Purcell  07/2018
//---------------------------------------------------------------------------------------
struct EXPORT_VTABLE_ATTRIBUTE GenericSubstructureElement : SubstructureElement
    {
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(SubstructureElement, Dgn::PhysicalElement)

    protected:
        //! @private
        BRIDGESTRUCTURALPHYSICAL_EXPORT explicit GenericSubstructureElement(Dgn::PhysicalElementCR element);
        BRIDGESTRUCTURALPHYSICAL_EXPORT explicit GenericSubstructureElement(Dgn::PhysicalElementR element) : T_Super(element) {}
        BRIDGESTRUCTURALPHYSICAL_EXPORT explicit GenericSubstructureElement(Dgn::PhysicalElementR element,
			ILinearlyLocatedSingleAt::CreateAtParams const& atParams);
           
    public:
        DECLARE_BRIDGESTRUCTURALPHYSICAL_QUERYCLASS_METHODS(GenericSubstructureElement)
        DECLARE_BRIDGESTRUCTURALPHYSICAL_ELEMENT_BASE_METHODS(GenericSubstructureElement, Dgn::PhysicalElement)
		BRIDGESTRUCTURALPHYSICAL_EXPORT static GenericSubstructureElementPtr Create(Dgn::PhysicalElement::CreateParams createParams, ILinearlyLocatedSingleAt::CreateAtParams const& atParams, double skew);
    }; //GenericSubstructureElement

END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE