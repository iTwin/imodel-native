/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlCrudTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlCrudAsserter.h"
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     09/2013
//=======================================================================================    
struct ECSqlTestFrameworkFixture : public ECDbTestFixture
    {
protected:
    void RunTest(ECSqlTestDataset const&, ECSqlAsserterList const&) const;

    virtual void RunTest(ECSqlTestDataset const&) const = 0;

public:
    ECSqlTestFrameworkFixture() : ECDbTestFixture() {}
    virtual ~ECSqlTestFrameworkFixture() {}
    };

//=======================================================================================    
// @bsiclass                                     Krischan.Eberle                  09/13
//=======================================================================================    
struct ECSqlSelectTestFramework : public ECSqlTestFrameworkFixture
    {
protected:
    static const int PerClassRowCount = 10;

    ECSqlSelectTestFramework() : ECSqlTestFrameworkFixture() {}

    virtual void RunTest (ECSqlTestDataset const&) const override;
    using ECSqlTestFrameworkFixture::RunTest;

public:
    virtual ~ECSqlSelectTestFramework () {}

    virtual void SetUp () override;
    };


//=======================================================================================    
// @bsiclass                                     Krischan.Eberle                  11/13
//=======================================================================================    
struct ECSqlNonSelectTestFrameworkFixture : public ECSqlTestFrameworkFixture
    {
protected:
    static const int PerClassRowCount = 10;

    virtual void RunTest(ECSqlTestDataset const&) const override;
    using ECSqlTestFrameworkFixture::RunTest;

    ECSqlNonSelectTestFrameworkFixture() : ECSqlTestFrameworkFixture() {}

public:
    virtual ~ECSqlNonSelectTestFrameworkFixture() {}

    virtual void SetUp () override;
    };

END_ECDBUNITTESTS_NAMESPACE