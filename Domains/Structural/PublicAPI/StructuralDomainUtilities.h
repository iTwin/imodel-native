/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "StructuralDomainApi.h"
#include <Json/Json.h>

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct StructuralDomainUtilities
    {
    STRUCTURAL_DOMAIN_EXPORT static BentleyStatus                    RegisterDomainHandlers();

    STRUCTURAL_DOMAIN_EXPORT static BeSQLite::DbResult               InsertLinkTableRelationship(BeSQLite::EC::ECInstanceKey& relKey, Dgn::DgnDbR db, Utf8StringCR schemaName, Utf8StringCR relationshipClassName, BeSQLite::EC::ECInstanceKey source, BeSQLite::EC::ECInstanceKey target);

    STRUCTURAL_DOMAIN_EXPORT static Utf8String                       BuildDynamicSchemaName(Utf8StringCR modelCodeName);
    STRUCTURAL_DOMAIN_EXPORT static Utf8String                       BuildPhysicalModelCode(Utf8StringCR modelCodeName);
    STRUCTURAL_DOMAIN_EXPORT static Utf8String                       BuildTypeDefinitionModelCode(Utf8StringCR modelCodeName);

    STRUCTURAL_DOMAIN_EXPORT static ECN::ECSchemaCP                  GetStructuralDynamicSchema(StructuralPhysicalModelCPtr model);
    STRUCTURAL_DOMAIN_EXPORT static ECN::ECSchemaPtr                 GetUpdateableSchema(StructuralPhysicalModelCPtr model);
    STRUCTURAL_DOMAIN_EXPORT static Utf8String                       GetSchemaNameFromModel(StructuralPhysicalModelCPtr model);
    STRUCTURAL_DOMAIN_EXPORT static StructuralPhysicalModelPtr       GetStructuralPhysicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static ECN::ECClassCP                   GetExistingECClass(Dgn::DgnDbPtr db, Utf8StringCR schemaName, Utf8StringCR className);

    STRUCTURAL_DOMAIN_EXPORT static ECN::ECSchemaCP                  CreateStructuralDynamicSchema(Utf8StringCR modelCodeName, StructuralPhysicalModelPtr model);
    STRUCTURAL_DOMAIN_EXPORT static BentleyStatus                    CreateStructuralModels(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr, bool createDynamicSchema = true, ECN::ECSchemaPtr suppliedDynamicSchema = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static StructuralPhysicalModelPtr       CreateStructuralPhysicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static Dgn::PhysicalElementPtr          CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelCR model, Utf8CP categoryName = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static ECN::ECEntityClassP              CreatePhysicalElementEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr, Utf8StringCR className);

    STRUCTURAL_DOMAIN_EXPORT static ECN::ECEntityClassP              CreateDefinitionElementEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR className);
    STRUCTURAL_DOMAIN_EXPORT static Dgn::DefinitionElementPtr        CreateDefinitionElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::DefinitionModelCR model, Utf8CP categoryName = nullptr);
    STRUCTURAL_DOMAIN_EXPORT static ECN::ECSchemaCP                  InsertSuppliedSchema(ECN::ECSchemaPtr suppliedDynamicSchema, StructuralPhysicalModelPtr model);

    STRUCTURAL_DOMAIN_EXPORT static Dgn::SchemaStatus                UpdateSchemaInDb(Dgn::DgnDbR db, ECN::ECSchemaR updatedSchema);
    STRUCTURAL_DOMAIN_EXPORT static BentleyStatus                    UpdateSchemaNameInModel(Utf8StringCR schemaName, StructuralPhysicalModelPtr model);

    static Utf8String                                                CreateCodeSpecNameFromECClass(ECN::ECClassCP ecClass) { Utf8String codeSpecName = ecClass->GetSchema().GetName() + "-" + ecClass->GetName(); return codeSpecName; }
    static Dgn::DgnCode                                              CreateCode(Dgn::DgnModelCR model, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, model, codeValue); }
    static Dgn::DgnCode                                              CreateCode(Dgn::DgnModelCR model, ECN::ECClassCP ecClass, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(CreateCodeSpecNameFromECClass(ecClass).c_str(), model, codeValue); }
    template <class T> static RefCountedPtr<T>                       QueryById(Dgn::DgnModelCR model, Dgn::DgnElementId id) { Dgn::DgnDbR    db = model.GetDgnDb(); return db.Elements().GetForEdit<T>(id); }
    template <class T> static RefCountedPtr<T>                       QueryByCode(Dgn::DgnModelCR model, Dgn::DgnCodeCR code) { Dgn::DgnDbR  db = model.GetDgnDb(); return QueryById<T>(model, db.Elements().QueryElementIdByCode(code)); }
    template <class T> static RefCountedPtr<T>                       QueryByCodeValue(Dgn::DgnModelCR model, Utf8StringCR codeValue) { Dgn::DgnCode code = CreateCode(model, codeValue); return QueryByCode<T>(model, code); }
    template <class T> static RefCountedPtr<T>                       QueryByCodeValue(Utf8CP codeSpecName, Dgn::DgnModelCR model, Utf8StringCR codeValue) { Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(codeSpecName, model, codeValue); return QueryByCode<T>(model, code); }
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

END_BENTLEY_STRUCTURAL_NAMESPACE
