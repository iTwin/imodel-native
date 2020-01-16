/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "BridgeStructuralPhysical.h"

BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass Abstract Base class for physical elements describing substructures of a bridge.    
// Nick.Purcell  07/2018
//---------------------------------------------------------------------------------------
/* IMPLEMENT SKEW METHODS*/
struct EXPORT_VTABLE_ATTRIBUTE SubstructureElement : LinearReferencing::LinearPhysicalElement, LinearReferencing::ILinearlyLocatedSingleAt
    {
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(LinearReferencing::LinearPhysicalElement, Dgn::PhysicalElement)

    protected:
        //! @private
        BRIDGESTRUCTURALPHYSICAL_EXPORT explicit SubstructureElement(Dgn::PhysicalElementCR element);
        BRIDGESTRUCTURALPHYSICAL_EXPORT explicit SubstructureElement(Dgn::PhysicalElementR element) : T_Super(element) {}
		BRIDGESTRUCTURALPHYSICAL_EXPORT explicit SubstructureElement(Dgn::PhysicalElementR element,
            ILinearlyLocatedSingleAt::CreateAtParams const& atParams);
		virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *get(); }
    public:
		DECLARE_BRIDGESTRUCTURALPHYSICAL_QUERYCLASS_METHODS(SubstructureElement)
        DECLARE_BRIDGESTRUCTURALPHYSICAL_ELEMENT_GET_METHODS(SubstructureElement, Dgn::PhysicalElement)

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
        DECLARE_BRIDGESTRUCTURALPHYSICAL_ELEMENT_GET_METHODS(GenericSubstructureElement, Dgn::PhysicalElement)
		BRIDGESTRUCTURALPHYSICAL_EXPORT static GenericSubstructureElementPtr Create(Dgn::PhysicalElement::CreateParams createParams, ILinearlyLocatedSingleAt::CreateAtParams const& atParams, double skew);

        BRIDGESTRUCTURALPHYSICAL_EXPORT GenericSubstructureElementCPtr Insert(Dgn::DgnDbStatus* status = nullptr);
        BRIDGESTRUCTURALPHYSICAL_EXPORT GenericSubstructureElementCPtr Update(Dgn::DgnDbStatus* status = nullptr);
    }; //GenericSubstructureElement

END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE