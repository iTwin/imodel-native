/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "BridgeStructuralPhysical.h"

BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass Helper class to query/manage Bridge-related categories.       Diego.Diaz     05/2017
//---------------------------------------------------------------------------------------
struct BridgeCategory : NonCopyableClass
    {
    //__PUBLISH_SECTION_END__
    private:
        static void InsertCategory(Dgn::DefinitionModelR, Utf8CP, Dgn::ColorDef const&);
        static Dgn::DgnCategoryId QueryCategoryId(Dgn::DgnDbR, Utf8CP);
        static Dgn::DgnModelId GetModelId(Dgn::DgnDbR db);
        static Dgn::DefinitionModelPtr GetModel(Dgn::DgnDbR db);

    public:
        static void InsertDomainCategories(Dgn::DgnDbR);

        static Utf8CP GetPartitionName() { return "BridgeCategories"; }

    //__PUBLISH_SECTION_START__
    public:
        //! Return the DgnCategoryId for bridge elements
        BRIDGESTRUCTURALPHYSICAL_EXPORT static Dgn::DgnCategoryId Get(Dgn::DgnDbR);
    }; // BridgeStructuralPhysicalDomain

END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE
