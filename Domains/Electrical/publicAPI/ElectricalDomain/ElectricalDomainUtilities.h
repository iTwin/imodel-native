/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ElectricalDomain\ElectricalDomainApi.h>
#include <Json\Json.h>


#ifdef __ELECTRICAL_DOMAIN_BUILD__
#define ELECTRICAL_DOMAIN_EXPORT EXPORT_ATTRIBUTE
#else
#define ELECTRICAL_DOMAIN_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::ELECTRICAL_PHYSICAL %ELECTRICAL_PHYSICAL data types */
#define BEGIN_BENTLEY_ELECTRICAL_DOMAIN_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace ElectricalDomain {
#define END_BENTLEY_ELECTRICAL_DOMAIN_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ELECTRICAL_DOMAIN        using namespace BentleyApi::ElectricalDomain;



BEGIN_BENTLEY_NAMESPACE

namespace ElectricalDomain
	{

	//=======================================================================================
	// @bsiclass                                    BentleySystems
	//=======================================================================================
	struct  ElectricalDomainUtilities
		{
		ELECTRICAL_DOMAIN_EXPORT static Utf8String                                           BuildPhysicalModelCode(Utf8StringCR modelCodeName);
		ELECTRICAL_DOMAIN_EXPORT static Utf8String                                           BuildTypeDefinitionModelCode(Utf8StringCR modelCodeName);
		ELECTRICAL_DOMAIN_EXPORT static Utf8String                                           BuildDynamicSchemaName(Utf8StringCR modelCodeName);
		ELECTRICAL_DOMAIN_EXPORT static BentleyStatus                                        RegisterDomainHandlers();
		ELECTRICAL_DOMAIN_EXPORT static BentleyStatus                                        CreateElectricalModels(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr, bool createDynamicSchema = true, ECN::ECSchemaPtr suppliedDynamicSchema = nullptr);
		ELECTRICAL_DOMAIN_EXPORT static ElectricalPhysical::ElectricalPhysicalModelPtr       GetElectricalPhyicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
		ELECTRICAL_DOMAIN_EXPORT static ElectricalPhysical::ElectricalTypeDefinitionModelPtr GetElectricalTypeDefinitionModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
		ELECTRICAL_DOMAIN_EXPORT static ElectricalPhysical::ElectricalPhysicalModelPtr       CreateElectricalPhyicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
		ELECTRICAL_DOMAIN_EXPORT static ElectricalPhysical::ElectricalTypeDefinitionModelPtr CreateElectricalTypeDefinitionModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
		ELECTRICAL_DOMAIN_EXPORT static ECN::ECSchemaCP                                      CreateElectricalDynamicSchema(Utf8StringCR modelCodeName, ElectricalPhysical::ElectricalPhysicalModelPtr model);
		ELECTRICAL_DOMAIN_EXPORT static ECN::ECSchemaCP                                      GetElectricalDynamicSchema(ElectricalPhysical::ElectricalPhysicalModelCPtr model);
		ELECTRICAL_DOMAIN_EXPORT static Utf8String                                           GetSchemaNameFromModel(ElectricalPhysical::ElectricalPhysicalModelCPtr model);
		ELECTRICAL_DOMAIN_EXPORT static BentleyStatus                                        UpdateSchemaNameInModel(Utf8StringCR schemaName, ElectricalPhysical::ElectricalPhysicalModelPtr model);
		ELECTRICAL_DOMAIN_EXPORT static ECN::ECSchemaPtr                                     GetUpdateableSchema(ElectricalPhysical::ElectricalPhysicalModelCPtr model);
		ELECTRICAL_DOMAIN_EXPORT static Dgn::SchemaStatus                                    UpdateSchemaInDb(Dgn::DgnDbR db, ECN::ECSchemaR updatedSchema);
		ELECTRICAL_DOMAIN_EXPORT static ECN::ECEntityClassP                                  CreatePhysicalElementEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr, Utf8StringCR     className);
		ELECTRICAL_DOMAIN_EXPORT static ECN::ECEntityClassP                                  CreatePhysicalElementEntityClassFromArchPhysicalClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr, Utf8StringCR className, Utf8StringCR archClassName);
		ELECTRICAL_DOMAIN_EXPORT static ECN::ECEntityClassP                                  CreatePhysicalTypeEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className);
		ELECTRICAL_DOMAIN_EXPORT static ECN::ECEntityClassP                                  CreateUniqueAspetClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className);
		ELECTRICAL_DOMAIN_EXPORT static ECN::IECInstancePtr                                  AddAspect(Dgn::PhysicalModelCR model, Dgn::PhysicalElementPtr element, Utf8StringCR schemaName, Utf8StringCR className);


		ELECTRICAL_DOMAIN_EXPORT static ECN::ECClassCP                                       GetExistingECClass(Dgn::DgnDbPtr db, Utf8StringCR schemaName, Utf8StringCR  className);


		ELECTRICAL_DOMAIN_EXPORT static Dgn::PhysicalElementPtr                              CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelCR model, Utf8CP categoryName = nullptr);
		ELECTRICAL_DOMAIN_EXPORT static Dgn::PhysicalTypePtr                                 CreatePhysicalTypeElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::DefinitionModelCR model);

		ELECTRICAL_DOMAIN_EXPORT static ECN::ECSchemaCP                                      InsertSuppliedSchema(ECN::ECSchemaPtr suppliedDynamicSchema, ElectricalPhysical::ElectricalPhysicalModelPtr model);

		static Utf8String                                                                    CreateCodeSpecNameFromECClass(ECN::ECClassCP ecClass) { Utf8String codeSpecName = ecClass->GetSchema().GetName() + "-" + ecClass->GetName(); return codeSpecName; }


		static Dgn::DgnCode                                                                  CreateCode(Dgn::DgnModelCR model, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(BENTLEY_ELECTRICAL_PHYSICAL_AUTHORITY, model, codeValue); }
		static Dgn::DgnCode                                                                  CreateCode(Dgn::DgnModelCR model, ECN::ECClassCP ecClass, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(CreateCodeSpecNameFromECClass(ecClass).c_str(), model, codeValue); }


		template <class T> static RefCountedPtr<T>                                           QueryById(Dgn::DgnModelCR model, Dgn::DgnElementId id) { Dgn::DgnDbR    db = model.GetDgnDb(); return db.Elements().GetForEdit<T>(id); }
		template <class T> static RefCountedPtr<T>                                           QueryByCode(Dgn::DgnModelCR model, Dgn::DgnCodeCR code) { Dgn::DgnDbR  db = model.GetDgnDb(); return QueryById<T>(model, db.Elements().QueryElementIdByCode(code)); }
		template <class T> static RefCountedPtr<T>								             QueryByCodeValue(Dgn::DgnModelCR model, Utf8StringCR codeValue) { Dgn::DgnCode code = CreateCode(model, codeValue); return QueryByCode<T>(model, code); }

		};

	//=======================================================================================
	// @bsiclass                                    BentleySystems
	//=======================================================================================

