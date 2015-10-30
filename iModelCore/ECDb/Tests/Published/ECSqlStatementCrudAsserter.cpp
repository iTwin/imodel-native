/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlStatementCrudAsserter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementCrudAsserter.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//*************** ECSqlAsserter ***************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  03/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
NativeLogging::ILogger* ECSqlAsserter::s_logger = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlAsserter::Assert(ECSqlTestItem const& testItem) const
    {
    _Assert(testItem);
    LogECSqlSupport(testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlAsserter::LogECSqlSupport(ECSqlTestItem const& testItem) const
    {
    if (GetLogger().isSeverityEnabled(NativeLogging::LOG_DEBUG))
        {
        const auto expectedResultCategory = testItem.GetExpectedResultCategory();
        const auto expectedResultCategoryStr = ECSqlExpectedResult::CategoryToString(expectedResultCategory);

        Utf8String logMessage (expectedResultCategoryStr);
        logMessage.append(": ");

        auto description = testItem.GetExpectedResultDescription();
        if (!Utf8String::IsNullOrEmpty(description))
            logMessage.append(description).append(" ");

        logMessage.append(testItem.GetECSql().c_str());
        GetLogger().debug(logMessage.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  03/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
NativeLogging::ILogger& ECSqlAsserter::GetLogger()
    {
    if (s_logger == nullptr)
        {
        s_logger = NativeLogging::LoggingManager::GetLogger(L"ECSqlSupport");
        }

    return *s_logger;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlAsserter::AssertPrepare (ECSqlTestItem const& testItem, ECSqlStatement& statement, PrepareECSqlExpectedResult const& expectedResult) const
    {
    const auto expectedToSucceed = expectedResult.IsExpectedToSucceed ();

    auto ecsql = testItem.GetECSql ().c_str ();
    auto stat = PrepareStatement (statement, ecsql, !expectedToSucceed);

    if (expectedToSucceed)
        ASSERT_EQ (ECSqlStatus::Success, stat) << "Preparation failed unexpectedly.";

    if (stat != ECSqlStatus::Success)
        {
        ASSERT_FALSE (expectedToSucceed) << "Preparation did not fail with expected error code.";
        return;
        }

    stat = BindParameters (statement, testItem.GetParameterValues (), !expectedToSucceed);
    
    if (expectedToSucceed)
        ASSERT_EQ (ECSqlStatus::Success, stat) << "Binding parameters failed unexpectedly.";
    else
        {
        Utf8CP assertMessage = nullptr;
        if (testItem.GetParameterValues ().empty ())
            assertMessage = "Preparation did not fail with expected error code.";
        else
            assertMessage = "Preparation or binding did not fail with expected error code.";

        ASSERT_FALSE (stat == ECSqlStatus::Success) << assertMessage << " " << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlAsserter::PrepareStatement (ECSqlStatement& statement, Utf8CP ecsql, bool disableBeAsserts) const
    {
    DisableBeAsserts d (disableBeAsserts);

    return statement.Prepare (GetECDb(), ecsql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlAsserter::BindParameters (ECSqlStatement& statement, vector<ECSqlTestItem::ParameterValue> const& parameterValues, bool disableBeAsserts) const
    {
    DisableBeAsserts d (disableBeAsserts);

    for (size_t i = 0; i < parameterValues.size (); i++)
        {
        auto const& parameterValue = parameterValues[i];
        int parameterIndex = -1;
        if (parameterValue.IsNamedParameter ())
            parameterIndex = statement.GetParameterIndex (parameterValue.GetName ());
        else
            //parameters are 1-based in ECSQL
            parameterIndex = static_cast<int> (i) + 1;

        if (parameterValue.IsDateTime ())
            {
            auto stat = BindDateTimeParameter (statement, parameterIndex, parameterValue.GetDateTime ());
            if (stat != ECSqlStatus::Success)
                return stat;

            continue;
            }
        else if (parameterValue.IsIGeometry ())
            {
            auto stat = BindIGeometryParameter (statement, parameterIndex, parameterValue.GetIGeometry ());
            if (stat != ECSqlStatus::Success)
                return stat;

            continue;
            }
        else if (parameterValue.IsECInstanceId ())
            {
            auto stat = statement.BindId (parameterIndex, parameterValue.GetECInstanceId ());
            if (stat != ECSqlStatus::Success)
                return stat;

            continue;
            }

        auto const& value = parameterValue.GetValue ();
        if (value.IsNull ())
            {
            auto stat = statement.BindNull (parameterIndex);
            if (stat != ECSqlStatus::Success)
                return stat;

            continue;
            }

        if (!value.IsPrimitive ())
            return ECSqlStatus::Error;

        auto stat = ECSqlStatus::Success;
        switch (value.GetPrimitiveType ())
            {
            case ECN::PRIMITIVETYPE_Binary:
                {
                size_t blobSize;
                auto blob = value.GetBinary (blobSize);
                stat = statement.BindBinary (parameterIndex, static_cast<const void* const> (blob), (int) blobSize, IECSqlBinder::MakeCopy::No);
                break;
                }
            case ECN::PRIMITIVETYPE_Boolean:
                {
                stat = statement.BindBoolean (parameterIndex, value.GetBoolean ());
                break;
                }
            case ECN::PRIMITIVETYPE_DateTime:
                {
                BeAssert (false && "DateTime should have already been handled separately");
                break;
                }
            case ECN::PRIMITIVETYPE_Double:
                {
                stat = statement.BindDouble (parameterIndex, value.GetDouble ());
                break;
                }
            case ECN::PRIMITIVETYPE_IGeometry:
                {
                BeAssert (false && "IGeometry should have already been handled separately");
                break;
                }
            case ECN::PRIMITIVETYPE_Integer:
                {
                stat = statement.BindInt (parameterIndex, value.GetInteger ());
                break;
                }
            case ECN::PRIMITIVETYPE_Long:
                {
                stat = statement.BindInt64 (parameterIndex, value.GetLong ());
                break;
                }
            case ECN::PRIMITIVETYPE_Point2D:
                {
                stat = statement.BindPoint2D (parameterIndex, value.GetPoint2D ());
                break;
                }
            case ECN::PRIMITIVETYPE_Point3D:
                {
                stat = statement.BindPoint3D (parameterIndex, value.GetPoint3D ());
                break;
                }
            case ECN::PRIMITIVETYPE_String:
                {
                stat = statement.BindText (parameterIndex, value.GetUtf8CP (), IECSqlBinder::MakeCopy::Yes);
                break;
                }
            }

        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlAsserter::BindDateTimeParameter (ECSqlStatement& statement, int parameterIndex, DateTimeCR dateTimeParameter)
    {
    return statement.BindDateTime (parameterIndex, dateTimeParameter);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlAsserter::BindIGeometryParameter (ECSqlStatement& statement, int parameterIndex, IGeometryCP geomParameter)
    {
    //if geom is null, return, as default is to bind null
    if (geomParameter == nullptr)
        return ECSqlStatus::Success;

    return statement.BindGeometry (parameterIndex, *geomParameter);
    }

//*************** ECSqlSelectAsserter ***************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlSelectAsserter::_Assert (ECSqlTestItem const& testItem) const
    {
    PrepareECSqlExpectedResult const* expectedResultForPrepare = nullptr;
    ResultCountECSqlExpectedResult const* expectedResultForResultCount = nullptr;
    const auto assertPrepare = testItem.GetExpectedResults ().TryGet<PrepareECSqlExpectedResult> (expectedResultForPrepare, ECSqlExpectedResult::Type::Prepare);
    const auto assertStepSelect = testItem.GetExpectedResults ().TryGet<ResultCountECSqlExpectedResult> (expectedResultForResultCount, ECSqlExpectedResult::Type::ResultCount);

    ECSqlExpectedResult const* expectedResultOfLastStep = nullptr;
    ECSqlStatement statement;
    if (assertPrepare)
        {
        AssertPrepare (testItem, statement, *expectedResultForPrepare);
        expectedResultOfLastStep = expectedResultForPrepare;
        }

    if (assertStepSelect)
        {
        AssertStep (testItem, statement, *expectedResultForResultCount);
        expectedResultOfLastStep = expectedResultForResultCount;
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlSelectAsserter::AssertStep (ECSqlTestItem const& testItem, ECSqlStatement& statement, ResultCountECSqlExpectedResult const& expectedResult) const
    {
    const auto expectedToSucceed = expectedResult.IsExpectedToSucceed ();
    auto stat = Step (statement, !expectedToSucceed);

    auto ecsql = testItem.GetECSql ().c_str ();

    //If step failed, assert that this was as expected.
    if (stat != BE_SQLITE_ROW && stat != BE_SQLITE_DONE)
        {
        ASSERT_FALSE (expectedToSucceed) << "Step should have failed for ECSQL '" << ecsql << "'";
        return;
        }

    //if first step succeeded, perform other tests
    const auto expectedColumnCount = expectedResult.GetExpectedColumnCount ();
    const auto actualColumnCount = statement.GetColumnCount ();
    ASSERT_EQ (expectedToSucceed, expectedColumnCount == actualColumnCount) << "Unexpected result set column count for ECSQL '" << ecsql << "'. Expected column count: " << expectedColumnCount << " Actual column count: " << actualColumnCount;

    auto actualRowCount = 0;
    while (stat == BE_SQLITE_ROW)
        {
        //first step already happened further up, so we have at least one row if we enter the while loop.
        actualRowCount++;
        AssertCurrentRow (testItem, statement);
        stat = statement.Step ();
        }

    ASSERT_NE (BE_SQLITE_ERROR, stat) << "Step failed for ECSQL '" << ecsql;

    if (expectedResult.HasExpectedRowCount ())
        {
        const auto expectedRowCount = expectedResult.GetExpectedRowCount ();
        ASSERT_EQ (expectedToSucceed, expectedRowCount == actualRowCount) << "Unexpected row count for ECSQL '" << ecsql << "'. Expected row count: " << expectedRowCount << " Actual row count: " << actualRowCount;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlSelectAsserter::AssertCurrentRow (ECSqlTestItem const& testItem, ECSqlStatement const& statement) const
    {
    int columnCount = statement.GetColumnCount ();
    for (int i = 0; i < columnCount; i++)
        {
        IECSqlValue const& value = statement.GetValue (i);
        AssertCurrentCell (testItem, statement, value, nullptr);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlSelectAsserter::AssertCurrentCell (ECSqlTestItem const& testItem, ECSqlStatement const& statement, IECSqlValue const& ecsqlValue, ECTypeDescriptor const* parentDataType) const
    {
    AssertColumnInfo (testItem, statement, ecsqlValue, parentDataType);

    auto const& columnInfo = ecsqlValue.GetColumnInfo ();
    auto const& dataType = columnInfo.GetDataType ();

    if (dataType.IsPrimitive ())
        {
        AssertCurrentCell (testItem, statement, ecsqlValue, dataType, CreateIsExpectedToSucceedDelegateForAssertCurrentRow (parentDataType, dataType));
        }
    else if (dataType.IsStruct ())
        {
        auto const& structValue = ecsqlValue.GetStruct ();
        for (int i = 0; i < structValue.GetMemberCount (); i++)
            AssertCurrentCell (testItem, statement, structValue.GetValue (i), &dataType);
        }
    else // array
        {
        auto const& arrayValue = ecsqlValue.GetArray ();
        AssertArrayCell (testItem, statement, arrayValue, dataType);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlSelectAsserter::AssertCurrentCell (ECSqlTestItem const& testItem, ECSqlStatement const& statement, IECSqlValue const& value, ECTypeDescriptor const& columnDataType,
                                                   std::function<bool (ECN::ECTypeDescriptor const&)> isExpectedToSucceedDelegate) const
    {
    if (value.IsNull ())
        return;

    auto getValueCallList = CreateGetValueCallList (value);
    for (auto const& getValueCallItem : getValueCallList)
        {
        auto const& getValueDataType = getValueCallItem.first;
        auto const& getValueCall = getValueCallItem.second;
        const bool expectedToSucceed = isExpectedToSucceedDelegate (getValueDataType);

        if (!expectedToSucceed)
            BeTest::SetFailOnAssert (false);

        ECDbIssueListener issueListener(GetECDb());

        //do the actual call to IECSqlValue::GetXXX
        getValueCall();
        
        if (!expectedToSucceed)
            BeTest::SetFailOnAssert (true);

        ECDbIssue issue = issueListener.GetIssue();

        Utf8String assertMessage (GetValueCallToString (getValueDataType));
        assertMessage.append (" for a ").append (DataTypeToString (columnDataType)).append (" column.");

        if (expectedToSucceed)
            ASSERT_FALSE (issue.IsIssue()) << assertMessage.c_str ();
        else
            ASSERT_TRUE(issue.IsIssue()) << assertMessage.c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlSelectAsserter::AssertArrayCell (ECSqlTestItem const& testItem, ECSqlStatement const& statement, IECSqlArrayValue const& arrayValue, ECTypeDescriptor const& arrayType) const
    {
    for (IECSqlValue const* arrayElement : arrayValue)
        {
        AssertCurrentCell (testItem, statement, *arrayElement, &arrayType);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlSelectAsserter::AssertColumnInfo (ECSqlTestItem const& testItem, ECSqlStatement const& statement, IECSqlValue const& value, ECTypeDescriptor const* parentDataType) const
    {
    ECDbIssueListener issueListener(GetECDb());
    auto const& columnInfo = value.GetColumnInfo ();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "IECSqlValue::GetColumnInfo unexpectedly caused an error.";

    auto prop = columnInfo.GetProperty ();
    auto const& propPath = columnInfo.GetPropertyPath ();
    auto const& rootClass = columnInfo.GetRootClass ();

    if (columnInfo.IsGeneratedProperty ())
        {
        EXPECT_STREQ ("ECSqlSelectClause", rootClass.GetName ().c_str ()) << "IECSqlValue::GetColumnInfo().GetRootClass() is expected to return the anonymous class for generated properties.";
        EXPECT_TRUE (Utf8String::IsNullOrEmpty (columnInfo.GetRootClassAlias ())) << "IECSqlValue::GetColumnInfo ().GetRootClassAlias() is expected to return nullptr for generated properties.";
        }

    EXPECT_GT ((int)propPath.Size (), 0) << "IECSqlValue::GetColumnInfo().GetPropertyPath() is not expected to return empty property path.";
    EXPECT_FALSE (propPath.ToString ().empty ()) << "IECSqlValue::GetColumnInfo().GetPropertyPath().ToString() is not expected to be empty.";

    if (parentDataType != nullptr && parentDataType->IsArray ())
        {
        // array element value
        ASSERT_TRUE (columnInfo.GetProperty () == nullptr) << "Array IECSqlValue::GetColumnInfo().GetProperty() is expected to return null";
        ASSERT_EQ (static_cast<int> (ECSqlPropertyPath::Entry::Kind::ArrayIndex), static_cast<int> (propPath.GetLeafEntry ().GetKind ())) << "For primitive array values the leaf entry of the prop path is expected to be the current array index.";
        }
    else
        {
        //not an array element value
        ASSERT_TRUE (prop != nullptr) << "IECSqlValue::GetColumnInfo().GetProperty() is not expected to return nullptr.";
        ASSERT_EQ (static_cast<int> (ECSqlPropertyPath::Entry::Kind::Property), static_cast<int> (propPath.GetLeafEntry ().GetKind ())) << "For non-array values the leaf entry of the prop path is never expected to be an array index.";;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECSqlSelectAsserter::Step (ECSqlStatement& statement, bool disableBeAsserts) const
    {
    DisableBeAsserts d (disableBeAsserts);
    return statement.Step ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlSelectAsserter::GetValueCallList ECSqlSelectAsserter::CreateGetValueCallList (IECSqlValue const& value)
    {
    GetValueCallList list;
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_Binary),
        [&value] () 
        {
        int blobSize = -1;
        value.GetBinary (&blobSize); 
        }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_Boolean),
        [&value] () {value.GetBoolean (); }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_DateTime),
        [&value] () {value.GetDateTime (); }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_Double),
        [&value] () {value.GetDouble (); }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_IGeometry),
        [&value] () {value.GetGeometry ();  }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_Integer),
        [&value] () {value.GetInt (); }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_Long),
        [&value] () {value.GetInt64 (); }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_Point2D),
        [&value] () {value.GetPoint2D (); }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_Point3D),
        [&value] () {value.GetPoint3D (); }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreatePrimitiveTypeDescriptor (PRIMITIVETYPE_String),
        [&value] () {value.GetText (); }));
    list.push_back (GetValueCall (ECTypeDescriptor::CreateStructTypeDescriptor (),
        [&value] () {value.GetStruct (); }));
    //using struct array descriptor here, but will use it as array descriptor generically
    list.push_back (GetValueCall (ECTypeDescriptor::CreateStructArrayTypeDescriptor (),
        [&value] () {value.GetArray (); }));

    return list;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::function<bool (ECN::ECTypeDescriptor const&)> ECSqlSelectAsserter::CreateIsExpectedToSucceedDelegateForAssertCurrentRow (ECTypeDescriptor const* parentDataType, ECTypeDescriptor const& dataType) 
    {
    //for prim array elements, we need exact type match
    if (parentDataType != nullptr && parentDataType->IsPrimitiveArray ())
        {
        return [&dataType] (ECTypeDescriptor const& requestedDataType) 
            {
            return requestedDataType.IsPrimitive () && dataType.GetPrimitiveType () == requestedDataType.GetPrimitiveType ();
            };
        }

    //regular case allows implicit conversions between prim types (except date time and points)
    return [&dataType] (ECTypeDescriptor const& requestedDataType) 
        {
        bool isExpectedToSucceed = (dataType.IsPrimitive () && requestedDataType.IsPrimitive ()) ||
            (dataType.IsStruct () && requestedDataType.IsStruct ()) ||
            (dataType.IsArray () && requestedDataType.IsArray ());

        if (!isExpectedToSucceed)
            return false;

        if (dataType.IsPrimitive ())
            {
            auto primType = dataType.GetPrimitiveType ();
            auto requestedPrimType = requestedDataType.GetPrimitiveType ();
            if (primType == PRIMITIVETYPE_DateTime || requestedPrimType == PRIMITIVETYPE_DateTime ||
                primType == PRIMITIVETYPE_Point2D || requestedPrimType == PRIMITIVETYPE_Point2D ||
                primType == PRIMITIVETYPE_Point3D || requestedPrimType == PRIMITIVETYPE_Point3D ||
                primType == PRIMITIVETYPE_IGeometry || requestedPrimType == PRIMITIVETYPE_IGeometry)
                return primType == requestedPrimType;
            }

        return true;
        };
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String ECSqlSelectAsserter::GetValueCallToString (ECTypeDescriptor const& dataType)
    {
    if (dataType.IsPrimitive ())
        {
        Utf8String str ("Get");
        str.append (DataTypeToString (dataType));
        return str;
        }
    else if (dataType.IsStruct ())
        return "GetStruct";
    else
        return "GetArray";

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String ECSqlSelectAsserter::DataTypeToString (ECTypeDescriptor const& dataType)
    {
    if (dataType.IsPrimitive ())
        {
        switch (dataType.GetPrimitiveType ())
            {
            case PRIMITIVETYPE_Binary:
                return "Binary";

            case PRIMITIVETYPE_Boolean:
                return "Boolean";

            case PRIMITIVETYPE_DateTime:
                return "DateTime";

            case PRIMITIVETYPE_Double:
                return "Double";

            case PRIMITIVETYPE_Integer:
                return "Int";

            case PRIMITIVETYPE_Long:
                return "int64_t";

            case PRIMITIVETYPE_Point2D:
                return "Point2D";

            case PRIMITIVETYPE_Point3D:
                return "Point3D";

            case PRIMITIVETYPE_String:
                return "String";

            case PRIMITIVETYPE_IGeometry:
                return "IGeometry";
            }
        }
    else if (dataType.IsStruct ())
        return "ECStruct";
    else if (dataType.IsArray ())
        return "ECArray";

    BeAssert (false);
    return "";
    }


//*************** ECSqlNonSelectAsserter ***************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlNonSelectAsserter::_Assert (ECSqlTestItem const& testItem) const
    {
    PrepareECSqlExpectedResult const* expectedResultForPrepare = nullptr;
    ECSqlExpectedResult const* expectedResultForStep = nullptr;
    const auto assertPrepare = testItem.GetExpectedResults ().TryGet<PrepareECSqlExpectedResult> (expectedResultForPrepare, ECSqlExpectedResult::Type::Prepare);
    const auto assertStepNonSelect = testItem.GetExpectedResults ().TryGet<ECSqlExpectedResult> (expectedResultForStep, ECSqlExpectedResult::Type::Generic);

    ECSqlExpectedResult const* expectedResultOfLastStep = nullptr;
    ECSqlStatement statement;
    if (assertPrepare)
        {
        AssertPrepare (testItem, statement, *expectedResultForPrepare);
        expectedResultOfLastStep = expectedResultForPrepare;
        }

    if (assertStepNonSelect)
        {
        AssertStep (testItem, statement, *expectedResultForStep);
        expectedResultOfLastStep = expectedResultForStep;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlNonSelectAsserter::AssertStep (ECSqlTestItem const& testItem, ECSqlStatement& statement, ECSqlExpectedResult const& expectedResult) const
    {
    const bool expectedToSucceed = expectedResult.IsExpectedToSucceed ();

    Utf8CP ecsql = testItem.GetECSql().c_str ();
    //if this is an insert statement test the ECInstanceId overload, too
    if (BeStringUtilities::Strnicmp ("INSERT INTO", ecsql, 11) == 0)
        {
        ECInstanceKey ecInstanceKey;
        DbResult stat = Step (ecInstanceKey, statement, !expectedToSucceed);

        if (expectedToSucceed)
            {
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Step (ECInstanceKey&) should have succeeded for ECSQL '" << ecsql << "'";
            ASSERT_TRUE (ecInstanceKey.IsValid ());
            }
        else
            ASSERT_NE(BE_SQLITE_DONE, stat) << "Step (ECInstanceKey&) should have failed for ECSQL '" << ecsql << "'.";

        return;
        }

    DbResult stepStat = Step(statement, !expectedToSucceed);
    if (expectedToSucceed)
        ASSERT_EQ(BE_SQLITE_DONE, stepStat) << "Step should have succeeded for ECSQL '" << ecsql << "'.";
    else
        ASSERT_NE(BE_SQLITE_DONE, stepStat) << "Step should have failed for ECSQL '" << ecsql << "'.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECSqlNonSelectAsserter::Step (ECInstanceKey& generatedECInstanceKey, ECSqlStatement& statement, bool disableBeAsserts) const
    {
    DisableBeAsserts d (disableBeAsserts);

    return statement.Step (generatedECInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECSqlNonSelectAsserter::Step (ECSqlStatement& statement, bool disableBeAsserts) const
    {
    DisableBeAsserts d (disableBeAsserts);

    return statement.Step ();
    }

END_ECDBUNITTESTS_NAMESPACE


