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

    //! Identifies a root seed file
    enum class SeedDbId
        {
        //! A seed file with:
        //!     * A single spatial model that is empty.
        //!     * A single camera view that points at this model
        //!     * A single Category
        OneSpatialModel = 1,    
        };

    //! Specifies optional features of a seed file
    struct SeedDbOptions
        {
        bool        testDomain;     //!< If true, then the Test domain is imported into the seed file
        bool        cameraView;     //!< If true, then the seed file contains a camera view pointing at the first spatial model

        Utf8String ToKey() const;
        
        //! Construct SeedDbOptions
        //! @param wantCameraView   If true, then the seed file will contain a camera view
        //! @param testDomain       If true, then the Test domain will be imported into the seed file
        SeedDbOptions(bool wantTestDomain = false, bool wantCameraView = true) : testDomain(wantTestDomain), cameraView(wantCameraView) {}
        };

    //! Information about a root seed file
    struct SeedDbInfo
        {
        SeedDbId    id;             //!< The ID of the seed file
        SeedDbOptions options;      //!< The options for the seed file
        BeFileName  fileName;       //!< The filename 
        DgnCode     modelCode;      //!< The DgnCode of the first DgnModel
        Utf8String  categoryName;   //!< The name of the first Category
        Utf8String  viewName;       //!< The name of the first view, if any.

        SeedDbInfo() : id(SeedDbId::OneSpatialModel) {}
        bool operator< (SeedDbInfo const& rhs) const;
        Utf8String ToKey() const;
        };

private:
    //! Create a DgnDb in the test output directory. The specified name must be a relative path, including an optional subdirectory path, and a filename.
    //! If the file already exists, that is an ERROR, indicating that two test groups are trying to create seed files with the same names.
    //! Each test group should use its own group-specific, unique name for its subdirectory and/or seed files.
    //! @param relPath  The subdirectory/filename for the new file. Be sure to use forward slash (/) as a directory separator.
    //! @praam isRoot If true, the caller is DgnDbTestUtils and it wants to create a file in the root directory
    //! @param mustBeBriefcase If true, the new DgnDb is marked as a (fake) briefcase.
    //! @return a pointer to the newly created file, or nullptr if the location is invalid
    //! @note Normally, this should be called only in the TC_SETUP function, once for an entire test group
    static DgnDbPtr CreateDgnDb(WCharCP relPath, bool isRoot, bool mustBeBriefcase);

    static SeedDbInfo GetOneSpatialModelSeedDb(SeedDbOptions const& options);

public:
    //! @name Working with root seed files
    //! @{

    //! Create a DgnDb in the test output directory. The specified name must be a relative path, including an optional subdirectory path, and a filename.
    //! If the file already exists, that is an ERROR, indicating that two test groups are trying to create seed files with the same names.
    //! Each test group should use its own group-specific, unique name for its subdirectory and/or seed files.
    //! @param relPath  The subdirectory/filename for the new file. Be sure to use forward slash (/) as a directory separator.
    //! @param mustBeBriefcase If true, the new DgnDb is marked as a (fake) briefcase. This is the default.
    //! @return a pointer to the newly created file, or nullptr if the location is invalid
    //! @note Normally, this should be called only in the TC_SETUP function, once for an entire test group
    static DgnDbPtr CreateSeedDb(WCharCP relPath, bool mustBeBriefcase = true) {return CreateDgnDb(relPath, false, mustBeBriefcase);}

    //! Get information about a root seed file. 
    //! @param seedId   Identifies the seed file that is of interest.
    //! @param options  Optional features of the seed file that you want
    //! @return Information about the requested seed file. 
    //! @note This function may create the seed file as a side effect, if it hasn't been created already.
    //! @see OpenDgnDb, OpenDgnDbCopy
    static SeedDbInfo GetSeedDb(SeedDbId seedId, SeedDbOptions const& options = SeedDbOptions());

    //! Open the specified seed file read-only
    //! @param relSeedPath Identifies a pre-existing seed file. If you want to open a seed file that was created by your test group's TC_SETUP logic, then you must specify the
    //! relative path to it. If you want to open a program-wide seed file, call GetSeedDb to get its name. 
    //! @return a pointer to the open DgnDb, or nullptr if the file does not exist
    //! @see GetSeedDb, OpenDgnDbCopy
    static DgnDbPtr OpenSeedDb(WCharCP relSeedPath);
        
    //! Open <em>a copy of</em> the specified seed file for reading and writing. The result will be a private copy for the use of the caller.
    //! The copy will always be located in a subdirectory with the same name as the caller's test case.
    //! @note The copy of the file is automatically assigned a unique name, to avoid name collisions with other tests.
    //! @param relSeedPath Identifies a pre-existing seed file. If you want to open a seed file that was created by your test group's TC_SETUP logic, then you must specify the
    //! relative path to it. If you want to open a program-wide seed file, call GetSeedDb to get its name. 
    //! @param newName optional. all or part of the name of the copy. If null, then the name of the copy will be based on the name of the input seed file. If not null, then
    //! the name of the copy will be based on \a newName and will be modified as necessary to make it unique.
    //! @return a pointer to the open DgnDb, or nullptr if the seed file does not exist.
    //! @see OpenDgnDb, ReOpenDgnDbCopy, GetSeedDb
    static DgnDbPtr OpenSeedDbCopy(WCharCP relSeedPath, WCharCP newName = nullptr);

    //! @}

    //! @name Populating a DgnDb
    //! @{

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

    //! @}

    //! @name Utilities for finding things
    //! @{

    //! Open the specified seed file.
    //! @param relPath Identifies a seed file that was created for the group in the TC_SETUP function. Be sure to use forward slash (/) as a directory separator.
    //! @param mode the file open mode
    //! @return a pointer to the open DgnDb, or nullptr if the file does not exist
    static DgnDbPtr OpenDgnDb(WCharCP relPath, DgnDb::OpenMode mode);

    //! Look up a model by its name
    template<typename T>
    static
    RefCountedPtr<T> GetModelByName(DgnDbR db, Utf8StringCR cmname)
        {
        return db.Models().Get<T>(db.Models().QueryModelId(DgnModel::CreateModelCode(cmname)));
        }

    //! @}

    //! @name Working with files and directories in the test Output directory
    //! @{

    //! Create a subdirectory in the test output directory. 
    //! If the directory already exists, that is an ERROR, indicating that two test groups are trying to use the same output directory.
    //! Each test group should use its own group-specific, unique name for its subdirectory and/or seed files.
    //! @param relPath  The name of the subdirectory to create. Be sure to use forward slash (/) as a directory separator.
    //! @note Normally, this should be called only in the TC_SETUP function, once for an entire test group
    static BeFileNameStatus CreateSubDirectory(WCharCP relPath);

    //! Empty a subdirectory in the test output directory. 
    //! @param relPath  The name of the subdirectory to empty. Be sure to use forward slash (/) as a directory separator.
    static void EmptySubDirectory(WCharCP relPath);

    //! @}

};

END_BENTLEY_DGNPLATFORM_NAMESPACE