/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    std::map<BentleyApi::BeFileName, iModelBridgeDocumentProperties> m_docProps;
    std::map<BentleyApi::Utf8String, BentleyApi::BeFileName> m_guids;

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

    void _QueryAllFilesAssignedToBridge(BentleyApi::bvector<BentleyApi::BeFileName>& fns, wchar_t const* bridgeRegSubKey) override
        {
        auto iBridge = m_assignments.find(bridgeRegSubKey);
        if (iBridge != m_assignments.end())
            {
            for (auto const& fn: iBridge->second)
                fns.push_back(BentleyApi::BeFileName(fn.c_str()));
            }
        }

    void BuildGuidMap()
        {
        for (auto const& props : m_docProps)
            {
            m_guids[props.second.m_docGuid] = props.first;
            }
        }

    BentleyApi::BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties& props, BentleyApi::BeFileNameCR fn) override
        {
        auto found = m_docProps.find(fn);
        if (found == m_docProps.end())
            return BentleyApi::BSIERROR;
        props = found->second;
        return BentleyApi::BSISUCCESS;
        }

    BentleyApi::BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BentleyApi::BeFileNameR localFilePath, BentleyApi::BeSQLite::BeGuid const& docGuid) override
        {
        auto found = m_guids.find(docGuid.ToString());
        if (found == m_guids.end())
            return BentleyApi::BSIERROR;
        return _GetDocumentProperties(props, found->second);
        }

    BentleyApi::BentleyStatus _AssignFileToBridge(BentleyApi::BeFileNameCR fn, wchar_t const* bridgeRegSubKey, BeGuidCP guid) override {BeAssert(false); return BentleyApi::BSIERROR;}
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
                      bool isUpdate,
                      bool detectDeletedFiles)
    {
    // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
    RootModelConverter::RootModelSpatialParams params(paramsIn);
    params.SetBridgeRegSubKey(bridgeRegSubKey);
    params.SetDocumentPropertiesAccessor(docaccessor);
    params.SetInputFileName(inputFileName);
    params.SetKeepHostAlive(true);
    RootModelConverter converter(params);
    converter.SetDgnDb(db);
    converter.SetIsUpdating(isUpdate);
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

    ASSERT_EQ(BentleyApi::SUCCESS, converter.MakeDefinitionChanges());

    converter.ConvertData();
    
    ASSERT_FALSE(converter.WasAborted());

    if (detectDeletedFiles)
        converter._DetectDeletedDocuments();

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
    void RunBridge(DefinitionModelIds&, WCharCP bridgeName, MultiBridgeTestDocumentAccessor&, bool isFirstTime, bool detectDeletedFiles = false);
    void DoMergeDefinitions(bool mergeDefinitions);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiBridgeTests::RunBridge(DefinitionModelIds& defids, WCharCP bridgeName, MultiBridgeTestDocumentAccessor& docaccessor, bool isFirstTime, bool detectDeletedFiles)
    {
    // Copy m_dgnDbFileName into the bridge's own workdir - that is its briefcase
    BentleyApi::BeFileName briefcaseName = copyFileTo(m_dgnDbFileName, getSubdir(GetOutputDir(), bridgeName, isFirstTime));
    
    auto db = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    ASSERT_TRUE(db.IsValid());

    // The bridge runs its conversion, writing to its briefcase
    doConvert(defids, *db, m_v8FileName, m_params, docaccessor, bridgeName, !isFirstTime, detectDeletedFiles);
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

    m_params.SetMergeDefinitions(false);

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

#ifdef COMMENT_OUT // We are not converting saved views by default any more. If we want to re-enable this part of the test, we would have to use a special cfg file that turns on saved views.
    EXPECT_TRUE (isCodeValueInList(mViews, "MasterView"));
#endif
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

/*---------------------------------------------------------------------------------**//**
* This test verifies that multiple bridges will create a common set of definitions 
* such as Categories in the dictionary model with no dups, merging by name.
* @bsimethod                                    Sam.Wilson                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiBridgeTests::DoMergeDefinitions(bool mergeDefinitions)
    {
    LineUpFiles(WPrintfString(L"MergedDefinitions%d.bim", mergeDefinitions).c_str(), L"master3d.dgn", false);
    BentleyApi::BeFileName master1 = m_v8FileName;
    BentleyApi::BeFileName master2;
    MakeWritableCopyOf(master2, GetInputFileName(L"master3d.dgn"), L"master3d_2.dgn");
    
    BentleyApi::BeFileName ref1;
    MakeWritableCopyOf(ref1, L"ref3d.dgn");
    BentleyApi::BeFileName ref2;
    MakeWritableCopyOf(ref2, GetInputFileName(L"ref3d.dgn"), L"ref3d_2.dgn");

    MultiBridgeTestDocumentAccessor docaccessor;
    // m is assigned to the master files
    docaccessor.m_assignments[L"m"].push_back(m_v8FileName);
    docaccessor.m_assignments[L"m"].push_back(master2);
    // r is assigned to the reference files
    docaccessor.m_assignments[L"r"].push_back(ref1);
    docaccessor.m_assignments[L"r"].push_back(ref2);

    m_params.SetMergeDefinitions(mergeDefinitions);

    m_v8FileName = master1;
    DefinitionModelIds m1Defs, r1Defs;
    RunBridge(m1Defs, L"m", docaccessor, true);
    RunBridge(r1Defs, L"r", docaccessor, true);

    m_v8FileName = master2;
    DefinitionModelIds m2Defs, r2Defs;
    RunBridge(m2Defs, L"m", docaccessor, true);
    RunBridge(r2Defs, L"r", docaccessor, true);

    auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    ASSERT_TRUE(db.IsValid());

    std::vector<DgnElementCPtr> commonCats, m1Categories, r1Categories, m2Categories, r2Categories;
    getElementsInModel(commonCats,  *db, db->GetDictionaryModel().GetModelId(), "bis.Category");
    getElementsInModel(m1Categories, *db, m1Defs.m_definitionModelId, "bis.Category");
    getElementsInModel(r1Categories, *db, r1Defs.m_definitionModelId, "bis.Category");
    getElementsInModel(m2Categories, *db, m2Defs.m_definitionModelId, "bis.Category");
    getElementsInModel(r2Categories, *db, r2Defs.m_definitionModelId, "bis.Category");

    EXPECT_EQ(mergeDefinitions, isCodeValueInList(commonCats,  "MasterLevel"))   << "Levels should be merged by name into the dictionary model";
    EXPECT_EQ(!mergeDefinitions, isCodeValueInList(m1Categories, "MasterLevel"));
    EXPECT_EQ(false,             isCodeValueInList(r1Categories, "MasterLevel"));
    EXPECT_EQ(!mergeDefinitions, isCodeValueInList(m2Categories, "MasterLevel"));
    EXPECT_EQ(false,             isCodeValueInList(r2Categories, "MasterLevel"));
    EXPECT_EQ(mergeDefinitions, isCodeValueInList(commonCats,  "ReferenceLevel"))<< "Levels should be merged by name into the dictionary model";
    EXPECT_EQ(!mergeDefinitions, isCodeValueInList(r1Categories, "ReferenceLevel"));
    EXPECT_EQ(false,             isCodeValueInList(m2Categories, "ReferenceLevel"));
    EXPECT_EQ(!mergeDefinitions, isCodeValueInList(r2Categories, "ReferenceLevel"));
    EXPECT_EQ(false,             isCodeValueInList(m1Categories, "ReferenceLevel"));
    EXPECT_TRUE (isCodeValueInList(m1Categories, "Default")) << "Each bridge creates its own private Default Category";
    EXPECT_TRUE (isCodeValueInList(r1Categories, "Default")) << "Each bridge creates its own private Default Category";
    EXPECT_TRUE (isCodeValueInList(m2Categories, "Default")) << "Each bridge creates its own private Default Category";
    EXPECT_TRUE (isCodeValueInList(r2Categories, "Default")) << "Each bridge creates its own private Default Category";
    }

/*---------------------------------------------------------------------------------**//**
* This test verifies that multiple bridges will create a common set of definitions 
* such as Categories in the dictionary model with no dups, merging by name.
* @bsimethod                                    Sam.Wilson                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiBridgeTests, MergedDefinitions)
    {
    DoMergeDefinitions(true);
    DoMergeDefinitions(false);
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

static void getModelsInView(DgnModelIdSet& modelsInView, DgnDbR db, Utf8CP codeValue)
    {
    auto getViewSelStmt = db.GetPreparedECSqlStatement("SELECT ecinstanceid from bis.SpatialViewDefinition where CodeValue=?");
    getViewSelStmt->BindText(1, codeValue, BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(BE_SQLITE_ROW, getViewSelStmt->Step());
    auto viewDef = db.Elements().Get<SpatialViewDefinition>(getViewSelStmt->GetValueId<DgnElementId>(0));
    ASSERT_TRUE(viewDef.IsValid());
    auto modSel = db.Elements().Get<ModelSelector>(viewDef->GetModelSelectorId());
    ASSERT_TRUE(modSel.IsValid());
    modelsInView = modSel->GetModels();
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

    docaccessor.m_docProps[m_v8FileName] = iModelBridgeDocumentProperties("10000000-0000-0000-0000-000000000001", "wurnm", "durnm", "otherm", "");
    docaccessor.m_docProps[mRefFileName] = iModelBridgeDocumentProperties("20000000-0000-0000-0000-000000000002", "wurnmr", "durnmr", "othermr", "");
    docaccessor.m_docProps[rRefFileName] = iModelBridgeDocumentProperties("30000000-0000-0000-0000-000000000003", "wurnrr", "durnrr", "otherrr", "");

    DefinitionModelIds mDefs;
    RunBridge(mDefs, L"m", docaccessor, true);
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());

        // Having run only bridge 'm', we should have only models from files that are assigned to 'm'. 
        // No model from a file assigned to 'r' should be in the iModel.
        auto masterModels = getModelsByName(*db, "master"); ASSERT_EQ(1, masterModels.size()) << "No model (master) a file assigned to 'r' should be in the iModel";;
        auto rrefModels = getModelsByName(*db, "rref"); ASSERT_EQ(0, rrefModels.size()) << "No model (rref) from a file assigned to 'r' should be in the iModel";
        auto mrefModels = getModelsByName(*db, "mref"); ASSERT_EQ(1, mrefModels.size());

        ASSERT_EQ(2, countElementsInModel(*masterModels[0]));
        ASSERT_EQ(2, countElementsInModel(*mrefModels[0]));

        DgnModelIdSet modelsInView;
        getModelsInView(modelsInView, *db, "Default Views - View 1");
        ASSERT_EQ(modelsInView.size(), 2) << "No model (rref) from a file assigned to 'r' should be in the iModel and so should not be in the view.";
        }

    DefinitionModelIds rDefs;
    RunBridge(rDefs, L"r", docaccessor, true);
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());

        // Now that we have run both bridges, we should see all of the original models.
        auto masterModels = getModelsByName(*db, "master"); ASSERT_EQ(1, masterModels.size());
        auto rrefModels = getModelsByName(*db, "rref"); ASSERT_EQ(1, rrefModels.size());
        auto mrefModels = getModelsByName(*db, "mref"); ASSERT_EQ(1, mrefModels.size());

        ASSERT_EQ(2, countElementsInModel(*masterModels[0]));
        ASSERT_EQ(2, countElementsInModel(*rrefModels[0]));
        ASSERT_EQ(2, countElementsInModel(*mrefModels[0]));

            DgnModelIdSet modelsInView;
        getModelsInView(modelsInView, *db, "Default Views - View 1");
        ASSERT_EQ(modelsInView.size(), 2) << "Even though 'r' converted the model rref, since it did not also update the view, the view should still be lacking rref.";
        }

    // Run m one more time.
    RunBridge(mDefs, L"m", docaccessor, false);
    if (true)
        {
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());

        // The model and element counts should not have changed.
        auto masterModels = getModelsByName(*db, "master"); ASSERT_EQ(1, masterModels.size());
        auto rrefModels = getModelsByName(*db, "rref"); ASSERT_EQ(1, rrefModels.size());
        auto mrefModels = getModelsByName(*db, "mref"); ASSERT_EQ(1, mrefModels.size());

        ASSERT_EQ(2, countElementsInModel(*masterModels[0]));
        ASSERT_EQ(2, countElementsInModel(*rrefModels[0]));
        ASSERT_EQ(2, countElementsInModel(*mrefModels[0]));

        // m should have updated the view definition to include rref.
        DgnModelIdSet modelsInView;
        getModelsInView(modelsInView, *db, "Default Views - View 1");
        ASSERT_EQ(modelsInView.size(), 3) << "bridge 'm' should have update the view to include rref, since it was mapped into the iModel by bridge 'r'.";
        }
    }

/*---------------------------------------------------------------------------------**//**
* This test verifies that a bridge will delete only the models that it created
* when the master file goes away.
* @bsimethod                                    Sam.Wilson                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiBridgeTests, DetectDeletedModels)
    {
    LineUpFiles(L"MultiBridgeTest_DetectDeletedModels.bim", L"master3d.dgn", false); 
    
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

    docaccessor.m_docProps[m_v8FileName] = iModelBridgeDocumentProperties("10000000-0000-0000-0000-000000000001", "wurnm", "durnm", "otherm", "");
    docaccessor.m_docProps[mRefFileName] = iModelBridgeDocumentProperties("20000000-0000-0000-0000-000000000002", "wurnmr", "durnmr", "othermr", "");
    docaccessor.m_docProps[rRefFileName] = iModelBridgeDocumentProperties("30000000-0000-0000-0000-000000000003", "wurnrr", "durnrr", "otherrr", "");

    docaccessor.BuildGuidMap();

    //  Run both bridges. 
    DefinitionModelIds mDefs, rDefs;
    RunBridge(mDefs, L"m", docaccessor, true);
    RunBridge(rDefs, L"r", docaccessor, true);
    RunBridge(mDefs, L"m", docaccessor, false);

    if (true)
        {
        // The bridges should have created their respective models.
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());

        auto masterModels = getModelsByName(*db, "master"); ASSERT_EQ(1, masterModels.size());
        auto rrefModels = getModelsByName(*db, "rref"); ASSERT_EQ(1, rrefModels.size());
        auto mrefModels = getModelsByName(*db, "mref"); ASSERT_EQ(1, mrefModels.size());

        ASSERT_EQ(2, countElementsInModel(*masterModels[0]));
        ASSERT_EQ(2, countElementsInModel(*rrefModels[0]));
        ASSERT_EQ(2, countElementsInModel(*mrefModels[0]));
        }

    // Delete rref.dgn and mref.dgn and remove them from the registry. 
    BentleyApi::BeFileName::BeDeleteFile(rRefFileName.c_str());
    BentleyApi::BeFileName::BeDeleteFile(mRefFileName.c_str());
    ASSERT_TRUE(!rRefFileName.DoesPathExist());
    ASSERT_TRUE(!mRefFileName.DoesPathExist());
    docaccessor.m_docProps.erase(rRefFileName);
    docaccessor.m_docProps.erase(mRefFileName);

    // Run bridge "r"
    RunBridge(rDefs, L"r", docaccessor, false, true);

    if (true)
        {
        // R should have deleted "rref" and no other model.
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());

        auto masterModels = getModelsByName(*db, "master"); ASSERT_EQ(1, masterModels.size()) << "Master.dgn is still there. No bridge should have deleted its models.";
        auto rrefModels = getModelsByName(*db, "rref");     ASSERT_EQ(0, rrefModels.size()) << "Bridge R should notice that rref.dgn was deleted and delete the models from it.";
        auto mrefModels = getModelsByName(*db, "mref");     ASSERT_EQ(1, mrefModels.size()) << "Bridge R should not delete models from files not assigned to it";

        ASSERT_EQ(2, countElementsInModel(*masterModels[0]));
        // rref is gone
        ASSERT_EQ(2, countElementsInModel(*mrefModels[0]));
        }

    // Run bridge "m"
    RunBridge(rDefs, L"m", docaccessor, false, true);

    if (true)
        {
        // M should have deleted "mref"
        auto db = DgnDb::OpenDgnDb(nullptr, m_dgnDbFileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
        ASSERT_TRUE(db.IsValid());

        auto masterModels = getModelsByName(*db, "master"); ASSERT_EQ(1, masterModels.size()) << "Master.dgn is still there. No bridge should have deleted its models.";
        auto rrefModels = getModelsByName(*db, "rref");     ASSERT_EQ(0, rrefModels.size());
        auto mrefModels = getModelsByName(*db, "mref");     ASSERT_EQ(0, mrefModels.size()) << "Bridge M should notice that mref.dgn was deleted and delete the models from it.";

        ASSERT_EQ(2, countElementsInModel(*masterModels[0]));
        // rref is gone
        // mref is gone
        }
    }