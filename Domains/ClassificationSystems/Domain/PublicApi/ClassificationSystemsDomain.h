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

    void InsertDefinitionSystems(Dgn::DgnDbR db) const;
    ClassificationSystemClassDefinitionGroupPtr InsertGroup(Dgn::DgnDbR db, Utf8CP name) const;
    void InsertCIBSE(Dgn::DgnDbR db, ClassificationSystemClassDefinitionGroupCR group, Utf8CP name) const;
    void InsertASHRAE2004(Dgn::DgnDbR db, ClassificationSystemClassDefinitionGroupCR group, Utf8CP name) const;
    void InsertASHRAE2007(Dgn::DgnDbR db, ClassificationSystemClassDefinitionGroupCR group, Utf8CP name) const;
    void InsertASHRAE2010(Dgn::DgnDbR db, ClassificationSystemClassDefinitionGroupCR group, Utf8CP name) const;
//__PUBLISH_SECTION_END__

//__PUBLISH_SECTION_START__
public:
    ClassificationSystemsDomain ();
    ~ClassificationSystemsDomain ();

};

END_CLASSIFICATIONSYSTEMS_NAMESPACE
