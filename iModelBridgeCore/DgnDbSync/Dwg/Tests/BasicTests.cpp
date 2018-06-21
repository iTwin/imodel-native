/*--------------------------------------------------------------------------------------+
|
|  $Source: Dwg/Tests/BasicTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImporterBaseFixture.h"
#include "ImporterCommandBuilder.h"
#include "DwgFileEditor.h"

/*================================================================================**//**
* @bsiclass                                                     Umar Hayat      05/16
+===============+===============+===============+===============+===============+======*/
struct BasicTests : public ImporterTestBaseFixture
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId QueryElementId (DgnDbCR db, Utf8CP className, Utf8CP codeValue) const
    {
    auto classId= db.Schemas().GetClassId (GENERIC_DOMAIN_NAME, className);
    auto stmt = db.GetPreparedECSqlStatement ("SELECT ECInstanceId from " BIS_SCHEMA(BIS_CLASS_Element) " WHERE ECClassId=? AND CodeValue=?");
    stmt->BindId (1, classId);
    stmt->BindText (2, codeValue, BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    return BE_SQLITE_ROW == stmt->Step() ? stmt->GetValueId<DgnElementId>(0) : DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    FindElement (DwgDbHandleCR entityHandle, uint64_t dwgModelId, SyncInfoReader const& reader, bool shouldExist = true) const
    {
    DwgSyncInfo::DwgModelSyncInfoId infoId;
    reader.MustFindModelSyncInfo (infoId, dwgModelId, DwgSyncInfo::ModelSourceType::ModelSpace);
    DgnElementId    elementId;
    int count = shouldExist ? 1 : 0;
    reader.MustFindElementByDwgEntityHandle (elementId, infoId, entityHandle, count);
    return  elementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbHandle AddCircle (uint64_t& dwgModelId) const
    {
    DwgFileEditor   editor(m_dwgFileName);
    editor.AddCircleInDefaultModel ();

    // save off the entity handle
    DwgDbHandle entityHandle = editor.GetCurrentObjectId().GetHandle ();
    EXPECT_TRUE (!entityHandle.IsNull());

    dwgModelId = editor.GetModelspaceId().ToUInt64 ();

    editor.SaveFile ();
    return  entityHandle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DeleteEntity (DwgDbHandleCR entityHandle, uint64_t& modelspaceId) const
    {
    DwgFileEditor   editor(m_dwgFileName);
    editor.DeleteEntity (entityHandle);
    modelspaceId = editor.GetModelspaceId().ToUInt64 ();
    editor.SaveFile ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  CountImportedElements () const
    {
    auto db = OpenExistingDgnDb (m_dgnDbFileName, Db::OpenMode::Readonly);
    EXPECT_TRUE (db.IsValid());
    EXPECT_TRUE (db->IsDbOpen());

    size_t numElements = db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialElement)).BuildIdList<DgnElementId>().size ();
    EXPECT_EQ (numElements, GetCount());
    return  numElements;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckDbElement (size_t expectedCount, DwgDbHandleCR entityHandle, uint64_t dwgModelId, bool shouldExist) const
    {
    // check expected element count
    auto db = OpenExistingDgnDb (m_dgnDbFileName, Db::OpenMode::Readonly);
    EXPECT_EQ (expectedCount, db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialElement)).BuildIdList<DgnElementId>().size()) << "Spaitial element count in the DgnDb is incorrect.";

    // retreive element ID for the requested entity handle from the syncInfo:
    SyncInfoReader reader (m_dgnDbFileName);
    auto elementId = FindElement (entityHandle, dwgModelId, reader, shouldExist);
    
    // check the presence of the imported element
    EXPECT_EQ (shouldExist, elementId.IsValid()) << "The requested elememnt is not added/deleted as it should!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtractPlacementOrigins (DPoint3dArrayR origins, T_EntityHandles handles) const
    {
    // find imported GeometricElement3d's in db and return their placement origins:
    origins.clear ();

    auto db = OpenExistingDgnDb (m_dgnDbFileName, Db::OpenMode::Readonly);
    EXPECT_TRUE (db.IsValid());

    SyncInfoReader  reader(m_dgnDbFileName);
    DwgFileEditor   editor(m_dwgFileName);

    auto modelspaceId = editor.GetModelspaceId().ToUInt64 ();
    auto iter = db->Elements().MakeIterator (BIS_SCHEMA(BIS_CLASS_SpatialElement));
    for (auto& handle : handles)
        {
        auto elementId = FindElement (handle, modelspaceId, reader);
        for (auto& entry : iter)
            {
            auto element = db->Elements().GetElement (elementId);
            ASSERT_TRUE (element.IsValid());
            auto geom = element->ToGeometrySource3d ();
            ASSERT_NOT_NULL (geom);
            origins.push_back (geom->GetPlacement().GetOrigin());
            break;
            }
        }
    EXPECT_EQ (origins.size(), handles.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckDwgEntity (size_t expectedCount, DwgDbHandleCR entityHandle, bool shouldExist) const
    {
    DwgFileEditor   editor(m_dwgFileName);
    bool    foundEntity = false;
    size_t  numEntities = editor.CountAndCheckModelspaceEntity (foundEntity, entityHandle);
    EXPECT_EQ (expectedCount, numEntities) << "DWG modelspace has incorrect entity count!";
    EXPECT_EQ (shouldExist, foundEntity) << "The requested entity is not added/deleted in the DWG file!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void MoveEntitiesBy (T_EntityHandles const& handles, DPoint3dCR delta) const
    {
    DwgFileEditor editor (m_dwgFileName);
    editor.TransformEntitiesBy (handles, Transform::From(delta));
    editor.SaveFile ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckXrefAttached (BeFileNameCR masterFilename, DwgStringCR xrefBlockname)
    {
    // check xref instance in the modelspace
    DwgFileEditor   editor(masterFilename);
    editor.FindXrefInsert (xrefBlockname);

    auto id = editor.GetCurrentObjectId ();
    EXPECT_TRUE (id.IsValid()) << "Xref instance is not found in modelspace of the master file!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckXrefNested (BeFileNameCR masterFilename, DwgStringCR xrefBlockname)
    {
    /*-----------------------------------------------------------------------------------
    A nested xref has no instance in master file, so check the block only.

    When this unit test is run as a part of a full build, the test can fail if
        1) the unit test is run as a part of a full build,
        2) the master DWG file is opened for write.
    When this happens, the nested xref block is not seen in the block table in the master file.
    This may all be a RealDWG2018 bug, but we workaround it by opening the master file as read-only.
    -----------------------------------------------------------------------------------*/
    DwgFileEditor   editor(masterFilename, FileShareMode::DenyWrite);
    editor.FindXrefBlock (xrefBlockname);

    auto id = editor.GetCurrentObjectId ();
    EXPECT_TRUE (id.IsValid()) << "Nested xRef block is not found in the master file!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckDefaultView (Utf8StringCR expectedName)
    {
    auto db = OpenExistingDgnDb (m_dgnDbFileName);
    ASSERT_TRUE (db.IsValid());

    auto viewId = ViewDefinition::QueryDefaultViewId (*db);
    EXPECT_TRUE (viewId.IsValid()) << "Default view is not saved!";
    auto view = ViewDefinition::Get (*db, viewId);
    ASSERT_TRUE (view.IsValid()) << "Failed getting the default view!";
    auto name = view->GetName ();
    EXPECT_EQ (name, expectedName) << "Wrong default view name!";
    }
};  // BasicTests

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicTests, CreateIBimFromDwg)
    {
    LineUpFiles(L"createIBim.ibim", L"basictype.dwg", true); 
    EXPECT_EQ (5, GetCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicTests, UpdateElements_AddDelete)
    {
    LineUpFiles(L"addDeleteTest.ibim", L"basictype.dwg", true); 
    // imported 5 elements from 5 entities?
    size_t  numElements = CountImportedElements ();

    // add a circle in DWG file
    uint64_t    modelspaceId = 0;
    DwgDbHandle entityHandle = AddCircle (modelspaceId);

    // update DgnDb
    DoUpdate (m_dgnDbFileName, m_dwgFileName);
    // exactly 1 element imported?
    EXPECT_EQ (1, GetCount());
    CheckDbElement (numElements + 1, entityHandle, modelspaceId, true);

    // delete the circle from the DWG file
    DeleteEntity (entityHandle, modelspaceId);

    // update DgnDb again
    DoUpdate (m_dgnDbFileName, m_dwgFileName);
    // none imported?
    EXPECT_EQ (0, GetCount());
    /*-----------------------------------------------------------------------------------
    At this time, tests run through TMR, and only TMR, failed on reading in the updated DgnDb!
    The old file prior to update was read in! Opening/closing to workaround it!
    For time being, skip this check for PRG buids.
    -----------------------------------------------------------------------------------*/
    if (true)
        OpenExistingDgnDb (m_dgnDbFileName, Db::OpenMode::Readonly);
    CheckDwgEntity (numElements, entityHandle, false);
#ifndef PRG
    CheckDbElement (numElements, entityHandle, modelspaceId, false);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicTests, UpdateElements_DeleteMove)
    {
    LineUpFiles(L"deleteMoveTest.ibim", L"basictype.dwg", true); 

    // will edit these entities in file basictype.dwg:
    T_EntityHandles handles;
    handles.push_back (DwgDbHandle(0x183));    // the yellow line, to be deleted
    handles.push_back (DwgDbHandle(0x182));    // the red circle
    handles.push_back (DwgDbHandle(0x184));    // the cyan arc
    handles.push_back (DwgDbHandle(0x187));    // the green rectangle
    handles.push_back (DwgDbHandle(0x186));    // the pink pline

    DPoint3dArray   origins;
    ExtractPlacementOrigins (origins, handles);

    // delete the last element from db and from our list as well:
    auto deleteHandle = handles.back ();
    uint64_t    modelspaceId = 0;
    DeleteEntity (deleteHandle, modelspaceId);

    // move remaining entities to a new location, in DWG units:
    auto delta = DPoint3d::From (20.0, 30.0, 5.0);
    MoveEntitiesBy (handles, delta);

    // update DgnDb
    DoUpdate (m_dgnDbFileName, m_dwgFileName);
    // exactly 4 elements imported?
    EXPECT_EQ (4, GetCount());

    // prepare to check moved elements - hopefully enough to have file flushed before re-opened at next step:
    delta.Scale (GetScaleDwgToMeters());
    handles.pop_back ();
    origins.pop_back ();
    size_t  checkCount = 0;

    auto db = OpenExistingDgnDb (m_dgnDbFileName, Db::OpenMode::Readonly);
    ASSERT_TRUE (db.IsValid());
    EXPECT_TRUE (db->IsDbOpen());

    // is the deleted element in the DgnDb
    SyncInfoReader  reader (m_dgnDbFileName);
    auto id = FindElement (deleteHandle, modelspaceId, reader, false);
    EXPECT_FALSE (id.IsValid());

    // check element placement origins, in Meters
    auto iter = db->Elements().MakeIterator (BIS_SCHEMA(BIS_CLASS_SpatialElement));
    for (auto& handle : handles)
        {
        id = FindElement (handle, modelspaceId, reader);
        for (auto& entry : iter)
            {
            if (entry.GetElementId() == id)
                {
                auto element = db->Elements().GetElement (id);
                ASSERT_TRUE (element.IsValid()) << "Invalid DgnElementCPtr!";
                auto geom = element->ToGeometrySource3d ();
                ASSERT_NOT_NULL (geom) << "Not an element of GeometricElement3d!";
                auto origin = geom->GetPlacement().GetOrigin ();
                origin.Subtract (delta);
                EXPECT_TRUE (origin.AlmostEqual(origins[checkCount++], 0.01)) << "Element is not moved to the right location!";
                EXPECT_LE (checkCount, origins.size()) << "Element count to be checked is not the same.";
                }
            }
        }
    EXPECT_EQ (checkCount, origins.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicTests, CreateNewDwg)
    {
    LineUpFilesForNewDwg(L"createNewDwgTest.ibim", L"createdfromcratch.dwg");
    InitializeImporterOptions (m_dwgFileName, false);
    
    DwgImporter*    importer = new DwgImporter(m_options);
    ASSERT_NOT_NULL(importer);

    DwgFileEditor   editor;
    editor.CreateFile (m_dwgFileName);
    T_EntityHandles handles;
    editor.AddEntitiesInDefaultModel (handles);
    editor.SaveFile ();

    DoConvert (importer, m_dgnDbFileName, m_dwgFileName);

    EXPECT_EQ (handles.size(), GetCount()) << "Entities created in DWG file do not match elements imported in the DgnDb!";

    delete importer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicTests, AttachXrefs)
    {
    LineUpFilesForNewDwg(L"xrefAttachTest.ibim", L"master.dwg");
    InitializeImporterOptions (m_dwgFileName, false);

    // create a host for DwgFileEditor
    DwgImporter*    importer = new DwgImporter(m_options);
    ASSERT_NOT_NULL (importer);

    // prepare basictype.dwg for a nested xref:
    BentleyApi::BeFileName  nestedXrefName;
    ImporterTests::MakeWritableCopyOf(nestedXrefName, L"basictype.dwg");
    EXPECT_PRESENT (nestedXrefName.c_str());

    auto xrefFileName = GetOutputFileName(L"xref1.dwg");

    // create xref1.dwg and attach basictype.dwg into the modelspace:
    DwgFileEditor   editor;
    editor.CreateFile (xrefFileName);
    editor.AttachXrefInDefaultModel (nestedXrefName, DPoint3d::From(1.5,1.5,0.0), 0.7853);
    editor.SaveFile ();
    CheckXrefAttached (xrefFileName, L"basictype");

    // create master.dwg and attach xref1.dwg into the modelspace:
    editor.CreateFile (m_dwgFileName);
    editor.AttachXrefInDefaultModel (xrefFileName, DPoint3d::From(-1.5,-1.5,0.0), 0.7853);
    editor.SaveFile ();
    CheckXrefAttached (m_dwgFileName, L"xref1");
    CheckXrefNested (m_dwgFileName, L"basictype");

    DoConvert (importer, m_dgnDbFileName, m_dwgFileName);
    delete importer;

    auto db = OpenExistingDgnDb (m_dgnDbFileName);
    ASSERT_TRUE (db.IsValid());
    auto& models = db->Models ();

    // check results - expect 3 PhysicalModel's
    size_t  count = 0;
    bool    hasXref1 = false, hasNested = false;
    for (auto const& entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel)))
        {
        auto model = models.GetModel (entry.GetModelId());
        ASSERT_TRUE(model.IsValid());
        auto name = model->GetName ();
        if (name.StartsWith("basictype"))
            hasNested = true;
        else if (name.StartsWith("xref1"))
            hasXref1 = true;
        count++;
        }
    EXPECT_EQ (3, count) << "Should have total 3 physical models!";
    EXPECT_TRUE (hasNested) << "Missing nested xRef basictype";
    EXPECT_TRUE (hasXref1) << "Missing direct xRef xref1!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicTests, ChangeAndActivateLayout)
    {
    LineUpFiles(L"testlayout.ibim", L"basictype.dwg", true); 
    CheckDefaultView ("Model[basictype]");

    // create a host for DwgFileEditor as well as updating db:
    InitializeImporterOptions (m_dwgFileName, true);
    DwgImporter*    updater = new DwgImporter(m_options);
    ASSERT_NOT_NULL (updater);

    // rename Layout2 as "test layout" and activate it in source DWG file:
    DwgFileEditor   editor(m_dwgFileName);
    editor.RenameAndActivateLayout (L"Layout2", L"test layout");
    editor.SaveFile ();

    DoUpdate (updater, m_dgnDbFileName, m_dwgFileName);

    delete updater;

    // output sheet model/view should be renamed:
    CheckDefaultView ("test layout[basictype]");
    }
