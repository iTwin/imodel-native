/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "BridgeStructuralPhysical.h"

BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass The DgnDomain for the BridgeStructuralPhysical schema.  Nick.Purcell 05/2018
//---------------------------------------------------------------------------------------
struct BridgeStructuralPhysicalDomain
    {
    private:
        BridgeStructuralPhysicalDomain() {}

    public:
		BRIDGESTRUCTURALPHYSICAL_EXPORT static Dgn::DgnDbStatus SetUpModelHierarchy(BridgeCR bridgeCR);

        static WCharCP GetSchemaRelativePath() { return BBP_SCHEMA_PATH; }
        BRIDGESTRUCTURALPHYSICAL_EXPORT static void OnSchemaImported(Dgn::SubjectCR subject);
    }; // BridgeStructuralPhysicalDomain

/*=================================================================================
! Breaks down a bridge element into its structural components.  Matt.Balnis 09/2018
=================================================================================*/
struct StructuralSystem : GeometricElementWrapper<Dgn::PhysicalElement>
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::PhysicalElement)

protected:
	explicit StructuralSystem(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit StructuralSystem(Dgn::PhysicalElementR element) : T_Super(element) {}
public:
	DECLARE_BRIDGESTRUCTURALPHYSICAL_QUERYCLASS_METHODS(StructuralSystem);
    DECLARE_BRIDGESTRUCTURALPHYSICAL_ELEMENT_BASE_METHODS(StructuralSystem, Dgn::PhysicalElement);
	BRIDGESTRUCTURALPHYSICAL_EXPORT static StructuralSystemCPtr Insert(Dgn::PhysicalModelCR parentModel, BridgeCR bridgeCR);
    BRIDGESTRUCTURALPHYSICAL_EXPORT static Dgn::DgnElementId Query(Dgn::PhysicalModelCR);
    BRIDGESTRUCTURALPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scopeModel, Utf8StringCR codeValue);
}; //StructuralSystem


END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE
