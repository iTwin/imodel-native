/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlAsserter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestDataset.h"
#include "ECSqlExpectedResultImpls.h"
#include <Logging/bentleylogging.h>

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE

//=======================================================================================  
//! ECSqlAsserter for the ECSql test framework.
// @bsiclass                                                 Krischan.Eberle     07/2013
//=======================================================================================    
struct ECSqlAsserter
    {
protected:
    struct DisableBeAsserts
        {
        public:
            explicit DisableBeAsserts(bool disable = true)
                {
                BeTest::SetFailOnAssert(!disable);
                }

            ~DisableBeAsserts()
                {
                BeTest::SetFailOnAssert(true);
                }
        };

#define DISABLE_BEASSERTS DisableBeAsserts disableBeAsserts;

private:
    static BentleyApi::NativeLogging::ILogger* s_logger;

    ECDb& m_ecdb;

    virtual void _Assert(ECSqlTestItem const& testItem) const = 0;
    virtual Utf8CP _GetTargetOperationName() const = 0;

    void LogECSqlSupport(ECSqlTestItem const& testItem) const;
    static BentleyApi::NativeLogging::ILogger& GetLogger();

protected:
    ECDb& GetECDb() const { return m_ecdb; }

    void AssertPrepare (ECSqlTestItem const& testItem, ECSqlStatement& statement, PrepareECSqlExpectedResult const& expectedResult) const;

    ECSqlStatus PrepareStatement (ECSqlStatement& statement, Utf8CP ecsql, bool disableBeAsserts) const;
    ECSqlStatus BindParameters (ECSqlStatement& statement, std::vector<ECSqlTestItem::ParameterValue> const& parameterValues, bool disableBeAsserts) const;
    static ECSqlStatus BindDateTimeParameter (ECSqlStatement& statement, int parameterIndex, DateTimeCR dateTimeParameter);
    static ECSqlStatus BindIGeometryParameter (ECSqlStatement& statement, int parameterIndex, IGeometryCP geomParameter);

public:
    explicit ECSqlAsserter (ECDb& ecdb) {}
    virtual ~ECSqlAsserter () {}

    void Assert(ECSqlTestItem const& testItem) const;
    };

typedef std::vector<std::unique_ptr<ECSqlAsserter>> ECSqlAsserterList;

//=======================================================================================  
//! ECSqlAsserter for the ECSQL test framework for SELECT
// @bsiclass                                                 Krischan.Eberle     07/2013
//=======================================================================================    
struct ECSqlSelectAsserter : public ECSqlAsserter
    {
private:
    typedef std::pair<ECN::ECTypeDescriptor, std::function<void ()>> GetValueCall;
    typedef std::vector<GetValueCall> GetValueCallList;

    virtual void _Assert (ECSqlTestItem const& testItem) const override;

    void AssertStep (ECSqlTestItem const& testItem, ECSqlStatement& statement, ResultCountECSqlExpectedResult const& expectedResult) const;

    void AssertCurrentRow (ECSqlTestItem const& testItem, ECSqlStatement const& statement) const;
    void AssertCurrentCell (ECSqlTestItem const& testItem, ECSqlStatement const& statement, IECSqlValue const& ecsqlValue, ECN::ECTypeDescriptor const* parentDataType) const;
    void AssertCurrentCell (ECSqlTestItem const& testItem, ECSqlStatement const& statement, IECSqlValue const& ecsqlValue, ECN::ECTypeDescriptor const& columnDataType, std::function<bool (ECN::ECTypeDescriptor const&)> isExpectedToSucceedDelegate) const;
    void AssertArrayCell (ECSqlTestItem const& testItem, ECSqlStatement const& statement, IECSqlArrayValue const& ecsqlArrayValue, ECN::ECTypeDescriptor const& arrayType) const;

    void AssertColumnInfo (ECSqlTestItem const& testItem, ECSqlStatement const& statement, IECSqlValue const& ecsqlValue, ECN::ECTypeDescriptor const* parentDataType) const;

    DbResult Step (ECSqlStatement& statement, bool disableBeAsserts) const;

    static std::function<bool (ECN::ECTypeDescriptor const&)> CreateIsExpectedToSucceedDelegateForAssertCurrentRow (ECN::ECTypeDescriptor const* parentDataType, ECN::ECTypeDescriptor const& dataType);
    static GetValueCallList CreateGetValueCallList (IECSqlValue const& ecsqlValue);
    static Utf8String GetValueCallToString (ECN::ECTypeDescriptor const& dataType);
    static Utf8String DataTypeToString (ECN::ECTypeDescriptor const& dataType);

public:
    explicit ECSqlSelectAsserter (ECDb& ecdb) : ECSqlAsserter(ecdb) {}
    ~ECSqlSelectAsserter () {}
    };

//=======================================================================================  
//! ECSqlAsserter for the ECSQL test framework for non-SELECT
// @bsiclass                                                 Krischan.Eberle     11/2013
//=======================================================================================    
struct ECSqlNonSelectAsserter : public ECSqlAsserter
    {
private:
    virtual void _Assert (ECSqlTestItem const& testItem) const override;

    void AssertStep (ECSqlTestItem const& testItem, ECSqlStatement& statement, ECSqlExpectedResult const& expectedResult) const;

    DbResult Step (ECInstanceKey& generatedECInstanceKey, ECSqlStatement& statement, bool disableBeAsserts) const;
    DbResult Step (ECSqlStatement& statement, bool disableBeAsserts) const;

public:
    explicit ECSqlNonSelectAsserter (ECDb& ecdb) : ECSqlAsserter(ecdb) {}
    ~ECSqlNonSelectAsserter () {}
    };

END_ECSQLTESTFRAMEWORK_NAMESPACE
