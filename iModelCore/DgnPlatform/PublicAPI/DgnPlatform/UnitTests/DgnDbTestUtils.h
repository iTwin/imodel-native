//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/UnitTests/DgnDbTestUtils.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once 

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
    //! Identifies a root seed DgnDb
    enum class SeedDbId
        {
        //! A seed DgnDb with:
        //!     * A single spatial model that is empty.
        //!     * A single camera view that points at this model
        //!     * A single Category
        OneSpatialModel = 1,    
        };

    //! Specifies optional features of a seed DgnDb
    struct SeedDbOptions
        {
        bool        testDomain;     //!< If true, then the Test domain is imported into the seed DgnDb
        bool        cameraView;     //!< If true, then the seed DgnDb contains a camera view pointing at the first spatial model

        WString ToKey() const;
        
        //! Construct SeedDbOptions
        //! @param wantTestDomain   If true, then the Test domain will be imported into the seed DgnDb
        //! @param wantCameraView   If true, then the seed DgnDb will contain a camera view
        SeedDbOptions(bool wantTestDomain = false, bool wantCameraView = true) : testDomain(wantTestDomain), cameraView(wantCameraView) {}
        };

    //! Information about a root seed DgnDb
    struct SeedDbInfo
        {
        SeedDbId    id;             //!< The ID of the seed DgnDb
        SeedDbOptions options;      //!< The options for the seed DgnDb
        BeFileName  fileName;       //!< The filename 
        DgnCode     modelCode;      //!< The DgnCode of the first DgnModel
        Utf8String  categoryName;   //!< The name of the first Category
        Utf8String  viewName;       //!< The name of the first view, if any.

        SeedDbInfo() : id(SeedDbId::OneSpatialModel) {}
        };

private:
    static DgnDbPtr CreateDgnDb(WCharCP relPath, bool isRoot, bool mustBeBriefcase);

    static SeedDbInfo GetOneSpatialModelSeedDb(SeedDbOptions const& options);

