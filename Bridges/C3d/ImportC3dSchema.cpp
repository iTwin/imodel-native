/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "C3dImporter.h"
#include <ECPresentation/RulesDriven/RuleSetEmbedder.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

#define C3DSCHEMA_SchemaName    "AdskCivil3dSchema"
#define C3DSCHEMA_VERSION_Major 2
#define C3DSCHEMA_VERSION_Write 0
#define C3DSCHEMA_VERSION_Minor 0
#define C3DSCHEMA_FileName      L"C3dSchema.02.00.00.ecschema.xml"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG
USING_NAMESPACE_C3D

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/19
+===============+===============+===============+===============+===============+======*/
struct C3dSchemaFactory
{
private:
    DgnDbR  m_dgndb;
    ECSchemaPtr& m_targetSchema;
    BeFileName  m_schemaPath;
    C3dImporterR m_importer;

public:
    C3dSchemaFactory (ECSchemaPtr& schema, C3dImporterR importer) : m_targetSchema(schema), m_importer(importer), m_dgndb(importer.GetDgnDb()) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus UpdateSchema (ECSchemaReadContextR schemaContext)
    {
    // WIP - upgrade C3D schema??
    if (m_targetSchema->GetVersionWrite() >= C3DSCHEMA_VERSION_Write)
        m_targetSchema = nullptr;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus CreateSchema (SchemaKeyCR key, ECSchemaReadContextR schemaContext)
    {
    auto status = ECSchema::ReadFromXmlFile(m_targetSchema, m_schemaPath, schemaContext);
    if (status != SchemaReadStatus::Success)
        return  ECObjectsStatus::SchemaNotFound;

    auto checksum = m_targetSchema->ComputeCheckSum ();

    return  ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus CreateOrUpdateSchema (ECSchemaReadContextR schemaContext)
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

    m_schemaPath = searchDir.AppendToPath (C3DSCHEMA_FileName);

    SchemaKey key(C3DSCHEMA_SchemaName, C3DSCHEMA_VERSION_Major, C3DSCHEMA_VERSION_Write, C3DSCHEMA_VERSION_Minor);

    m_targetSchema = m_importer.GetDgnDb().GetSchemaLocater().LocateSchema (key, SchemaMatchType::Latest, schemaContext);

    return m_targetSchema.IsValid() ? this->UpdateSchema(schemaContext) : this->CreateSchema(key, schemaContext);
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

    auto createStatus = this->CreateOrUpdateSchema (*schemaContext);
    if (createStatus != ECObjectsStatus::Success)
        return  static_cast<BentleyStatus>(createStatus);

    if (m_targetSchema.IsNull())
        return  BentleyStatus::BSISUCCESS;

    // import schemas
    bvector<ECSchemaCP> schemas(1, m_targetSchema.get());

    schemaContext->RemoveSchemaLocater (m_dgndb.GetSchemaLocater());

    auto importStatus = m_dgndb.ImportSchemas (schemas);
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
BentleyStatus   C3dImporter::_MakeSchemaChanges ()
    {
    auto status = T_Super::_MakeSchemaChanges();
    if (status != BentleyStatus::BSISUCCESS)
        return  status;

    ECSchemaPtr c3dSchema;
    this->SetTaskName (ProgressMessage::TASK_IMPORTING(), "C3D schemas");

    C3dSchemaFactory factory(c3dSchema, *this);
    status = factory.MakeSchemaChanges ();

    if (status == BentleyStatus::BSISUCCESS)
        m_c3dSchema = m_dgndb->Schemas().GetSchema (C3DSCHEMA_SchemaName, true);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   C3dImporter::_ImportEntitySection ()
    {
    // get the Alighment model created or found during initialization stage
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
