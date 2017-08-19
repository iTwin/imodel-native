/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralMaterial/PublicAPI/StructuralMaterial/StructuralMaterialDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralMaterialDefinitions.h"
#include "StructuralCommon/PublicAPI/StructuralCommon/StructuralCommonDefinitions.h"

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Structural Physical schema.
//! @ingroup GROUP_StructuralPhyscal
//=======================================================================================
struct StructuralMaterialDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(StructuralMaterialDomain , STRUCTURAL_DOMAIN_EXPORT)

    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
        static void InsertDomainCodeSpecs(Dgn::DgnDbR db);

    public:
        StructuralMaterialDomain();
        STRUCTURAL_DOMAIN_EXPORT static Dgn::CodeSpecId QueryStructuralPhysicalCodeSpecId(Dgn::DgnDbCR dgndb);
        STRUCTURAL_DOMAIN_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    };

struct StructuralMaterialCategory : NonCopyableClass
    {
    friend struct StructuralMaterialDomain;

    private:
        static void InsertDomainCategories(Dgn::DgnDbR);
        static Dgn::DgnCategoryId InsertCategory(Dgn::DgnDbR, Utf8CP, Dgn::ColorDef const&);
        
    public:
        //! Get the DgnSubCategoryId
        STRUCTURAL_DOMAIN_EXPORT static Dgn::DgnCategoryId QueryStructuralMaterialCategoryId(Dgn::DgnDbR db, Utf8CP categoryName);
    };


END_BENTLEY_STRUCTURAL_NAMESPACE