#define ElectricalDomainSettingsVersion 1

	struct ElectricalDomainSettings : Json::Value
		{

		public:

		private:
			BE_JSON_NAME(version);
			BE_JSON_NAME(classlibraryName);

			ElectricalDomainSettings(Utf8CP modelDomainName = nullptr) { SetVersion(ElectricalDomainSettingsVersion); SetclasslibraryName(modelDomainName); }
			void SetVersion(int version) { (*this)[json_version()] = version; }
			void SetclasslibraryName(Utf8CP classlibraryName) { if (classlibraryName && *classlibraryName) (*this)[json_classlibraryName()] = classlibraryName; }
			JsonValueCR GetValue(Utf8CP key) const { return (*this)[key]; }

		public:
			void operator = (const Json::Value  value) { SetVersion(value[json_version()].asInt()); SetclasslibraryName(value[json_classlibraryName()].asString().c_str()); }
			int GetVersion() const { return (GetValue(json_version()).asInt()); }
			Utf8String GetclasslibraryName() const { return GetValue(json_classlibraryName()).asString(); }

			static ElectricalDomainSettings CreateElectricalDomainSettings(Utf8CP modelDomainName = nullptr) { return ElectricalDomainSettings(modelDomainName); }

		};
	}

END_BENTLEY_NAMESPACE


