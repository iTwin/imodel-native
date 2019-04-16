/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

        if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::AecUnits::AecUnitsDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
            return BentleyStatus::ERROR;

        if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::ArchitecturalPhysical::ArchitecturalPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
			return BentleyStatus::ERROR;

		//if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::BuildingCommon::BuildingCommonDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
		//	return BentleyStatus::ERROR;

        if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(Profiles::ProfilesDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
            return BentleyStatus::ERROR;

        if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(Forms::FormsDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
            return BentleyStatus::ERROR;

        if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::Structural::StructuralPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
            return BentleyStatus::ERROR;

		if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::BuildingPhysical::BuildingPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
			return BentleyStatus::ERROR;

		if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain( Dgn::FunctionalDomain::GetDomain() , Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
			return BentleyStatus::ERROR;

		//if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(BentleyApi::MechanicalFunctional::MechanicalFunctionalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
		//	return BentleyStatus::ERROR;

//        if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain( ConstraintModel::ConstraintModelDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
//            return BentleyStatus::ERROR;

        if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain( Grids::GridsDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
            return BentleyStatus::ERROR;

		return BentleyStatus::SUCCESS;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BuildingPhysical::BuildingPhysicalModelPtr BuildingDomainUtilities::CreateBuildingPhyicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
		{

		if (!parentSubject.IsValid())
			{
			parentSubject = db.Elements().GetRootSubject();
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

	Dgn::FunctionalModelPtr BuildingDomainUtilities::CreateBuildingFunctionalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
		{

		if (!parentSubject.IsValid())
			{
			parentSubject = db.Elements().GetRootSubject();
			}

		// Create the partition and the BuildingPhysicalModel.

		Utf8String FunctionalModelCode = BuildFunctionalModelCode(modelCodeName);

		Dgn::FunctionalPartitionCPtr partition = Dgn::FunctionalPartition::CreateAndInsert(*parentSubject, FunctionalModelCode);

		if (!partition.IsValid())
			return nullptr;

		Dgn::FunctionalModelPtr model = Dgn::FunctionalModel::Create(*partition);

		if (Dgn::DgnDbStatus::Success != model->Insert())
			return nullptr;

		return model;

		}

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    Dgn::SpatialLocationModelPtr BuildingDomainUtilities::CreateBuildingSpatialLocationModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
        {

        if (!parentSubject.IsValid())
            {
            parentSubject = db.Elements().GetRootSubject();
            }

        // Create the partition and the BuildingPhysicalModel.

        Utf8String phyModelCode = BuildSpatialLocationModelCode(modelCodeName);

        Dgn::SpatialLocationPartitionCPtr partition = Dgn::SpatialLocationPartition::CreateAndInsert(*parentSubject, phyModelCode);

        if (!partition.IsValid())
            return nullptr;

        Dgn::SpatialLocationModelPtr model = Dgn::SpatialLocationModel::Create (*partition);

        if (Dgn::DgnDbStatus::Success != model->Insert())
            return nullptr;

        return model;

        }

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Dgn::DocumentListModelPtr BuildingDomainUtilities::CreateBuildingDocumentListModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
		{

		if (!parentSubject.IsValid())
			{
			parentSubject = db.Elements().GetRootSubject();
			}

		// Create the partition and the DocumentListModel

		Utf8String documentListModelCode = BuildDocumentListModelCode(modelCodeName);

		Dgn::DocumentPartitionCPtr partition = Dgn::DocumentPartition::CreateAndInsert(*parentSubject, documentListModelCode);

		if (!partition.IsValid())
			return nullptr;

		Dgn::DocumentListModelPtr model = Dgn::DocumentListModel::Create (*partition);

		if (Dgn::DgnDbStatus::Success != model->Insert())
			return nullptr;

		return model;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Dgn::DrawingModelPtr BuildingDomainUtilities::CreateBuildingDrawingModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::DocumentListModelCR docListModel)
		{

		// Create the partition and the DocumentListModel

		Dgn::DrawingPtr drawing = Dgn::Drawing::Create(docListModel, modelCodeName);

		if (!drawing.IsValid())
			return nullptr;

		Dgn::DgnDbStatus status;

		Dgn::DgnElementCPtr element = drawing->Insert(&status);

		if (Dgn::DgnDbStatus::Success != status)
			return nullptr;

		drawing = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::Drawing>(docListModel, element->GetElementId());

		Dgn::DrawingModelPtr model = Dgn::DrawingModel::Create (*drawing);

		if (Dgn::DgnDbStatus::Success != model->Insert())
			return nullptr;

		return model;

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

	BuildingPhysical::BuildingTypeDefinitionModelPtr BuildingDomainUtilities::CreateBuildingTypeDefinitionModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
		{
        if (parentSubject.IsNull())
            parentSubject = db.Elements().GetRootSubject();

		Utf8String defModelCode = BuildTypeDefinitionModelCode(modelCodeName);

		Dgn::DefinitionPartitionCPtr defPartition = Dgn::DefinitionPartition::CreateAndInsert(*parentSubject, defModelCode);

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

	BentleyStatus BuildingDomainUtilities::CreateBuildingModels(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject, bool createDynamicSchema, ECN::ECSchemaPtr suppliedDynamicSchema)
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

		BuildingPhysical::BuildingTypeDefinitionModelPtr typeDefinitionModel = CreateBuildingTypeDefinitionModel(modelCodeName, db, parentSubject);

		if (!typeDefinitionModel.IsValid())
			return BentleyStatus::ERROR;

		return BentleyStatus::SUCCESS;

		}


	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	BuildingPhysical::BuildingPhysicalModelPtr BuildingDomainUtilities::GetBuildingPhyicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
		{
        if (parentSubject.IsNull())
            parentSubject = db.Elements().GetRootSubject();

        Dgn::DgnCode partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, BuildPhysicalModelCode(modelCodeName));
        Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
        Dgn::PhysicalPartitionCPtr partition = db.Elements().Get<Dgn::PhysicalPartition>(partitionId);
        if (!partition.IsValid())
            return nullptr;

        return dynamic_cast<BuildingPhysical::BuildingPhysicalModelP>(partition->GetSubModel().get());
		}

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    Dgn::FunctionalModelPtr BuildingDomainUtilities::GetBuildingFunctionalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
        {
        if (parentSubject.IsNull())
            parentSubject = db.Elements().GetRootSubject();

        Dgn::DgnCode partitionCode = Dgn::FunctionalPartition::CreateCode(*parentSubject, BuildFunctionalModelCode(modelCodeName));
        Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
        Dgn::FunctionalPartitionCPtr partition = db.Elements().Get<Dgn::FunctionalPartition>(partitionId);
        if (!partition.IsValid())
            return nullptr;

        return dynamic_cast<Dgn::FunctionalModelP>(partition->GetSubModel().get());
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    Dgn::DocumentListModelPtr BuildingDomainUtilities::GetBuildingDocumentListModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
        {
        if (parentSubject.IsNull())
            parentSubject = db.Elements().GetRootSubject();

        Dgn::DgnCode partitionCode = Dgn::DocumentPartition::CreateCode(*parentSubject, BuildDocumentListModelCode(modelCodeName));
        Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
        Dgn::DocumentPartitionCPtr partition = db.Elements().Get<Dgn::DocumentPartition>(partitionId);
        if (!partition.IsValid())
            return nullptr;

        return dynamic_cast<Dgn::DocumentListModelP>(partition->GetSubModel().get());
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    BuildingPhysical::BuildingTypeDefinitionModelPtr BuildingDomainUtilities::GetBuildingTypeDefinitionModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
        {
        if (parentSubject.IsNull())
            parentSubject = db.Elements().GetRootSubject();

        Dgn::DgnCode partitionCode = Dgn::DefinitionPartition::CreateCode(*parentSubject, BuildTypeDefinitionModelCode(modelCodeName));
        Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
        Dgn::DefinitionPartitionCPtr partition = db.Elements().Get<Dgn::DefinitionPartition>(partitionId);
        if (!partition.IsValid())
            return nullptr;

        return dynamic_cast<BuildingPhysical::BuildingTypeDefinitionModelP>(partition->GetSubModel().get());
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

	Utf8String  BuildingDomainUtilities::BuildFunctionalModelCode(Utf8StringCR modelCodeName)
		{
		return modelCodeName + ":Functional";
		}

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    Utf8String  BuildingDomainUtilities::BuildSpatialLocationModelCode(Utf8StringCR modelCodeName)
        {
        return modelCodeName + ":SpatialLocation";
        }

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String  BuildingDomainUtilities::BuildDocumentListModelCode(Utf8StringCR modelCodeName)
		{
		return modelCodeName + ":DocList";
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------

	Utf8String  BuildingDomainUtilities::BuildDrawingModelCode(Utf8StringCR modelCodeName)
		{
		return modelCodeName + ":Drawing";
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
        Utf8String schemaName = modelCodeName;

        if (schemaName.Contains("") || schemaName.Contains("-"))
            {
            schemaName.Sprintf("RVT%d", rand());
            }
		return schemaName + "Dynamic";
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

	ECN::ECEntityClassP   BuildingDomainUtilities::CreatePhysicalElementEntityClassFromArchPhysicalClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR  className, Utf8StringCR  archClassName)
		{
		ECN::ECClassCP baseClass = GetExistingECClass(db, BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, archClassName);

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

	Dgn::PhysicalElementPtr  BuildingDomainUtilities::CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelCR model, Utf8CP categoryName )
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
        localCategoryName.Trim();
        Dgn::DgnDbTable::ReplaceInvalidCharacters(localCategoryName, Dgn::DgnCategory::GetIllegalCharacters(), '_');
		if (nullptr != categoryName)
			localCategoryName = categoryName;

		Dgn::DgnCategoryId categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalCategoryId(db, localCategoryName.c_str());

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

    Dgn::DrawingGraphicPtr  BuildingDomainUtilities::CreateDrawingGraphic(Utf8StringCR schemaName, Utf8StringCR className, Dgn::DrawingModelCR model, Dgn::DgnCategoryId categoryId)
        {

        Dgn::DgnDbR db = model.GetDgnDb();
        Dgn::DgnModelId modelId = model.GetModelId();

        ECN::ECClassCP drawingGraphicClass = db.GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

        if (nullptr == drawingGraphicClass)
            return nullptr;

        ECN::ECClassId classId = drawingGraphicClass->GetId();

        Dgn::ElementHandlerP elmHandler = Dgn::dgn_ElementHandler::Element::FindHandler(db, classId);
        if (NULL == elmHandler)
            return nullptr;

        Dgn::GeometricElement2d::CreateParams params(db, modelId, classId, categoryId);

        Dgn::DgnElementPtr element = elmHandler->Create(params);

        Dgn::DrawingGraphicPtr drawingGraphicElement = dynamic_pointer_cast<Dgn::DrawingGraphic>(element);

        auto geomSource = drawingGraphicElement->ToGeometrySourceP();

        if (nullptr == geomSource)
            return nullptr;

        geomSource->SetCategoryId(categoryId);

        return drawingGraphicElement;

        }

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------


	Dgn::FunctionalComponentElementPtr  BuildingDomainUtilities::CreateFunctionalComponentElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::FunctionalModelCR model)
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

		Dgn::FunctionalComponentElement::CreateParams params (db, modelId, classId);

		Dgn::DgnElementPtr element = elmHandler->Create(params);

		Dgn::FunctionalComponentElementPtr buildingElement = dynamic_pointer_cast<Dgn::FunctionalComponentElement>(element);

		return buildingElement;

		}


    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------


    Dgn::FunctionalBreakdownElementPtr  BuildingDomainUtilities::CreateFunctionalBreakdownElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::FunctionalModelCR model)
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

        Dgn::FunctionalBreakdownElement::CreateParams params(db, modelId, classId);

        Dgn::DgnElementPtr element = elmHandler->Create(params);

        Dgn::FunctionalBreakdownElementPtr buildingElement = dynamic_pointer_cast<Dgn::FunctionalBreakdownElement>(element);

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

		Dgn::PhysicalType::CreateParams params(db, model.GetModelId(), classId);

//		Dgn::DgnCategoryId categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalCategoryId(db, className.c_str());

//	Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

		Dgn::DgnElementPtr element = elmHandler->Create(params);

		Dgn::PhysicalTypePtr buildingElement = dynamic_pointer_cast<Dgn::PhysicalType>(element);

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

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    Units::UnitCP    BuildingDomainUtilities::GetUnitCPFromProperty(Dgn::DgnElementCR element, Utf8StringCR propertyName)
        {
        ECN::ECClassCP elementClass = element.GetElementClass();

        if (nullptr == elementClass)
            return nullptr;

        ECN::ECPropertyP prop = elementClass->GetPropertyP(propertyName);

        if (nullptr == prop)
            return nullptr;

        ECN::KindOfQuantityCP propUnit = prop->GetKindOfQuantity();

        if (nullptr == propUnit)
            return nullptr;

        Units::UnitCP u = propUnit->GetPersistenceUnit();

        return u;
        }


    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    BentleyStatus   BuildingDomainUtilities::SetDoublePropertyFromStringWithUnits(Dgn::DgnElementR element, Utf8StringCR propertyName, Utf8StringCR propertyValueString)
        {

        Units::UnitCP u = GetUnitCPFromProperty(element, propertyName);

        if (nullptr == u)
            return BentleyStatus::ERROR;

        Utf8String numbers = "-1234567890.+";

        int pos = propertyValueString.find_first_not_of(numbers, 0);

        Utf8String valueString = propertyValueString.substr(0, pos);
        Utf8String unitSuffix  = propertyValueString.substr(pos, propertyValueString.length());

        unitSuffix = unitSuffix.Trim();

        Units::UnitCP u1 = element.GetDgnDb().Schemas().GetUnit("Units", unitSuffix.c_str());

        double value = atof(valueString.c_str());

        double converted;
        Units::UnitsProblemCode code = u1->Convert(converted, value, u);

        if ( Units::UnitsProblemCode::NoProblem != code)
            return BentleyStatus::ERROR;
        
        ECN::ECValue doubleValue;

        doubleValue.SetDouble(converted);

        Dgn::DgnDbStatus status = element.SetPropertyValue(propertyName.c_str(), doubleValue);

        if( Dgn::DgnDbStatus::Success != status)
            return BentleyStatus::ERROR;

        return BentleyStatus::SUCCESS;

        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    BentleyStatus   BuildingDomainUtilities::GetDoublePropertyUsingUnitString(Dgn::DgnElementCR element, Utf8StringCR propertyName, Utf8StringCR unitString, double& value)
        {
        Units::UnitCP u = GetUnitCPFromProperty(element, propertyName);

        if (nullptr == u)
            return BentleyStatus::ERROR;

        Units::UnitCP u1 = element.GetDgnDb().Schemas().GetUnit("Units", unitString.c_str());

        ECN::ECValue propVal;

        if(Dgn::DgnDbStatus::Success != element.GetPropertyValue(propVal, propertyName.c_str()))
            return BentleyStatus::ERROR;

        double storedValue = propVal.GetDouble();

        Units::UnitsProblemCode code = u->Convert(value, storedValue, u1);

        if (Units::UnitsProblemCode::NoProblem != code)
            return BentleyStatus::ERROR;

        return BentleyStatus::SUCCESS;

        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    BentleyStatus   BuildingDomainUtilities::SetDoublePropertyUsingUnitString(Dgn::DgnElementR element, Utf8StringCR propertyName, Utf8StringCR unitString, double value)
        {
        Units::UnitCP u = GetUnitCPFromProperty(element, propertyName);

        if (nullptr == u)
            return BentleyStatus::ERROR;

        Units::UnitCP u1 = element.GetDgnDb().Schemas().GetUnit("Units", unitString.c_str());

        double converted;
        Units::UnitsProblemCode code = u1->Convert(converted, value, u);

        if (Units::UnitsProblemCode::NoProblem != code)
            return BentleyStatus::ERROR;

        ECN::ECValue doubleValue;

        doubleValue.SetDouble(converted);

        Dgn::DgnDbStatus status = element.SetPropertyValue(propertyName.c_str(), doubleValue);

        if (Dgn::DgnDbStatus::Success != status)
            return BentleyStatus::ERROR;

        return BentleyStatus::SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    Dgn::DgnDbStatus BuildingDomainUtilities::FindOrCreateSubCategory(Dgn::DgnDbPtr dgnDb, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId &subCategoryId, Utf8CP subCategoryName)
        {

        Dgn::DefinitionModelR dictionary = dgnDb->GetDictionaryModel();
        Utf8String subName(subCategoryName);

        Dgn::DgnCode subCategoryCode = Dgn::DgnSubCategory::CreateCode(*dgnDb, categoryId, subName);

        subCategoryId = Dgn::DgnSubCategory::QuerySubCategoryId(*dgnDb, subCategoryCode);


        if (!subCategoryId.IsValid())
            {
            Dgn::DgnSubCategory::CreateParams subParams(*dgnDb, categoryId, subName, Dgn::DgnSubCategory::Appearance());
            Dgn::DgnSubCategory subCategory(subParams);

            Dgn::DgnDbStatus status;
            Dgn::DgnElementCPtr element = subCategory.Insert(&status);
            if (status != Dgn::DgnDbStatus::Success)
                return status;

            dgnDb->SaveChanges();
            subCategoryId = subCategory.GetSubCategoryId();
            }
        return Dgn::DgnDbStatus::Success;
        }


    }

END_BENTLEY_NAMESPACE



