/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "WebServicesTestsHelper.h"

//class ReusableFileBaseTest : public WSClientBaseTest
//    {
//    private:
//        static std::shared_ptr<ObservableECDb> s_reusableECDb;
//        static BeFileName s_seedECDbPath;
//
//    protected:
//        //! Create new ECDb for testing
//        std::shared_ptr<ObservableECDb> CreateTestFile();
//
//        //! Create or use reuse ECDb for testing
//        std::shared_ptr<ObservableECDb> GetTestDb();
//
//        //! Override this to supply custom schema for test
//        virtual ECSchemaPtr GetTestSchema() = 0;
//    };

struct SeedFile
    {
    public:
        typedef std::function<void(BeFileNameCR flePath)> Callback;
    private:
        uint64_t m_id = 0;
        Utf8String m_name;
        BeFileName m_seedFilePath;
        BeFileName m_testFilePath;

        static BeAtomic<uint64_t> s_id;

    public:
        Callback onSetupSeedFile = [] (BeFileNameCR) {};
        Callback onSetupTestFile = [] (BeFileNameCR) {};

    protected:
        //! Called once
        virtual void SetupSeedFile(BeFileNameCR filePath) { onSetupSeedFile(filePath); };

        //! Called for each GetTestFile()
        virtual void SetupTestFile(BeFileNameCR filePath) { onSetupTestFile(filePath); };

    public:
        SeedFile(Utf8String name = "testFile",
            Callback onSetupSeedFile = [] (BeFileNameCR) {},
            Callback onSetupTestFile = [] (BeFileNameCR) {});

        BeFileName GetTestFile();
    };
