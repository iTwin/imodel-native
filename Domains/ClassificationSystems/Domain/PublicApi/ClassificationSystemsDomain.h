/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/PublicApi/ClassificationSystemsDomain.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDomain.h>


BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! The DgnDomain for the building schema.
//! @private
//=======================================================================================
struct ClassificationSystemsDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ClassificationSystemsDomain, CLASSIFICATIONSYSTEMSDOMAIN_EXPORT)

private:
    WCharCP _GetSchemaRelativePath () const override { return CLASSIFICATIONSYSTEMS_SCHEMA_PATH; }
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;

    void InsertDomainAuthorities (Dgn::DgnDbR) const;
    static void InsertCodeSpec (Dgn::DgnDbR, Utf8CP);
//__PUBLISH_SECTION_END__

//__PUBLISH_SECTION_START__
public:
    ClassificationSystemsDomain ();
    ~ClassificationSystemsDomain ();

    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT static bool EnsureECSchemaIsLoaded(Dgn::DgnDbR db);
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT static bool EnsureDomainCategoriesExist (Dgn::DgnDbR db);
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT static void EnsureDomainAuthoritiesExist (Dgn::DgnDbR db);

};

END_CLASSIFICATIONSYSTEMS_NAMESPACE
