/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Vytautas.Kaniusonis             03/2018
//=======================================================================================
struct DgnUtilsTestFixture : public BuildingSharedTestFixtureBase
    {
    public:
        DgnUtilsTestFixture() {};
        ~DgnUtilsTestFixture() {};

        void SetUp() override;
        void TearDown() override;

        static DgnDbR GetDgnDb() { return *DgnClientFx::DgnClientApp::App().Project(); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Vytautas.Kaniusonis             03/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
void DgnUtilsTestFixture::SetUp()
    {
    BuildingSharedTestFixtureBase::SetUp();
    DgnDbR db = *DgnClientApp::App().Project();

    CodeSpecPtr codeSpec = CodeSpec::Create(db, "TestCodeSpec");
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        BeAssert(codeSpec->GetCodeSpecId().IsValid());
        }

    db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Vytautas.Kaniusonis             03/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
void DgnUtilsTestFixture::TearDown()
    {
    BuildingSharedTestFixtureBase::TearDown();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Vytautas.Kaniusonis             03/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
void InsertElementWithCode(InformationModelP model, DgnCode code, DgnDbR db)
    {
    UrlLinkPtr link = UrlLink::Create(UrlLink::CreateParams(*model, "TestLink", "TestLink"));
    link->SetCode(code);
    link->Insert();
    db.SaveChanges();
    }

//--------------------------------------------------------------------------------------
// @betest                                       Vytautas.Kaniusonis             03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnUtilsTestFixture, CreateNameTemplates)
    {
    DgnDbR db = GetDgnDb();
    auto codeSpec = db.CodeSpecs().QueryCodeSpecId("TestCodeSpec");
    DgnElementId elementId = db.Elements().GetRootSubjectId();

    Utf8String initialName = "TestBuilding";
    Utf8String expectedName = "TestBuilding-0";
    DgnCode code = BuildingUtils::GetNamedElementCode(db, codeSpec, initialName, elementId);
    ASSERT_TRUE(initialName.Equals(code.GetValueUtf8()));

    InformationModelP model = db.Elements().GetRootSubject()->GetModel()->ToInformationModelP();

    InsertElementWithCode(model, code, db);

    expectedName = "TestBuilding-0";
    code = BuildingUtils::GetNamedElementCode(db, codeSpec, initialName, elementId);
    ASSERT_TRUE(expectedName.Equals(code.GetValueUtf8()));

    InsertElementWithCode(model, code, db);

    expectedName = "TestBuilding-1";
    code = BuildingUtils::GetNamedElementCode(db, codeSpec, initialName, elementId);
    ASSERT_TRUE(expectedName.Equals(code.GetValueUtf8()));

    InsertElementWithCode(model, code, db);

    initialName = "TestBuilding-50";
    expectedName = "TestBuilding-50";
    code = BuildingUtils::GetNamedElementCode(db, codeSpec, initialName, elementId);
    ASSERT_TRUE(expectedName.Equals(code.GetValueUtf8()));

    InsertElementWithCode(model, code, db);

    expectedName = "TestBuilding-51";
    code = BuildingUtils::GetNamedElementCode(db, codeSpec, initialName, elementId);
    ASSERT_TRUE(expectedName.Equals(code.GetValueUtf8()));
    }

END_BUILDING_SHARED_NAMESPACE