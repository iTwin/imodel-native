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

    void InsertStandardDefinitionSystems(Dgn::DgnDbR db) const;
    void InsertMasterFormatDefinitions(Dgn::DgnDbR db) const;
    void InsertUniFormatDefinitions(Dgn::DgnDbR db) const;
    void InsertOmniClass11Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass12Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass13_2006Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass13_2010Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass14Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass21Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass22_2006Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass22_2010Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass23Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass32Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass33Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass34Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass36_04Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass36_24Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass41Definitions(Dgn::DgnDbR db) const;
    void InsertOmniClass49Definitions(Dgn::DgnDbR db) const; 
    ClassificationSystemPtr InsertSystem(Dgn::DgnDbR db, Utf8CP name) const;
    ClassificationGroupPtr InsertGroup(ClassificationSystemCR system, Utf8CP name) const;
    ClassificationSystemCPtr TryAndGetSystem(Dgn::DgnDbR db, Utf8CP name) const;
    void InsertCIBSE(ClassificationSystemCR system, ClassificationGroupCR group, Utf8CP name) const;
    void InsertASHRAE2004(ClassificationSystemCR system, ClassificationGroupCR group, Utf8CP name) const;
    void InsertASHRAE2007(ClassificationSystemCR system, ClassificationGroupCR group, Utf8CP name) const;
    void InsertASHRAE2010(ClassificationSystemCR system, ClassificationGroupCR group, Utf8CP name) const;
    UniFormatClassDefinitionPtr InsertUniFormat(ClassificationSystemCR system, Utf8CP code, Utf8CP name, UniFormatClassDefinitionCP specializes) const;
    MasterFormatClassDefinitionPtr InsertMasterFormat(ClassificationSystemCR system, Utf8CP code, Utf8CP name, MasterFormatClassDefinitionCP specializes) const;
    OmniClassClassDefinitionPtr InsertOmniClass(ClassificationSystemCR system, Utf8CP code, Utf8CP name, OmniClassClassDefinitionCP specializes) const;
//__PUBLISH_SECTION_END__

//__PUBLISH_SECTION_START__
public:
    ClassificationSystemsDomain ();
    ~ClassificationSystemsDomain ();

};

END_CLASSIFICATIONSYSTEMS_NAMESPACE
