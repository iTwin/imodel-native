/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include "BridgeStructuralPhysical.h"

BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                  Nick.Purcell                  06/2018
//---------------------------------------------------------------------------------------
struct EXPORT_VTABLE_ATTRIBUTE Bridge : LinearReferencing::LinearPhysicalElement, LinearReferencing::ISegmentableLinearElement, LinearReferencing::ILinearlyLocatedSingleFromTo
    {
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(LinearReferencing::LinearPhysicalElement, Dgn::PhysicalElement)

    protected:
        //! @private
        explicit Bridge(Dgn::PhysicalElementCR element) : T_Super(element) {}
        explicit Bridge(Dgn::PhysicalElementR element) : T_Super(element) {}

        //! @private
        explicit Bridge(Dgn::PhysicalElementR element, CreateFromToParams const& fromToParams);

        virtual Dgn::DgnElementCR _ILinearElementToDgnElement() const override { return *get(); }
        virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *get(); }

    public:
        DECLARE_BRIDGESTRUCTURALPHYSICAL_QUERYCLASS_METHODS(Bridge)
        DECLARE_BRIDGESTRUCTURALPHYSICAL_ELEMENT_GET_METHODS(Bridge, Dgn::PhysicalElement)

        BRIDGESTRUCTURALPHYSICAL_EXPORT bvector<LinearReferencing::LinearLocationReference> QueryOrderedSupports() const;

        BRIDGESTRUCTURALPHYSICAL_EXPORT bvector<Dgn::DgnElementId> QueryOrderedSuperstructures() const;

        BRIDGESTRUCTURALPHYSICAL_EXPORT static BridgePtr Create(Dgn::PhysicalModelCR pysModel, Dgn::DgnCodeCR code,
            CreateFromToParams const& fromToParams);

        BRIDGESTRUCTURALPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scopeModel, Utf8StringCR codeValue);

        BRIDGESTRUCTURALPHYSICAL_EXPORT Dgn::PhysicalModelCP QueryStructuralSystemModel() const;

        BRIDGESTRUCTURALPHYSICAL_EXPORT Dgn::PhysicalModelCP QueryMultidisciplinaryModel() const;

        BRIDGESTRUCTURALPHYSICAL_EXPORT BridgeCPtr Insert(Dgn::DgnDbStatus* stat = nullptr);
        BRIDGESTRUCTURALPHYSICAL_EXPORT BridgeCPtr Update(Dgn::DgnDbStatus* stat = nullptr);
    }; // Bridge
END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE