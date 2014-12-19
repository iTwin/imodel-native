/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ECDB/Published/ECSqlCrudTestFixture.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestFixture.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     09/2013
//=======================================================================================    
struct ECSqlCrudTestFixture : public ECSqlTestFixture
    {
    protected:
        //! @deprecated Will become private once ECDbStatement API is removed
        void RunTest (ECSqlTestDataset const& dataset, ECSqlCrudAsserterList const& asserters) const;

        virtual void RunTest (ECSqlTestDataset const& dataset) const = 0;

    public:
        ECSqlCrudTestFixture () : ECSqlTestFixture () {}
        virtual ~ECSqlCrudTestFixture () {}
    };

//=======================================================================================    
// @bsiclass                                     Krischan.Eberle                  09/13
//=======================================================================================    
struct ECSqlSelectTestFixture : public ECSqlCrudTestFixture
    {
private:
    static std::unique_ptr<ECDbTestProject> s_perSessionTestProject;

    static bool HasPerSessionProject ();

    virtual ECDbR _SetUp (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount) override;
    virtual ECDbTestProject& _GetTestProject () const override;

protected:
    static const int PerClassRowCount = 10;

    virtual void RunTest (ECSqlTestDataset const& dataset) const override;
    using ECSqlCrudTestFixture::RunTest;

public:
    ECSqlSelectTestFixture () 
        : ECSqlCrudTestFixture ()
        {}

    virtual ~ECSqlSelectTestFixture () {}

    virtual void SetUp () override;
    using ECSqlCrudTestFixture::SetUp;
    };


//=======================================================================================    
// @bsiclass                                     Krischan.Eberle                  11/13
//=======================================================================================    
struct ECSqlNonSelectTestFixture : public ECSqlCrudTestFixture
    {
protected:
    static const int PerClassRowCount = 10;

    virtual void RunTest (ECSqlTestDataset const& dataset) const override;
    using ECSqlCrudTestFixture::RunTest;

public:
    ECSqlNonSelectTestFixture () 
        : ECSqlCrudTestFixture ()
        {}

    virtual ~ECSqlNonSelectTestFixture () {}

    virtual void SetUp () override;
    using ECSqlCrudTestFixture::SetUp;
    };

END_ECDBUNITTESTS_NAMESPACE