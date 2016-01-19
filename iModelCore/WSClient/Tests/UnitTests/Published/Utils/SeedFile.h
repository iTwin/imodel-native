/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/SeedFile.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    private:
        Utf8String m_name;
        BeFileName m_seedFilePath;
        BeFileName m_testFilePath;

    protected:
        //! Called once
        virtual void SetupSeedFile(BeFileNameCR filePath) = 0;

        //! Called for each GetTestFile()
        virtual void SetupTestFile(BeFileNameCR filePath) {};

    public:
        SeedFile(Utf8String name = "testFile") : m_name(name) {}
        BeFileName GetTestFile();
    };