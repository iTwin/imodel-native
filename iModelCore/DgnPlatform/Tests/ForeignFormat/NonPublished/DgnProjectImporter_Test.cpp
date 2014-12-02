/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ForeignFormat/NonPublished/DgnProjectImporter_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ForeignFormatTests.h"
#include <Logging/BentleyLogging.h>
#include <DgnPlatform/ForeignFormat/DgnV8ProjectImporter.h>
#include <DgnPlatform/DgnCore/DgnProgressMeter.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/BeTimeUtilities.h>
//for hosts
#include <Windows.h>
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
#include <DgnPlatform/DgnCore/DgnCoreAPI.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnView/DgnViewLib.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE     
USING_NAMESPACE_FOREIGNFORMAT     
USING_NAMESPACE_DGNV8 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void CreateBlankBoundProject (DgnProjectPtr& project, BeFileName const& projectFileName,  bool deleteIfExists = true)
    {
    if (deleteIfExists && projectFileName.DoesPathExist())
        EXPECT_TRUE(BeFileNameStatus::Success == BeFileName::BeDeleteFile (projectFileName))<<"Failed to delete file";

    CreateProjectParams params;

    DgnFileStatus result;
    project = DgnProject::CreateProject (&result, projectFileName, params);

    ASSERT_TRUE( project.IsValid());
    ASSERT_TRUE( result == DGNFILE_STATUS_Success);

    Utf8String projectFileNameUtf8 (projectFileName.GetName());
    ASSERT_TRUE( projectFileNameUtf8 == Utf8String(project->GetDbFileName()));
    }
