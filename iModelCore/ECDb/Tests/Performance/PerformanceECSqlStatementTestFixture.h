/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECSqlStatementTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDbApi.h>
#include <Bentley/NonCopyableClass.h>

#include "PerformanceTestFixture.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      10/2013
//=======================================================================================    
struct PerformanceECSqlStatementTestFixture : public PerformanceTestFixture
    {
public:
    //=======================================================================================    
    //! @bsiclass                                                 Krischan.Eberle      09/2012
    //=======================================================================================    
    struct GetValueAsserter : NonCopyableClass
        {
        private: 
            int m_propertyIndex;
            Utf8CP m_propertyName;

        protected:
            GetValueAsserter(int propertyIndex, Utf8CP propertyName) : m_propertyIndex(propertyIndex), m_propertyName(propertyName) {}

        public:
            virtual ~GetValueAsserter () {}
            int GetPropertyIndex () const { return m_propertyIndex; }
            Utf8CP GetPropertyName () const { return m_propertyName; }
            virtual void AssertGetValue (BeSQLite::EC::ECSqlStatement const& statement) const = 0;
        };

    //=======================================================================================    
    //! @bsiclass                                                 Krischan.Eberle      09/2012
    //=======================================================================================    
    struct GetIntegerValueAsserter : public GetValueAsserter
        {
        public:
            GetIntegerValueAsserter (int propertyIndex, Utf8CP propertyName) : GetValueAsserter (propertyIndex, propertyName) {}
            ~GetIntegerValueAsserter () {}

            virtual void AssertGetValue (BeSQLite::EC::ECSqlStatement const& statement) const override;
        };

    //=======================================================================================    
    //! @bsiclass                                                 Krischan.Eberle      09/2012
    //=======================================================================================    
    struct GetStringValueAsserter : public GetValueAsserter
        {
        public:
            GetStringValueAsserter(int propertyIndex, Utf8CP propertyName) : GetValueAsserter(propertyIndex, propertyName) {}
            ~GetStringValueAsserter () {}

            virtual void AssertGetValue (BeSQLite::EC::ECSqlStatement const& statement) const override;
        };

    //=======================================================================================    
    //! @bsiclass                                                 Krischan.Eberle      09/2012
    //=======================================================================================    
    struct GetPoint3DValueAsserter : public GetValueAsserter
        {
        public:
            GetPoint3DValueAsserter(int propertyIndex, Utf8CP propertyName) : GetValueAsserter(propertyIndex, propertyName) {}
            ~GetPoint3DValueAsserter () {}

            virtual void AssertGetValue (BeSQLite::EC::ECSqlStatement const& statement) const override;
        };

private:
    static WCharCP const TEST_DB_NAME;
    static Utf8CP const TEST_CLASS_NAME;
    static Utf8CP const s_selectClause;
    static const int TESTCLASS_INSTANCE_COUNT;

    static bool s_testDbExists;

    static ECN::ECClassCP GetTestClass (BeSQLite::EC::ECDbR testDb);
    static void PopulateTestDb (BeSQLite::EC::ECDbR db);

    static void CreateECSQL (BeSQLite::EC::ECSqlSelectBuilder& ecsql, ECN::ECClassCR searchClass, Utf8CP selectClause);

    static bool TestDbExists ();

protected:
    PerformanceECSqlStatementTestFixture ();
    virtual ~PerformanceECSqlStatementTestFixture () {}

    virtual void InitializeTestDb () override;

    void RunGetValueWithIsNullTest(const GetValueAsserter& asserter, StopWatch &totalStopWatch, Utf8String testDetails) const;
    void RunGetValueWithoutIsNullTest(const GetValueAsserter& asserterm, StopWatch &totalStopWatch, Utf8String testDetails) const;
    };

END_ECDBUNITTESTS_NAMESPACE