public:
    //! @name Working with root seed DgnDbs
    //! @{

    //! Create a DgnDb from scratch in the test output directory. 
    //! The specified name must be a relative path, including an optional subdirectory path, and a filename.
    //! If the file already exists, that is an ERROR, indicating that two test groups are trying to create seed DgnDbs with the same names.
    //! Each test group should use its own group-specific, unique name for its subdirectory and/or seed DgnDbs.
    //! @param relPath  The subdirectory/filename for the new file. Be sure to use forward slash (/) as a directory separator.
    //! @param mustBeBriefcase If true, the new DgnDb is marked as a (fake) briefcase. This is the default.
    //! @return a pointer to the newly created file, or nullptr if the location is invalid
    //! @note Normally, this should be called only once for an entire test group in the group's TC_SETUP function.
    //! @see GetSeedDb for access to a pre-existing "library" of seed DgnDbs. It is faster to copy and modify a library seed DgnDb than to create a new seed from scratch.
    static DgnDbPtr CreateSeedDb(WCharCP relPath, bool mustBeBriefcase = true) {return CreateDgnDb(relPath, false, mustBeBriefcase);}

    //! Get information about a root seed DgnDb. DgnDbTestUtils offers a "library" of seed DgnDbs. You can open them read-only or you
    //! can open a writable copy of one of these seed DgnDbs in your tests. If the library contains roughly what you need, then 
    //! opening a copy will save you from having to create your own seed DgnDb from scratch, and that saves a lot of time when running the tests.
    //! @param seedId   Identifies the seed DgnDb that is of interest.
    //! @param options  Optional features of the seed DgnDb that you want
    //! @return Information about the requested seed DgnDb. 
    //! @note This function may create the seed DgnDb as a side effect, if it hasn't been created already.
    //! @see OpenDgnDb
    static SeedDbInfo GetSeedDb(SeedDbId seedId, SeedDbOptions const& options = SeedDbOptions());

    //! Open the specified seed DgnDb read-only
    //! @param relSeedPath Identifies a pre-existing seed DgnDb. If you want to open a seed DgnDb that was created by your test group's TC_SETUP logic, then you must specify the
    //! relative path to it. If you want to open a program-wide seed DgnDb, call GetSeedDb to get its relative path. 
    //! @return a pointer to the open DgnDb, or nullptr if the seed DgnDb does not exist
    //! @see GetSeedDb
    static DgnDbPtr OpenSeedDb(WCharCP relSeedPath);
        
    //! Open <em>a copy of</em> the specified seed DgnDb for reading and writing. The result will be a private copy for the use of the caller.
    //! The copy will always be located in a subdirectory with the same name as the caller's test case.
    //! @note The copy of the file is automatically assigned a unique name, to avoid name collisions with other tests.
    //! @param relSeedPath Identifies a pre-existing seed DgnDb. If you want to open a seed DgnDb that was created by your test group's TC_SETUP logic, then you must specify the
    //! relative path to it. If you want to open a program-wide seed DgnDb, call GetSeedDb to get its name. 
    //! @param newName optional. all or part of the name of the copy. If null, then the name of the copy will be based on the name of the input seed DgnDb. If not null, then
    //! the name of the copy will be based on \a newName and will be modified as necessary to make it unique.
    //! @return a pointer to the open DgnDb, or nullptr if the seed DgnDb does not exist.
    //! @see OpenDgnDb, GetSeedDb
    static DgnDbPtr OpenSeedDbCopy(WCharCP relSeedPath, WCharCP newName = nullptr);

    //! Create <em>a copy of</em> the specified seed DgnDb for reading and writing. The result will be a private copy for the use of the caller.
    //! The copy will always be located in a subdirectory with the same name as the caller's test case.
    //! @note The copy of the file is automatically assigned a unique name, to avoid name collisions with other tests.
    //! @param[out] actualName  Set to the name of the file that was created. May not be the same as \a newName, if a unique name was generated.
    //! @param[in] relSeedPath Identifies a pre-existing seed DgnDb. If you want to open a seed DgnDb that was created by your test group's TC_SETUP logic, then you must specify the
    //! relative path to it. If you want to open a program-wide seed DgnDb, call GetSeedDb to get its name. 
    //! @param[in] newName optional. all or part of the name of the copy. If null, then the name of the copy will be based on the name of the input seed DgnDb. If not null, then
    //! the name of the copy will be based on \a newName and will be modified as necessary to make it unique.
    //! @return a non-zero error status if the seed DgnDb does not exist or the copy could not be created.
    //! @see OpenDgnDb, GetSeedDb
    static DgnDbStatus MakeSeedDbCopy(BeFileNameR actualName, WCharCP relSeedPath, WCharCP newName);

    //! @}

    //! @name Populating a DgnDb
    //! @{

    //! Insert a SpatialModel 
    static SpatialModelPtr InsertSpatialModel(DgnDbR, DgnCode modelCode);
    //! Create a Camera view of the specified SpatialModel 
    static DgnViewId InsertCameraView(SpatialModelR, Utf8CP viewName = nullptr);
    static void FitView(DgnDbR db, DgnViewId viewId);

    //! Create a new modelselector
    static ModelSelectorCPtr InsertNewModelSelector(DgnDbR db, Utf8CP name, DgnModelId model)
        {
        ModelSelector modSel(db, name, model);
        return db.Elements().Insert(modSel);
        }

    //! Create a new Category
    static DgnCategoryId InsertCategory(DgnDbR, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance = DgnSubCategory::Appearance(), DgnCategory::Scope scope = DgnCategory::Scope::Physical, DgnCategory::Rank rank = DgnCategory::Rank::Application);

    //! Create a new CodeAuthority
    static DgnAuthorityId InsertNamespaceAuthority(DgnDbR, Utf8CP authorityName);
    
    //! Update the project extents
    static void UpdateProjectExtents(DgnDbR);

    //! @}

    //! @name Utilities for finding things
    //! @{

    //! Open the specified seed DgnDb.
    //! @param relPath Identifies a seed DgnDb that was created for the group in the TC_SETUP function. Be sure to use forward slash (/) as a directory separator.
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
    //! Each test group should use its own group-specific, unique name for its subdirectory and/or seed DgnDbs.
    //! @param relPath  The name of the subdirectory to create. Be sure to use forward slash (/) as a directory separator.
    //! @note Normally, this should be called only in the TC_SETUP function, once for an entire test group
    static BeFileNameStatus CreateSubDirectory(WCharCP relPath);

    //! Empty a subdirectory in the test output directory. 
    //! @param relPath  The name of the subdirectory to empty. Be sure to use forward slash (/) as a directory separator.
    static void EmptySubDirectory(WCharCP relPath);

    //! @}

};

END_BENTLEY_DGNPLATFORM_NAMESPACE