/*---------------------------------------------------------------------------------**//**
// @bsiclass                                                    Julija.Suboc     07/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProjectProperties
    {
    size_t m_levelCount;
    size_t m_viewCount;
    size_t m_styleCount;
    bmap<DgnModelId, UInt32> m_modelInfo;
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                            Julija.Suboc                08/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    ProjectProperties()
        {
        m_levelCount = 0;
        m_viewCount = 0;
        m_styleCount = 0;
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                            Julija.Suboc                08/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void GetProjectProperties(DgnProjectPtr& project)
        {
        DgnModels& modelTable =  project->Models();
        for (DgnModels::Iterator::Entry const& entry: modelTable.MakeIterator())
            {
            DgnModelId id = entry.GetModelId();
            DgnModelP model = project->Models().GetModelById(id);
            model->FillSections(DgnModelSections::Model);
            UInt32 elmInModel = model->GetElementCount(DgnModelSections::GraphicElements);
            m_modelInfo.Insert(id, elmInModel);
            }
        m_viewCount = project->Views().MakeIterator().QueryCount();
        m_levelCount = project->Levels().MakeIterator().QueryCount();
        DgnStyles& styleTable = project->Styles();
        m_styleCount = styleTable.DisplayStyles().MakeIterator().QueryCount() + styleTable.LineStyles().MakeIterator().QueryCount();
        }
    };
   
/*---------------------------------------------------------------------------------**//**
// Implementing test publisher to access some protected importer functions
// @bsiclass                                                    Julija.Suboc     07/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestPublisher : DgnV8ProjectImporter
    {
    DEFINE_T_SUPER(DgnV8ProjectImporter)
    private:
        Utf8String m_password;
    protected:
        virtual Utf8CP _GetProtectedFilePassword() override {return m_password.empty()? nullptr: m_password.c_str();}
    public:
    TestPublisher (DgnProjectR project, DgnImportParams const& params) :
    DgnV8ProjectImporter (project, params)
        {
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Julija.Suboc                09/13
    //---------------------------------------------------------------------------------------
    void SetPassword (Utf8CP p) {m_password = p;}
    //---------------------------------------------------------------------------------------
    // Should be called only after function PerformImport is called otherwise configuration 
    // file is not ready for reading 
    // @bsimethod                                        Julija.Suboc                09/13
    //---------------------------------------------------------------------------------------
    ImportConfig* GetConfig () 
        {
        return &m_config;}
    //---------------------------------------------------------------------------------------
    // Call protected function
    // @bsimethod                                        Julija.Suboc                07/13
    //---------------------------------------------------------------------------------------
    virtual void _ReportIssue(IssueReportSeverity severity, IssueReportCategory::Number category, WCharCP source, Utf8CP message) override
        {
        T_Super::_ReportIssue(severity, category, source, message);
        }
    //---------------------------------------------------------------------------------------
    // Sets seed file
    // @bsimethod                                        Julija.Suboc                07/13
    //---------------------------------------------------------------------------------------
    bool SetSeedFileIfExists (WCharCP seedFileName)
        {
        BeFileName seedFilePath = DgnDbTestDgnManager::GetSeedFilePath(seedFileName);
        if(!BeFileName::DoesPathExist(seedFilePath))
            return false;
        SetSeedFile (seedFilePath);
        return true;
        }
    };

/*---------------------------------------------------------------------------------**//**
// Testing some of the DgnProjectImporter functions
// @bsiclass                                                    Julija.Suboc     07/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnProjectImporterTests : public testing::Test
    {
    ScopedDgnHost m_autoDgnHost;
    DgnProjectPtr m_project;
    void SetUp()
        {
        CreateBlankBoundProject (m_project, DgnDbTestDgnManager::GetOutputFilePath  (L"fileToImportData.idgndb"));
        }
    public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Julija Suboc     07/13
    //---------------------------------------------------------------------------------------
    DgnModelP LoadModel(Utf8CP name)
        {
        DgnModels& modelTable =  m_project->Models();
        DgnModelId id = modelTable.QueryModelId(name);
        return  m_project->Models().GetModelById (id);
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Julija Suboc     07/13
    //---------------------------------------------------------------------------------------
    PersistentElementRefP GetGraphicElement(DgnModelP model)
        {
        GraphicElementRefList* graphicList = model->GetGraphicElementsP();
        PersistentElementRefP elm = NULL;
        if (graphicList != NULL)
            {
            for(PersistentElementRefP elmIter : *graphicList)
                {
                elm = elmIter;
                }
            }
        return elm;
        }
    };
//---------------------------------------------------------------------------------------
// test case covers DgnImportIssues::OpenReportFile() and DgnImportIssues::Report()
// @bsimethod                                        Julija.Suboc                07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnProjectImporterTests, IssueReporter)
    {
    BeFileName output;
    //Prepare output directory
    BeTest::GetHost().GetOutputRoot (output);
    ASSERT_TRUE(output.DoesPathExist())<<"Output directory returned from host does not exists";
    output.AppendToPath(L"Issue Reporter Test");
    if(!output.DoesPathExist())
        ASSERT_TRUE(BeFileNameStatus::Success ==BeFileName::CreateNewDirectory(output))<<"Failed to create directory for issue reporter files";
    //Prepare issue log
    BeFileName issueLog;
    issueLog.AppendToPath(output);
    issueLog.AppendToPath(L"issue.log");
    //Prepare log file
    if(issueLog.DoesPathExist())
        ASSERT_TRUE(BeFileNameStatus::Success ==BeFileName::BeDeleteFile(issueLog.GetName()))<<"Failed to delete old log file";
    DgnImportParams params (issueLog.GetName());
    //Prepare publisher
    TestPublisher publisher (*m_project, params);
    bool isSeedSet = publisher.SetSeedFileIfExists(L"2dMetricGeneral.dgn.v8");
    ASSERT_TRUE(isSeedSet)<<"Failed to set seed";
    IssueReportSeverity severity = IssueReportSeverity::Info;
    publisher._ReportIssue(severity, IssueReportCategory::Unsupported, L"From Test", "Test issue reported!");
    //log file is released after import only   
    DgnProgressMeter::Abort progress = publisher.PerformImport();
    EXPECT_EQ(DgnProgressMeter::ABORT_No, progress)<<"Failed to import file.";
    //Verify that file has issue reported
    BeFile file;
    bvector<byte> logData;
    BeFileStatus status = file.Open(issueLog.GetName(), BeFileAccess::Read);
    ASSERT_TRUE(BeFileStatus::Success == status);
    file.ReadEntireFile(logData);
    size_t sizeAfterLog = logData.size();
    ASSERT_TRUE(10 < sizeAfterLog); 
    file.Close();
    }
/*---------------------------------------------------------------------------------**//**
* Import the ElementSymbologyByLevel.dgn.v8 that is used by other tests
* @bsimethod                                    Majd.Uddin                   05/12
* Updated test                                  Julija Suboc                 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectImporterTests, ImportElementSymbology)
    {
    DgnImportParams params;
    TestPublisher publisher (*m_project, params);
    bool isSeedSet = publisher.SetSeedFileIfExists(L"ElementsSymbologyByLevel.dgn.v8");
    ASSERT_TRUE(isSeedSet)<<"Failed to set seed";
    DgnProgressMeter::Abort progress = publisher.PerformImport();
    EXPECT_EQ(DgnProgressMeter::ABORT_No, progress)<<"Failed to import file.";
    //Verify that the new .dgndb file has the correct versions
    DgnVersion schemaVer = m_project->GetSchemaVersion();
    ASSERT_EQ (PROJECT_CURRENT_VERSION_Major, schemaVer.GetMajor()) << "The Schema Major Version is: " << schemaVer.GetMajor();
    ASSERT_EQ (PROJECT_CURRENT_VERSION_Minor, schemaVer.GetMinor()) << "The Schema Minor Version is: " << schemaVer.GetMinor();
    ASSERT_EQ (PROJECT_CURRENT_VERSION_Sub1, schemaVer.GetSub1()) << "The Schema Sub1 Version is: " << schemaVer.GetSub1();
    ASSERT_EQ (PROJECT_CURRENT_VERSION_Sub2, schemaVer.GetSub2()) << "The Schema Sub2 Version is: " << schemaVer.GetSub2();
    }	

/*---------------------------------------------------------------------------------**//**
* Trying to import a seed again into other DgnProject
* @bsimethod                                    Majd.Uddin                   04/12
* Updated test                                  Julija Suboc                 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectImporterTests, ImportSeedTwice)
    {
    //Import 2dMetricGeneral.dgn.v8. It should work
    DgnImportParams params;
    TestPublisher publisher (*m_project, params);
    bool seedAdded = publisher.SetSeedFileIfExists (L"2dMetricGeneral.dgn.v8");
    ASSERT_TRUE(seedAdded)<<"Failed to set seed";
    DgnProgressMeter::Abort progress = publisher.PerformImport();
    EXPECT_EQ(DgnProgressMeter::ABORT_No, progress)<<"Failed to import file.";
    //Improt 2dMetricGeneral.dgn.v8 again into another project
    DgnProjectPtr project2;
    CreateBlankBoundProject (project2, DgnDbTestDgnManager::GetOutputFilePath  (L"projectToImport2.idgndb"));
    TestPublisher publisher2 (*project2, params);
    seedAdded = publisher2.SetSeedFileIfExists (L"2dMetricGeneral.dgn.v8");
    ASSERT_TRUE(seedAdded)<<"Failed to set seed";
   
    progress = publisher2.PerformImport();
    EXPECT_EQ(DgnProgressMeter::ABORT_No, progress)<<"Failed to import file.";
    }

/*---------------------------------------------------------------------------------**//**
* Adding a foreign file
* @bsimethod                                     Julija Suboc                 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectImporterTests, DISABLED_ImportSeedFileWithForeignFile)
    {
    DgnImportParams params;
    TestPublisher publisher (*m_project, params);
    bool isSeedSet = publisher.SetSeedFileIfExists(L"DgnProjectImporterTests_seedFileWithForeignFile.i.dgn.v8");
    ASSERT_TRUE(isSeedSet)<<"Failed to set seed";
    DgnProgressMeter::Abort progress = publisher.PerformImport();
    EXPECT_EQ(DgnProgressMeter::ABORT_No, progress)<<"Failed to import file.";
    ProjectProperties propertiesToVerify;
    propertiesToVerify.GetProjectProperties(m_project);
    //Verify file content
    EXPECT_EQ(4, propertiesToVerify.m_viewCount);
    EXPECT_EQ(4, propertiesToVerify.m_levelCount);
    EXPECT_EQ(0, propertiesToVerify.m_styleCount);
    //Verify Default model element (every model has only one graphic element)
    DgnModelP modelP = LoadModel("Default");
    EXPECT_TRUE(modelP->Is3d());
    PersistentElementRefP elm = GetGraphicElement(modelP);
    ASSERT_TRUE(elm != NULL) << "Failed to get graphic element";
    EXPECT_EQ(3, elm->GetLegacyType())<<"Expected line element";
    LevelId element1level = elm->GetLevel();
    //Verify seed model element
    modelP = LoadModel("SeedModel2D");
    EXPECT_FALSE(modelP->Is3d());
    elm = GetGraphicElement(modelP);
    ASSERT_TRUE(elm != NULL) << "Failed to get graphic element";
    EXPECT_EQ(6, elm->GetLegacyType())<<"Expected elipse element";
    LevelId element2level = elm->GetLevel();
    //Check element properties
    EditElementHandle eeh (elm);
#ifdef WIP_FF_TESTS
    TestElementPropertiesGetter query;
    PropertyContext::QueryElementProperties (eeh, &query);
    EXPECT_EQ(103, query.GetColor().GetValue());
    EXPECT_EQ(5, query.GetLinestyle().GetValue());
#endif
    //Check that elements have diffrent levels
    EXPECT_TRUE(element1level != element2level)<<"Elements should have diffrent level";

    //Verify foregnDefault model element
    modelP = LoadModel("foreignDefault");
    ASSERT_TRUE( modelP != NULL );
    EXPECT_TRUE(modelP->Is3d());
    elm = GetGraphicElement(modelP);
    ASSERT_TRUE(elm != NULL) << "Failed to get graphic element";
    EXPECT_EQ(27, elm->GetLegacyType())<<"Expected line element";
    element1level = elm->GetLevel();
    //Verify foreignModel3D
    modelP = LoadModel("foreignModel3D");
    EXPECT_TRUE(modelP->Is3d());
    elm = GetGraphicElement(modelP);
    ASSERT_TRUE(elm != NULL) << "Failed to get graphic element";
    EXPECT_EQ(23, elm->GetLegacyType())<<"Expected elipse element";
    element2level = elm->GetLevel();
    //Check that elements have diffrent levels
    EXPECT_TRUE(element1level != element2level)<<"Elements should have diffrent level";
    }

/*---------------------------------------------------------------------------------**//**
* Import the Demo Dgn
* @bsimethod                                    Majd.Uddin                   04/12
* Updated test                                  Julija Suboc                 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectImporterTests, DISABLED_ImportDemoDgn)
    {
    //Import Demo.Dgn
    DgnImportParams params;
    TestPublisher publisher (*m_project, params);
    bool isSeedSet = publisher.SetSeedFileIfExists(L"demo.dgn.v8");
    ASSERT_TRUE(isSeedSet)<<"Failed to set seed";
    DgnProgressMeter::Abort progress = publisher.PerformImport();
    EXPECT_EQ(DgnProgressMeter::ABORT_No, progress)<<"Failed to import file.";
    //Verify that file is added. Submitted issue to access provenance data #29030
    ProjectProperties propertiesToVerify;
    propertiesToVerify.GetProjectProperties(m_project);
    //Verify file content
    EXPECT_EQ(17, propertiesToVerify.m_modelInfo.size());
    EXPECT_EQ(7, propertiesToVerify.m_levelCount);
    }
/*---------------------------------------------------------------------------------**//**
* Import file with password protection
* Updated test                                  Julija Suboc                 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectImporterTests, ImportProtectedFileWithValidPassword)
    {
    DgnImportParams params;
    TestPublisher publisher (*m_project, params);
    bool isSeedSet = publisher.SetSeedFileIfExists(L"DgnProjectImporterTests_fileWithPassword.i.dgn.v8");
    ASSERT_TRUE(isSeedSet)<<"Failed to set seed";
    publisher.SetPassword("Bentley\\ ");
    DgnProgressMeter::Abort progress = publisher.PerformImport();
    EXPECT_EQ(DgnProgressMeter::ABORT_No, progress)<<"Failed to import file.";
    //Verify that file is added. Submitted issue to access provenance data #29030
    ProjectProperties propertiesToVerify;
    propertiesToVerify.GetProjectProperties(m_project);
    //Verify file content
    EXPECT_EQ(1, propertiesToVerify.m_modelInfo.size());
    EXPECT_EQ(2, propertiesToVerify.m_levelCount);
    }
/*---------------------------------------------------------------------------------**//**
* Import file with password protection
* Updated test                                  Julija Suboc                 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectImporterTests, ImportProtectedFileWithInValidPassword)
    {
    DgnImportParams params;
    TestPublisher publisher (*m_project, params);
    bool isSeedSet = publisher.SetSeedFileIfExists(L"DgnProjectImporterTests_fileWithPassword.i.dgn.v8");
    ASSERT_TRUE(isSeedSet)<<"Failed to set seed";
    publisher.SetPassword("bentley\\ ");
    DgnProgressMeter::Abort progress = publisher.PerformImport();
    EXPECT_EQ(DgnProgressMeter::ABORT_Yes, progress)<<"Failed to import file.";
    //Verify that file is added. Submitted issue to access provenance data #29030
    ProjectProperties propertiesToVerify;
    propertiesToVerify.GetProjectProperties(m_project);
    //Verify file content
    EXPECT_EQ(0, propertiesToVerify.m_modelInfo.size());
    EXPECT_EQ(0, propertiesToVerify.m_levelCount);
    EXPECT_EQ(0, propertiesToVerify.m_viewCount);
    EXPECT_EQ(0, propertiesToVerify.m_styleCount);
    }
/*---------------------------------------------------------------------------------**//**
* Import file with password protection
* Updated test                                  Julija Suboc                 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectImporterTests, ImportFileWithExpiredDate)
    {
    DgnImportParams params;
    TestPublisher publisher (*m_project, params);
    bool isSeedSet = publisher.SetSeedFileIfExists(L"DgnProjectImporterTests_Expires08_29_2013.i.dgn.v8");
    ASSERT_TRUE(isSeedSet)<<"Failed to set seed";
    DgnProgressMeter::Abort progress = publisher.PerformImport();
    EXPECT_EQ(DgnProgressMeter::ABORT_Yes, progress)<<"Failed to import file.";
    //Verify that file is added. Submitted issue to access provenance data #29030
    ProjectProperties propertiesToVerify;
    propertiesToVerify.GetProjectProperties(m_project);
    //Verify file content
    EXPECT_EQ(0, propertiesToVerify.m_modelInfo.size());
    EXPECT_EQ(0, propertiesToVerify.m_levelCount);
    EXPECT_EQ(0, propertiesToVerify.m_viewCount);
    EXPECT_EQ(0, propertiesToVerify.m_styleCount);
    }

/*---------------------------------------------------------------------------------**//**
// Tests with hosts implemented following recomendations in SampleIDgnToIDgnDbPublisher
// @bsiclass                                                    Julija.Suboc     09/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnProjectImporterTestsWithHosts : public testing::Test
    {
    //---------------------------------------------------------------------------------------
    //Copied from SampleIDgnToIDgndbPublisher
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    static BeFileName getDataDir()
        {
        WChar fullExePath[MAX_PATH];
        if (0 == ::GetModuleFileNameW(NULL, fullExePath, MAX_PATH))
            {
            BeAssert(false);
            fullExePath[0] = 0;
            }
        BeFileName baseDir(BeFileName::DevAndDir, fullExePath);
        baseDir.AppendToPath(L"DgnPlatformAssetsDirectory");
        return baseDir;
        }
    //---------------------------------------------------------------------------------
    //! @bsiclass                                               Julija.Suboc    09/13
    //---------------------------------------------------------------------------------
    struct ThumbnailViewManager : DgnPlatform::IViewManager
        {
        protected:
            virtual DgnPlatform::DgnDisplay::WindowP _GetTopWindow(int) override {return nullptr;}
            //  _GetQvSystemContext is okay for Windows and iOS, not for Android.
            virtual DgnPlatform::DgnDisplay::QvSystemContextP _GetQvSystemContext() override {return nullptr;}
            virtual bool                _DoesHostHaveFocus()        override {return true;}
            virtual IndexedViewSetR     _GetActiveViewSet()         override {return *(IndexedViewSetP)nullptr;}
            //---------------------------------------------------------------------------------------
            //Copied from SampleIDgnToIDgndbPublisher
            // @bsimethod                                   Shaun.Sewall                    04/12
            //---------------------------------------------------------------------------------------
            bool _PreferSeparateQvThread () override
                {
            #if defined (WIP_CFGVAR) // *** DgnDbImporter_USE_SEPARATE_QV_THREAD - Query importer xml config file!
                return ConfigurationManager::IsVariableDefined(L"DgnDbImporter_USE_SEPARATE_QV_THREAD");
            #endif
                return false;
                }

            //---------------------------------------------------------------------------------------
            //Copied from SampleIDgnToIDgndbPublisher
            // @bsimethod                                   Shaun.Sewall                    04/12
            //---------------------------------------------------------------------------------------
            bool _ForceSoftwareRendering ()
                {
            #if defined (WIP_CFGVAR) // *** DgnDbImporter_USE_HARDWARE_RENDER - Query importer xml config file!
                return !ConfigurationManager::IsVariableDefined(L"DgnDbImporter_USE_HARDWARE_RENDER");
            #endif
                return false;
                }
            virtual int                 _GetDynamicsStopInterval()  override {return 200;}
        };
    //---------------------------------------------------------------------------------------
    //! @bsiclass                                                   Julija.Suboc    09/13
    //---------------------------------------------------------------------------------------
    struct ThumbnailIKnownLocationsAdmin : DgnPlatform::WindowsKnownLocationsAdmin
        {
        DEFINE_T_SUPER (DgnPlatform::WindowsKnownLocationsAdmin);
        BeFileName m_appDir;
        virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override
            {
            m_appDir= getDataDir();
            return m_appDir;
            }
        };
    //---------------------------------------------------------------------------------------
    //! @bsiclass                                                   Julija.Suboc    09/13
    //---------------------------------------------------------------------------------------
    struct ThumbnailHost : DgnPlatform::DgnViewLib::Host
        {
        protected:
            virtual void _SupplyProductName (WStringR name) override {name.assign(L"TestPublisher");}
            virtual IViewManagerR _SupplyViewManager() override {return *new ThumbnailViewManager ();}
            //GeoCord admin is not implemented
            // Solid kernel admin is not implemented
            virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override {return *new ThumbnailIKnownLocationsAdmin();}
            virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override {return BeSQLite::L10N::SqlangFiles(BeFileName());}
        public:
            ThumbnailHost(){};
        };
};
/*---------------------------------------------------------------------------------**//**
* Test case to cover ImportConfig functions. Import needs to be performed before 
* trying to get config file, because ImportConfig::ReadFromXmlFile() is not exported 
* and is called only while prforming import.  
* Updated test                                  Julija Suboc                 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectImporterTestsWithHosts, ConfigFile)
    {
    //Find test data and output directory
    ScopedDgnHost* autoDgnHost = new ScopedDgnHost();
    BeFileName projectFileName = DgnDbTestDgnManager::GetOutputFilePath  (L"fileToImportData.idgndb");
   // ASSERT_TRUE(BeFileStatus::Success ==BeFileName::BeDeleteFile (projectFileName))<<"Failed to delete file";
    BeFileName configFile;
    DgnDbTestDgnManager::FindTestData (configFile, L"DgnProjectImporterTests_ImportConfig.xml", __FILE__);
    delete autoDgnHost;
    //Initialize publisher and perform import to get configuration file
    ThumbnailHost viewHost;
    DgnViewLib::Initialize(viewHost, true);
    ASSERT_TRUE(configFile.DoesPathExist())<<"Configuration file could not be found";
    DgnProjectPtr project;
    CreateBlankBoundProject (project, projectFileName);

    DgnImportParams params (NULL, configFile.GetName());
    TestPublisher publisher (*project, params);    
    bool isSeedSet = publisher.SetSeedFileIfExists(L"ElementsSymbologyByLevel.dgn.v8");
    ASSERT_TRUE(isSeedSet)<<"Failed to set seed";
    //Test get config to test configuration file functions
    publisher.PerformImport();
    ImportConfig* config = publisher.GetConfig();
    //tests for GetXPathBool()
    EXPECT_TRUE(config->GetXPathBool("ImportConfig/Raster/@importAttachments", false));
    EXPECT_TRUE(config->GetXPathBool("ImportConfig/Raster/@importAttachments", true));
    EXPECT_FALSE(config->GetXPathBool("ImportConfig/References/@newLevelDisplay", true));
    EXPECT_FALSE(config->GetXPathBool("ImportConfig/References/@newLevelDisplay", false));
    EXPECT_FALSE(config->GetXPathBool("ImportConfig/Raster/@propertyDoesNotExists", false));
    EXPECT_TRUE(config->GetXPathBool("ImportConfig/Raster/@propertyDoesNotExists", true));
    //tests for GetXPathInt64()
    EXPECT_EQ(768, config->GetXPathInt64("/ImportConfig/Thumbnails/@pixelResolution", 23));
    EXPECT_EQ(23, config->GetXPathInt64("/ImportConfig/Thumbnails/@propertyDoesNotExists", 23))<<"Issue TFS#35741";
    //tests for GetXPathString()
    Utf8String stringProperty = config->GetXPathString("/ImportConfig/Thumbnails/@viewTypes", "Default");
    EXPECT_EQ(0, stringProperty.CompareTo("Physical Drawing"));
    Utf8String stringPropertyDoesNotExists = config->GetXPathString("/ImportConfig/Thumbnails/@propertyDoesNotExists", "Default");
    EXPECT_EQ(0, stringPropertyDoesNotExists.CompareTo("Default"));
    //tests for GetXPathDouble()
    double defaultDouble = 2.23234;
    EXPECT_EQ(3.1415926, config->GetXPathDouble("/ImportConfig/MathConst/@pi", defaultDouble))<<"Issue TFS#35741";
    EXPECT_EQ(defaultDouble, config->GetXPathDouble("/ImportConfig/MAthConst/@propertyDoesNotExists", defaultDouble))<<"Issue TFS#35741";
    //tests for OptionExists()
    EXPECT_TRUE(config->OptionExists("CompactDatabase"));
    EXPECT_TRUE(config->OptionExists("OptimizeGraphics"));
    EXPECT_FALSE(config->OptionExists("OptionDoesNotExists"));
    //tests for GetOptionValueBool()
    EXPECT_TRUE(config->GetOptionValueBool("CompactDatabase", false));
    EXPECT_TRUE(config->GetOptionValueBool("CompactDatabase", true));
    EXPECT_FALSE(config->GetOptionValueBool("ConvertSeedDrawingsTo3d", true));
    EXPECT_FALSE(config->GetOptionValueBool("ConvertSeedDrawingsTo3d", false));
    EXPECT_TRUE(config->GetOptionValueBool("OptionDoesNotExists", true));
    EXPECT_FALSE(config->GetOptionValueBool("OptionDoesNotExists", false));
    //tests for GetOptionValueString()
    Utf8String optionString = config->GetOptionValueString("TestString", "Default");
    EXPECT_EQ(0, optionString.CompareTo("Fake option added for test"));
    Utf8String optionStringDoesNotExists = config->GetOptionValueString("OptionDoesNotExists", "Default");
    EXPECT_EQ(0, optionStringDoesNotExists.CompareTo("Default"));
    //tests for GetOptionValueInt64()
    EXPECT_EQ(543, config->GetOptionValueInt64("TestInt", 345));
    EXPECT_EQ(345, config->GetOptionValueInt64("OptionDoesNotExists", 345));
    //tests for GetOptionValueDouble()
    EXPECT_EQ(3.232323, config->GetOptionValueDouble("TestDouble", 2.12));
    EXPECT_EQ(2.12, config->GetOptionValueDouble("OptionDoesNotExists", 2.12));
    //tests for EvaluateXPath()
    Utf8String expression;
    EXPECT_EQ(SUCCESS, config->EvaluateXPath(expression, "/ImportConfig/Thumbnails/@viewTypes"));
    EXPECT_EQ(0, expression.CompareTo("Physical Drawing"));
    EXPECT_EQ(SUCCESS, config->EvaluateXPath(expression, "/ImportConfig/Thumbnails/@propertyDoesNotExists"));
    EXPECT_EQ(0, expression.CompareTo(""));
    //TO DO
    bool hostTerminated = false;
    ASSERT_TRUE(hostTerminated)<<"Need to terminate host but currently this action crashes. If host will not be terminated, "
                     "all tests that will be executed will probbably fail with error about registered host. TFS #36135";
    //DgnPlatform::DgnViewLib::GetHost ().Terminate (true);
    }
