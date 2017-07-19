/*--------------------------------------------------------------------------------------+
|
|     $Source: ConcreteSchema/PublicAPI/Concrete/ConcreteDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "ConcreteDefinitions.h"

BEGIN_BENTLEY_CONCRETE_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Concrete schema.
//! @ingroup GROUP_Concrete
//=======================================================================================
struct ConcreteDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(ConcreteDomain, CONCRETE_EXPORT)

    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_CONCRETE_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;
        static void InsertDomainCodeSpecs(Dgn::DgnDbR db);

    public:
        ConcreteDomain();
        CONCRETE_EXPORT static Dgn::CodeSpecId QueryConcreteCodeSpecId(Dgn::DgnDbCR dgndb);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct ConcreteCategory : NonCopyableClass
    {
    friend struct ConcreteDomain;

    private:
        static void InsertDomainCategories(Dgn::DgnDbR);
        static Dgn::DgnCategoryId InsertCategory(Dgn::DgnDbR, Utf8CP, Dgn::ColorDef const&);
        static Dgn::DgnSubCategoryId InsertSubCategory(Dgn::DgnDbR, Dgn::DgnCategoryId, Utf8CP, Dgn::ColorDef const&);

    public:
        //! Get the DgnSubCategoryId
        CONCRETE_EXPORT static Dgn::DgnCategoryId QueryStructuralPhysicalCategoryId(Dgn::DgnDbR db, Utf8CP categoryName);
    };


END_BENTLEY_CONCRETE_NAMESPACE