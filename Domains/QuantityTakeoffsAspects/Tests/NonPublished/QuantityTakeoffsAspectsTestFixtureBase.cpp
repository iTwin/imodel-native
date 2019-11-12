/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QuantityTakeoffsAspectsTestFixtureBase.h"
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsDomain.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <DgnPlatform/DgnPlatformAPI.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

#define PROJECT_SEED_NAME       L"QuantityTakeoffsAspectsTests/QuantityTakeoffsAspectsTestSeed.bim"
#define PROJECT_DEFAULT_NAME    L"QuantityTakeoffsAspectsTests/Default.bim"
#define PROJECT_TEMP_FOLDER     L"QuantityTakeoffsAspectsTests"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void StaticRegisterDomains()
    {
    DgnDomains::RegisterDomain(Dgn::FunctionalDomain::GetDomain(), DgnDomain::Required::Yes);
    DgnDomains::RegisterDomain(QuantityTakeoffsAspectsDomain::GetDomain(), DgnDomain::Required::Yes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void QuantityTakeoffsAspectsTestFixtureBase::RegisterDomains()
    {
    StaticRegisterDomains();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName QuantityTakeoffsAspectsTestFixtureBase::GetGivenProjectPath()
    {
    return SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_DEFAULT_NAME);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void QuantityTakeoffsAspectsTestFixtureBase::CopyFile(BeFileName projectPath)
    {
    BeFileName::BeCopyFile(SharedRepositoryManagerTestBase::BuildProjectPath(PROJECT_SEED_NAME), projectPath.c_str(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void QuantityTakeoffsAspectsTestFixtureBase::SetUpTestCase ()
    {
    BeFileName seedPath = SharedRepositoryManagerTestBase::BuildProjectPath (PROJECT_SEED_NAME);
    BeFileName outPath;
    BeTest::GetHost ().GetOutputRoot (outPath);
    BeFileName::CreateNewDirectory (outPath);
    seedPath.BeDeleteFile ();
    CreateSeedDb(PROJECT_SEED_NAME, PROJECT_TEMP_FOLDER, &StaticRegisterDomains);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles QuantityTakeoffsAspectsTestFixtureBase::_GetApplicationSqlangs()
    {
    BeFileName documentsRoot;
    BeTest::GetHost().GetDocumentsRoot(documentsRoot);

    BeFileName sqlangDbPath = BeFileName(documentsRoot).AppendToPath(L"../sqlang/QuantityTakeoffsAspects_en.sqlang.db3");
    return L10N::SqlangFiles(sqlangDbPath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbR QuantityTakeoffsAspectsTestFixtureBase::GetDgnDb()
    {
    return *DgnClientFx::DgnClientApp::App().Project();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void QuantityTakeoffsAspectsTestFixtureBase::SetUp()
    {
    BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTestBase::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void QuantityTakeoffsAspectsTestFixtureBase::TearDown()
    {
    BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTestBase::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialCategoryCPtr QuantityTakeoffsAspectsTestFixtureBase::CreateAndInsertCategory(DgnDbR db, Utf8StringCR name)
    {
    Dgn::DgnSubCategory::Appearance appearance;
    appearance.SetColor(Dgn::ColorDef::White());
    Dgn::SpatialCategory category(db.GetDictionaryModel(), name, Dgn::DgnCategory::Rank::Domain);
    return category.Insert(appearance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr QuantityTakeoffsAspectsTestFixtureBase::CreateAndInsertModel(DgnDbR db, Utf8StringCR name)
    {
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*rootSubject, name);
    db.Elements().Insert(*partition);
    return PhysicalModel::CreateAndInsert(*partition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
GenericPhysicalObjectPtr QuantityTakeoffsAspectsTestFixtureBase::CreateAndInsertObject(DgnDbR db)
    {
    PhysicalModelPtr model = CreateAndInsertModel(db, "TestModel");
    if (model.IsNull())
        return nullptr;

    SpatialCategoryCPtr category = CreateAndInsertCategory(db, "TestObject");
    if (category.IsNull())
        return nullptr;

    GenericPhysicalObjectPtr object = GenericPhysicalObject::Create(*model, category->GetCategoryId());
    if (object->Insert().IsNull())
        return nullptr;

    return object;
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE