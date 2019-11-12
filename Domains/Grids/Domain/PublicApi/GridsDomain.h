/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDomain.h>
#include <Grids/Domain/GridsMacros.h>

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! The DgnDomain for the building schema.
//! @private
//=======================================================================================
struct GridsDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(GridsDomain, GRIDSDOMAIN_EXPORT)

private:
    WCharCP _GetSchemaRelativePath () const override { return GRIDS_SCHEMA_PATH; }
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
    virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

    static StatusInt InsertCategory (Dgn::DgnDbR dgnDb,
                                     Utf8CP name,
                                     Dgn::ColorDef* pColor,
                                     bool* pIsVisible = NULL,
                                     bool* pIsPlotted = NULL,
                                     bool* pIsSnappable = NULL,
                                     bool* pIsLocatabled = NULL,
                                     uint32_t* pWeight = NULL,
                                     Dgn::DgnStyleId* pStyleId = NULL,
                                     int32_t* pDisplayPriority = NULL,
                                     Dgn::RenderMaterialId* pMaterialId = NULL,
                                     double* pTransparency = NULL);

    void InsertDomainAuthorities (Dgn::DgnDbR) const;
    static void InsertCodeSpec (Dgn::DgnDbR, Utf8CP);
//__PUBLISH_SECTION_END__

//__PUBLISH_SECTION_START__
public:
    GridsDomain ();
    ~GridsDomain ();

    GRIDSDOMAIN_EXPORT static bool EnsureECSchemaIsLoaded(Dgn::DgnDbR db);
    GRIDSDOMAIN_EXPORT static bool EnsureDomainCategoriesExist (Dgn::DgnDbR db);
    GRIDSDOMAIN_EXPORT static void EnsureDomainAuthoritiesExist (Dgn::DgnDbR db);
};

END_GRIDS_NAMESPACE
