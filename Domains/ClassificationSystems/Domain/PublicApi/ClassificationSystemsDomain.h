/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    void InsertDomainAuthorities(Dgn::DgnDbR db) const;
    void InsertCodeSpec(Dgn::DgnDbR db, Utf8CP name) const;
    Dgn::DgnCode GetSystemCode(Dgn::DgnDbR db, Utf8CP name) const;
    ClassificationSystemPtr InsertSystem(Dgn::DgnDbR db, Utf8StringCR name, Utf8StringCR edition) const;
    ClassificationTablePtr InsertTable(ClassificationSystemCR system, Utf8CP name) const;
    ClassificationGroupPtr InsertGroup(ClassificationTableCR table, Utf8CP name) const;
    ClassificationSystemCPtr TryAndGetSystem(Dgn::DgnDbR db, Utf8StringCR name, Utf8StringCR edition) const;
    ClassificationTableCPtr TryAndGetTable(ClassificationSystemCR system, Utf8CP name) const;
    ClassificationPtr InsertClassification(ClassificationTableCR table, Utf8CP name, Utf8CP id, Utf8CP description, ClassificationGroupCP group, ClassificationCP specializes) const;
//__PUBLISH_SECTION_END__

//__PUBLISH_SECTION_START__
public:
    ClassificationSystemsDomain ();
    ~ClassificationSystemsDomain ();
    
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertStandardDefinitionSystems(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertMasterFormatDefinitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertUniFormatDefinitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass11Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass12Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass13_2006Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass13_2010Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass14Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass21Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass22_2006Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass22_2010Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass23Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass32Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass33Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass34Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass36_04Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass36_24Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass41Definitions(Dgn::DgnDbR db) const;
    CLASSIFICATIONSYSTEMSDOMAIN_EXPORT void InsertOmniClass49Definitions(Dgn::DgnDbR db) const; 

};

END_CLASSIFICATIONSYSTEMS_NAMESPACE
