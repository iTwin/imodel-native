/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

#include <cmath> // for std::pow

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlCustomFunctionTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PowSqlFunction final : ScalarFunction
    {
private:

    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].IsNull() || args[1].IsNull())
            {
            ctx.SetResultError("Arguments to POW must not be NULL", -1);
            return;
            }

        double base = args[0].GetValueDouble();
        double exp = args[1].GetValueDouble();

        double res = std::pow(base, exp);
        ctx.SetResultDouble(res);
        }

public:
    PowSqlFunction() : ScalarFunction("POW", 2, DbValueType::FloatVal) {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct IntegerToBlobSqlFunction final : ScalarFunction
    {
    private:

        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
            {
            const void* blob = nullptr;
            int blobSize = 0;
            int64_t i = 0LL;
            if (!args[0].IsNull())
                {
                i = args[0].GetValueInt64();
                blob = &i;
                blobSize = (int) sizeof(i);
                }

            ctx.SetResultBlob(blob, blobSize, DbFunction::Context::CopyData::Yes);
            }

    public:
        IntegerToBlobSqlFunction() : ScalarFunction("INTEGERTOBLOB", 1, DbValueType::BlobVal) {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 05/15
//+---------------+---------------+---------------+---------------+---------------+------
struct ToBoolStrSqlFunction final : ScalarFunction
    {
    private:
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
            {
            if (nArgs != 1)
                {
                ctx.SetResultError("Function TOBOOLSTR expects 1 argument.", -1);
                return;
                }

            DbValue const& arg = args[0];
            if (arg.IsNull())
                {
                ctx.SetResultNull();
                return;
                }

            Utf8String val = arg.GetValueDouble() != 0.0 ? "true" : "false";
            ctx.SetResultText(val.c_str(), (int) val.size(), DbFunction::Context::CopyData::Yes);
            }

    public:
        ToBoolStrSqlFunction() : ScalarFunction("TOBOOLSTR", 1, DbValueType::TextVal) {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
 struct SumOfSquares final : AggregateFunction
     {
     private:

         double runningSum = 0;
         int valueToSet = 0;
         void _StepAggregate (AggregateFunction::Context& ctx, int nArgs, DbValue* args) override
             {
             if (nArgs != 1)
                 {
                 valueToSet = 2;
                 return;
                 }
             if (args[0].IsNull ())
                 {
                 valueToSet = 2;
                 return;
                 }

             double currentVal = std::pow (args[0].GetValueDouble (), 2);
             runningSum += currentVal;
             valueToSet = 1;
             }

         void _FinishAggregate (AggregateFunction::Context& ctx) override
             {
             switch (valueToSet)
                 {
                     case 1:
                         {
                         ctx.SetResultDouble (runningSum);
                         break;
                         }
                     case 2:
                         {
                         ctx.SetResultError ("SOS Function Expects one non NULL Parameter", -1);
                         break;
                         }
                     default:
                         break;
                 }
             }

     public:
         SumOfSquares () : AggregateFunction ("SOS", 1, DbValueType::FloatVal) {}
     };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlCustomFunctionTestFixture, RegisterUnregisterCustomSqlFunction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(3));

    PowSqlFunction func;
    ASSERT_EQ(0, m_ecdb.AddFunction(func));

    PowSqlFunction func2;
    ASSERT_EQ(0, m_ecdb.AddFunction(func2)) << "Adding same ECSQL function twice is expected to succeed.";

    ASSERT_EQ(0, m_ecdb.RemoveFunction(func));
    ASSERT_EQ(0, m_ecdb.RemoveFunction(func)) << "Removing an unregistered ECSQL function is expected to succeed";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                    01/19
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlCustomFunctionTestFixture, searchForAFunction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("searchForAFunction.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(3));

    PowSqlFunction powSqlFunction;
    ASSERT_EQ(0, m_ecdb.AddFunction(powSqlFunction));

    IntegerToBlobSqlFunction integerToBlobSqlFunction;;
    ASSERT_EQ(0, m_ecdb.AddFunction(integerToBlobSqlFunction));

    ToBoolStrSqlFunction toBoolStrSqlFunction;
    ASSERT_EQ(0, m_ecdb.AddFunction(toBoolStrSqlFunction));

    SumOfSquares sumOfSquares;
    ASSERT_EQ(0, m_ecdb.AddFunction(sumOfSquares));

    DbFunction* tmp;
    ASSERT_TRUE(m_ecdb.TryGetSqlFunction(tmp, powSqlFunction.GetName(), powSqlFunction.GetNumArgs()));
    ASSERT_EQ(tmp, &powSqlFunction);

    ASSERT_TRUE(m_ecdb.TryGetSqlFunction(tmp, integerToBlobSqlFunction.GetName(), integerToBlobSqlFunction.GetNumArgs()));
    ASSERT_EQ(tmp, &integerToBlobSqlFunction);

    ASSERT_TRUE(m_ecdb.TryGetSqlFunction(tmp, toBoolStrSqlFunction.GetName(), toBoolStrSqlFunction.GetNumArgs()));
    ASSERT_EQ(tmp, &toBoolStrSqlFunction);

    ASSERT_TRUE(m_ecdb.TryGetSqlFunction(tmp, sumOfSquares.GetName(), sumOfSquares.GetNumArgs()));
    ASSERT_EQ(tmp, &sumOfSquares);

    ASSERT_EQ(0, m_ecdb.RemoveFunction(powSqlFunction));
    ASSERT_EQ(0, m_ecdb.RemoveFunction(integerToBlobSqlFunction));
    ASSERT_EQ(0, m_ecdb.RemoveFunction(toBoolStrSqlFunction));
    ASSERT_EQ(0, m_ecdb.RemoveFunction(sumOfSquares));

    ASSERT_FALSE(m_ecdb.TryGetSqlFunction(tmp, powSqlFunction.GetName(), powSqlFunction.GetNumArgs()));
    ASSERT_FALSE(m_ecdb.TryGetSqlFunction(tmp, integerToBlobSqlFunction.GetName(), integerToBlobSqlFunction.GetNumArgs()));
    ASSERT_FALSE(m_ecdb.TryGetSqlFunction(tmp, toBoolStrSqlFunction.GetName(), toBoolStrSqlFunction.GetNumArgs()));
    ASSERT_FALSE(m_ecdb.TryGetSqlFunction(tmp, sumOfSquares.GetName(), sumOfSquares.GetNumArgs()));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlCustomFunctionTestFixture, CallUnregisteredSqlFunction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(3));

    Utf8CP ecsql = "SELECT I,POW(I,2) FROM ecsql.P";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql)) << "ECSQL preparation expected to fail with unregistered custom ECSQL function";

    PowSqlFunction func;
    ASSERT_EQ(0, m_ecdb.AddFunction(func));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << "ECSQL preparation expected to succeed with registered custom ECSQL function";
    ASSERT_EQ(0, m_ecdb.RemoveFunction(func));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlCustomFunctionTestFixture, NumericSqlFunction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("NumericSqlFunction.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ecsql.P (I) VALUES (10)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ecsql.P (I) VALUES (-4)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)"));

    
    PowSqlFunction func;
    ASSERT_EQ(0, m_ecdb.AddFunction(func));

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I,POW(I,2) FROM ecsql.P WHERE I IS NOT NULL"));

        int rowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            rowCount++;

            int base = stmt.GetValueInt(0);
            int actualFuncResult = stmt.GetValueInt(1);

            ASSERT_EQ(std::pow(base, 2), actualFuncResult);
            }

        ASSERT_EQ(2, rowCount);
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I, POW(I,?) FROM ecsql.P WHERE I IS NOT NULL"));

        int exp = 3;
        stmt.BindInt(1, exp);

        while (stmt.Step() == BE_SQLITE_ROW)
            {
            int base = stmt.GetValueInt(0);
            int actualFuncResult = stmt.GetValueInt(1);

            ASSERT_EQ(std::pow(base, exp), actualFuncResult);
            }

        stmt.Reset();
        stmt.ClearBindings();
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT POW(I,NULL) FROM ecsql.P"));

        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT POW(NULL,2) FROM ecsql.P"));
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT POW(NULL,NULL) FROM ecsql.P"));
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT POW(I,2) FROM ecsql.P WHERE I IS NULL"));
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

    ASSERT_EQ(0, m_ecdb.RemoveFunction(func));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlCustomFunctionTestFixture, StringSqlFunction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(3));

    ToBoolStrSqlFunction func;
    ASSERT_EQ(0, m_ecdb.AddFunction(func));

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TOBOOLSTR(1) FROM ecsql.P LIMIT 1")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        IECSqlValue const& val = stmt.GetValue(0);
        ASSERT_STREQ("true", val.GetText());
        ASSERT_EQ(ECN::PRIMITIVETYPE_String, val.GetColumnInfo().GetDataType().GetPrimitiveType());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlCustomFunctionTestFixture, BlobSqlFunction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(3));

    //insert one more test row which has a NULL column
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        m_ecdb.SaveChanges();
        }

        IntegerToBlobSqlFunction func;
        ASSERT_EQ(0, m_ecdb.AddFunction(func));

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT L,INTEGERTOBLOB(L) FROM ecsql.P WHERE L IS NOT NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        int rowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            rowCount++;
            int64_t expectedInt = stmt.GetValueInt64(0);
            int blobSize = -1;
            void const* actualBlob = stmt.GetValueBlob(1, &blobSize);

            ASSERT_EQ(sizeof(expectedInt), blobSize);
            int64_t actualInt = -1LL;
            memcpy(&actualInt, actualBlob, sizeof(actualInt));
            ASSERT_EQ(expectedInt, actualInt);
            }

        ASSERT_TRUE(rowCount > 0);
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT L,INTEGERTOBLOB(L) FROM ecsql.P WHERE L IS NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        int rowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            rowCount++;
            int blobSize = -1;
            void const* actualBlob = stmt.GetValueBlob(1, &blobSize);

            ASSERT_EQ(0, blobSize);
            ASSERT_TRUE(actualBlob == nullptr);
            }

        ASSERT_TRUE(rowCount > 0);
        }
    ASSERT_EQ(0, m_ecdb.RemoveFunction(func));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
 TEST_F (ECSqlCustomFunctionTestFixture, AggregateFunction)
     {
     ASSERT_EQ(SUCCESS, SetupECDb("ecsqlfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
     ASSERT_EQ(SUCCESS, PopulateECDb(3));

     SumOfSquares func;
     ECSqlStatement stmt;

     ASSERT_EQ (ECSqlStatus::InvalidECSql, stmt.Prepare (m_ecdb, "SELECT SOS(D) FROM ecsql.P")) << "ECSql Preparetion expected to fail with unregistered ECSql function";
     stmt.Finalize ();

     ASSERT_EQ (0, m_ecdb.AddFunction (func));

     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare (m_ecdb, "SELECT SOS(D) FROM ecsql.P")) << "ECSql Preparetion expected to succeed after adding custom ECSql function to Db";
     ASSERT_TRUE (stmt.Step () == BE_SQLITE_ROW);
     double actualSumOfSquares = stmt.GetValueDouble (0);
     stmt.Finalize ();

     ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (m_ecdb, "SELECT D FROM ecsql.P"));
     double expectedSumOfSquares = 0;

     while (stmt.Step () != BE_SQLITE_DONE)
         {
         expectedSumOfSquares += std::pow (stmt.GetValueDouble (0), 2);
         }
     //Verify that the value returned by the Custom ECSql function is the same is expected.
     ASSERT_EQ (expectedSumOfSquares, actualSumOfSquares);
     stmt.Finalize ();

     ASSERT_EQ (0, m_ecdb.RemoveFunction (func));
     ASSERT_EQ (ECSqlStatus::InvalidECSql, stmt.Prepare (m_ecdb, "SELECT SOS(D) FROM ecsql.P ")) << "ECSql Preparation expected to fail when custom ECSql function is removed from Db";
     stmt.Finalize ();

     //Adding Custom ECSql function to the Db is expected to Succeed once it was removed 
     ASSERT_EQ (0, m_ecdb.AddFunction (func));

     ASSERT_EQ (ECSqlStatus::InvalidECSql, stmt.Prepare (m_ecdb, "SELECT SOS() FROM ecsql.P")) << "ECSql Prepration is expected to fail when no argument is passed to SOS";
     stmt.Finalize ();

     ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (m_ecdb, "SELECT SOS(NUll) FROM ecsql.P")) << "ECSql Prepration is expected to succeed when NULL is passed to SOS";
     ASSERT_EQ (BE_SQLITE_ERROR, stmt.Step())<< "Step is expected to fail if function is called with NULL argument";
     }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlCustomFunctionTestFixture, JulianDayFromStringFunction)
    {
    //---------------------------------------------------------------------------------------
    // Syntax: JULIANDAYFROMSTRING(TEXT) : JulianDay double
    // @bsiclass                                     Krischan.Eberle                 03/15
    //+---------------+---------------+---------------+---------------+---------------+------
    struct JulianDayFromStringFunction final : ScalarFunction
        {
        private:
            void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
                {
                if (nArgs != 1)
                    {
                    ctx.SetResultError("Wrong number of arguments for function JULIANDAYFROMSTRING.", -1);
                    return;
                    }

                if (args[0].IsNull())
                    {
                    ctx.SetResultError("Argument to JULIANDAYFROMSTRING must not be NULL", -1);
                    return;
                    }

                Utf8CP dateStr = args[0].GetValueText();
                DateTime dt;
                if (SUCCESS != DateTime::FromString(dt, dateStr))
                    ctx.SetResultError("Invalid date string format.", -1);
                else
                    {
                    double jd = -1.0;
                    dt.ToJulianDay(jd);
                    ctx.SetResultDouble(jd);
                    }
                }

        public:
            JulianDayFromStringFunction() : ScalarFunction("JULIANDAYFROMSTRING", 1, DbValueType::FloatVal) {}
        };

    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    DateTime dt(2015, 3, 24);
    Utf8String dtStr = dt.ToString();
    //insert test data
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P (DtUtc, S) VALUES (?,?)"));
    stmt.BindDateTime(1, dt);
    stmt.BindText(2, dtStr.c_str(), IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    m_ecdb.SaveChanges();
    }


    JulianDayFromStringFunction sqlFunc;
    ASSERT_EQ(0, m_ecdb.AddFunction(sqlFunc));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT DtUtc, JULIANDAYFROMSTRING(S) FROM ecsql.P"));

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        DateTime expectedDt = stmt.GetValueDateTime(0);
        DateTime actualDt = stmt.GetValueDateTime(1);
        ASSERT_EQ(expectedDt.GetYear(), actualDt.GetYear());
        ASSERT_EQ(expectedDt.GetMonth(), actualDt.GetMonth());
        ASSERT_EQ(expectedDt.GetDay(), actualDt.GetDay());
        ASSERT_EQ(expectedDt.GetHour(), actualDt.GetHour());
        ASSERT_EQ(expectedDt.GetMinute(), actualDt.GetMinute());
        ASSERT_EQ(expectedDt.GetSecond(), actualDt.GetSecond());
        }
    }

END_ECDBUNITTESTS_NAMESPACE