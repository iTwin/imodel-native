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

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/18
+===============+===============+===============+===============+===============+======*/
struct  DwgFileEditor
{
private:
    DwgDbDatabasePtr    m_dwgdb;
    DwgDbObjectId       m_currentObjectId;

public:
    DwgFileEditor (BeFileNameCR infile) { OpenFile(infile); }
    void    OpenFile (BeFileNameCR infile);
    void    SaveFile ();
    void    AddCircleInDefaultModel ();
    void    DeleteEntity (DwgDbHandleCR entityHandle);
    void    TransformEntitiesBy (T_EntityHandles const& handles, TransformCR transform);
    size_t  CountAndCheckModelspaceEntity (bool& found, DwgDbHandleCR entityHandle) const;

    DwgDbObjectIdCR GetCurrentObjectId () const;

}; // DwgFileEditor
