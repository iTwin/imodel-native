/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/ImportConfigTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include "ImportConfigEditor.h"
#include "GeomTestHelper.h"
#include <VersionedDgnV8Api/ECObjects/ECObjectsAPI.h>

#define TESTMODELNEW   "TestModelNew"
#define TESTMODELNEWW L"TestModelNew"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct ImportConfigTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);
    
    void SetUp();
    void TearDown();
    void DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportConfigTests::DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input)
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
void ImportConfigTests::SetUp()
    {
    T_Super::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportConfigTests::TearDown()
    {
    T_Super::TearDown();
    }
Utf8CP TestSchema = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECSchemaReference name =\"Bentley_Standard_CustomAttributes\" version =\"01.04\" prefix =\"bsca\" />"
"    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"n\" typeName=\"int\" />"
"    </ECClass>"
"    <ECClass typeName=\"ClassB\" >"
"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
"    </ECClass>"
"</ECSchema>";
/*---------------------------------------------------------------------------------**//**
// By default Convert should import Design links
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImportConfigTests, FileLink)
    {
    LineUpFiles(L"FileLink.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";

    BentleyApi::BeFileName linkFile;
    MakeWritableCopyOf(linkFile, L"test2d.dgn");
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId elementId;
    v8editor.AddLine(&elementId);
    v8editor.AddFileLink(elementId, linkFile);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    DgnElementIdSet linkIdSet = EmbeddedFileLink::Query(*db);
    EXPECT_EQ(1, linkIdSet.size());
    EXPECT_EQ(1, db->EmbeddedFiles().MakeIterator().QueryCount());
    m_wantCleanUp = false;
    }

/*---------------------------------------------------------------------------------**//**
// If import configuration is Not to embed desgin links , it Should Not
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImportConfigTests, FileLink_NoEmbed)
    {
    LineUpFiles(L"FileLink_NoEmbed.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";

    BentleyApi::BeFileName linkFile;
    MakeWritableCopyOf(linkFile, L"test2d.dgn");
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId elementId;
    v8editor.AddLine(&elementId);
    v8editor.AddFileLink(elementId, linkFile);
    v8editor.Save();

    ImportConfigEditor config;
    config.m_options.EmbedDgnLinkFiles = false;
    config.CreateConfig();
    m_params.SetConfigFile(config.m_importFileName);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    DgnElementIdSet linkIdSet = EmbeddedFileLink::Query(*db);
    EXPECT_EQ(0, linkIdSet.size());
    EXPECT_EQ(0, db->EmbeddedFiles().MakeIterator().QueryCount()) << "With EmbedDgnLinkFiles = false configuration. Link files should not be embedded in dgndb";
    m_wantCleanUp = false;
    }

/*---------------------------------------------------------------------------------**//**
// Non Dgn file links should be ported to DgnDb
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImportConfigTests, FileLinkNonDgn)
    {
    LineUpFiles(L"FileLinkNonDgn.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";

    BentleyApi::BeFileName linkFile;
    MakeWritableCopyOf(linkFile, L"chkmrk.bmp");
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId elementId;
    v8editor.AddLine(&elementId);
    v8editor.AddFileLink(elementId, linkFile);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    DgnElementIdSet linkIdSet = EmbeddedFileLink::Query(*db);
    EXPECT_EQ(1, linkIdSet.size());
    EXPECT_EQ(1, db->EmbeddedFiles().MakeIterator().QueryCount());
    m_wantCleanUp = false;
    }

/*---------------------------------------------------------------------------------**//**
// Test Validate if we set the configuration to embedded configuration file
* @bsimethod                                    Umar.Hayat                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImportConfigTests, embedConfigFile)
    {
    LineUpFiles(L"embedConfigFile.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    // Change configuration
    ImportConfigEditor config;
    config.m_options.EmbedConfigFile = true;
    config.CreateConfig();
    m_params.SetConfigFile(config.m_importFileName);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    DbEmbeddedFileTable embedFileTable = db->EmbeddedFiles();
    ASSERT_TRUE(0 != embedFileTable.MakeIterator().QueryCount());

    //TODO: Verify content of the embedded file are same as the original one
    }

/*---------------------------------------------------------------------------------**//**
//  Tests validate that USED fonts are published to DgnDb
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImportConfigTests, FontImport)
    {
    LineUpFiles(L"FontImport.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::DgnTextStylePtr  textStyle_ttf = DgnV8Api::DgnTextStyle::Create(L"TextStyle_ttf", *v8editor.m_file);
    DgnV8Api::DgnFont& font_ttf = DgnV8Api::DgnFontManager::GetDefaultTrueTypeFont();
    WString fontName_ttf = font_ttf.GetName();
    ASSERT_SUCCESS(textStyle_ttf->SetProperty(DgnV8Api::TextStyleProperty::TextStyle_Font, &font_ttf));
    ASSERT_SUCCESS(textStyle_ttf->Add());
    v8editor.AddTextElement(*textStyle_ttf);

    DgnV8Api::DgnTextStylePtr  textStyle_shx = DgnV8Api::DgnTextStyle::Create(L"TextStyle_shx", *v8editor.m_file);
    DgnV8Api::DgnFont& font_shx = DgnV8Api::DgnFontManager::GetDefaultShxFont();
    WString fontName_shx = font_shx.GetName();
    ASSERT_SUCCESS(textStyle_shx->SetProperty(DgnV8Api::TextStyleProperty::TextStyle_Font, &font_shx));
    ASSERT_SUCCESS(textStyle_shx->Add());
    v8editor.AddTextElement(*textStyle_shx);

    DgnV8Api::DgnTextStylePtr  textStyle_rsc = DgnV8Api::DgnTextStyle::Create(L"TextStyle_rsc", *v8editor.m_file);
    DgnV8Api::DgnFont& font_rsc = DgnV8Api::DgnFontManager::GetDefaultRscFont();
    WString fontName_rsc = font_rsc.GetName();
    ASSERT_SUCCESS(textStyle_rsc->SetProperty(DgnV8Api::TextStyleProperty::TextStyle_Font, &font_rsc));
    ASSERT_SUCCESS(textStyle_rsc->Add());
    v8editor.AddTextElement(*textStyle_rsc);

    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    Utf8String fontName;
    BeStringUtilities::WCharToUtf8(fontName, fontName_ttf.c_str());
    DgnDbApi::DgnFontCP  pfont = db->Fonts().FindFontByTypeAndName(DgnFontType::TrueType, fontName.c_str());
    EXPECT_TRUE(NULL != pfont);

    BeStringUtilities::WCharToUtf8(fontName, fontName_shx.c_str());
    pfont = db->Fonts().FindFontByTypeAndName(DgnFontType::Shx, fontName.c_str());
    EXPECT_TRUE(NULL != pfont);

    BeStringUtilities::WCharToUtf8(fontName, fontName_rsc.c_str());
    pfont = db->Fonts().FindFontByTypeAndName(DgnFontType::Rsc, fontName.c_str());
    EXPECT_TRUE(NULL != pfont);

    // fonts are present in the dgn_fonts table , does this means it is also embedded ??
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImportConfigTests, FontImport_Always)
    {
    LineUpFiles(L"FontImport_Always.ibim", L"Test3d.dgn", false);

    // Change configuration
    BentleyApi::BeFileName linkFile;
    MakeWritableCopyOf(linkFile, L"Outwrite.ttf");
    ImportConfigEditor config;
    BentleyApi::BeFileName outPath = GetOutputDir();
    outPath.AppendSeparator();
    Utf8String fontPath;
    BeStringUtilities::WCharToUtf8(fontPath, outPath.c_str());
    config.m_fontsSearchPaths = fontPath;
    static const Utf8CP FONT_NAME = "Outwrite";
    static const Utf8CP FONT_TYPE_STR = "TrueType";
    static const DgnFontType FONT_TYPE = DgnFontType::TrueType;
    config.m_fontConfigList.push_back(ImportConfigEditor::FontConfig(FONT_TYPE_STR, FONT_NAME, "Always"));
    config.CreateConfig();
    m_params.SetConfigFile(config.m_importFileName);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    Utf8String fontName("Outwrite");

    // N.B. Merely forcing a font's data to be embedded does not grant it an ID (deferred until actually used -- i.e. fonts and their data are distinct in the DB).
    // Use DbFaceData directly to verify that embedding occured.
    // Also, in this case, the provided Outwrite font only contains a "bold" face.
    DgnFonts::DbFaceDataDirect::FaceKey outwriteRegular(FONT_TYPE, FONT_NAME, DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Bold);
    EXPECT_TRUE(db->Fonts().DbFaceData().Exists(outwriteRegular)) << "Unable to find font";
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImportConfigTests, CreateANDVerifyECClassViews)
    {
    LineUpFiles(L"CreateANDVerifyECClassViews.ibim", L"Test3d.dgn", false); 
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    m_wantCleanUp = false;
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, TestSchema, *schemaContext));
    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));

    DgnV8Api::ElementId elementId;
    v8editor.AddLine(&elementId);
    v8editor.Save();
    // Change configuration
    ImportConfigEditor config;
    config.m_options.CreateECClassViews = true;
    config.CreateConfig();
    m_params.SetConfigFile(config.m_importFileName);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);

    EXPECT_TRUE(db->IsDbOpen());

    BentleyApi::BeSQLite::Statement statement;
    EXPECT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK, statement.Prepare(*db, "select '[' || name || ']'  from sqlite_master where type = 'view' and instr (name,'.') and instr(sql, '--### ECCLASS VIEW')"));
   
    ASSERT_TRUE(statement.Step() != BE_SQLITE_DONE) << "ECClassViews not found ";

    while (statement.Step() == BE_SQLITE_ROW)
    {
       // printf ("\n ViewName : %s \n", statement.GetValueText (0));
        BentleyApi::BeSQLite::Statement stmt;
        BentleyApi::Utf8String sql;
        sql.Sprintf("SELECT * FROM %s", statement.GetValueText(0));
        //printf("Select sql:  %s \n", sql.c_str());
        EXPECT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK, stmt.Prepare(*db, sql.c_str())) << "ECClassView " << stmt.GetValueText(0) << " has invalid DDL: " << db->GetLastError().c_str() << " in DgnDb : " << db->GetFileName();
    }
    statement.Finalize();
    db->CloseDb();
    }