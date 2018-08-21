/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Tests/DwgFileEditor.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DwgFileEditor.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DGNDBSYNC_DWG
USING_NAMESPACE_DWGDB

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgFileEditor::DwgFileEditor (BeFileNameCR infile, FileShareMode openMode)
    {
    // check if a DwgImporter has been instantiated.
    BeAssert (DwgImportHost::GetHost()._IsValid());
    OpenFile (infile, openMode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgFileEditor::CreateFile (BeFileNameCR infile)
    {
    if (infile.DoesPathExist());
        infile.BeDeleteFile ();
    m_dwgdb = new DwgDbDatabase (true, true);
    ASSERT_FALSE (m_dwgdb.IsNull());
    DwgImportHost::SetWorkingDatabase (m_dwgdb.get());
    m_fileName = infile;
    m_openMode = FileShareMode::DenyNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgFileEditor::OpenFile (BeFileNameCR infile, FileShareMode openMode)
    {
    EXPECT_PRESENT (infile.c_str());

    m_openMode = openMode;
    m_dwgdb = DwgImportHost::GetHost().ReadFile (infile, false, false, m_openMode);
    ASSERT_TRUE (m_dwgdb.IsValid());

    DwgImportHost::SetWorkingDatabase (m_dwgdb.get());
    m_fileName = infile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgFileEditor::SaveFile ()
    {
    ASSERT_TRUE (m_dwgdb.IsValid());

    BeFileName  originalFile (m_dwgdb->GetFileName().c_str());
    if (originalFile.empty())
        {
        // saving DWG to a new file:
        m_fileName.BeDeleteFile ();

        DwgDbStatus status = m_dwgdb->SaveAs (m_fileName.c_str());
        EXPECT_DWGDBSUCCESS (status);
        }
    else
        {
        // saving DWG back into an existing file, but only when the file was opened for write!
        ASSERT_GT (m_openMode, FileShareMode::DenyWrite) << "DWG file was not opened for write!";

        BeFileName  tempFile = originalFile;
        tempFile.AppendExtension (L"tmp");

        DwgDbStatus status = m_dwgdb->SaveAs (tempFile.c_str());
        EXPECT_DWGDBSUCCESS (status);

        ASSERT_EQ (originalFile.BeDeleteFile(), BeFileNameStatus::Success);
        ASSERT_EQ (BeFileName::BeMoveFile(tempFile, originalFile), BeFileNameStatus::Success);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgFileEditor::AddCircleInDefaultModel ()
    {
    ASSERT_TRUE (m_dwgdb.IsValid());

    // create a new circle
    DwgDbCirclePtr  circle = DwgDbCircle::Create () ;
    ASSERT_FALSE (circle.IsNull()) << "Circle cannot be created!";

    ASSERT_DWGDBSUCCESS (circle->SetCenter(DPoint3d::From(10.0, 13.0, 0.0)));
    ASSERT_DWGDBSUCCESS (circle->SetRadius(3.5));

    // open modelspace for write
    DwgDbBlockTableRecordPtr    modelspace (m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS (modelspace.OpenStatus()) << "Modelspace block cannot be opened for write";
    
    // add the circle into modelspace
    AppendEntity (DwgDbEntity::Cast(circle.get()), modelspace.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgFileEditor::DeleteEntity (DwgDbHandleCR entityHandle)
    {
    ASSERT_TRUE (DwgImportHost::GetHost()._IsValid());
    ASSERT_TRUE (m_dwgdb.IsValid());
    ASSERT_FALSE (entityHandle.IsNull());

    m_currentObjectId = m_dwgdb->GetObjectId (entityHandle);
    EXPECT_TRUE (m_currentObjectId.IsValid()) << "Input entity handle not found in DWG!";

    DwgDbEntityPtr  entity (m_currentObjectId, DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS (entity.OpenStatus()) << "Cannot open entity for write!";

    if (DwgDbStatus::Success == entity.OpenStatus())
        ASSERT_DWGDBSUCCESS (entity->Erase()) << "Requested entity cannot be deleted from DWG file!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectIdCR DwgFileEditor::GetCurrentObjectId () const
    {
    return  m_currentObjectId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   DwgFileEditor::GetModelspaceId () const
    {
    return  m_dwgdb.IsValid() ? m_dwgdb->GetModelspaceId() : DwgDbObjectId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgFileEditor::TransformEntitiesBy (T_EntityHandles const& handles, TransformCR transform)
    {
    ASSERT_TRUE (m_dwgdb.IsValid());

    DwgDbBlockTableRecordPtr modelspace (m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS (modelspace.OpenStatus()) << "Modelspace block cannot be opened for write";
    
    auto iter = modelspace->GetBlockChildIterator ();
    for (iter.Start(); !iter.Done(); iter.Step())
        {
        auto found = std::find_if (handles.begin(), handles.end(), [&](DwgDbHandleCR h) { return iter.GetEntityId().GetHandle()==h; });
        if (found != handles.end())
            {
            DwgDbEntityPtr  entity(iter.GetEntityId(), DwgDbOpenMode::ForWrite);
            ASSERT_DWGDBSUCCESS (entity.OpenStatus()) << "An entity cannot be opened for write!";
            ASSERT_DWGDBSUCCESS (entity->TransformBy(transform)) << "Entity is not transformed!";
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  DwgFileEditor::CountAndCheckModelspaceEntity (bool& found, DwgDbHandleCR entityHandle) const
    {
    size_t  count = 0;
    found = false;
    DwgDbBlockTableRecordPtr modelspace (m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForRead);
    if (modelspace.OpenStatus() == DwgDbStatus::Success)
        {
        auto iter = modelspace->GetBlockChildIterator ();
        for (iter.Start(); !iter.Done(); iter.Step())
            {
            if (iter.GetEntityId().GetHandle() == entityHandle)
                found = true;
            count++;
            }
        }
    return  count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgFileEditor::AppendEntity (DwgDbEntityP entity, DwgDbBlockTableRecordP block, bool closeEntity)
    {
    ASSERT_NOT_NULL (entity);
    ASSERT_NOT_NULL (block);
    m_currentObjectId = block->AppendEntity (*entity);
    ASSERT_TRUE (m_currentObjectId.IsValid()) << "Entity cannot be added in block!";
    if (closeEntity)
        ASSERT_DWGDBSUCCESS (entity->Close()) << "Entity cannot be closed!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgFileEditor::AddEntitiesInDefaultModel (T_EntityHandles& handles)
    {
    ASSERT_TRUE (m_dwgdb.IsValid());
    handles.clear ();

    // open modelspace for write
    DwgDbBlockTableRecordPtr    modelspace (m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS (modelspace.OpenStatus()) << "Modelspace block cannot be opened for write";
    
    // Create a line
    DwgDbLinePtr    line = DwgDbLine::Create ();
    ASSERT_FALSE (line.IsNull());
    ASSERT_DWGDBSUCCESS (line->SetStartPoint(DPoint3d::From(0.0, 0.0, 0.0)));
    ASSERT_DWGDBSUCCESS (line->SetEndPoint(DPoint3d::From(4.0, 4.0, 5.0)));
    ASSERT_DWGDBSUCCESS (line->SetColorIndex(5));
    ASSERT_DWGDBSUCCESS (line->SetLineweight(DwgDbLineWeight::Weight015));
    AppendEntity (DwgDbEntity::Cast(line.get()), modelspace.get());
    handles.push_back (m_currentObjectId.GetHandle());

    // Create a light weight polyline of 3 points
    DwgDbPolylinePtr    pline = DwgDbPolyline::Create ();
    ASSERT_FALSE (pline.IsNull());
    ASSERT_DWGDBSUCCESS (pline->AddVertexAt(0, DPoint2d::From(1.0, 0.0), 0.0, 0.1, 0.2));
    ASSERT_DWGDBSUCCESS (pline->AddVertexAt(1, DPoint2d::From(2.0, 2.0), -0.5, 0.4, 0.3));
    ASSERT_DWGDBSUCCESS (pline->AddVertexAt(2, DPoint2d::From(4.0, 0.0), 0.0, 0.3, 0.0));
    pline->SetElevation (1.5);
    ASSERT_DWGDBSUCCESS (pline->SetThickness(2.0));
    ASSERT_DWGDBSUCCESS (pline->SetColorIndex(4));
    AppendEntity (DwgDbEntity::Cast(pline.get()), modelspace.get());
    handles.push_back (m_currentObjectId.GetHandle());

    // Create a paperspace viewport - must be made a db resident before setting data
    DwgDbViewportPtr    viewport = DwgDbViewport::Create ();
    ASSERT_FALSE (viewport.IsNull());

    DwgDbBlockTableRecordPtr    paperspace0 (m_dwgdb->GetPaperspaceId(), DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS (paperspace0.OpenStatus()) << "Paperspace block cannot be opened for write";
    AppendEntity (DwgDbEntity::Cast(viewport.get()), paperspace0.get(), false);

    ASSERT_DWGDBSUCCESS (viewport->SetWidth(2.0));
    ASSERT_DWGDBSUCCESS (viewport->SetHeight(1.0));
    ASSERT_DWGDBSUCCESS (viewport->SetCenterPoint(DPoint3d::From(5.0,3.5)));
    ASSERT_DWGDBSUCCESS (viewport->SetIsOn(true));
    ASSERT_DWGDBSUCCESS (viewport->SetAnnotationScale(0.5));
    ASSERT_DWGDBSUCCESS (viewport->SetCustomScale(0.5));
    ASSERT_DWGDBSUCCESS (viewport->EnableGrid(true));
    ASSERT_DWGDBSUCCESS (viewport->EnableUcsIcon(true));
    ASSERT_DWGDBSUCCESS (viewport->SetColorIndex(2));
    viewport->Close ();    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgFileEditor::AttachXrefInDefaultModel (BeFileNameCR infile, DPoint3dCR origin, double angle)
    {
    ASSERT_PRESENT (infile);
    // use base file name as block name
    DwgString blockName (infile.GetFileNameWithoutExtension().c_str());

    // ensure VISRETAIN is on, such that layers from the xRef file will be added into the master file
    ASSERT_DWGDBSUCCESS (m_dwgdb->SetVISRETAIN(true));

    // create an xref block
    DwgDbObjectId   xrefId = m_dwgdb->CreateXrefBlock (infile.c_str(), blockName);
    ASSERT_TRUE (xrefId.IsValid()) << "Given DWG cannot be attached as an xRef!";

    // create an instance
    DwgDbBlockReferencePtr  insert = DwgDbBlockReference::Create ();
    ASSERT_TRUE (!insert.IsNull()) << "Failed creating a block reference for the xref!";
    ASSERT_DWGDBSUCCESS (insert->SetPosition(origin)) << "Failed setting xref insert's position!";
    ASSERT_DWGDBSUCCESS (insert->SetRotation(angle)) << "Failed setting xref insert's rotation!";
    ASSERT_DWGDBSUCCESS (insert->SetBlockTableRecord(xrefId)) << "Failed setting xref insert's block table record ID!";

    // append the insert entity in the modelspace
    DwgDbBlockTableRecordPtr    modelspace(m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS (modelspace.OpenStatus()) << "Unable to open master file's modelspace block!";
    this->AppendEntity (DwgDbEntity::Cast(insert), modelspace, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgFileEditor::FindXrefInsert (DwgStringCR blockName)
    {
    DwgDbBlockTableRecordPtr    modelspace(m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForRead);
    ASSERT_DWGDBSUCCESS (modelspace.OpenStatus()) << "Unable to open DWG file's modelspace block!";
    auto iter = modelspace->GetBlockChildIterator ();
    ASSERT_TRUE (iter.IsValid());

    m_currentObjectId.SetNull ();

    for (iter.Start(); !iter.Done(); iter.Step())
        {
        DwgDbBlockReferencePtr   insert(iter.GetEntityId(), DwgDbOpenMode::ForRead);
        if (insert.OpenStatus() != DwgDbStatus::Success)
            continue;
        DwgDbBlockTableRecordPtr    block(insert->GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
        ASSERT_DWGDBSUCCESS (block.OpenStatus()) << "Unable to xRef block!";
        EXPECT_PRESENT (block->GetPath().c_str());

        DwgDbXrefStatus status = block->GetXrefStatus ();
        EXPECT_TRUE (status==DwgDbXrefStatus::Resolved || status==DwgDbXrefStatus::Unresolved) << "An invalid xRef block!";
        EXPECT_FALSE (block->IsAnonymous()) << "An xRef block should not be anonymous!";
        if (block->IsExternalReference() && block->GetName().EqualsI(blockName.c_str()))
            {
            m_currentObjectId = insert->GetObjectId ();
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgFileEditor::FindXrefBlock (DwgStringCR blockName)
    {
    m_currentObjectId.SetNull ();

    DwgDbBlockTablePtr  blockTable(m_dwgdb->GetBlockTableId(), DwgDbOpenMode::ForRead);
    ASSERT_DWGDBSUCCESS (blockTable.OpenStatus()) << "Unable to open DWG's block table!";

    m_currentObjectId = blockTable->GetByName (blockName.c_str(), false);
    if (m_currentObjectId.IsValid())
        {
        DwgDbBlockTableRecordPtr    block(m_currentObjectId, DwgDbOpenMode::ForRead);
        ASSERT_DWGDBSUCCESS (block.OpenStatus()) << "Unable to open nested xRef block!";
        EXPECT_PRESENT (block->GetPath().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgFileEditor::RenameAndActivateLayout (DwgStringCR oldName, DwgStringCR newName)
    {
    DwgDbLayoutManagerPtr   manager = DwgImportHost::GetHost().GetLayoutManager ();
    ASSERT_TRUE (manager.IsValid()) << "Unable to instantiate DwgDbLayoutManager!";

    ASSERT_DWGDBSUCCESS (manager->RenameLayout(oldName, newName, m_dwgdb.get())) << "Unable to name a layout!";

    m_currentObjectId = manager->FindLayoutByName (newName, m_dwgdb.get());
    ASSERT_TRUE (m_currentObjectId.IsValid()) << "Cannot find layout from given name!";
    ASSERT_DWGDBSUCCESS (manager->ActivateLayout(m_currentObjectId)) << "Unable to activate a layout!";

    DwgDbLayoutPtr  layout(m_currentObjectId, DwgDbOpenMode::ForRead);
    ASSERT_DWGDBSUCCESS (layout.OpenStatus()) << "Failed to open the active layout object!";
    
    // check printable origin
    DPoint2d    point;
    ASSERT_DWGDBSUCCESS (layout->GetPaperImageOrigin(point)) << "Failed to read printable origin!";
    EXPECT_EQ (point.x, 0.0) << "The printable origin.x should be 0.0!";
    EXPECT_EQ (point.y, 0.0) << "The printable origin.y should be 0.0!";

    ASSERT_DWGDBSUCCESS (layout->GetPlotOrigin(point)) << "Failed to read plot origin offset!";
    EXPECT_EQ (point.x, 0.0) << "The plot origin offset.x should be 0.0!";
    EXPECT_EQ (point.y, 0.0) << "The plot origin offset.y should be 0.0!";

    EXPECT_EQ (layout->GetPlotBy(), DwgDbLayout::PlotBy::Layout) << "The plot type should be PlotBy::Layout!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgFileEditor::CreateGroup (Utf8StringCR name, DwgDbObjectIdArrayCR members)
    {
    // create a new group with input members
    DwgDbDictionaryPtr  groups (m_dwgdb->GetGroupDictionaryId(), DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS(groups.OpenStatus()) << "Failed to open the group dictionary!";
    EXPECT_FALSE(groups->Has(name)) << "A group with the same name exists in DWG!";

    // create a new group object
    DwgDbGroupPtr   group = DwgDbGroup::Create ();
    ASSERT_DWGDBSUCCESS(group.OpenStatus()) << "Error creating a new DWG group object!";

    // add the entry to the dictionary
    DwgDbObjectId   entryId;
    groups->SetAt (name, DwgDbObject::Cast(group.get()), &entryId);
    EXPECT_TRUE(entryId.IsValid()) << "Error adding a new entry to the group dictionary!";

    // append the input members to the group
    for (auto member : members)
        ASSERT_DWGDBSUCCESS(group->Append(member));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgFileEditor::UpdateGroup (Utf8StringCR name, DwgDbObjectIdArrayCR members)
    {
    // update existing group with potentially different members
    DwgDbDictionaryPtr  groups (m_dwgdb->GetGroupDictionaryId(), DwgDbOpenMode::ForRead);
    ASSERT_DWGDBSUCCESS(groups.OpenStatus()) << "Failed to open the group dictionary!";
    EXPECT_TRUE(groups->Has(name)) << "The requested group does not exist in DWG!";

    // get group ID by name:
    DwgDbObjectId   existingId;
    EXPECT_DWGDBSUCCESS(groups->GetIdAt(existingId, name)) << "Failed finding existing group by name!";
    // open the group object:
    DwgDbGroupPtr   group(existingId, DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS(group.OpenStatus()) << "Error opening existing DWG group for write!";

    // empty the group, then add the input members to it:
    ASSERT_DWGDBSUCCESS(group->Clear());
    for (auto member : members)
        ASSERT_DWGDBSUCCESS(group->Append(member));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgFileEditor::GetModelspaceEntities (DwgDbObjectIdArrayR ids) const
    {
    ids.clear ();

    DwgDbBlockTableRecordPtr modelspace (m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForRead);
    if (modelspace.OpenStatus() != DwgDbStatus::Success)
        return  modelspace.OpenStatus();

    auto iter = modelspace->GetBlockChildIterator ();
    for (iter.Start(); !iter.Done(); iter.Step())
        ids.push_back (iter.GetEntityId());

    return  ids.size() > 0 ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
    }
