/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralPhysicalDefinitions.h"

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Structural Physical schema.
//! @ingroup GROUP_StructuralPhyscal
//=======================================================================================
struct StructuralPhysicalDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(StructuralPhysicalDomain, STRUCTURAL_DOMAIN_EXPORT)

    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
        static void InsertDomainCodeSpecs(Dgn::DgnDbR db);

    public:
        StructuralPhysicalDomain();
        STRUCTURAL_DOMAIN_EXPORT static Dgn::CodeSpecId QueryStructuralPhysicalCodeSpecId(Dgn::DgnDbCR dgndb);
        STRUCTURAL_DOMAIN_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    };

struct StructuralPhysicalCategory : NonCopyableClass
    {
    friend struct StructuralPhysicalDomain;

    private:
        static void InsertDomainCategories(Dgn::DgnDbR);
        static Dgn::DgnCategoryId InsertCategory(Dgn::DgnDbR, Utf8CP, Dgn::ColorDef const&, Dgn::DgnCategory::Rank rank = Dgn::DgnCategory::Rank::Domain);
        
    public:
        //! Get the DgnSubCategoryId
        STRUCTURAL_DOMAIN_EXPORT static Dgn::DgnCategoryId QueryStructuralPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName);
    };


END_BENTLEY_STRUCTURAL_NAMESPACE

