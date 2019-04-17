/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\StructuralDomainUtilities.h"

BE_JSON_NAME(StructuralDomain)

USING_NAMESPACE_BENTLEY_STRUCTURAL
BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct DomainHandlerRegisterHelper //use critical sections?
    {
    protected:
        DomainHandlerRegisterHelper();
    public:
        ~DomainHandlerRegisterHelper() {};
        static DomainHandlerRegisterHelper& Get(); //singleton
        BentleyStatus GetRegistrationStatus() const { return m_registrationStatus; };
    private:
        bool RegisterDomain(Dgn::DgnDomain& domain);
    private:
        BentleyStatus m_registrationStatus;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DomainHandlerRegisterHelper& DomainHandlerRegisterHelper::Get()
    {
    static DomainHandlerRegisterHelper oObject;

    return oObject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool DomainHandlerRegisterHelper::RegisterDomain(Dgn::DgnDomain& domain)
    {
    m_registrationStatus = Dgn::DgnDomains::RegisterDomain(domain, Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
    BeAssert(BentleyStatus::SUCCESS == m_registrationStatus);
    
    return BentleyStatus::SUCCESS == m_registrationStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DomainHandlerRegisterHelper::DomainHandlerRegisterHelper() : m_registrationStatus(BentleyStatus::SUCCESS)
    {
    bool bStart(true);

    bStart = bStart && RegisterDomain(BentleyApi::Structural::StructuralPhysicalDomain::GetDomain());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus StructuralDomainUtilities::RegisterDomainHandlers()
    {
    return DomainHandlerRegisterHelper::Get().GetRegistrationStatus();
    }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 Utf8String StructuralDomainUtilities::BuildDynamicSchemaName(Utf8StringCR modelCodeName)
     {
     return modelCodeName + "Dynamic";
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 Utf8String StructuralDomainUtilities::BuildPhysicalModelCode(Utf8StringCR modelCodeName)
     {
     return modelCodeName + ":Physical";
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 Utf8String StructuralDomainUtilities::BuildTypeDefinitionModelCode(Utf8StringCR modelCodeName)
     {
     return modelCodeName + ":TypeDefinition";
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 ECN::ECSchemaCP StructuralDomainUtilities::GetStructuralDynamicSchema(StructuralPhysicalModelCPtr model)
     {
     StructuralDomainSettings outSettings = StructuralDomainSettings::CreateStructuralDomainSettings();

     outSettings = model->GetJsonProperties(json_StructuralDomain());

     if (Utf8String::IsNullOrEmpty(outSettings.GetclasslibraryName().c_str()))
         return nullptr;

     return model->GetDgnDb().Schemas().GetSchema(outSettings.GetclasslibraryName().c_str());
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 ECN::ECSchemaPtr StructuralDomainUtilities::GetUpdateableSchema(StructuralPhysicalModelCPtr model)
     {
     ECN::ECSchemaCP schema = GetStructuralDynamicSchema(model);

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
 Utf8String StructuralDomainUtilities::GetSchemaNameFromModel(StructuralPhysicalModelCPtr model)
     {
     StructuralDomainSettings outSettings = StructuralDomainSettings::CreateStructuralDomainSettings();

     outSettings = model->GetJsonProperties(json_StructuralDomain());

     return outSettings.GetclasslibraryName();
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 StructuralPhysicalModelPtr StructuralDomainUtilities::GetStructuralPhysicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
     {
     if (parentSubject.IsNull())
         {
         parentSubject = db.Elements().GetRootSubject();
         }

     Dgn::DgnCode partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, BuildPhysicalModelCode(modelCodeName));
     Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
     Dgn::PhysicalPartitionCPtr partition = db.Elements().Get<Dgn::PhysicalPartition>(partitionId);
     if (!partition.IsValid())
         return nullptr;

     return dynamic_cast<StructuralPhysicalModelP>(partition->GetSubModel().get());
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 ECN::ECClassCP StructuralDomainUtilities::GetExistingECClass(Dgn::DgnDbPtr db, Utf8StringCR schemaName, Utf8StringCR className)
     {
     ECN::ECSchemaCP schema = db->Schemas().GetSchema(schemaName.c_str());

     if (nullptr == schema)
         return nullptr;

     return schema->GetClassCP(className.c_str());
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 ECN::ECSchemaCP StructuralDomainUtilities::CreateStructuralDynamicSchema(Utf8StringCR modelCodeName, StructuralPhysicalModelPtr model)
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
 BentleyStatus StructuralDomainUtilities::CreateStructuralModels(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject, bool createDynamicSchema, ECN::ECSchemaPtr suppliedDynamicSchema)
     {
     StructuralPhysicalModelPtr physicalModel = CreateStructuralPhysicalModel(modelCodeName, db, parentSubject);

     if (!physicalModel.IsValid())
         return BentleyStatus::ERROR;

     if (createDynamicSchema && !suppliedDynamicSchema.IsValid())
         {
         if (nullptr == CreateStructuralDynamicSchema(modelCodeName, physicalModel))
             return BentleyStatus::ERROR;
         }
     else if (suppliedDynamicSchema.IsValid())
         {
         if (nullptr == InsertSuppliedSchema(suppliedDynamicSchema, physicalModel))
             return BentleyStatus::ERROR;
         }

     return BentleyStatus::SUCCESS;
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 StructuralPhysicalModelPtr StructuralDomainUtilities::CreateStructuralPhysicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
     {
     if (!parentSubject.IsValid())
         {
         parentSubject = db.Elements().GetRootSubject();
         }

     // Create the partition and the StructuralPhysicalModel.
     Utf8String phyModelCode = BuildPhysicalModelCode(modelCodeName);

     Dgn::PhysicalPartitionCPtr partition = Dgn::PhysicalPartition::CreateAndInsert(*parentSubject, phyModelCode);

     if (!partition.IsValid())
         return nullptr;

     StructuralPhysicalModelPtr physicalModel = StructuralPhysicalModel::Create(*partition);

     return physicalModel;
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 Dgn::PhysicalElementPtr StructuralDomainUtilities::CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::PhysicalModelCR model, Utf8CP categoryName)
     {
     Dgn::DgnDbR db = model.GetDgnDb();
     Dgn::DgnModelId modelId = model.GetModelId();

     // Find the class

     ECN::ECClassCP structuralClass = db.GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

     if (nullptr == structuralClass)
         return nullptr;

     ECN::ECClassId classId = structuralClass->GetId();

     Dgn::ElementHandlerP elmHandler = Dgn::dgn_ElementHandler::Element::FindHandler(db, classId);
     if (NULL == elmHandler)
         return nullptr;

     Utf8String localCategoryName = structuralClass->GetDisplayLabel();

     if (nullptr != categoryName)
         localCategoryName = categoryName;

     Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(db, localCategoryName.c_str());

     Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

     Dgn::DgnElementPtr element = elmHandler->Create(params);

     Dgn::PhysicalElementPtr structuralElement = dynamic_pointer_cast<Dgn::PhysicalElement>(element);

     auto geomSource = structuralElement->ToGeometrySourceP();

     if (nullptr == geomSource)
         return nullptr;

     geomSource->SetCategoryId(categoryId);

     return structuralElement;
     }

 Dgn::DefinitionElementPtr StructuralDomainUtilities::CreateDefinitionElement(Utf8StringCR schemaName, Utf8StringCR className, Dgn::DefinitionModelCR model, Utf8CP categoryName /*= nullptr*/)
    {
     Dgn::DgnDbR db = model.GetDgnDb();
     Dgn::DgnModelId modelId = model.GetModelId();

     // Find the class
     ECN::ECClassCP structuralClass = db.GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

     if (nullptr == structuralClass)
        {
         return nullptr;
        }

     ECN::ECClassId classId = structuralClass->GetId();

     Dgn::ElementHandlerP elmHandler = Dgn::dgn_ElementHandler::Definition::FindHandler(db, classId);
     
     if (NULL == elmHandler)
        {
         return nullptr;
        }

     Dgn::DefinitionElement::CreateParams params(db, modelId, classId);

     Dgn::DgnElementPtr element = elmHandler->Create(params);

     Dgn::DefinitionElementPtr structuralElement = dynamic_pointer_cast<Dgn::DefinitionElement>(element);

     return structuralElement;
    }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 ECN::ECEntityClassP StructuralDomainUtilities::CreateDefinitionElementEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR className)
    {
     ECN::ECClassCP baseClass = GetExistingECClass(db, BIS_ECSCHEMA_NAME, BIS_CLASS_DefinitionElement);

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
 ECN::ECEntityClassP StructuralDomainUtilities::CreatePhysicalElementEntityClass(Dgn::DgnDbPtr db, ECN::ECSchemaPtr schema, Utf8StringCR className)
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
 ECN::ECSchemaCP StructuralDomainUtilities::InsertSuppliedSchema(ECN::ECSchemaPtr suppliedDynamicSchema, StructuralPhysicalModelPtr model)
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
 Dgn::SchemaStatus StructuralDomainUtilities::UpdateSchemaInDb(Dgn::DgnDbR db, ECN::ECSchemaR updatedSchema)
     {
     bvector<ECN::ECSchemaCP> schemas;

     ECN::ECSchemaCP b = &updatedSchema;

     schemas.push_back(b);

     return db.ImportSchemas(schemas);
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                   Bentley.Systems
 //---------------------------------------------------------------------------------------
 BentleyStatus StructuralDomainUtilities::UpdateSchemaNameInModel(Utf8StringCR schemaName, StructuralPhysicalModelPtr model)
     {
     StructuralDomainSettings settings = StructuralDomainSettings::CreateStructuralDomainSettings(schemaName.c_str());

     model->SetJsonProperties(json_StructuralDomain(), settings);

     if (Dgn::DgnDbStatus::Success != model->Update())
         {
         return BentleyStatus::ERROR;
         }

     return BentleyStatus::SUCCESS;
     }

 BeSQLite::DbResult StructuralDomainUtilities::InsertLinkTableRelationship(BeSQLite::EC::ECInstanceKey& relKey, Dgn::DgnDbR db, Utf8StringCR schemaName, Utf8StringCR relationshipClassName, BeSQLite::EC::ECInstanceKey source, BeSQLite::EC::ECInstanceKey target)
    {
    BeSQLite::DbResult returnStatus(BeSQLite::DbResult::BE_SQLITE_ERROR); //pesimistic point of view

    ECN::ECClassCP relClass = db.GetClassLocater().LocateClass(schemaName.c_str(), relationshipClassName.c_str());

    ECN::StandaloneECRelationshipInstancePtr relInstance = ECN::StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClass->GetRelationshipClassCP())->CreateRelationshipInstance();
    BeAssert(relInstance.IsValid());

    BeSQLite::EC::ECInstanceId sourceId = source.GetInstanceId();
    BeAssert(sourceId.IsValid());
    BeSQLite::EC::ECInstanceId targetId = target.GetInstanceId();
    BeAssert(targetId.IsValid());
     
    returnStatus = db.InsertLinkTableRelationship(relKey, *relClass->GetRelationshipClassCP(), sourceId, targetId, relInstance.get());
    BeAssert(BeSQLite::DbResult::BE_SQLITE_OK == returnStatus);

    return returnStatus;
    }
 
END_BENTLEY_STRUCTURAL_NAMESPACE

