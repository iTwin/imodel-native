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
void DwgFileEditor::OpenFile (BeFileNameCR infile)
    {
    m_dwgdb = DwgImportHost::GetHost().ReadFile (infile, false, false, FileShareMode::DenyNo);

    ASSERT_TRUE (m_dwgdb.IsValid());

    DwgImportHost::SetWorkingDatabase (m_dwgdb.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgFileEditor::SaveFile ()
    {
    BeFileName  originalFile (m_dwgdb->GetFileName().c_str());
    BeFileName  tempFile = originalFile;
    tempFile.AppendExtension (L"tmp");

    DwgDbStatus status = m_dwgdb->SaveAs (tempFile.c_str());
    EXPECT_DWGDBSUCCESS (status);

    ASSERT_EQ (originalFile.BeDeleteFile(), BeFileNameStatus::Success);
    ASSERT_EQ (BeFileName::BeMoveFile(tempFile, originalFile), BeFileNameStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgFileEditor::AddCircleInDefaultModel ()
    {
    // create a new circle
    DwgDbCirclePtr  circle = DwgDbCircle::Create () ;
    ASSERT_FALSE (circle.IsNull()) << "Circle cannot be created!";

    ASSERT_DWGDBSUCCESS (circle->SetCenter(DPoint3d::From(10.0, 13.0, 0.0)));
    ASSERT_DWGDBSUCCESS (circle->SetRadius(3.5));

    // open modelspace for write
    DwgDbBlockTableRecordPtr    modelspace (m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS (modelspace.OpenStatus()) << "Modelspace block cannot be opened for write";
    
    // add the circle into modelspace
    DwgDbEntityP    entity = DwgDbEntity::Cast (circle.get());
    ASSERT_NOT_NULL (entity);

    m_currentObjectId = modelspace->AppendEntity (*entity);
    ASSERT_TRUE (m_currentObjectId.IsValid()) << "Circle not added into Modelspace!";

    ASSERT_DWGDBSUCCESS (circle->Close()) << "Circle cannot be closed!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgFileEditor::DeleteEntity (DwgDbHandleCR entityHandle)
    {
    ASSERT_FALSE (entityHandle.IsNull());

    m_currentObjectId = m_dwgdb->GetObjectId (entityHandle);
    EXPECT_TRUE (m_currentObjectId.IsValid()) << "Input entity handle not found in DWG!";

    DwgDbEntityPtr  entity (m_currentObjectId, DwgDbOpenMode::ForWrite);
    ASSERT_DWGDBSUCCESS (entity.OpenStatus()) << "Cannot open entity for write!";

    if (DwgDbStatus::Success == entity.OpenStatus())
        entity->Erase ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectIdCR DwgFileEditor::GetCurrentObjectId () const
    {
    return  m_currentObjectId;
    }
