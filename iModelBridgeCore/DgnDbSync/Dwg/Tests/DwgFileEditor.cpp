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
void DwgFileEditor::CreateFile (BeFileNameCR infile)
    {
    m_dwgdb = new DwgDbDatabase ();
    ASSERT_FALSE (m_dwgdb.IsNull());
    DwgImportHost::SetWorkingDatabase (m_dwgdb.get());
    m_fileName = infile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgFileEditor::OpenFile (BeFileNameCR infile)
    {
    m_dwgdb = DwgImportHost::GetHost().ReadFile (infile, false, false, FileShareMode::DenyNo);
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
        // saving DWG back into an existing file:
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
