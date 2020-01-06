/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "C3dInternal.h"

BEGIN_C3D_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/19
+===============+===============+===============+===============+===============+======*/
struct C3dSchemaFactory
{
private:
    DgnDbR  m_dgndb;
    C3dImporterR m_importer;
    ECSchemaPtr m_c3dSchema;
    BeFileName  m_c3dSchemaPath;
    // Civil domain schemas
    ECSchemaPtr m_linearReferenceSchema;
    ECSchemaPtr m_roadRailAlignmentSchema;
    ECSchemaPtr m_roadRailPhysicalSchema;
    ECSchemaPtr m_roadPhysicalSchema;
    ECSchemaPtr m_railPhysicalSchema;
    bvector<ECSchemaCP> m_schemasToImport;

public:
    C3dSchemaFactory (C3dImporterR importer) : m_importer(importer), m_dgndb(importer.GetDgnDb()) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus UpdateC3dSchema (ECSchemaReadContextR schemaContext)
    {
    // WIP - upgrade C3D schema??
    if (m_c3dSchema->GetVersionWrite() >= C3DSCHEMA_VERSION_Write)
        m_c3dSchema = nullptr;

    if (m_c3dSchema.IsValid())
        m_schemasToImport.push_back (m_c3dSchema.get());

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus CreateC3dSchema (SchemaKeyCR key, ECSchemaReadContextR schemaContext)
    {
    auto status = ECSchema::ReadFromXmlFile(m_c3dSchema, m_c3dSchemaPath, schemaContext);
    if (status != SchemaReadStatus::Success)
        return  status;

    auto checksum = m_c3dSchema->ComputeCheckSum ();

    m_schemasToImport.push_back (m_c3dSchema.get());

    return  SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus    AddCivilDomainSchemas (ECSchemaReadContextR schemaContext)
    {
    BeFileName  m_assetsDir(m_importer.GetOptions().GetAssetsDir());
    WString     schemaPath = m_assetsDir;
    BeFileName::AppendToPath (schemaPath, LinearReferencingDomain::GetSchemaRelativePath());

    auto status = ECSchema::ReadFromXmlFile (m_linearReferenceSchema, schemaPath.c_str(), schemaContext);
    if (status != SchemaReadStatus::Success)
        return  status;

    m_schemasToImport.push_back (m_linearReferenceSchema.get());

    schemaPath = m_assetsDir;
    BeFileName::AppendToPath (schemaPath, RoadRailAlignment::RoadRailAlignmentDomain::GetSchemaRelativePath());

    status = ECSchema::ReadFromXmlFile (m_roadRailAlignmentSchema, schemaPath.c_str(), schemaContext);
    if (status != SchemaReadStatus::Success)
        return  status;
    
    m_schemasToImport.push_back (m_roadRailAlignmentSchema.get());

    schemaPath = m_assetsDir;
    BeFileName::AppendToPath (schemaPath, RoadRailPhysical::RoadRailPhysicalDomain::GetSchemaRelativePath());

    status = ECSchema::ReadFromXmlFile (m_roadRailPhysicalSchema, schemaPath.c_str(), schemaContext);
    if (status != SchemaReadStatus::Success)
        return  status;
    
    m_schemasToImport.push_back (m_roadRailPhysicalSchema.get());

    schemaPath = m_assetsDir;
    BeFileName::AppendToPath (schemaPath, RoadPhysical::RoadPhysicalDomain::GetSchemaRelativePath());

    status = ECSchema::ReadFromXmlFile (m_roadPhysicalSchema, schemaPath.c_str(), schemaContext);
    if (status != SchemaReadStatus::Success)
        return  status;
    
    m_schemasToImport.push_back (m_roadPhysicalSchema.get());

    schemaPath = m_assetsDir;
    BeFileName::AppendToPath (schemaPath, RailPhysical::RailPhysicalDomain::GetSchemaRelativePath());

    status = ECSchema::ReadFromXmlFile (m_railPhysicalSchema, schemaPath.c_str(), schemaContext);
    if (status != SchemaReadStatus::Success)
        return  status;
    
    m_schemasToImport.push_back (m_railPhysicalSchema.get());

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus    CreateOrUpdateSchemas (ECSchemaReadContextR schemaContext)
    {
    BeFileName baseDir(m_importer.GetOptions().GetAssetsDir());
    baseDir.AppendToPath (L"ECSchemas");

    BeFileName searchDir(baseDir);
    searchDir.AppendToPath (L"Dgn");
    schemaContext.AddSchemaPath (searchDir.c_str());

    searchDir.SetName (baseDir);
    searchDir.AppendToPath (L"ECDb");
    schemaContext.AddSchemaPath (searchDir.c_str());

    searchDir.SetName (baseDir);
    searchDir.AppendToPath (L"Standard");
    schemaContext.AddSchemaPath (searchDir.c_str());

    searchDir.SetName (baseDir);
    searchDir.AppendToPath (L"Domain");
    schemaContext.AddSchemaPath (searchDir.c_str());

    searchDir.SetName (baseDir);
    searchDir.AppendToPath (L"Application");
    schemaContext.AddSchemaPath (searchDir.c_str());

    auto status = this->AddCivilDomainSchemas (schemaContext);
    if (status != SchemaReadStatus::Success)
        return  status;

    m_c3dSchemaPath = searchDir.AppendToPath (C3DSCHEMA_FileName);

    SchemaKey key(C3DSCHEMA_SchemaName, C3DSCHEMA_VERSION_Major, C3DSCHEMA_VERSION_Write, C3DSCHEMA_VERSION_Minor);

    m_c3dSchema = m_importer.GetDgnDb().GetSchemaLocater().LocateSchema (key, SchemaMatchType::Latest, schemaContext);

    if (m_c3dSchema.IsValid())
        status = this->UpdateC3dSchema (schemaContext);
    else
        status = this->CreateC3dSchema (key, schemaContext);
        
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MakeSchemaChanges ()
    {
    if (m_dgndb.BriefcaseManager().LockSchemas().Result() != RepositoryStatus::Success)
        return  static_cast<BentleyStatus>(DgnDbStatus::LockNotHeld);
    
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext (false, true);
    if (schemaContext.IsNull())
        return  static_cast<BentleyStatus>(ECObjectsStatus::SchemaNotFound);

    auto createStatus = this->CreateOrUpdateSchemas (*schemaContext);
    if (createStatus != SchemaReadStatus::Success)
        return  static_cast<BentleyStatus>(createStatus);

    if (m_schemasToImport.empty())
        return  BentleyStatus::BSIERROR;

    schemaContext->RemoveSchemaLocater (m_dgndb.GetSchemaLocater());

    auto importStatus = m_dgndb.ImportSchemas (m_schemasToImport);
    if (importStatus != SchemaStatus::Success)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::SchemaImportError(), Utf8PrintfString("C3D schema import status=%d", (int)importStatus).c_str());
        return  static_cast<BentleyStatus>(importStatus);
        }

    return  BentleyStatus::BSISUCCESS;
    }

};  // C3dSchemaFactory

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP   C3dImporter::GetC3dECClass (Utf8StringCR className) const
    {
    ECClassCP   ecClass = nullptr;
    auto c3dSchema = this->GetC3dSchema ();
    if (c3dSchema != nullptr)
        ecClass = c3dSchema->GetClassCP (DwgHelper::ValidateECNameFrom(className).c_str());
    return  ecClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECInstancePtr C3dImporter::CreateC3dECInstance (Utf8StringCR className) const
    {
    auto ecClass = this->GetC3dECClass (className.c_str());
    if (ecClass != nullptr)
        {
        auto ecEnabler = ecClass->GetDefaultStandaloneEnabler ();
        if (ecEnabler.IsValid())
            return  ecEnabler->CreateInstance ();
        }
    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus C3dImporter::InsertArrayProperty (DgnElementR element, Utf8StringCR propertyName, uint32_t arraySize) const
    {
    DgnDbStatus status = DgnDbStatus::BadRequest;
    if (arraySize > 0 && !propertyName.empty())
        {
        uint32_t    propertyIndex = 0;

        status = element.GetPropertyIndex (propertyIndex, propertyName.c_str());
        if (status == DgnDbStatus::Success)
            {
            element.InsertPropertyArrayItems (propertyIndex, 0, arraySize);

            ECValue ecValue;
            status = element.GetPropertyValue (ecValue, propertyName.c_str());

            if (status == DgnDbStatus::Success && (!ecValue.IsArray() || ecValue.GetArrayInfo().GetCount() != arraySize))
                status = DgnDbStatus::WrongClass;
            }
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   C3dImporter::_MakeSchemaChanges ()
    {
    auto status = T_Super::_MakeSchemaChanges();
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    iModelBridge::PushChanges (T_Super::GetDgnDb(), T_Super::GetOptions(), "DWG schemas");

    this->SetStepName (ProgressMessage::TASK_IMPORTING(), "Civil domain & C3D schemas");

    StopWatch totalTimer (true);
    StopWatch timer (true);
    timer.Start ();

    C3dSchemaFactory factory(*this);
    status = factory.MakeSchemaChanges ();

    if (status == BentleyStatus::BSISUCCESS)
        m_c3dSchema = m_dgndb->Schemas().GetSchema (C3DSCHEMA_SchemaName, true);

    iModelBridge::LogPerformance (timer, "Importing Civil domain and C3D schemas");

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   C3dImporter::_ImportEntitySection ()
    {
    // get the Alignment model created or found during initialization stage
    auto& db = T_Super::GetDgnDb ();
    auto const& jobSubject = T_Super::GetJobSubject ();
    auto alignedCode = Subject::CreateCode (jobSubject, CIVIL_ALIGNED_SUBJECT);
    auto alignedId = db.Elements().QueryElementIdByCode (alignedCode);
    if (alignedId.IsValid())
        {
        auto alignSubject = db.Elements().Get<Subject>(alignedId);
        BeAssert (alignSubject.IsValid() && "The Aligned subject is not available!");
        m_roadNetworkModel = RoadRailPhysical::PhysicalModelUtilities::QueryRoadNetworkModel(*alignSubject, ALIGNMENTS_PARTITION_NAME, ROADNETWORK_MODEL_NAME);
        m_railNetworkModel = RoadRailPhysical::PhysicalModelUtilities::QueryRailNetworkModel(*alignSubject, ALIGNMENTS_PARTITION_NAME, RAILNETWORK_MODEL_NAME);

        auto alignments = DesignAlignments::Query (*m_roadNetworkModel, DESIGNALIGNMENTS_NAME);
        if (alignments.IsValid())
            m_alignmentModel = alignments->GetAlignmentModel ();
        }
    
    // get the C3D schema imported through _MakeSchemaChnages
    m_c3dSchema = db.Schemas().GetSchema (C3DSCHEMA_SchemaName, true);
    if (m_c3dSchema == nullptr)
        T_Super::ReportError (IssueCategory::Unknown(), Issue::Error(), "Missing C3D schema");

    // begin importing entities
    return T_Super::_ImportEntitySection ();
    }

END_C3D_NAMESPACE
