/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralMaterials/PublicAPI/StructuralMaterials/StructuralMaterialsDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralMaterialsDefinitions.h"

BEGIN_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Architectural Physical schema.
//! @ingroup GROUP_ArchitecturalPhyscal
//=======================================================================================
struct StructuralMaterialsDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(StructuralMaterialsDomain, STRUCTURAL_MATERIALS_EXPORT)

    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
        static void InsertDomainCodeSpecs(Dgn::DgnDbR db);

    public:
        StructuralMaterialsDomain();
        STRUCTURAL_MATERIALS_EXPORT static Dgn::CodeSpecId QueryStructuralMaterialsCodeSpecId(Dgn::DgnDbCR dgndb);
    };


END_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE


