/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <BuildingDomain\BuildingDomainApi.h>
#include <Json\Json.h>
#include "dgnplatform\FunctionalDomain.h"


DGNPLATFORM_REF_COUNTED_PTR(FunctionalComponentElement)
DGNPLATFORM_REF_COUNTED_PTR(FunctionalBreakdownElement)


#ifdef __BUILDING_DOMAIN_BUILD__
#define BUILDING_DOMAIN_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDING_DOMAIN_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::ARCHITECTURAL_PHYSICAL %ARCHITECTURAL_PHYSICAL data types */
#define BEGIN_BENTLEY_BUILDING_DOMAIN_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace BuildingDomain {
#define END_BENTLEY_BUILDING_DOMAIN_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_BUILDING_DOMAIN        using namespace BentleyApi::BuildingDomain;



BEGIN_BENTLEY_NAMESPACE

namespace BuildingDomain
	{


	//=======================================================================================
	// @bsiclass                                    BentleySystems
	//=======================================================================================
	struct  BuildingDomainUtilities
		{
		BUILDING_DOMAIN_EXPORT static Utf8String                                       BuildPhysicalModelCode            (Utf8StringCR modelCodeName);
		BUILDING_DOMAIN_EXPORT static Utf8String                                       BuildFunctionalModelCode          (Utf8StringCR modelCodeName);
        BUILDING_DOMAIN_EXPORT static Utf8String                                       BuildSpatialLocationModelCode     (Utf8StringCR modelCodeName);
        BUILDING_DOMAIN_EXPORT static Utf8String                                       BuildDrawingModelCode             (Utf8StringCR modelCodeName);
		BUILDING_DOMAIN_EXPORT static Utf8String                                       BuildDocumentListModelCode        (Utf8StringCR modelCodeName);
		BUILDING_DOMAIN_EXPORT static Utf8String                                       BuildTypeDefinitionModelCode      (Utf8StringCR modelCodeName);
		BUILDING_DOMAIN_EXPORT static Utf8String                                       BuildDynamicSchemaName            (Utf8StringCR modelCodeName);
		BUILDING_DOMAIN_EXPORT static BentleyStatus                                    RegisterDomainHandlers            ();
		BUILDING_DOMAIN_EXPORT static BentleyStatus                                    CreateBuildingModels              (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr, bool createDynamicSchema = true, ECN::ECSchemaPtr suppliedDynamicSchema = nullptr);
		BUILDING_DOMAIN_EXPORT static BuildingPhysical::BuildingPhysicalModelPtr       GetBuildingPhyicalModel           (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
        BUILDING_DOMAIN_EXPORT static Dgn::FunctionalModelPtr                          GetBuildingFunctionalModel        (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
        BUILDING_DOMAIN_EXPORT static Dgn::DocumentListModelPtr                        GetBuildingDocumentListModel      (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
        BUILDING_DOMAIN_EXPORT static BuildingPhysical::BuildingTypeDefinitionModelPtr GetBuildingTypeDefinitionModel    (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
		BUILDING_DOMAIN_EXPORT static BuildingPhysical::BuildingPhysicalModelPtr       CreateBuildingPhyicalModel        (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
		BUILDING_DOMAIN_EXPORT static Dgn::DrawingModelPtr                             CreateBuildingDrawingModel        (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::DocumentListModelCR docListModel);
		BUILDING_DOMAIN_EXPORT static Dgn::FunctionalModelPtr                          CreateBuildingFunctionalModel     (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
        BUILDING_DOMAIN_EXPORT static Dgn::SpatialLocationModelPtr                     CreateBuildingSpatialLocationModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
        BUILDING_DOMAIN_EXPORT static Dgn::DocumentListModelPtr                        CreateBuildingDocumentListModel   (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
		BUILDING_DOMAIN_EXPORT static BuildingPhysical::BuildingTypeDefinitionModelPtr CreateBuildingTypeDefinitionModel (Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
		BUILDING_DOMAIN_EXPORT static ECN::ECSchemaCP                                  CreateBuildingDynamicSchema       (Utf8StringCR modelCodeName, BuildingPhysical::BuildingPhysicalModelPtr model);
		BUILDING_DOMAIN_EXPORT static ECN::ECSchemaCP                                  GetBuildingDynamicSchema          (BuildingPhysical::BuildingPhysicalModelCPtr model);
		BUILDING_DOMAIN_EXPORT static Utf8String                                       GetSchemaNameFromModel            (BuildingPhysical::BuildingPhysicalModelCPtr model);
		BUILDING_DOMAIN_EXPORT static BentleyStatus                                    UpdateSchemaNameInModel           (Utf8StringCR schemaName, BuildingPhysical::BuildingPhysicalModelPtr model);
		BUILDING_DOMAIN_EXPORT static ECN::ECSchemaPtr                                 GetUpdateableSchema               (BuildingPhysical::BuildingPhysicalModelCPtr model);
		BUILDING_DOMAIN_EXPORT static Dgn::SchemaStatus                                UpdateSchemaInDb                  (Dgn::DgnDbR db, ECN::ECSchemaR updatedSchema);
		BUILDING_DOMAIN_EXPORT static ECN::ECEntityClassP                              CreatePhysicalElementEntityClass  (Dgn::DgnDbPtr db, ECN::ECSchemaPtr, Utf8StringCR     className);
		BUILDING_DOMAIN_EXPORT static ECN::ECEntityClassP                              CreatePhysicalElementEntityClassFromArchPhysicalClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr, Utf8StringCR className, Utf8StringCR archClassName);
		BUILDING_DOMAIN_EXPORT static ECN::ECEntityClassP                              CreatePhysicalTypeEntityClass     (Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className);
		BUILDING_DOMAIN_EXPORT static ECN::ECEntityClassP                              CreateUniqueAspetClass            (Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className);
		BUILDING_DOMAIN_EXPORT static ECN::IECInstancePtr                              AddAspect                         (Dgn::PhysicalModelCR model, Dgn::PhysicalElementPtr element, Utf8StringCR schemaName, Utf8StringCR className);


		BUILDING_DOMAIN_EXPORT static ECN::ECClassCP                                   GetExistingECClass                (Dgn::DgnDbPtr db, Utf8StringCR schemaName, Utf8StringCR  className);


        BUILDING_DOMAIN_EXPORT static Dgn::FunctionalBreakdownElementPtr               CreateFunctionalBreakdownElement  (Utf8StringCR schemaName, Utf8StringCR className, Dgn::FunctionalModelCR model);
        BUILDING_DOMAIN_EXPORT static Dgn::FunctionalComponentElementPtr               CreateFunctionalComponentElement  (Utf8StringCR schemaName, Utf8StringCR className, Dgn::FunctionalModelCR model);
		BUILDING_DOMAIN_EXPORT static Dgn::PhysicalElementPtr                          CreatePhysicalElement             (Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelCR model, Utf8CP categoryName = nullptr );
		BUILDING_DOMAIN_EXPORT static Dgn::PhysicalTypePtr                             CreatePhysicalTypeElement         (Utf8StringCR schemaName, Utf8StringCR className, Dgn::DefinitionModelCR model);
        BUILDING_DOMAIN_EXPORT static Dgn::DrawingGraphicPtr                           CreateDrawingGraphic              (Utf8StringCR schemaName, Utf8StringCR className, Dgn::DrawingModelCR model, Dgn::DgnCategoryId categoryId);


		BUILDING_DOMAIN_EXPORT static ECN::ECSchemaCP                                  InsertSuppliedSchema              (ECN::ECSchemaPtr suppliedDynamicSchema, BuildingPhysical::BuildingPhysicalModelPtr model);

		                        static Utf8String                                      CreateCodeSpecNameFromECClass(ECN::ECClassCP ecClass) {	Utf8String codeSpecName = ecClass->GetSchema().GetName() + "-" + ecClass->GetName(); return codeSpecName;}
			

		                       static Dgn::DgnCode                                     CreateCode(Dgn::DgnModelCR model, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, model, codeValue); }
							   static Dgn::DgnCode                                     CreateCode(Dgn::DgnModelCR model, ECN::ECClassCP ecClass, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(CreateCodeSpecNameFromECClass(ecClass).c_str(), model, codeValue); }
							   

		template <class T> static RefCountedPtr<T>                                     QueryById(Dgn::DgnModelCR model, Dgn::DgnElementId id) { Dgn::DgnDbR    db = model.GetDgnDb(); return db.Elements().GetForEdit<T>(id); }
		template <class T> static RefCountedPtr<T>                                     QueryByCode(Dgn::DgnModelCR model, Dgn::DgnCodeCR code) { Dgn::DgnDbR  db = model.GetDgnDb(); return QueryById<T>(model, db.Elements().QueryElementIdByCode(code)); }
		template <class T> static RefCountedPtr<T>									   QueryByCodeValue(Dgn::DgnModelCR model, Utf8StringCR codeValue) { Dgn::DgnCode code = CreateCode(model, codeValue); return QueryByCode<T>(model, code); }

        BUILDING_DOMAIN_EXPORT static Units::UnitCP                                    GetUnitCPFromProperty(Dgn::DgnElementCR element, Utf8StringCR propertyName);

        

        BUILDING_DOMAIN_EXPORT static BentleyStatus                                    SetDoublePropertyFromStringWithUnits(Dgn::DgnElementR element, Utf8StringCR propertyName, Utf8StringCR propertyValueString);
        BUILDING_DOMAIN_EXPORT static BentleyStatus                                    SetDoublePropertyUsingUnitString(Dgn::DgnElementR element, Utf8StringCR propertyName, Utf8StringCR unitString, double value);
        BUILDING_DOMAIN_EXPORT static BentleyStatus                                    GetDoublePropertyUsingUnitString(Dgn::DgnElementCR element, Utf8StringCR propertyName, Utf8StringCR unitString, double& value);
        BUILDING_DOMAIN_EXPORT static Dgn::DgnDbStatus                                 FindOrCreateSubCategory(Dgn::DgnDbPtr dgnDb, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId &subCategoryId, Utf8CP subCategoryName);

        };

	//=======================================================================================
	// @bsiclass                                    BentleySystems
	//=======================================================================================

    #define BuildingDomainSettingsVersion 1

	struct BuildingDomainSettings : Json::Value
		{

		public:

		private:
			BE_JSON_NAME(version);
			BE_JSON_NAME(classlibraryName);

			BuildingDomainSettings(Utf8CP modelDomainName = nullptr) { SetVersion(BuildingDomainSettingsVersion); SetclasslibraryName(modelDomainName); }
			void SetVersion(int version) { (*this)[json_version()] = version; }
			void SetclasslibraryName(Utf8CP classlibraryName) { if (classlibraryName && *classlibraryName) (*this)[json_classlibraryName()] = classlibraryName; }
			JsonValueCR GetValue(Utf8CP key) const { return (*this)[key]; }

		public:
			void operator = (const Json::Value  value) { SetVersion(value[json_version()].asInt()); SetclasslibraryName(value[json_classlibraryName()].asString().c_str()); }
			int GetVersion() const { return (GetValue(json_version()).asInt()); }
			Utf8String GetclasslibraryName() const { return GetValue(json_classlibraryName()).asString(); }

			static BuildingDomainSettings CreateBuildingDomainSettings(Utf8CP modelDomainName = nullptr) { return BuildingDomainSettings(modelDomainName); }

		};
	}

END_BENTLEY_NAMESPACE


