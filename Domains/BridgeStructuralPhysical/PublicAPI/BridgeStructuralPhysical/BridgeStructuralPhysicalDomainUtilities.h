/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "BridgeStructuralPhysical.h"
#include <Json/Json.h>

BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass                                  Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
struct BridgeStructuralPhysicalDomainUtilities
    {
    BRIDGESTRUCTURALPHYSICAL_EXPORT static BentleyStatus                    RegisterDomainHandlers();
    BRIDGESTRUCTURALPHYSICAL_EXPORT static Utf8String                       BuildPhysicalModelCode(Utf8StringCR modelCodeName);

    BRIDGESTRUCTURALPHYSICAL_EXPORT static PhysicalModelPtr					GetPhysicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
    BRIDGESTRUCTURALPHYSICAL_EXPORT static PhysicalModelPtr					CreatePhysicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
	BRIDGESTRUCTURALPHYSICAL_EXPORT static Dgn::PhysicalElementPtr          CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, PhysicalModelCPtr phsyModel, Utf8CP categoryName = nullptr);
	BRIDGESTRUCTURALPHYSICAL_EXPORT static StructuralMemberPtr              CreateStructuralMember(Utf8StringCR schemaName, Utf8StringCR className, PhysicalModelCPtr phsyModel, Utf8CP categoryName = nullptr,
		DgnElementId parentId = DgnElementId(), DgnClassId parentRelClassId = DgnClassId());
    
	BRIDGESTRUCTURALPHYSICAL_EXPORT static PhysicalModelPtr                 QueryPhysicalModel(SubjectCPtr parentSubject, Utf8StringCR name);
};
END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE 
