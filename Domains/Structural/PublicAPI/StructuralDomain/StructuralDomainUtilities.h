/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/StructuralDomain/StructuralDomainUtilities.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <StructuralDomain/StructuralDomainApi.h>
#include <Json/Json.h>


#ifdef __STRUCTURAL_DOMAIN_BUILD__
#define STRUCTURAL_DOMAIN_EXPORT EXPORT_ATTRIBUTE
#else
#define STRUCTURAL_DOMAIN_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::STRUCTURAL_DOMAIN %STRUCTURAL_DOMAIN data types */
#define BEGIN_BENTLEY_STRUCTURAL_DOMAIN_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace StructuralDomain {
#define END_BENTLEY_STRUCTURAL_DOMAIN_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_STRUCTURAL_DOMAIN        using namespace BentleyApi::StructuralDomain;


BEGIN_BENTLEY_STRUCTURAL_DOMAIN_NAMESPACE

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct StructuralDomainUtilities
    {
    STRUCTURAL_DOMAIN_EXPORT static Utf8String                                              BuildDynamicSchemaName(Utf8StringCR modelCodeName);
    STRUCTURAL_DOMAIN_EXPORT static Utf8String                                              BuildPhysicalModelCode(Utf8StringCR modelCodeName);
    STRUCTURAL_DOMAIN_EXPORT static Utf8String                                              BuildTypeDefinitionModelCode(Utf8StringCR modelCodeName);

    STRUCTURAL_DOMAIN_EXPORT static BentleyStatus                                           RegisterDomainHandlers();

    STRUCTURAL_DOMAIN_EXPORT static ECN::ECSchemaCP                                         GetStructuralDynamicSchema(StructuralPhysical::StructuralPhysicalModelCPtr model);
    STRUCTURAL_DOMAIN_EXPORT static ECN::ECSchemaPtr                                        GetUpdateableSchema(StructuralPhysical::StructuralPhysicalModelCPtr model);
    STRUCTURAL_DOMAIN_EXPORT static Utf8String                                              GetSchemaNameFromModel(StructuralPhysical::StructuralPhysicalModelCPtr model);
    STRUCTURAL_DOMAIN_EXPORT static StructuralPhysical::StructuralPhysicalModelPtr          GetStructuralPhyicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static StructuralPhysical::StructuralTypeDefinitionModelPtr    GetStructuralTypeDefinitionModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static ECN::ECClassCP                                          GetExistingECClass(Dgn::DgnDbPtr db, Utf8StringCR schemaName, Utf8StringCR className);

    STRUCTURAL_DOMAIN_EXPORT static ECN::ECSchemaCP                                         CreateStructuralDynamicSchema(Utf8StringCR modelCodeName, StructuralPhysical::StructuralPhysicalModelPtr model);
    STRUCTURAL_DOMAIN_EXPORT static BentleyStatus                                           CreateStructuralModels(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr, bool createDynamicSchema = true, ECN::ECSchemaPtr suppliedDynamicSchema = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static StructuralPhysical::StructuralPhysicalModelPtr          CreateStructuralPhysicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static StructuralPhysical::StructuralTypeDefinitionModelPtr    CreateStructuralTypeDefinitionModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static Dgn::PhysicalElementPtr                                 CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelCR model, Utf8CP categoryName = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static ECN::ECEntityClassP                                     CreatePhysicalElementEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr, Utf8StringCR className);

    STRUCTURAL_DOMAIN_EXPORT static ECN::ECSchemaCP                                         InsertSuppliedSchema(ECN::ECSchemaPtr suppliedDynamicSchema, StructuralPhysical::StructuralPhysicalModelPtr model);

    STRUCTURAL_DOMAIN_EXPORT static Dgn::SchemaStatus                                       UpdateSchemaInDb(Dgn::DgnDbR db, ECN::ECSchemaR updatedSchema);
    STRUCTURAL_DOMAIN_EXPORT static BentleyStatus                                           UpdateSchemaNameInModel(Utf8StringCR schemaName, StructuralPhysical::StructuralPhysicalModelPtr model);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================

#define StructuralDomainSettingsVersion 1

struct StructuralDomainSettings : Json::Value
    {
    private:
        BE_JSON_NAME(version);
        BE_JSON_NAME(classlibraryName);

        StructuralDomainSettings(Utf8CP modelDomainName = nullptr) { SetVersion(StructuralDomainSettingsVersion); SetclasslibraryName(modelDomainName); }
        void SetVersion(int version) { (*this)[json_version()] = version; }
        void SetclasslibraryName(Utf8CP classlibraryName) { if (classlibraryName && *classlibraryName) (*this)[json_classlibraryName()] = classlibraryName; }
        JsonValueCR GetValue(Utf8CP key) const { return (*this)[key]; }

    public:
        void operator = (const Json::Value  value) { SetVersion(value[json_version()].asInt()); SetclasslibraryName(value[json_classlibraryName()].asString().c_str()); }
        int GetVersion() const { return (GetValue(json_version()).asInt()); }
        Utf8String GetclasslibraryName() const { return GetValue(json_classlibraryName()).asString(); }

        static StructuralDomainSettings CreateStructuralDomainSettings(Utf8CP modelDomainName = nullptr) { return StructuralDomainSettings(modelDomainName); }
    };

END_BENTLEY_STRUCTURAL_DOMAIN_NAMESPACE
