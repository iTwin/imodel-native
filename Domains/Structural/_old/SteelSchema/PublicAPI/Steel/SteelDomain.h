/*--------------------------------------------------------------------------------------+
|
|     $Source: _old/SteelSchema/PublicAPI/Steel/SteelDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "SteelDefinitions.h"

BEGIN_BENTLEY_STEEL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Steel schema.
//! @ingroup GROUP_Steel
//=======================================================================================
struct SteelDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(SteelDomain, STEEL_EXPORT)

    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_STEEL_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
        static void InsertDomainCodeSpecs(Dgn::DgnDbR db);

    public:
        SteelDomain();
        STEEL_EXPORT static Dgn::CodeSpecId QuerySteelCodeSpecId(Dgn::DgnDbCR dgndb);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct SteelCategory : NonCopyableClass
    {
    friend struct SteelDomain;

    private:
        static void InsertDomainCategories(Dgn::DgnDbR);
        static Dgn::DgnCategoryId InsertCategory(Dgn::DgnDbR, Utf8CP, Dgn::ColorDef const&);
        static Dgn::DgnSubCategoryId InsertSubCategory(Dgn::DgnDbR, Dgn::DgnCategoryId, Utf8CP, Dgn::ColorDef const&);

    public:
        //! Get the DgnSubCategoryId
        STEEL_EXPORT static Dgn::DgnCategoryId QueryStructuralPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName);
    };


END_BENTLEY_STEEL_NAMESPACE

