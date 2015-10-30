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
struct ECSqlCrudTestFixture : public ECDbTestFixture
    {
protected:
    void RunTest(ECSqlTestDataset const&, ECSqlCrudAsserterList const&) const;

    virtual void RunTest(ECSqlTestDataset const&) const = 0;

public:
    ECSqlCrudTestFixture() : ECDbTestFixture() {}
    virtual ~ECSqlCrudTestFixture() {}
    };

//=======================================================================================    
// @bsiclass                                     Krischan.Eberle                  09/13
//=======================================================================================    
struct ECSqlSelectTestFixture : public ECSqlCrudTestFixture
    {
protected:
    static const int PerClassRowCount = 10;

    ECSqlSelectTestFixture() : ECSqlCrudTestFixture() {}

    virtual void RunTest (ECSqlTestDataset const&) const override;
    using ECSqlCrudTestFixture::RunTest;

public:
    virtual ~ECSqlSelectTestFixture () {}

    virtual void SetUp () override;
    };


//=======================================================================================    
// @bsiclass                                     Krischan.Eberle                  11/13
//=======================================================================================    
struct ECSqlNonSelectTestFixture : public ECSqlCrudTestFixture
    {
protected:
    static const int PerClassRowCount = 10;

    virtual void RunTest (ECSqlTestDataset const&) const override;
    using ECSqlCrudTestFixture::RunTest;

    ECSqlNonSelectTestFixture() : ECSqlCrudTestFixture() {}

public:

    virtual ~ECSqlNonSelectTestFixture () {}

    virtual void SetUp () override;
    };

END_ECDBUNITTESTS_NAMESPACE