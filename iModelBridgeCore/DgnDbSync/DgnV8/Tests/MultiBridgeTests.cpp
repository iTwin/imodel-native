/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/MultiBridgeTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*
    Tests of running multiple bridges on the same v8 files. This happens, for instance,
    when the ABD bridge is assigned to process the master file, while the V8 or ORD 
    is assigned to process some its reference files. In that case, the bridge framework
    runs all of the bridges on the same set of files, all starting with the same "root"
    master model. Each bridge must find its own assigned content and convert only that.
*/
#include "ConverterTestsBaseFixture.h"

struct MultiBridgeTestDocumentAccessor : iModelBridge::IDocumentPropertiesAccessor
    {
    std::map<BentleyApi::WString, std::vector<BentleyApi::BeFileName>> m_assignments;

    bool _IsFileAssignedToBridge(BentleyApi::BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override
        {
        auto iBridge = m_assignments.find(bridgeRegSubKey);
        if (iBridge == m_assignments.end())
            return false;
        for (auto const& doc : iBridge->second)
            {
            if (doc.EqualsI(fn))
                return true;
            }
        return false;
        }

    BentleyApi::BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties& props, BentleyApi::BeFileNameCR fn) override {return BentleyApi::BSIERROR;}
    BentleyApi::BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BentleyApi::BeFileNameR localFilePath, BentleyApi::BeSQLite::BeGuid const& docGuid) override {BeAssert(false); return BentleyApi::BSIERROR;}
    BentleyApi::BentleyStatus _AssignFileToBridge(BentleyApi::BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override {BeAssert(false); return BentleyApi::BSIERROR;}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/17
//---------------------------------------------------------------------------------------
static BentleyApi::BeFileName getSubdir(BentleyApi::BeFileNameCR baseDir, WCharCP subdir, bool createIt)
    {
    BentleyApi::BeFileName workDirName(baseDir);
    workDirName.AppendToPath(subdir);
    if (createIt)
        {
        BentleyApi::BeFileName::EmptyAndRemoveDirectory(workDirName.c_str());
        BentleyApi::BeFileName::CreateNewDirectory(workDirName.c_str());
        }
    return workDirName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/17
//---------------------------------------------------------------------------------------
static BentleyApi::BeFileName copyFileTo(BentleyApi::BeFileNameCR sourceFileName, BentleyApi::BeFileNameCR destDir)
    {
    BentleyApi::BeFileName destFileName(destDir);
    destFileName.AppendToPath(sourceFileName.GetBaseName());
    BentleyApi::BeFileName::BeCopyFile(sourceFileName, destFileName);
    return destFileName;
    }

//----------------------------------------------------------------------------------------
// @bsiclass                                    Sam.Wilson                  05/2018
//----------------------------------------------------------------------------------------
struct DefinitionModelIds
    {
    DgnModelId m_definitionModelId; // Where Categories and LineStyles are stored
    DgnModelId m_drawingListModelId;
    DgnModelId m_sheetListModelId;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/17
//---------------------------------------------------------------------------------------
static void doConvert(DefinitionModelIds& defids,
                      DgnDbR db, BentleyApi::BeFileNameCR inputFileName, 
                      RootModelConverter::RootModelSpatialParams const& paramsIn, 
                      MultiBridgeTestDocumentAccessor& docaccessor,
                      WCharCP bridgeRegSubKey,
                      bool isUpdate)
    {
    // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
    RootModelConverter::RootModelSpatialParams params(paramsIn);
    params.SetBridgeRegSubKey(bridgeRegSubKey);
    params.SetDocumentPropertiesAccessor(docaccessor);
    params.SetInputFileName(inputFileName);
    params.m_keepHostAliveForUnitTests = true;
    RootModelConverter converter(params);
    converter.SetDgnDb(db);
    converter.SetIsUpdating(isUpdate);
    
    //  If I am creating a new local file or if I just acquired a briefcase for an existing repository, then I will have to bootstrap syncinfo.
    auto syncInfoFileName = SyncInfo::GetDbFileName(db.GetFileName());
    if (!syncInfoFileName.DoesPathExist())
        {
        ASSERT_EQ(BSISUCCESS, SyncInfo::CreateEmptyFile(syncInfoFileName));
        }
    converter.AttachSyncInfo();
    
    ASSERT_EQ(BentleyApi::SUCCESS, converter.InitRootModel());
    
    converter.MakeSchemaChanges();
    ASSERT_FALSE(converter.WasAborted());
    
    if (!isUpdate)
        {
        ASSERT_EQ(TestRootModelCreator::ImportJobCreateStatus::Success, converter.InitializeJob());
        }
    else
        {
        ASSERT_EQ(RootModelConverter::ImportJobLoadStatus::Success, converter.FindJob());
        }

    converter.Process();
    
    ASSERT_FALSE(converter.WasAborted());

    defids.m_definitionModelId = converter.GetJobDefinitionModel()->GetModelId();
    defids.m_sheetListModelId = converter.GetSheetListModelId();
    defids.m_drawingListModelId = converter.GetDrawingListModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void getElementsInModel(std::vector<DgnElementCPtr>& cats, DgnDbR db, DgnModelId mid, Utf8CP bisClassName)
    {
    auto stmt = db.GetPreparedECSqlStatement(Utf8PrintfString("select ECInstanceId from %s where model.id = ?", bisClassName).c_str());
    stmt->BindId(1, mid);
    while(BentleyApi::BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        cats.push_back(db.Elements().GetElement(stmt->GetValueId<DgnElementId>(0)));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isCodeValueInList(std::vector<DgnElementCPtr> const& elems, BentleyApi::Utf8CP codeValue)
    {
    for (auto c : elems)
        {
        if (0 == strcmp(codeValue, c->GetCode().GetValueUtf8CP()))
            return true;
        }
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsiclass                                    Sam.Wilson                  05/2018
//----------------------------------------------------------------------------------------
struct MultiBridgeTests : public ConverterTestBaseFixture
    {
    void RunBridge(DefinitionModelIds&, WCharCP bridgeName, MultiBridgeTestDocumentAccessor&, bool isFirstTime);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiBridgeTests::RunBridge(DefinitionModelIds& defids, WCharCP bridgeName, MultiBridgeTestDocumentAccessor& docaccessor, bool isFirstTime)
    {
    // Copy m_dgnDbFileName into the bridge's own workdir - that is its briefcase
    BentleyApi::BeFileName briefcaseName = copyFileTo(m_dgnDbFileName, getSubdir(GetOutputDir(), bridgeName, isFirstTime));
    
    auto db = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    ASSERT_TRUE(db.IsValid());

    // The bridge runs its conversion, writing to its briefcase
    doConvert(defids, *db, m_v8FileName, m_params, docaccessor, bridgeName, !isFirstTime);
    db->SaveChanges();

    // Copy the briefcase back to m_dgnDbFileName -- pretend that bridge1 pushed to iModelHub
    BentleyApi::BeFileName::BeCopyFile(briefcaseName, m_dgnDbFileName);
    }

/*---------------------------------------------------------------------------------**//**
* This test verifies that a bridge will create definitions, such as Categories and
* and views only for the V8 files that are assigned to it.
* @bsimethod                                    Sam.Wilson                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiBridgeTests, FileSpecificDefinitions)
    {
    LineUpFiles(L"MultiBridgeTest.bim", L"master3d.dgn", false);
    
    BentleyApi::BeFileName refV8FileName;
    MakeWritableCopyOf(refV8FileName, L"ref3d.dgn");

    MultiBridgeTestDocumentAccessor docaccessor;
    // m is assigned to the master file
    docaccessor.m_assignments[L"m"].push_back(m_v8FileName);
    // r is assigned to the reference file
    docaccessor.m_assignments[L"r"].push_back(refV8FileName);

    DefinitionModelIds mDefs, rDefs;
    RunBridge(mDefs, L"m", docaccessor, true);
    RunBridge(rDefs, L"r", docaccessor, true);

    auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    ASSERT_TRUE(db.IsValid());

    std::vector<DgnElementCPtr> mCategories, rCategories;
    getElementsInModel(mCategories, *db, mDefs.m_definitionModelId, "bis.Category");
    getElementsInModel(rCategories, *db, rDefs.m_definitionModelId, "bis.Category");

    EXPECT_TRUE (isCodeValueInList(mCategories, "MasterLevel"));
    EXPECT_FALSE(isCodeValueInList(rCategories, "MasterLevel"));
    EXPECT_TRUE (isCodeValueInList(rCategories, "ReferenceLevel"));
    EXPECT_FALSE(isCodeValueInList(mCategories, "ReferenceLevel"));

    std::vector<DgnElementCPtr> mViews, rViews;
    getElementsInModel(mViews, *db, mDefs.m_definitionModelId, "bis.ViewDefinition");
    getElementsInModel(rViews, *db, rDefs.m_definitionModelId, "bis.ViewDefinition");

    EXPECT_TRUE (isCodeValueInList(mViews, "MasterView"));
    EXPECT_FALSE(isCodeValueInList(rViews, "MasterView"));
    // EXPECT_TRUE (isCodeValueInList(rViews, "ReferenceView"));
    EXPECT_FALSE(isCodeValueInList(rViews, "ReferenceView"));  // Actually, named views in reference files are not converted at all
    EXPECT_FALSE(isCodeValueInList(mViews, "ReferenceView"));

    /*
    printf("\nBridge1\n------------\n");
    for (auto c : cats1)
        {
        printf("%s\n", c->GetCode().GetValueUtf8CP());
        }
    printf("\nBridge2\n------------\n");
    for (auto c : cats2)
        {
        printf("%s\n", c->GetCode().GetValueUtf8CP());
        }
        */
    }

static void attach(BentleyApi::BeFileNameCR masterFileName, DgnV8Api::ModelId masterModelId, BentleyApi::BeFileNameCR refFileName, WCharCP refModelName)
    {
    V8FileEditor v8editor;
    v8editor.Open(masterFileName);
    DgnV8Api::DgnModel* masterModel = nullptr;
    v8editor.GetAndLoadModel(masterModel, masterModelId);
    ASSERT_NE(masterModel, nullptr);
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(refFileName.c_str());
    DgnV8Api::DgnAttachment* attachment;
    ASSERT_EQ(BentleyApi::SUCCESS, masterModel->CreateDgnAttachment(attachment, *moniker, refModelName, true));
    attachment->SetNestDepth(99);
    ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
    v8editor.Save();
    }

static void addLineTo(BentleyApi::BeFileNameCR v8FileName, DgnV8Api::ModelId modelId)
    {
    V8FileEditor v8editor;
    v8editor.Open(v8FileName);
    DgnV8ModelP v8Model;
    v8editor.GetAndLoadModel(v8Model, modelId);
    v8editor.AddLine(nullptr, v8Model, Bentley::DPoint3d::From(0,0,0));
    v8editor.Save();
    }

static BentleyApi::bvector<DgnModelCPtr> getModelsByName(DgnDbR db, Utf8CP modelName)
    {
    BentleyApi::bvector<DgnModelCPtr> models;

    auto stmt = db.GetPreparedECSqlStatement("select el.ecinstanceid from bis.element el, bis.model m WHERE (el.ecinstanceid = m.ecinstanceid) AND (el.CodeValue = ?)");
    stmt->BindText(1, modelName, EC::IECSqlBinder::MakeCopy::No);
    while (BentleyApi::BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        auto modelId = stmt->GetValueId<DgnModelId>(0);
        models.push_back(db.Models().GetModel(modelId));
        }
    return models;
    }

static int countElementsInModel(BentleyApi::Dgn::DgnModelCR model)
    {
    auto stmt = model.GetDgnDb().GetPreparedECSqlStatement("select COUNT(*) from bis.element WHERE model.id = ?");
    stmt->BindId(1, model.GetModelId());
    if (BentleyApi::BeSQLite::BE_SQLITE_ROW != stmt->Step())
        return 0;
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* This test verifies that a bridge will process all of its assigned documents, 
* even if they are hidden behind reference attachments that are not assigned to it.
* @bsimethod                                    Sam.Wilson                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiBridgeTests, Sandwich)
    {
    LineUpFiles(L"MultiBridgeTest.bim", L"master3d.dgn", false); 
    
    // TRICKY: I will not use "master3d.dgn" as my master file. 
    //          Instead, I will make a copy of "ref3d.dgn" and call it master.dgn.
    //          That is because "master3d.dgn" already contains a reference attachment
    //          and ref3d.dgn does not. I want to start with a clean slate and create
    //          all of the attachments myself.
    
    // Create the sandwich:
    BentleyApi::BeFileName masterFileName, rRefFileName, mRefFileName;
    MakeWritableCopyOf(masterFileName, GetInputFileName(L"ref3d.dgn"), L"master.dgn");
    MakeWritableCopyOf(rRefFileName, GetInputFileName(L"ref3d.dgn"), L"rref.dgn");
    MakeWritableCopyOf(mRefFileName, GetInputFileName(L"ref3d.dgn"), L"mref.dgn");

    m_v8FileName = masterFileName;  // This is the master file that I want to use as my converter root

    attach(masterFileName, 0, rRefFileName, L"");        // master -> rref
    attach(rRefFileName,   0, mRefFileName, L"");        //           rref -> mref

    // Put an element in each model
    addLineTo(masterFileName, 0);
    addLineTo(rRefFileName, 0);
    addLineTo(mRefFileName, 0);

    MultiBridgeTestDocumentAccessor docaccessor;
    // m is assigned to the master file and to mref
    docaccessor.m_assignments[L"m"].push_back(m_v8FileName);
    docaccessor.m_assignments[L"m"].push_back(mRefFileName);
    // r is assigned to rref
    docaccessor.m_assignments[L"r"].push_back(rRefFileName);

    DefinitionModelIds mDefs, rDefs;
    RunBridge(mDefs, L"m", docaccessor, true);
    RunBridge(rDefs, L"r", docaccessor, true);

    auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    ASSERT_TRUE(db.IsValid());

    // Each model is created in the physical partition of each bridge.
    auto masterModels = getModelsByName(*db, "master"); ASSERT_EQ(2, masterModels.size());
    auto rrefModels = getModelsByName(*db, "rref"); ASSERT_EQ(1, rrefModels.size());
    auto mrefModels = getModelsByName(*db, "mref"); ASSERT_EQ(1, rrefModels.size());

    ASSERT_EQ(2, countElementsInModel(*masterModels[0]));
    ASSERT_EQ(2, countElementsInModel(*rrefModels[0]));
    ASSERT_EQ(2, countElementsInModel(*mrefModels[0]));
    }


