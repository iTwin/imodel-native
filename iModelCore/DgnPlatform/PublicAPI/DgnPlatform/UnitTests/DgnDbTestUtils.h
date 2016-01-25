//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/UnitTests/DgnDbTestUtils.h $
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
struct DgnDbTestUtils : public testing::Test
{
    // Insert the following macros into the definition of your test group subclass to declare that you want to set up shared resources for all of the tests in your group.
    //BETEST_DECLARE_TC_SETUP
    //BETEST_DECLARE_TC_TEARDOWN

    // Then do this in your group's .cpp file, in order to *define* your setup and teardown functions
    // BETEST_TC_SETUP(MyGroup) { ... one-time setup logic ... }
    // BETEST_TC_TEARDOWN(MyGroup) { ... one-time tear-down logic ... }

protected:
    static bool s_createdSeedFiles;

    //! Open the specified seed file.
    //! @param relPath Identifies a seed file that was created for the group in the TC_SETUP function. Be sure to use forward slash (/) as a directory separator.
    //! @param mode the file open mode
    //! @return a pointer to the open DgnDb, or nullptr if the file does not exist
    static DgnDbPtr OpenDgnDb(WCharCP relPath, DgnDb::OpenMode mode);

public:
    //! This function creates the program-wide seed files. It should be called before attempting to open open them. 
    static void CreateSeedFiles();

    //! Create a DgnDb in the test output directory. The specified name must be a relative path, including an optional subdirectory path, and a filename.
    //! If the file already exists, that is an ERROR, indicating that two test groups are trying to create seed files with the same names.
    //! Each test group should use its own group-specific, unique name for its subdirectory and/or seed files.
    //! @note Normally, this should be called only in the TC_SETUP function, once for an entire test group
    //! @param relPath  The subdirectory/filename for the new file. Be sure to use forward slash (/) as a directory separator.
    //! @return a pointer to the newly created file, or nullptr if the location is invalid
    //! @see CreateSeedFiles
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

    //! Get the filename (not the full file path) of the empty 3-D seed file
    static WCharCP GetEmpty3dSeedFileName();

    //! Get the name of the default model that is created in most of the seed files
    static DgnCode GetDefaultModelCode();

    //! Get the name of the default category that is created in most of the seed files
    static Utf8CP GetDefaultCategoryName();

    //! Get the name of the default spatial view that is created in most of the seed files
    static Utf8CP GetDefaultCameraViewName();

    //! Open the specified seed file read-only
    //! @param relSeedPath Identifies a seed file that was created for the group in the TC_SETUP function. Be sure to use forward slash (/) as a directory separator.
    //! @return a pointer to the open DgnDb, or nullptr if the file does not exist
    //! @see CreateSeedFiles
    static DgnDbPtr OpenDgnDb(WCharCP relSeedPath);
        
    //! Open <em>a copy of</em> the specified seed file for reading and writing. The result will be a private copy for the use of the caller.
    //! @note The copy of the file is automatically assigned a unique name, to avoid name collisions with other tests.
    //! @param relSeedPath Identifies a seed file that was created for the group in the TC_SETUP function. Be sure to use forward slash (/) as a directory separator.
    //! @return a pointer to the open DgnDb, or nullptr if the seed file does not exist or if a file by the new name does exist.
    //! @see CreateSeedFiles
    static DgnDbPtr OpenDgnDbCopy(WCharCP relSeedPath);

    //! Insert a SpatialModel 
    static SpatialModelPtr InsertSpatialModel(DgnDbR, DgnCode modelCode);
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