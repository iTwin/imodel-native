/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatform.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Manages seed file creation and the use of seed file copies for DgnPlatform unit tests
// @bsiclass                                                 Sam.Wilson     01/2016
//=======================================================================================
struct DgnPlatformSeedManager : NonCopyableClass
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
        SeedDbId    id;                 //!< The ID of the seed DgnDb
        SeedDbOptions options;          //!< The options for the seed DgnDb
        BeFileName  fileName;           //!< The filename 
        Utf8String  physicalPartitionName; //!< The name of the default PhysicalPartition
        Utf8String  categoryName;       //!< The name of the first Category
        Utf8String  viewName;           //!< The name of the first view, if any.

        SeedDbInfo() : id(SeedDbId::OneSpatialModel) {}
        };

private:
    static SeedDbInfo GetOneSpatialModelSeedDb(SeedDbOptions const& options);

public:
    static DgnDbPtr CreateSeedDb(WCharCP relPath) {return DgnDbTestUtils::CreateDgnDb(relPath, false);}

    //! Get information about a root seed DgnDb. DgnDbTestUtils offers a "library" of seed DgnDbs. You can open them read-only or you
    //! can open a writable copy of one of these seed DgnDbs in your tests. If the library contains roughly what you need, then 
    //! opening a copy will save you from having to create your own seed DgnDb from scratch, and that saves a lot of time when running the tests.
    //! @param seedId   Identifies the seed DgnDb that is of interest.
    //! @param options  Optional features of the seed DgnDb that you want
    //! @return Information about the requested seed DgnDb. 
    //! @note This function may create the seed DgnDb as a side effect, if it hasn't been created already.
    //! @see OpenDgnDb
    static SeedDbInfo GetSeedDb(SeedDbId seedId, SeedDbOptions const& options = SeedDbOptions());

    static DgnDbPtr OpenSeedDb(WCharCP relSeedPath) {return DgnDbTestUtils::OpenSeedDb(relSeedPath);}
        
    static DgnDbPtr OpenSeedDbCopy(WCharCP relSeedPath, WCharCP newName = nullptr) {return DgnDbTestUtils::OpenSeedDbCopy(relSeedPath, newName);}

    static DgnDbStatus MakeSeedDbCopy(BeFileNameR actualName, WCharCP relSeedPath, WCharCP newName) {return DgnDbTestUtils::MakeSeedDbCopy(actualName, relSeedPath, newName);}

    static DgnDbPtr OpenDgnDb(WCharCP relPath, DgnDb::OpenMode mode) {return DgnDbTestUtils::OpenDgnDb(relPath, mode);}

    static BeFileNameStatus CreateSubDirectory(WCharCP relPath) {return DgnDbTestUtils::CreateSubDirectory(relPath);}

    static void EmptySubDirectory(WCharCP relPath) {DgnDbTestUtils::EmptySubDirectory(relPath);}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
