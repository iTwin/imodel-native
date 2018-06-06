/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestIModelCreators.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"

// *** Instructions for adding new test file creators ***
// 1) Define the test file name with a TESTIMODEL_ macro
// 2) Add a new subclass of TestIModelCreator
// 3) Add a unique_ptr to the TESTIMODELCREATOR_LIST macro

#define TESTIMODEL_EMPTY "empty.bim"
#define TESTIMODELCREATOR_LIST {std::make_shared<EmptyTestIModelCreator>()}

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct TestIModelCreation final
    {
private:
    static bool s_hasRun;

    TestIModelCreation() = delete;
    ~TestIModelCreation() = delete;

public:
    static BentleyStatus Run();
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct TestIModelCreator : TestFileCreator
    {
protected:
    TestIModelCreator() : TestFileCreator() {}

public:
    virtual ~TestIModelCreator() {}

    static DgnDbPtr CreateNewTestFile(Utf8CP fileName);
    BentleyStatus ImportSchema(DgnDbR dgndb, SchemaItem const& schema) { return ImportSchemas(dgndb, {schema}); }
    static BentleyStatus ImportSchemas(DgnDbR dgndb, std::vector<SchemaItem> const& schemas);
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EmptyTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override { return CreateNewTestFile(TESTIMODEL_EMPTY) != nullptr ? SUCCESS : ERROR;  }

    public:
        ~EmptyTestIModelCreator() {}
    };
