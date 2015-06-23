/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/NamedVolume_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <ECDb/ECDbApi.h>
#include <DgnPlatform/DgnHandlers/DgnHandlersAPI.h>
#include <DgnPlatform/DgnCore/DgnMarkupProject.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"Dgn"))

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_BENTLEY_SQLITE_EC

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
bool AreVolumesEqual (const NamedVolume& volume1, const NamedVolume& volume2)
    {
    if (volume1.GetName() != volume2.GetName())
        return false;
    if (!volume1.GetOrigin().IsEqual (volume2.GetOrigin()))
        return false;
    if (volume1.GetHeight() != volume2.GetHeight())
        return false;

    bvector<DPoint2d> shape1 = volume1.GetShape();
    bvector<DPoint2d> shape2 = volume2.GetShape();
    if (shape1.size() != shape2.size())
        return false;
    for (int ii=0; ii< (int) shape1.size(); ii++)
        {
        if (!shape1[ii].IsEqual (shape2[ii]))
            return false;
        }
    return true;
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
TEST(NamedVolume, CrudTest)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdmSeed (L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::OPEN_Readonly);
    DgnDbTestDgnManager tdm (L"79_Main.i.idgndb", __FILE__, Db::OpenMode::OPEN_Readonly);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE (project != nullptr);

    BeFileName markupProjectFileName = DgnDbTestDgnManager::GetOutputFilePath  (L"NamedVolume");
    CreateDgnMarkupProjectParams cparms (*project);
    cparms.SetOverwriteExisting(true);
    cparms.SetSeedDb (BeFileName(tdmSeed.GetPath()));
    DbResult dgnFileStatus;
    DgnMarkupProjectPtr markupProject = DgnMarkupProject::CreateDgnDb (&dgnFileStatus, markupProjectFileName, cparms);
    ASSERT_TRUE (dgnFileStatus == BE_SQLITE_OK);
    ASSERT_TRUE (markupProject.IsValid());
    
    // Create
    Utf8String name = "CrudTest";
    DPoint3d origin = {0.0, 0.0, 0.0};
    DPoint2d shapePoints[5] = {{0.0, 0.0}, {0.0, 100.0}, {100.0, 100.0}, {100.0, 0.0}, {0.0, 0.0}};
    size_t numShapePoints = (size_t) (sizeof (shapePoints) / sizeof (DPoint2d));
    double height = 100.0;
    NamedVolume volume (name, origin, &shapePoints[0], numShapePoints, height);

    // Insert
    StatusInt status = volume.Insert (*markupProject);
    ASSERT_TRUE (status == SUCCESS);

    bool exists = NamedVolume::Exists (name, *markupProject);
    ASSERT_TRUE (exists);

    // Read
    std::unique_ptr<NamedVolume> readVolume = NamedVolume::Read (name, *markupProject);
    ASSERT_TRUE (readVolume != nullptr);
    bool volumesAreEqual = AreVolumesEqual (*readVolume, volume);
    ASSERT_TRUE (volumesAreEqual);

    // Update
    name = "CrudTest1";
    origin = {1.0, 1.0, 1.0};
    DPoint2d shapePoints2[5] = {{0.0, 0.0}, {0.0, 200.0}, {100.0, 200.0}, {200.0, 0.0}, {0.0, 0.0}};
    numShapePoints = (size_t) (sizeof (shapePoints2) / sizeof (DPoint2d));
    height = 200.0;
    NamedVolume updateVolume (name, origin, &shapePoints2[0], numShapePoints, height);

    volume.SetName (name);
    volume.SetOrigin (origin);
    volume.SetShape (&shapePoints2[0], numShapePoints);
    volume.SetHeight (height);
    status = volume.Update (*markupProject);
    ASSERT_TRUE (status == SUCCESS);

    readVolume = NamedVolume::Read (volume.GetName(), *markupProject);
    ASSERT_TRUE (readVolume != nullptr);
    volumesAreEqual = AreVolumesEqual (*readVolume, updateVolume);
    ASSERT_TRUE (volumesAreEqual);

    // Delete
    status = NamedVolume::Delete (volume.GetName(), *markupProject);
    ASSERT_TRUE (status == SUCCESS);
    
    exists = NamedVolume::Exists (volume.GetName(), *markupProject);
    ASSERT_FALSE (exists);
    }
    