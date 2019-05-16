/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/GeneratedInserts/PublicApi/GeneratedInsertsApi.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDomain.h>
#include <ClassificationSystems/ClassificationSystemsApi.h>

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! Dynamically generated classification inserts
//=======================================================================================
struct GeneratedInserts
{
private:
    ClassificationSystemPtr InsertSystem(Dgn::DgnDbR db, Dgn::DgnModelCR model, Utf8StringCR name, Utf8StringCR edition) const;
    ClassificationTablePtr InsertTable(ClassificationSystemCR system, Utf8CP name) const;
    ClassificationGroupPtr InsertGroup(ClassificationTableCR table, Utf8CP name) const;
    ClassificationSystemCPtr TryAndGetSystem(Dgn::DgnDbR db, Dgn::DgnModelCR model, Utf8StringCR name, Utf8StringCR edition) const;
    ClassificationTableCPtr TryAndGetTable(ClassificationSystemCR system, Utf8CP name) const;
    ClassificationPtr InsertClassification(ClassificationTableCR table, Utf8CP name, Utf8CP id, Utf8CP description, ClassificationGroupCP group, ClassificationCP specializes) const;
public:

    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertStandardDefinitionSystems(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertMasterFormatDefinitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertUniFormatDefinitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass11Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass12Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass13_2006Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass13_2010Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass14Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass21Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass22_2006Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass22_2010Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass23Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass32Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass33Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass34Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass36_04Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass36_24Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass41Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;
    CLASSIFICATIONSYSTEMSGENERATEDINSERTS_EXPORT void InsertOmniClass49Definitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const;

};

END_CLASSIFICATIONSYSTEMS_NAMESPACE
