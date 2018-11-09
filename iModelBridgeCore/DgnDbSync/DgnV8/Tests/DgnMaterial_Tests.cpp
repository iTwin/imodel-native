/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/DgnMaterial_Tests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include "ImportConfigEditor.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct DgnMaterialTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);

    void SetUp();
    void TearDown();
    void DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input);
    BentleyApi::BeFileName GetInputFileNameFromTestData(BentleyApi::WCharCP filename);
    void CopyOut(BentleyApi::BeFileName& outFile, BentleyApi::WCharCP filename);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName DgnMaterialTests::GetInputFileNameFromTestData(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"TestData");
    filepath.AppendToPath(filename);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMaterialTests::CopyOut(BentleyApi::BeFileName& outFile, BentleyApi::WCharCP filename)
    {
    outFile = GetOutputFileName(filename);
    BentleyApi::BeFileName inFile = GetInputFileNameFromTestData(filename);
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(inFile, outFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMaterialTests::DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input)
    {
    if (!m_noGcs)
        SetGcsDef();

    // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
    RootModelConverter::RootModelSpatialParams params(m_params);
    params.m_keepHostAliveForUnitTests = true;
    params.SetInputFileName(input);
    params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());

    RootModelConverter creator(params);
    creator.SetWantDebugCodes(true);
    auto db = OpenExistingDgnDb(output);
    ASSERT_TRUE(db.IsValid());
    creator.SetDgnDb(*db);
    creator.SetIsUpdating(false);
    creator.AttachSyncInfo();
    ASSERT_EQ(BentleyApi::SUCCESS, creator.InitRootModel());
    creator.MakeSchemaChanges();
    ASSERT_FALSE(creator.WasAborted());
    ASSERT_EQ(RootModelConverter::ImportJobCreateStatus::Success, creator.InitializeJob());
    creator.Process();
    ASSERT_FALSE(creator.WasAborted());
    db->SaveChanges();
    m_count = creator.GetElementsConverted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMaterialTests::SetUp()
    {
    T_Super::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMaterialTests::TearDown()
    {
    T_Super::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnMaterialTests, MaterialAttachment)
    {
    LineUpFiles(L"MaterialAttachment.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    {
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::LevelId level1Id = v8editor.AddV8Level("Level1"); // Create a level (in addition to the default level)
    DgnV8Api::MaterialPtr     material = DgnV8Api::Material::Create(*v8editor.m_defaultModel);
    ASSERT_FALSE(material.IsNull());
    material->SetName(L"Material35");
    material->GetPaletteR().Init(L"SavedPalette", v8editor.m_file->GetDocument().GetMoniker(), DgnV8Api::PaletteInfo::PALETTETYPE_Dgn);

    Bentley::MaterialManagerR  matManager = DgnV8Api::MaterialManager::GetManagerR();
    DgnV8Api::MaterialId materialId;
    EXPECT_TRUE(SUCCESS == matManager.SaveMaterial(&materialId, *material, v8editor.m_file.get()));

    DgnV8Api::EditElementHandle eeh;
    v8editor.CreateCone(eeh, false);
    eeh.GetElementP()->ehdr.level = level1Id;

    DgnV8Api::IMaterialPropertiesExtension *query = DgnV8Api::IMaterialPropertiesExtension::Cast(eeh.GetHandler());
    query->AddMaterialAttachment(eeh, materialId);
    eeh.AddToModel();
    }
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    RenderMaterialId materialId = RenderMaterial::QueryMaterialId(*GetJobDefinitionModel(*db), "Material35");
    EXPECT_TRUE(materialId.IsValid());
    RenderMaterialCPtr material = RenderMaterial::Get(*db, materialId);
    EXPECT_TRUE(material.IsValid());
    EXPECT_STREQ("SavedPalette", material->GetPaletteName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnMaterialTests, MaterialAssignment)
    {
    LineUpFiles(L"MaterialAssignment.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    {
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::LevelId level1Id = v8editor.AddV8Level("Level1"); // Create a level (in addition to the default level)
    DgnV8Api::MaterialPtr     material = DgnV8Api::Material::Create(*v8editor.m_defaultModel);
    ASSERT_FALSE(material.IsNull());
    material->SetName(L"Material35");
    material->GetSettingsR().SetBaseColor(255, 0, 0);
    material->GetPaletteR().Init(L"SavedPalette", v8editor.m_file->GetDocument().GetMoniker(), DgnV8Api::PaletteInfo::PALETTETYPE_Dgn);

    Bentley::MaterialManagerR  matManager = DgnV8Api::MaterialManager::GetManagerR();
    DgnV8Api::MaterialId materialId;
    EXPECT_TRUE(SUCCESS == matManager.SaveMaterial(&materialId, *material, v8editor.m_file.get()));
    DgnV8Api::MaterialAssignmentPtr matAssignment = DgnV8Api::MaterialAssignment::Create(materialId, L"Level1", 0, 10, *v8editor.m_file);
    DgnV8Api::MaterialTablePtr  matTable = matManager.LoadActiveTable(*v8editor.m_defaultModel);
    matTable->AddAssignment(*matAssignment);
    EXPECT_TRUE(SUCCESS == matManager.SaveTable(*matTable));

    DgnV8Api::EditElementHandle eeh;
    v8editor.CreateCone(eeh, false);
    eeh.GetElementP()->ehdr.level = level1Id;
    eeh.AddToModel();
    }
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    RenderMaterialId materialId = RenderMaterial::QueryMaterialId(*GetJobDefinitionModel(*db), "Material35");
    EXPECT_TRUE(materialId.IsValid());
    RenderMaterialCPtr material = RenderMaterial::Get(*db, materialId);
    EXPECT_TRUE(material.IsValid());
    EXPECT_STREQ("SavedPalette", material->GetPaletteName().c_str());
    }
