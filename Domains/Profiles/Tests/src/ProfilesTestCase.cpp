/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ProfilesTestCase.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesTestCase.h"
#include "TestHost.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName copyWorkingDb()
    {
    BeFileName baseDbPath = ProfilesTestHost::Instance().GetBaseDbPath();
    WString directory = baseDbPath.GetDirectoryName();
    WString name = baseDbPath.GetFileNameWithoutExtension();
    WString extension = baseDbPath.GetExtension();

    BeFileName workingDbPath (directory + name + L"_Working." + extension);

    BeFileNameStatus fileCopyStatus = BeFileName::BeCopyFile (baseDbPath, workingDbPath);
    BeAssert (fileCopyStatus == BeFileNameStatus::Success);

    return workingDbPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename ModelType, typename PartitionType>
static RefCountedPtr<ModelType> createDgnModel (DgnDb& db, Utf8CP pPartitionName)
    {
    SubjectCPtr rootSubjectPtr = db.Elements().GetRootSubject();

    RefCountedPtr<PartitionType> partitionPtr = PartitionType::Create (*rootSubjectPtr, pPartitionName);
    db.BriefcaseManager().AcquireForElementInsert (*partitionPtr);

    DgnDbStatus status;
    partitionPtr->Insert (&status);
    if (status != DgnDbStatus::Success)
        return nullptr;

    RefCountedPtr<ModelType> modelPtr = ModelType::Create (*partitionPtr);
    status = modelPtr->Insert();
    if (status != DgnDbStatus::Success)
        return nullptr;

    return modelPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesTestCase::ProfilesTestCase()
    : m_dbPtr (nullptr)
    , m_definitionModelPtr (nullptr)
    , m_physicalModelPtr (nullptr)
    {
    BeFileName workingDbPath = copyWorkingDb();

    DgnDb::OpenParams openParams (BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);
    BeSQLite::DbResult openStatus;
    m_dbPtr = DgnDb::OpenDgnDb (&openStatus, workingDbPath, openParams);
    BeAssert (m_dbPtr.IsValid());

    m_definitionModelPtr = createDgnModel<DefinitionModel, DefinitionPartition> (GetDb(), "ProfilesTestPartition_Definition");
    BeAssert (m_definitionModelPtr.IsValid());

    m_physicalModelPtr = createDgnModel<PhysicalModel, PhysicalPartition> (GetDb(), "ProfilesTestPartition_Physical");
    BeAssert (m_physicalModelPtr.IsValid());

    SpatialCategory category (m_dbPtr->GetDictionaryModel(), "ProfilesTestCategory", DgnCategory::Rank::Application);

    DgnDbStatus status;
    SpatialCategoryCPtr categoryPtr = category.Insert (DgnSubCategory::Appearance(), &status);
    BeAssert (status == DgnDbStatus::Success);
    m_categoryId = categoryPtr->GetCategoryId();

    CodeSpecPtr codeSpec = CodeSpec::Create(*m_dbPtr, "ProfilesMaterialCodeSpec", CodeScopeSpec::CreateRepositoryScope());
    BeAssert (codeSpec.IsValid());

    status = codeSpec->Insert();
    BeAssert (status == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesTestCase::~ProfilesTestCase()
    {
    m_dbPtr->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb& ProfilesTestCase::GetDb()
    {
    BeAssert (m_dbPtr.IsValid());
    return *m_dbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel& ProfilesTestCase::GetModel()
    {
    BeAssert (m_definitionModelPtr.IsValid());
    return *m_definitionModelPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModel& ProfilesTestCase::GetPhysicalModel()
    {
    BeAssert (m_physicalModelPtr.IsValid());
    return *m_physicalModelPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId ProfilesTestCase::GetCategoryId()
    {
    BeAssert (m_categoryId.IsValid());
    return m_categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ProfilesTestCase::CreateAndGetMaterialId(Utf8CP codeValue)
    {
    DgnClassId classId(m_dbPtr->Schemas().GetClassId(PRF_SCHEMA_NAME, "TempMaterial"));
    dgn_ElementHandler::Element* handler = Dgn::dgn_ElementHandler::Element::FindHandler(*m_dbPtr, classId);
    DgnElementPtr element = handler->Create(DgnElement::CreateParams(*m_dbPtr, m_definitionModelPtr->GetModelId(), classId));
    element->SetCode(CodeSpec::CreateCode(*m_dbPtr, "ProfilesMaterialCodeSpec", codeValue));
    Dgn::DgnDbStatus status;
    element->Insert(&status);
    if (status != Dgn::DgnDbStatus::Success)
        {
        BeAssert(false && "Failed to create Profile");
        return DgnElementId();
        }

    return element->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ProfilesTestCase::CreateAndGetProfileId()
    {
    CShapeProfile::CreateParams params(GetModel(), "C", 10, 10, 1, 1);
    CShapeProfilePtr profilePtr = CShapeProfile::Create(params);
    Dgn::DgnDbStatus status;
    profilePtr->Insert(&status);
    if (status != Dgn::DgnDbStatus::Success)
        {
        BeAssert(false && "Failed to create Profile");
        return DgnElementId();
        }

    return profilePtr->GetElementId();
    }
