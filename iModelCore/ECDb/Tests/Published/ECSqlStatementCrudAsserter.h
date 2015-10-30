/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlStatementCrudAsserter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlCrudAsserter.h"
#include "ECSqlExpectedResultImpls.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================  
//! ECSqlAsserter for the ECSqlStatement CRUD API.
// @bsiclass                                                 Krischan.Eberle     07/2013
//=======================================================================================    
struct ECSqlStatementCrudAsserter : public ECSqlCrudAsserter
    {
private:
    virtual Utf8CP _GetTargetOperationName() const override;

protected:
    void AssertPrepare (ECSqlTestItem const& testItem, ECSqlStatement& statement, PrepareECSqlExpectedResult const& expectedResult) const;

    ECSqlStatus PrepareStatement (ECSqlStatement& statement, Utf8CP ecsql, bool disableBeAsserts) const;
    ECSqlStatus BindParameters (ECSqlStatement& statement, std::vector<ECSqlTestItem::ParameterValue> const& parameterValues, bool disableBeAsserts) const;
    static ECSqlStatus BindDateTimeParameter (ECSqlStatement& statement, int parameterIndex, DateTimeCR dateTimeParameter);
    static ECSqlStatus BindIGeometryParameter (ECSqlStatement& statement, int parameterIndex, IGeometryCP geomParameter);

public:
    explicit ECSqlStatementCrudAsserter (ECDb& ecdb) : ECSqlCrudAsserter(ecdb) {}
    virtual ~ECSqlStatementCrudAsserter () {}
    };

//=======================================================================================  
//! ECSqlAsserter for the ECSqlStatement SELECT CRUD API.
// @bsiclass                                                 Krischan.Eberle     07/2013
//=======================================================================================    
struct ECSqlSelectStatementCrudAsserter : public ECSqlStatementCrudAsserter
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
    explicit ECSqlSelectStatementCrudAsserter (ECDb& ecdb) : ECSqlStatementCrudAsserter(ecdb) {}
    ~ECSqlSelectStatementCrudAsserter () {}
    };

//=======================================================================================  
//! ECSqlAsserter for the ECSqlStatement non-SELECT CRUD API.
// @bsiclass                                                 Krischan.Eberle     11/2013
//=======================================================================================    
struct ECSqlNonSelectStatementCrudAsserter : public ECSqlStatementCrudAsserter
    {
private:
    virtual void _Assert (ECSqlTestItem const& testItem) const override;

    void AssertStep (ECSqlTestItem const& testItem, ECSqlStatement& statement, ECSqlExpectedResult const& expectedResult) const;

    DbResult Step (ECInstanceKey& generatedECInstanceKey, ECSqlStatement& statement, bool disableBeAsserts) const;
    DbResult Step (ECSqlStatement& statement, bool disableBeAsserts) const;

public:
    explicit ECSqlNonSelectStatementCrudAsserter (ECDb& ecdb) : ECSqlStatementCrudAsserter(ecdb) {}
    ~ECSqlNonSelectStatementCrudAsserter () {}
    };

END_ECDBUNITTESTS_NAMESPACE
