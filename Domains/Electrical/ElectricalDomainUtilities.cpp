/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ElectricalDomainInternal.h"


BE_JSON_NAME(ElectricalDomain)

BEGIN_BENTLEY_NAMESPACE

namespace ElectricalDomain
	{
	template<class T, class U> RefCountedCPtr<T> const_pointer_cast(RefCountedCPtr<U> const & p) { return dynamic_cast<T const *>(p.get()); }


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BentleyStatus  ElectricalDomainUtilities::RegisterDomainHandlers()
		{
		if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::ElectricalPhysical::ElectricalPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
			return BentleyStatus::ERROR;

		return BentleyStatus::SUCCESS;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ElectricalPhysical::ElectricalPhysicalModelPtr ElectricalDomainUtilities::CreateElectricalPhyicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
		{

		if (!parentSubject.IsValid())
			{
			parentSubject = db.Elements().GetRootSubject();
			}

		// Create the partition and the ElectricalPhysicalModel.

		Utf8String phyModelCode = BuildPhysicalModelCode(modelCodeName);

		Dgn::PhysicalPartitionCPtr partition = Dgn::PhysicalPartition::CreateAndInsert(*parentSubject, phyModelCode);

		if (!partition.IsValid())
			return nullptr;

		ElectricalPhysical::ElectricalPhysicalModelPtr physicalModel = ElectricalPhysical::ElectricalPhysicalModel::Create(*partition);

		return physicalModel;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String ElectricalDomainUtilities::GetSchemaNameFromModel(ElectricalPhysical::ElectricalPhysicalModelCPtr model)
		{
		ElectricalDomainSettings outSettings = ElectricalDomainSettings::CreateElectricalDomainSettings();

		outSettings = model->GetJsonProperties(json_ElectricalDomain());

		return outSettings.GetclasslibraryName();
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BentleyStatus ElectricalDomainUtilities::UpdateSchemaNameInModel(Utf8StringCR schemaName, ElectricalPhysical::ElectricalPhysicalModelPtr model)
		{
		ElectricalDomainSettings settings = ElectricalDomainSettings::CreateElectricalDomainSettings(schemaName.c_str());

		model->SetJsonProperties(json_ElectricalDomain(), settings);

		if (Dgn::DgnDbStatus::Success != model->Update())
			{
			return BentleyStatus::ERROR;
			}

		return BentleyStatus::SUCCESS;

		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ElectricalPhysical::ElectricalTypeDefinitionModelPtr ElectricalDomainUtilities::CreateElectricalTypeDefinitionModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
		{
		if (parentSubject.IsNull())
			parentSubject = db.Elements().GetRootSubject();

		Utf8String defModelCode = BuildTypeDefinitionModelCode(modelCodeName);

		Dgn::DefinitionPartitionCPtr defPartition = Dgn::DefinitionPartition::CreateAndInsert(*parentSubject, defModelCode);

		if (!defPartition.IsValid())
			return nullptr;

		ElectricalPhysical::ElectricalTypeDefinitionModelPtr typeDefinitionModel = ElectricalPhysical::ElectricalTypeDefinitionModel::Create(*defPartition);

		return typeDefinitionModel;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECSchemaCP ElectricalDomainUtilities::GetElectricalDynamicSchema(ElectricalPhysical::ElectricalPhysicalModelCPtr model)
		{

		ElectricalDomainSettings outSettings = ElectricalDomainSettings::CreateElectricalDomainSettings();

		outSettings = model->GetJsonProperties(json_ElectricalDomain());

		if (Utf8String::IsNullOrEmpty(outSettings.GetclasslibraryName().c_str()))
			return nullptr;

		return model->GetDgnDb().Schemas().GetSchema(outSettings.GetclasslibraryName().c_str());

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECSchemaCP ElectricalDomainUtilities::InsertSuppliedSchema(ECN::ECSchemaPtr suppliedDynamicSchema, ElectricalPhysical::ElectricalPhysicalModelPtr model)
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

	ECN::ECSchemaCP ElectricalDomainUtilities::CreateElectricalDynamicSchema(Utf8StringCR modelCodeName, ElectricalPhysical::ElectricalPhysicalModelPtr model)
		{

		Dgn::DgnDbR db = model->GetDgnDb();
		Utf8String schemaName = BuildDynamicSchemaName(modelCodeName);
		Utf8String internalName = GetSchemaNameFromModel(model);

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

	BentleyStatus ElectricalDomainUtilities::CreateElectricalModels(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject, bool createDynamicSchema, ECN::ECSchemaPtr suppliedDynamicSchema)
		{

		ElectricalPhysical::ElectricalPhysicalModelPtr physicalModel = CreateElectricalPhyicalModel(modelCodeName, db, parentSubject);

		if (!physicalModel.IsValid())
			return BentleyStatus::ERROR;

		if (createDynamicSchema && !suppliedDynamicSchema.IsValid())
			{
			if (nullptr == CreateElectricalDynamicSchema(modelCodeName, physicalModel))
				return BentleyStatus::ERROR;
			}
		else if (suppliedDynamicSchema.IsValid())
			{
			if (nullptr == InsertSuppliedSchema(suppliedDynamicSchema, physicalModel))
				return BentleyStatus::ERROR;
			}

		ElectricalPhysical::ElectricalTypeDefinitionModelPtr typeDefinitionModel = CreateElectricalTypeDefinitionModel(modelCodeName, db, parentSubject);

		if (!typeDefinitionModel.IsValid())
			return BentleyStatus::ERROR;

		return BentleyStatus::SUCCESS;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ElectricalPhysical::ElectricalPhysicalModelPtr ElectricalDomainUtilities::GetElectricalPhyicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
		{
		if (parentSubject.IsNull())
			parentSubject = db.Elements().GetRootSubject();

		Dgn::DgnCode partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, BuildPhysicalModelCode(modelCodeName));
		Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
		Dgn::PhysicalPartitionCPtr partition = db.Elements().Get<Dgn::PhysicalPartition>(partitionId);
		if (!partition.IsValid())
			return nullptr;

		return dynamic_cast<ElectricalPhysical::ElectricalPhysicalModelP>(partition->GetSubModel().get());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ElectricalPhysical::ElectricalTypeDefinitionModelPtr ElectricalDomainUtilities::GetElectricalTypeDefinitionModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
		{
		if (parentSubject.IsNull())
			parentSubject = db.Elements().GetRootSubject();

		Dgn::DgnCode partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, BuildTypeDefinitionModelCode(modelCodeName));
		Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
		Dgn::DefinitionPartitionCPtr partition = db.Elements().Get<Dgn::DefinitionPartition>(partitionId);
		if (!partition.IsValid())
			return nullptr;

		return dynamic_cast<ElectricalPhysical::ElectricalTypeDefinitionModelP>(partition->GetSubModel().get());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String  ElectricalDomainUtilities::BuildPhysicalModelCode(Utf8StringCR modelCodeName)
		{
		return modelCodeName + ":Physical";
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String  ElectricalDomainUtilities::BuildTypeDefinitionModelCode(Utf8StringCR modelCodeName)
		{
		return modelCodeName + ":TypeDefinition";
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String  ElectricalDomainUtilities::BuildDynamicSchemaName(Utf8StringCR modelCodeName)
		{
		return modelCodeName + "Dynamic";
		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECSchemaPtr  ElectricalDomainUtilities::GetUpdateableSchema(ElectricalPhysical::ElectricalPhysicalModelCPtr model)
		{

		ECN::ECSchemaCP schema = GetElectricalDynamicSchema(model);

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

	Dgn::SchemaStatus  ElectricalDomainUtilities::UpdateSchemaInDb(Dgn::DgnDbR db, ECN::ECSchemaR updatedSchema)
		{
		bvector<ECN::ECSchemaCP> schemas;

		ECN::ECSchemaCP b = &updatedSchema;

		schemas.push_back(b);

		return db.ImportSchemas(schemas);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECClassCP   ElectricalDomainUtilities::GetExistingECClass(Dgn::DgnDbPtr db, Utf8StringCR schemaName, Utf8StringCR  className)
		{
		ECN::ECSchemaCP schema = db->Schemas().GetSchema(schemaName.c_str());

		if (nullptr == schema)
			return nullptr;

		return schema->GetClassCP(className.c_str());
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::ECEntityClassP   ElectricalDomainUtilities::CreatePhysicalTypeEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className)
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

	ECN::ECEntityClassP   ElectricalDomainUtilities::CreatePhysicalElementEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className)
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

	ECN::ECEntityClassP   ElectricalDomainUtilities::CreateUniqueAspetClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className)
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

		if (ECN::ECObjectsStatus::Success != schema->CreateRelationshipClass(newRelationshipClass, relationshipName, *(physicalElementClass->GetEntityClassCP()), "Physical Element", *newClass, "Unique Aspect"))
			{
			schema->DeleteClass(*newClass);
			return nullptr;
			}

		newRelationshipClass->SetStrength(ECN::StrengthType::Embedding);
		newRelationshipClass->SetStrengthDirection(ECN::ECRelatedInstanceDirection::Forward);
		newRelationshipClass->SetClassModifier(ECN::ECClassModifier::None);
		newRelationshipClass->GetSource().SetIsPolymorphic(true);
		newRelationshipClass->GetTarget().SetIsPolymorphic(false);
		newRelationshipClass->GetSource().SetMultiplicity(ECN::RelationshipMultiplicity::OneOne());
		newRelationshipClass->GetTarget().SetMultiplicity(ECN::RelationshipMultiplicity::ZeroOne());

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

	Dgn::PhysicalElementPtr  ElectricalDomainUtilities::CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelCR model, Utf8CP categoryName)
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

		Utf8String localCategoryName = buildingClass->GetDisplayLabel();

		if (nullptr != categoryName)
			localCategoryName = categoryName;

		Dgn::DgnCategoryId categoryId = ElectricalPhysical::ElectricalPhysicalCategory::QueryElectricalPhysicalCategoryId(db, localCategoryName.c_str());

		Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

		Dgn::DgnElementPtr element = elmHandler->Create(params);

		Dgn::PhysicalElementPtr electricalElement = dynamic_pointer_cast<Dgn::PhysicalElement>(element);

		auto geomSource = electricalElement->ToGeometrySourceP();

		if (nullptr == geomSource)
			return nullptr;

		geomSource->SetCategoryId(categoryId);

		return electricalElement;

		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Dgn::PhysicalTypePtr  ElectricalDomainUtilities::CreatePhysicalTypeElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::DefinitionModelCR model)
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

		Dgn::PhysicalType::CreateParams params(db, model.GetModelId(), classId);

		Dgn::DgnCategoryId categoryId = ElectricalPhysical::ElectricalPhysicalCategory::QueryElectricalPhysicalCategoryId(db, className.c_str());

		//Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

		Dgn::DgnElementPtr element = elmHandler->Create(params);

		Dgn::PhysicalTypePtr electricalElement = dynamic_pointer_cast<Dgn::PhysicalType>(element);

		return electricalElement;

		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	ECN::IECInstancePtr   ElectricalDomainUtilities::AddAspect(Dgn::PhysicalModelCR model, Dgn::PhysicalElementPtr element, Utf8StringCR schemaName, Utf8StringCR className)
		{

		// Find the class

		ECN::ECClassCP aspectClassP = model.GetDgnDb().GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

		if (nullptr == aspectClassP)
			return nullptr;

		// If the element is already persisted and has the Aspect class, you can't add another

		if (element->GetElementId().IsValid())
			{
			ECN::IECInstanceCP instance = Dgn::DgnElement::GenericUniqueAspect::GetAspect(*element, *aspectClassP);

			if (nullptr != instance)
				return nullptr;
			}

		ECN::StandaloneECEnablerPtr enabler = aspectClassP->GetDefaultStandaloneEnabler();

		if (!enabler.IsValid())
			return nullptr;

		ECN::IECInstancePtr instance = enabler->CreateInstance().get();
		if (!instance.IsValid())
			return nullptr;

		Dgn::DgnDbStatus status = Dgn::DgnElement::GenericUniqueAspect::SetAspect(*element, *instance);

		if (Dgn::DgnDbStatus::Success != status)
			return nullptr;

		return instance;

		}

	}

END_BENTLEY_NAMESPACE



