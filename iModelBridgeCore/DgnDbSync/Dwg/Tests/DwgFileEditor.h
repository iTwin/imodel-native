/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Tests/DwgFileEditor.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ImporterTests.h"

typedef bvector<DwgDbHandle>    T_EntityHandles;

USING_NAMESPACE_DGNDBSYNC_DWG

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/18
+===============+===============+===============+===============+===============+======*/
struct  DwgFileEditor
{
private:
    DwgDbDatabasePtr    m_dwgdb;
    DwgDbObjectId       m_currentObjectId;
    BeFileName          m_fileName;

public:
    // Constructors prerequsites implementation of IDwgDbHost through DwgImporter!
    // the default constructor
    DwgFileEditor () { BeAssert(DwgImportHost::GetHost()._IsValid()); }
    // constructor that opens an existing DWG file
    DwgFileEditor (BeFileNameCR infile) { BeAssert(DwgImportHost::GetHost()._IsValid()); OpenFile(infile); }
    void    CreateFile (BeFileNameCR infile);
    void    OpenFile (BeFileNameCR infile);
    void    SaveFile ();
    void    AddCircleInDefaultModel ();
    void    AddEntitiesInDefaultModel (T_EntityHandles& handles);
    void    DeleteEntity (DwgDbHandleCR entityHandle);
    void    TransformEntitiesBy (T_EntityHandles const& handles, TransformCR transform);
    void    AppendEntity (DwgDbEntityP entity, DwgDbBlockTableRecordP block, bool closeEntity = true);
    void    AttachXrefInDefaultModel (BeFileNameCR infile, DPoint3dCR origin = DPoint3d::FromZero(), double angle = 0.0);
    void    FindXrefInsert (DwgStringCR blockName);
    void    FindXrefBlock (DwgStringCR blockName);
    size_t  CountAndCheckModelspaceEntity (bool& found, DwgDbHandleCR entityHandle) const;

    DwgDbObjectIdCR GetCurrentObjectId () const;

}; // DwgFileEditor
