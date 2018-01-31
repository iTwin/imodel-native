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
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbHandle AddCircle (Utf8StringR codeValue) const
    {
    DwgFileEditor   editor(m_dwgFileName);
    editor.AddCircleInDefaultModel ();

    // save off the entity handle
    DwgDbHandle entityHandle = editor.GetCurrentObjectId().GetHandle ();
    EXPECT_TRUE (!entityHandle.IsNull());

    // save off the CodeValue
    codeValue.Sprintf ("Model:%llx", entityHandle.AsUInt64());

    editor.SaveFile ();
    return  entityHandle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DeleteEntity (DwgDbHandleCR entityHandle)
    {
    DwgFileEditor   editor(m_dwgFileName);
    editor.DeleteEntity (entityHandle);
    editor.SaveFile ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  CountImportedElements ()
    {
    auto db = OpenExistingDgnDb (m_dgnDbFileName, Db::OpenMode::Readonly);
    EXPECT_TRUE (db.IsValid());

    size_t numElements = db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialElement)).BuildIdList<DgnElementId>().size ();
    EXPECT_EQ (numElements, GetCount());
    return  numElements;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckDbElement (size_t expectedCount, Utf8StringCR codeValue, bool shouldExist)
    {
    // check expected element count
    auto db = OpenExistingDgnDb (m_dgnDbFileName, Db::OpenMode::Readonly);
    EXPECT_EQ (expectedCount, db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialElement)).BuildIdList<DgnElementId>().size());

    // check the presence of the imported element
    auto elementId = QueryElementId (*db.get(), GENERIC_CLASS_PhysicalObject, codeValue.c_str());
    EXPECT_EQ (shouldExist, elementId.IsValid());
    }
};

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
TEST_F(BasicTests, UpdateElements)
    {
    LineUpFiles(L"changeTest.ibim", L"basictype.dwg", true); 
    // imported 5 elements from 5 entities?
    size_t  numElements = CountImportedElements ();

    // add a circle in DWG file
    Utf8String  codeValue;
    DwgDbHandle entityHandle = AddCircle (codeValue);

    // update DgnDb
    DoUpdate (m_dgnDbFileName, m_dwgFileName);
    // exactly 1 element imported?
    EXPECT_EQ (1, GetCount());
    CheckDbElement (numElements + 1, codeValue, true);

    // delete the circle from the DWG file
    DeleteEntity (entityHandle);

    // update DgnDb again
    DoUpdate (m_dgnDbFileName, m_dwgFileName);
    // none imported?
    EXPECT_EQ (0, GetCount());
    CheckDbElement (numElements, codeValue, false);
    }
