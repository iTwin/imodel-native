/*--------------------------------------------------------------------------------------+
|
|     $Source: BuildingDomainUtilities.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BuildingDomainInternal.h"


BE_JSON_NAME(BuildingDomain)

BEGIN_BENTLEY_NAMESPACE

namespace BuildingDomain
	{
	template<class T, class U> RefCountedCPtr<T> const_pointer_cast(RefCountedCPtr<U> const & p) { return dynamic_cast<T const *>(p.get()); }


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BentleyStatus  BuildingDomainUtilities::RegisterDomainHandlers()
		{
		if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::ArchitecturalPhysical::ArchitecturalPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
			return BentleyStatus::ERROR;

		if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::BuildingCommon::BuildingCommonDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
			return BentleyStatus::ERROR;

		if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::BuildingPhysical::BuildingPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
			return BentleyStatus::ERROR;

		return BentleyStatus::SUCCESS;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BuildingPhysical::BuildingPhysicalModelPtr BuildingDomainUtilities::CreateBuildingPhyicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbPtr db, Dgn::SubjectCPtr parentSubject)
		{

		if (!parentSubject.IsValid())
			{
			parentSubject = db->Elements().GetRootSubject();
			}

		// Create the partition and the BuildingPhysicalModel.

		Utf8String phyModelCode = BuildPhysicalModelCode(modelCodeName);

		Dgn::PhysicalPartitionCPtr partition = Dgn::PhysicalPartition::CreateAndInsert(*parentSubject, phyModelCode);

		if (!partition.IsValid())
			return nullptr;

		BuildingPhysical::BuildingPhysicalModelPtr physicalModel = BuildingPhysical::BuildingPhysicalModel::Create(*partition);

		return physicalModel;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String BuildingDomainUtilities::GetSchemaNameFromModel(BuildingPhysical::BuildingPhysicalModelCPtr model)
		{
		BuildingDomainSettings outSettings = BuildingDomainSettings::CreateBuildingDomainSettings();

		outSettings = model->GetJsonProperties(json_BuildingDomain());

		return outSettings.GetclasslibraryName();
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BentleyStatus BuildingDomainUtilities::UpdateSchemaNameInModel(Utf8StringCR schemaName, BuildingPhysical::BuildingPhysicalModelPtr model)
		{
		BuildingDomainSettings settings = BuildingDomainSettings::CreateBuildingDomainSettings(schemaName.c_str());

		model->SetJsonProperties(json_BuildingDomain(), settings);

		if (Dgn::DgnDbStatus::Success != model->Update())
			{
			return BentleyStatus::ERROR;
			}

		return BentleyStatus::SUCCESS;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BuildingPhysical::BuildingTypeDefinitionModelPtr BuildingDomainUtilities::CreateBuildingTypeDefinitionModel(Utf8StringCR modelCodeName, Dgn::DgnDbPtr db)
		{
		Dgn::SubjectCPtr rootSubject = db->Elements().GetRootSubject();

		Utf8String defModelCode = BuildTypeDefinitionModelCode(modelCodeName);

		Dgn::DefinitionPartitionCPtr defPartition = Dgn::DefinitionPartition::CreateAndInsert(*rootSubject, defModelCode);

		if (!defPartition.IsValid())
			return nullptr;

		BuildingPhysical::BuildingTypeDefinitionModelPtr typeDefinitionModel = BuildingPhysical::BuildingTypeDefinitionModel::Create(*defPartition);


		return typeDefinitionModel;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECSchemaCP BuildingDomainUtilities::GetBuildingDynamicSchema(BuildingPhysical::BuildingPhysicalModelCPtr model)
		{

		BuildingDomainSettings outSettings = BuildingDomainSettings::CreateBuildingDomainSettings();

		outSettings = model->GetJsonProperties(json_BuildingDomain());

		if (Utf8String::IsNullOrEmpty(outSettings.GetclasslibraryName().c_str()))
			return nullptr;

		return model->GetDgnDb().Schemas().GetSchema(outSettings.GetclasslibraryName().c_str());

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECSchemaCP BuildingDomainUtilities::InsertSuppliedSchema(ECN::ECSchemaPtr suppliedDynamicSchema, BuildingPhysical::BuildingPhysicalModelPtr model)
		{
		bvector<ECN::ECSchemaCP> schemas;

		ECN::ECSchemaCP a = &(*suppliedDynamicSchema);

		schemas.push_back(a);

		if (Dgn::SchemaStatus::Success != model->GetDgnDb().ImportSchemas(schemas))
			return nullptr;

		model->GetDgnDb().SaveChanges();

		UpdateSchemaNameInModel(suppliedDynamicSchema->GetName(), model);

		return  model->GetDgnDb().Schemas().GetSchema(suppliedDynamicSchema->GetName().c_str());
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECSchemaCP BuildingDomainUtilities::CreateBuildingDynamicSchema(Utf8StringCR modelCodeName, BuildingPhysical::BuildingPhysicalModelPtr model)
		{

		Dgn::DgnDbR db          = model->GetDgnDb();
		Utf8String schemaName   = BuildDynamicSchemaName(modelCodeName);
		Utf8String internalName = GetSchemaNameFromModel( model);

		bool nameExists = false;

		if (!Utf8String::IsNullOrEmpty(internalName.c_str()))
			{
			schemaName = internalName;
			nameExists = true;
			}

		// Check to see if the schema exists. Should we get a unique name?

		ECN::ECSchemaCP existingSchema = db.Schemas().GetSchema(schemaName.c_str());

		if (nullptr != existingSchema)
			{
			if (!nameExists)
				{
				UpdateSchemaNameInModel(schemaName, model);
				}
			return existingSchema;
			}

		ECN::ECSchemaPtr dynSchema;

		Utf8String alias;

		alias.Sprintf("BLDG%d", rand());
		
		if (ECN::ECObjectsStatus::Success != ECN::ECSchema::CreateSchema(dynSchema, schemaName, alias, 1, 1, 0))
			return nullptr;

		ECN::ECSchemaCP bisSchema = db.Schemas().GetSchema(BIS_ECSCHEMA_NAME);

		if (nullptr == bisSchema)
			return nullptr;
		

		if (ECN::ECObjectsStatus::Success != dynSchema->AddReferencedSchema((ECN::ECSchemaR)(*bisSchema)))
			return nullptr;

		return InsertSuppliedSchema(dynSchema, model);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BentleyStatus BuildingDomainUtilities::CreateBuildingModels(Utf8StringCR modelCodeName, Dgn::DgnDbPtr db, Dgn::SubjectCPtr parentSubject, bool createDynamicSchema, ECN::ECSchemaPtr suppliedDynamicSchema)
		{

		BuildingPhysical::BuildingPhysicalModelPtr physicalModel = CreateBuildingPhyicalModel(modelCodeName, db, parentSubject);

		if (!physicalModel.IsValid())
			return BentleyStatus::ERROR;

		if (createDynamicSchema && !suppliedDynamicSchema.IsValid())
			{
			if (nullptr == CreateBuildingDynamicSchema(modelCodeName, physicalModel))
				return BentleyStatus::ERROR;
			}
		else if (suppliedDynamicSchema.IsValid())
			{
			if (nullptr == InsertSuppliedSchema(suppliedDynamicSchema, physicalModel) )
				return BentleyStatus::ERROR;
			}

		BuildingPhysical::BuildingTypeDefinitionModelPtr typeDefinitionModel = CreateBuildingTypeDefinitionModel(modelCodeName, db);

		if (!typeDefinitionModel.IsValid())
			return BentleyStatus::ERROR;

		return BentleyStatus::SUCCESS;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BuildingPhysical::BuildingPhysicalModelCPtr BuildingDomainUtilities::GetBuildingPhyicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbPtr db)
		{

		BuildingPhysical::BuildingPhysicalModelCPtr buildingModel;

		Dgn::ElementIterator itr = db->Elements().MakeIterator(BIS_SCHEMA("PhysicalPartition"));

		Utf8String PhysicalModelCode = BuildPhysicalModelCode(modelCodeName);

		for each (Dgn::ElementIteratorEntry ele in itr)
			{
			Utf8CP codeValue = ele.GetCodeValue();

			if (PhysicalModelCode != codeValue)
				continue;

			Dgn::DgnElementCPtr element = db->Elements().GetElement(ele.GetElementId());

			Dgn::PhysicalPartitionCPtr partition = const_pointer_cast<Dgn::PhysicalPartition>(element);

			if (partition.IsValid())
				{
				Dgn::DgnModelCPtr model = partition->GetSubModel();
				buildingModel = const_pointer_cast<BuildingPhysical::BuildingPhysicalModel>(model);
				}
			break;
			}

		return buildingModel;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String  BuildingDomainUtilities::BuildPhysicalModelCode(Utf8StringCR modelCodeName)
		{
		return modelCodeName + ":Physical";
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String  BuildingDomainUtilities::BuildTypeDefinitionModelCode(Utf8StringCR modelCodeName)
		{
		return modelCodeName + ":TypeDefinition";
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String  BuildingDomainUtilities::BuildDynamicSchemaName(Utf8StringCR modelCodeName)
		{
		return modelCodeName + "Dynamic";
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECSchemaPtr  BuildingDomainUtilities::GetUpdateableSchema(BuildingPhysical::BuildingPhysicalModelCPtr model)
		{

		ECN::ECSchemaCP schema = GetBuildingDynamicSchema(model);

		if (nullptr == schema)
			return nullptr;

		auto context = ECN::ECSchemaReadContext::CreateContext(false);

		ECN::SchemaKey k(schema->GetSchemaKey());

		ECN::ECSchemaPtr currSchema = model->GetDgnDb().GetSchemaLocater().LocateSchema(k, ECN::SchemaMatchType::Exact, *context);

		ECN::ECSchemaPtr updateableSchema;

		if (ECN::ECObjectsStatus::Success != currSchema->CopySchema(updateableSchema))
			return nullptr;

		updateableSchema->SetVersionMinor(currSchema->GetVersionMinor() + 1);

		return updateableSchema;
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Dgn::SchemaStatus  BuildingDomainUtilities::UpdateSchemaInDb(Dgn::DgnDbR db, ECN::ECSchemaR updatedSchema)
		{
		bvector<ECN::ECSchemaCP> schemas;

		ECN::ECSchemaCP b = &updatedSchema;

		schemas.push_back(b);

		return db.ImportSchemas(schemas);
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECClassCP   BuildingDomainUtilities::GetExistingECClass(Dgn::DgnDbPtr db, Utf8StringCR schemaName, Utf8StringCR  className)
		{
		ECN::ECSchemaCP schema = db->Schemas().GetSchema(schemaName.c_str());

		if (nullptr == schema)
			return nullptr;

		return schema->GetClassCP(className.c_str());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECEntityClassP   BuildingDomainUtilities::CreatePhysicalTypeEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className)
		{
		ECN::ECClassCP baseClass = GetExistingECClass(db, BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalType);

		if (nullptr == baseClass)
			return nullptr;

		ECN::ECEntityClassP newClass;

		if (ECN::ECObjectsStatus::Success != schema->CreateEntityClass(newClass, className))
			return nullptr;

		if (ECN::ECObjectsStatus::Success != newClass->AddBaseClass(*baseClass))
			{
			schema->DeleteClass(*newClass);
			return nullptr;
			}

		return newClass;

		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECEntityClassP   BuildingDomainUtilities::CreatePhysicalElementEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className)
		{
		ECN::ECClassCP baseClass = GetExistingECClass(db, BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalElement);

		if (nullptr == baseClass)
			return nullptr;

		ECN::ECEntityClassP newClass;

		if (ECN::ECObjectsStatus::Success != schema->CreateEntityClass(newClass, className))
			return nullptr;

		if (ECN::ECObjectsStatus::Success != newClass->AddBaseClass(*baseClass))
			{
			schema->DeleteClass(*newClass);
			return nullptr;
			}

		return newClass;

		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECEntityClassP   BuildingDomainUtilities::CreateUniqueAspetClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className)
		{
		ECN::ECClassCP baseClass = GetExistingECClass(db, BIS_ECSCHEMA_NAME, BIS_CLASS_ElementUniqueAspect);

		if (nullptr == baseClass)
			return nullptr;

		ECN::ECEntityClassP newClass;

		if (ECN::ECObjectsStatus::Success != schema->CreateEntityClass(newClass, className))
			return nullptr;

		if (ECN::ECObjectsStatus::Success != newClass->AddBaseClass(*baseClass))
			{
			schema->DeleteClass(*newClass);
			return nullptr;
			}

		// Allow with the class there is now a rule that there must be a relationship with this class as a constraint. A automatic
		// relationship is created with the name PhysicalElementOwnsUniqueAspectXXXX where XXXX is the name of the Aspect class.

		ECN::ECClassCP physicalElementClass = GetExistingECClass(db, BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalElement);

		Utf8String relationshipName;

		relationshipName.Sprintf("PhysicalElementOwnsUniqueAspect%s", className.c_str());

		ECN::ECRelationshipClassP newRelationshipClass;

		if (ECN::ECObjectsStatus::Success != schema->CreateRelationshipClass( newRelationshipClass, relationshipName, *(physicalElementClass->GetEntityClassCP()), "Physical Element", *newClass, "Unique Aspect"))
			{
			schema->DeleteClass(*newClass);
			return nullptr;
			}

		newRelationshipClass->SetStrength(ECN::StrengthType::Embedding);
		newRelationshipClass->SetStrengthDirection(ECN::ECRelatedInstanceDirection::Forward);
		newRelationshipClass->SetClassModifier(ECN::ECClassModifier::None);
		newRelationshipClass->GetSource().SetIsPolymorphic(true);
		newRelationshipClass->GetTarget().SetIsPolymorphic(false);
		newRelationshipClass->GetSource().SetMultiplicity (ECN::RelationshipMultiplicity::OneOne());
		newRelationshipClass->GetTarget().SetMultiplicity (ECN::RelationshipMultiplicity::ZeroOne());

		ECN::ECClassCP baseRelationshipClass = GetExistingECClass(db, BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsUniqueAspect); 

		if (ECN::ECObjectsStatus::Success != newRelationshipClass->AddBaseClass(*baseRelationshipClass))
			{
			schema->DeleteClass(*newClass);
			schema->DeleteClass(*newRelationshipClass);
			return nullptr;
			}

		return newClass;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Dgn::PhysicalElementPtr  BuildingDomainUtilities::CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelCR model)
		{

		Dgn::DgnDbR db = model.GetDgnDb();
		Dgn::DgnModelId modelId = model.GetModelId();

		// Find the class

		ECN::ECClassCP buildingClass = db.GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

		if (nullptr == buildingClass)
			return nullptr;

		ECN::ECClassId classId = buildingClass->GetId();

		Dgn::ElementHandlerP elmHandler = Dgn::dgn_ElementHandler::Element::FindHandler(db, classId);
		if (NULL == elmHandler)
			return nullptr;

		Dgn::DgnCategoryId categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalCategoryId(db, className.c_str());

		Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

		Dgn::DgnElementPtr element = elmHandler->Create(params);

		Dgn::PhysicalElementPtr buildingElement = dynamic_pointer_cast<Dgn::PhysicalElement>(element);

		auto geomSource = buildingElement->ToGeometrySourceP();

		if (nullptr == geomSource)
			return nullptr;

		geomSource->SetCategoryId(categoryId);

		return buildingElement;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Dgn::PhysicalTypePtr  BuildingDomainUtilities::CreatePhysicalTypeElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::DefinitionModelCR model)
		{

		Dgn::DgnDbR db = model.GetDgnDb();
		Dgn::DgnModelId modelId = model.GetModelId();

		// Find the class

		ECN::ECClassCP buildingClass = db.GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

		if (nullptr == buildingClass)
			return nullptr;

		ECN::ECClassId classId = buildingClass->GetId();

		Dgn::ElementHandlerP elmHandler = Dgn::dgn_ElementHandler::Element::FindHandler(db, classId);
		if (NULL == elmHandler)
			return nullptr;

		Dgn::PhysicalType::CreateParams(db, model.GetModelId(), classId);

		Dgn::DgnCategoryId categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalCategoryId(db, className.c_str());

		Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

		Dgn::DgnElementPtr element = elmHandler->Create(params);

		Dgn::PhysicalTypePtr buildingElement = dynamic_pointer_cast<Dgn::PhysicalType>(element);

		//auto geomSource = buildingElement->ToGeometrySourceP();

		//if (nullptr == geomSource)
		//	return nullptr;

	//	geomSource->SetCategoryId(categoryId);

		return buildingElement;

		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::IECInstancePtr   BuildingDomainUtilities::AddAspect(Dgn::PhysicalModelCR model, Dgn::PhysicalElementPtr element, Utf8StringCR schemaName, Utf8StringCR className)
		{

		// Find the class

		ECN::ECClassCP aspectClassP = model.GetDgnDb().GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

		if (nullptr == aspectClassP)
			return nullptr;

		// If the element is already persisted and has the Aspect class, you can't add another

		if (element->GetElementId().IsValid())
			{
			ECN::IECInstanceCP instance = Dgn::DgnElement::GenericUniqueAspect::GetAspect (*element, *aspectClassP);

			if (nullptr != instance)
				return nullptr;
			}

		ECN::StandaloneECEnablerPtr enabler = aspectClassP->GetDefaultStandaloneEnabler();

		if (!enabler.IsValid())
			return nullptr;

		ECN::IECInstancePtr instance = enabler->CreateInstance().get();
		if (!instance.IsValid())
			return nullptr;

		Dgn::DgnDbStatus status = Dgn::DgnElement::GenericUniqueAspect::SetAspect (*element, *instance);

		if (Dgn::DgnDbStatus::Success != status)
			return nullptr;

		return instance;


		}



    }

END_BENTLEY_NAMESPACE



