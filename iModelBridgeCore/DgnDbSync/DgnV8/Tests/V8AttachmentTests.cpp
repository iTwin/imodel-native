/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/V8AttachmentTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include "ImportConfigEditor.h"
#include <Bentley/BeTest.h>
#include <VersionedDgnV8Api/ECObjects/ECObjectsAPI.h>

#define TESTMODELNEW   "TestModelNew"
#define TESTMODELNEWW L"TestModelNew"

#define JustWaitHere system("pause");

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct V8AttachmentTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);
    void AttachForeignReferenceFile (WCharCP foreignFilename);
    void CheckForeignReferenceOutput (int numElementsInRef);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, AttachSameFile)
    {
    LineUpFiles(L"SameFile.ibim", L"Test3d.dgn", false);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    size_t originalModelCount = 0;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        originalModelCount = db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)).BuildIdSet().size();
        }

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    Bentley::DgnModelP newModel = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ true);
    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid, newModel);
    v8editor.AddLine(&eid);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    //  Add refV8File as an attachment to v8File
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
    DgnV8Api::DgnAttachment* attachment;
    ASSERT_EQ(BentleyApi::SUCCESS, v8editor.m_defaultModel->CreateDgnAttachment(attachment, *moniker, TESTMODELNEWW));
    ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalModelCount + 1, db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)).BuildIdSet().size());
        auto refsSubject = GetReferencesChildSubjectOf(*GetJobHierarchySubject(*db));
        ASSERT_TRUE(refsSubject.IsValid());
        DgnCode partitionCode = InformationPartitionElement::CreateCode(*refsSubject, TESTMODELNEW);
        DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
        ASSERT_TRUE(modelId.IsValid()) << "Model is direct reference , It should be imported";
        }

    BentleyApi::BeTest::SetFailOnAssert(false); // we expect an assertion failure in the V8 code (DgnCacheTxn::_SaveCustomEntryInUndo) because we are not running in an undoable V8 txn
    ASSERT_EQ(BentleyApi::SUCCESS, v8editor.m_defaultModel->DeleteDgnAttachment(attachment));
    BentleyApi::BeTest::SetFailOnAssert(true);
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalModelCount, db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)).BuildIdSet().size());
        auto refsSubject = GetReferencesChildSubjectOf(*GetJobHierarchySubject(*db));
        ASSERT_TRUE(refsSubject.IsValid());
        DgnCode partitionCode = InformationPartitionElement::CreateCode(*refsSubject, TESTMODELNEW);
        DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
        ASSERT_TRUE(!modelId.IsValid()) << "Model is direct reference , It should be deleted";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, AttachDifferentFile)
    {
    LineUpFiles(L"DifferentFile.ibim", L"Test3d.dgn", false);

    DoConvert(m_dgnDbFileName, m_v8FileName);
    size_t originalModelCount = 0;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        originalModelCount = db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)).BuildIdSet().size();
        }

    //--------------------------------------------------------------------------------------------------------
    // Added attachement from different file and verify update operation
    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalModelCount + 1, db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)).BuildIdSet().size());
        }

    //--------------------------------------------------------------------------------------------------------
    // Delete all attachements and verify update operation
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    for (DgnAttachmentP attachment : *v8editor.m_defaultModel->GetDgnAttachmentsP())
        {
        BentleyApi::BeTest::SetFailOnAssert(false); // we expect an assertion failure in the V8 code (DgnCacheTxn::_SaveCustomEntryInUndo) because we are not running in an undoable V8 txn
        ASSERT_EQ(BentleyApi::SUCCESS, v8editor.m_defaultModel->DeleteDgnAttachment(attachment));
        BentleyApi::BeTest::SetFailOnAssert(true);
        }
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalModelCount + 1, db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)).BuildIdSet().size()) << "The converter does NOT delete the attached model.";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, ElementUpdateInReferenceFile)
    {
    LineUpFiles(L"DifferentFile.ibim", L"Test3d.dgn", true);
    int originalGeomModelCount = 0;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        originalGeomModelCount = CountGeometricModels(*db);
        }

    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalGeomModelCount + 1, CountGeometricModels(*db));
        }

    //--------------------------------------------------------------------------------------------------------
    // Update elements in attachement and verify update operation
    V8FileEditor v8editor;
    v8editor.Open(refV8File);
    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalGeomModelCount + 1, CountGeometricModels(*db));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, AddAttachments)
    {
    LineUpFiles(L"DifferentFile.ibim", L"Test3d.dgn", true);
    int originalGeomModelCount = 0;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        originalGeomModelCount = CountGeometricModels(*db);
        }

    BentleyApi::BeFileName refV8File1;
    CreateAndAddV8Attachment(refV8File1, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalGeomModelCount + 1, CountGeometricModels(*db));
        }

    BentleyApi::BeFileName refV8File2;
    CreateAndAddV8Attachment(refV8File2, 2);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalGeomModelCount + 2, CountGeometricModels(*db));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, DeleteAttachment)
    {
    LineUpFiles(L"DifferentFile.ibim", L"Test3d.dgn", true);
    int originalGeomModelCount = 0;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        originalGeomModelCount = CountGeometricModels(*db);
        }

    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalGeomModelCount + 1, CountGeometricModels(*db));
        }

    //--------------------------------------------------------------------------------------------------------
    // Delete Attachement
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        Bentley::DgnAttachmentArrayP   attachements = v8editor.m_defaultModel->GetDgnAttachmentsP();
        ASSERT_TRUE(nullptr != attachements);
        ASSERT_TRUE(1 == attachements->size());
        Bentley::DgnAttachmentP firstAttachment = attachements->at(0);
        BentleyApi::BeTest::SetFailOnAssert(false); // V8 DeleteDgnAttachment asserts when undo is not enabled. Ignore that.
        EXPECT_TRUE(SUCCESS == v8editor.m_defaultModel->DeleteDgnAttachment(firstAttachment));
        BentleyApi::BeTest::SetFailOnAssert(true);
        v8editor.Save();
        }

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalGeomModelCount + 1, CountGeometricModels(*db)) << "The converter does NOT delete the attached model.";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, UnDisplayedReference)
    {
    LineUpFiles(L"DifferentFile.ibim", L"Test3d.dgn", true);
    int originalGeomModelCount = 0;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        originalGeomModelCount = CountGeometricModels(*db);
        }

    DgnV8Api::ElementId eid;
    {
    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

    V8FileEditor v8editor;
    v8editor.Open(refV8File);
    v8editor.AddLine(&eid);
    }

    //--------------------------------------------------------------------------------------------------------
    // Set Attachement Display = False
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        Bentley::DgnAttachmentArrayP   attachements = v8editor.m_defaultModel->GetDgnAttachmentsP();
        ASSERT_TRUE(nullptr != attachements);
        ASSERT_TRUE(1 == attachements->size());
        Bentley::DgnAttachmentP firstAttachment = attachements->at(0);
        firstAttachment->SetIsDisplayed(false);
        firstAttachment->WriteToModel();
        v8editor.Save();
        }

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalGeomModelCount, CountGeometricModels(*db));
        }

    //--------------------------------------------------------------------------------------------------------
    // Set Attachement Display =  True
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        Bentley::DgnAttachmentArrayP   attachements = v8editor.m_defaultModel->GetDgnAttachmentsP();
        ASSERT_TRUE(nullptr != attachements);
        ASSERT_TRUE(1 == attachements->size());
        Bentley::DgnAttachmentP firstAttachment = attachements->at(0);
        firstAttachment->SetIsDisplayed(true);
        firstAttachment->WriteToModel();
        v8editor.Save();
        }

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        EXPECT_EQ(originalGeomModelCount + 1, CountGeometricModels(*db));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void V8AttachmentTests::AttachForeignReferenceFile (WCharCP foreignFilename)
    {
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    BentleyApi::BeFileName inputFilename = GetInputFileName(foreignFilename);
    BentleyApi::BeFileName outputFilename = GetOutputFileName(foreignFilename);

    // Copy seed 3D V8DGN file
    MakeWritableCopyOf(outputFilename, inputFilename, inputFilename.GetFileNameAndExtension().c_str());

    // Put a line in the default
    Bentley::DgnModelP model = v8editor.m_defaultModel;
    ASSERT_TRUE(model->Is3d());
    DgnV8Api::ElementId id;
    v8editor.AddLine(&id, model);

    // Attach the input file of a foreign file format as a reference to the default model:
    DgnV8Api::DgnAttachment* attachment = nullptr;
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(outputFilename.c_str());
    ASSERT_EQ( BentleyApi::SUCCESS, model->CreateDgnAttachment(attachment, *moniker, L"Default", true));
    ASSERT_TRUE(nullptr != attachment);        
    ASSERT_EQ( BentleyApi::SUCCESS, attachment->WriteToModel());

    v8editor.Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void V8AttachmentTests::CheckForeignReferenceOutput (int expectedElements)
    {
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    // Should have the default, WebMercator, and the attachment
    countModels(*db, 0, 3);

    // Should have 1 of each of these elements:
    countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
    countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 1);
    countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle3d), 1);
    countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

    BentleyApi::Bstdcxx::bvector<DgnModelId>idlist;
    idlist = db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel), nullptr, "ORDER BY ECInstanceId ASC").BuildIdList();
    ASSERT_EQ(2, idlist.size());
    auto physical = db->Models().GetModel(idlist[0]);
    ASSERT_TRUE(physical.IsValid());

    // Should have 1 line element in the default model:
    countElements(*physical, 1);
    auto refmodel = db->Models().GetModel(idlist[1]);
    ASSERT_TRUE(refmodel.IsValid());

    // Count elements in the reference model converted from the foreign file format:
    countElements(*refmodel, expectedElements);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, Attach3dm)
    {
    LineUpFiles(L"rhino.ibim", L"Test3d.dgn", false);
    AttachForeignReferenceFile (L"HumanHead.3dm");
    DoConvert(m_dgnDbFileName, m_v8FileName); 
    CheckForeignReferenceOutput (83);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, Attach3ds)
    {
    LineUpFiles(L"3ds.ibim", L"Test3d.dgn", false);
    ASSERT_EQ( 0 , m_count ) << L"Expect an empty seed file!";
    AttachForeignReferenceFile (L"bed.3ds");
    DoConvert(m_dgnDbFileName, m_v8FileName); 
    CheckForeignReferenceOutput (13);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, AttachFbx)
    {
    LineUpFiles(L"fbx.ibim", L"Test3d.dgn", false);
    ASSERT_EQ( 0 , m_count ) << L"Expect an empty seed file!";
    AttachForeignReferenceFile (L"candles.fbx");
    DoConvert(m_dgnDbFileName, m_v8FileName); 
    CheckForeignReferenceOutput (103);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, AttachIfc)
    {
    LineUpFiles(L"ifc.ibim", L"Test3d.dgn", false);
    ASSERT_EQ( 0 , m_count ) << L"Expect an empty seed file!";
    AttachForeignReferenceFile (L"roof.ifc");
    DoConvert(m_dgnDbFileName, m_v8FileName); 
    CheckForeignReferenceOutput (846);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, AttachObj)
    {
    LineUpFiles(L"obj.ibim", L"Test3d.dgn", false);
    ASSERT_EQ( 0 , m_count ) << L"Expect an empty seed file!";
    AttachForeignReferenceFile (L"bottle.obj");
    DoConvert(m_dgnDbFileName, m_v8FileName); 
    // a mesh in a cell counts as 2 elements
    CheckForeignReferenceOutput (2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(V8AttachmentTests, AttachSkp)
    {
    LineUpFiles(L"sketchup.ibim", L"Test3d.dgn", false);
    ASSERT_EQ( 0 , m_count ) << L"Expect an empty seed file!";
    AttachForeignReferenceFile (L"Kubelis.skp");
    DoConvert(m_dgnDbFileName, m_v8FileName); 
    CheckForeignReferenceOutput (2);
    }

