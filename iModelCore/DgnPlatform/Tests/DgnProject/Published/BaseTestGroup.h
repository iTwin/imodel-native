//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/BaseTestGroup.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatform.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/DgnView.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Sam.Wilson     01/2016
//=======================================================================================
struct BaseTestGroup : public testing::Test
{
    // Insert the following macros into the definition of your test group subclass to declare that you want to set up shared resources for all of the tests in your group.
    //BETEST_DECLARE_TC_SETUP
    //BETEST_DECLARE_TC_TEARDOWN

    // Then do this in your group's .cpp file, in order to *define* your setup and teardown functions
    // BETEST_TC_SETUP(MyGroup) { ... one-time setup logic ... }
    // BETEST_TC_TEARDOWN(MyGroup) { ... one-time tear-down logic ... }

protected:
    Dgn::ScopedDgnHost m_testHost;

    static DgnDbPtr OpenDgnDb(WCharCP relPath, DgnDb::OpenMode mode);

public:
    //! Create a DgnDb in the test output directory. The specified name is a relative path, including an optional subdirectory path, and a filename.
    //! If the file already exists, that is an ERROR, indicating that two test groups are trying to create seed files with the same names.
    //! Each test group should use its own group-specific, unique name for its subdirectory and/or seed files.
    //! @note Normally, this should be called only in the TC_SETUP function, once for an entire test group
    //! @param relPath  The subdirectory/filename for the new file. Be sure to use forward slash (/) as a directory separator.
    //! @return a pointer to the newly created file, or nullptr if the location is invalid
    static DgnDbPtr CreateDgnDb(WCharCP relPath);

    //! Create a subdirectory in the test output directory. 
    //! If the directory already exists, that is an ERROR, indicating that two test groups are trying to use the same output directory.
    //! Each test group should use its own group-specific, unique name for its subdirectory and/or seed files.
    //! @param relPath  The name of the subdirectory to create. Be sure to use forward slash (/) as a directory separator.
    //! @note Normally, this should be called only in the TC_SETUP function, once for an entire test group
    static BeFileNameStatus CreateSubDirectory(WCharCP relPath);

    //! Empty a subdirectory in the test output directory. 
    //! @param relPath  The name of the subdirectory to empty. Be sure to use forward slash (/) as a directory separator.
    static void EmptySubDirectory(WCharCP relPath);

public:
    //! Open the specified seed file read-only
    //! @param relSeedPath Identifies a seed file that was created for the group in the TC_SETUP function. Be sure to use forward slash (/) as a directory separator.
    //! @return a pointer to the open DgnDb, or nullptr if the file does not exist
    static DgnDbPtr OpenDgnDb(WCharCP relSeedPath);
        
    //! Open <em>a copy of</em> the specified seed file read-write
    //! @param relSeedPath Identifies a seed file that was created for the group in the TC_SETUP function. Be sure to use forward slash (/) as a directory separator.
    //! @param nameOfCopy The name of the new file to create (in the same sub-directory as the seed file)
    //! @return a pointer to the open DgnDb, or nullptr if the file does not exist or could not be copied
    static DgnDbPtr CopyDgnDb(WCharCP relSeedPath, WCharCP nameOfCopy);

    //! Insert a SpatialModel 
    static SpatialModelPtr InsertSpatialModel(DgnDbR, Utf8CP modelName);
    //! Create a Camera view of the specified SpatialModel 
    static DgnViewId InsertCameraView(SpatialModelR, Utf8CP viewName = nullptr);
    static void FitView(DgnDbR db, DgnViewId viewId);
    
    //! Create a new Category
    static DgnCategoryId InsertCategory(DgnDbR, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance = DgnSubCategory::Appearance(), DgnCategory::Scope scope = DgnCategory::Scope::Physical, DgnCategory::Rank rank = DgnCategory::Rank::Application);

    //! Create a new CodeAuthority
    static DgnAuthorityId InsertNamespaceAuthority(DgnDbR, Utf8CP authorityName);
    
    //! Update the project extents
    static void UpdateProjectExtents(DgnDbR);

};

END_BENTLEY_DGNPLATFORM_NAMESPACE