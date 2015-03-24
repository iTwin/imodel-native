/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlCustomFunction_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFixture.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PowECSqlFunction : ECSqlFunction, ScalarFunction::IScalar
    {
private:

    virtual void _ComputeScalar(ScalarFunction::Context* ctx, int nArgs, DbValue* args) override
        {
        if (nArgs != GetArgCount ())
            {
            ctx->SetResultError("Function POW expects 2 arguments.", -1);
            return;
            }

        if (args[0].IsNull() || args[1].IsNull())
            {
            ctx->SetResultError("Arguments to POW must not be NULL", -1);
            return;
            }

        double base = args[0].GetValueDouble();
        double exp = args[1].GetValueDouble();

        double res = std::pow(base, exp);
        ctx->SetResultDouble(res);
        }

public:
    PowECSqlFunction() : ECSqlFunction("POW", 2, ECN::PrimitiveType::PRIMITIVETYPE_Double, this) {}
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_RegisterUnregisterCustomFunction)
    {
    const auto perClassRowCount = 3;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlfunctiontest.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

    PowECSqlFunction ecsqlFunc;
    ASSERT_EQ(SUCCESS, ecdb.AddECSqlFunction(ecsqlFunc));

    PowECSqlFunction ecsqlFunc2;
    ASSERT_EQ(ERROR, ecdb.AddECSqlFunction(ecsqlFunc2)) << "Adding same ECSQL function twice is expected to fail";

    ASSERT_EQ(SUCCESS, ecdb.RemoveECSqlFunction(ecsqlFunc));
    ASSERT_EQ(SUCCESS, ecdb.RemoveECSqlFunction(ecsqlFunc)) << "Removing an unregistered ECSQL function is expected to succeeed";
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_NumericECSqlFunction)
    {
    const auto perClassRowCount = 3;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlfunctiontest.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

    Utf8CP ecsql = "SELECT I,POW(I,2) FROM ecsql.P";

    ECSqlStatement stmt;
    ASSERT_EQ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare(ecdb, ecsql)) << "ECSQL preparation expected to fail with unregistered custom ECSQL function";


    PowECSqlFunction ecsqlFunc;
    ASSERT_EQ(SUCCESS, ecdb.AddECSqlFunction(ecsqlFunc));

    ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, ecsql)) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

    int rowCount = 0;
    while (stmt.Step() == ECSqlStepStatus::HasRow)
        {
        rowCount++;

        int base = stmt.GetValueInt(0);
        int actualFuncResult = stmt.GetValueInt(1);

        ASSERT_EQ(std::pow(base, 2), actualFuncResult);
        }

    ASSERT_EQ(perClassRowCount, rowCount);
    }

//---------------------------------------------------------------------------------------
// Syntax: DISTANCE (Point1, Point2) : double
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PointDistanceECSqlFunction : ECSqlFunction, ScalarFunction::IScalar
    {
private:
    virtual void _ComputeScalar(ScalarFunction::Context* ctx, int nArgs, DbValue* args) override
        {
        //The ECSQL function call DISTANCE (Point1, Point2) is mapped 
        //to the SQLite function call DISTANCE (Point1.X, Point1.Y, Point2.X, Point2.Y)
        if (nArgs != 4)
            {
            ctx->SetResultError("Wrong number of arguments for function DISTANCE.", -1);
            return;
            }

        double squareSum = 0.0;
        const int dimensions = nArgs / 2;
        for (int i = 0; i < dimensions; i++)
            {
            DbValue& coordinate1 = args[i];
            DbValue& coordinate2 = args[i+dimensions];

            if (coordinate1.IsNull() || coordinate2.IsNull())
                {
                ctx->SetResultError("Arguments to DISTANCE must not be NULL", -1);
                return;
                }

            squareSum += std::pow(coordinate1.GetValueDouble () - coordinate2.GetValueDouble (), 2);
            }

        double res = std::sqrt(squareSum);
        ctx->SetResultDouble(res);
        }

public:
    PointDistanceECSqlFunction() : ECSqlFunction("DISTANCE", 2, 4, ECN::PrimitiveType::PRIMITIVETYPE_Double, this) {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_PointECSqlFunction)
    {
    auto& ecdb = SetUp("pointecsqlfunctiontest.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), 0);

    DPoint2d p2d1 = DPoint2d::From(1.0, 1.0);
    DPoint2d p2d2 = DPoint2d::From(-1.0, -1.0);
    DPoint3d p3d1 = DPoint3d::From(1.0, 1.0, 1.0);
    DPoint3d p3d2 = DPoint3d::From(-1.0, -1.0, -1.0);
    //insert test data
        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "INSERT INTO ecsql.P (P2D,P3D) VALUES (?,?)"));

        stmt.BindPoint2D(1, p2d1);
        stmt.BindPoint3D(2, p3d1);

        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());

        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindPoint2D(1, p2d2);
        stmt.BindPoint3D(2, p3d2);

        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());

        //insert an empty row to check how functions deal with NULL columns
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());

        ecdb.SaveChanges();
        }


    PointDistanceECSqlFunction ecsqlFunc;
    ASSERT_EQ(SUCCESS, ecdb.AddECSqlFunction(ecsqlFunc));

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT lhs.P2D, rhs.P2D, DISTANCE (lhs.P2D, rhs.P2D) FROM ecsql.P lhs, ecsql.P rhs WHERE lhs.P2D IS NOT NULL and rhs.P2D IS NOT NULL"));

        while (stmt.Step() == ECSqlStepStatus::HasRow)
            {
            DPoint2d lhs = stmt.GetValuePoint2D(0);
            DPoint2d rhs = stmt.GetValuePoint2D(1);
            double actualDistance = stmt.GetValueDouble(2);

            EXPECT_DOUBLE_EQ(lhs.Distance(rhs), actualDistance);
            }
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT DISTANCE (P2D, P2D) FROM ecsql.P WHERE P2D IS NOT NULL"));

        while (stmt.Step() == ECSqlStepStatus::HasRow)
            {
            double actualDistance = stmt.GetValueDouble(0);
            EXPECT_DOUBLE_EQ(0.0, actualDistance);
            }
        }

        //with null columns
        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT DISTANCE (P2D, P2D) FROM ecsql.P"));

        while (stmt.Step() == ECSqlStepStatus::HasRow)
            {
            double actualDistance = stmt.GetValueDouble(0);
            EXPECT_DOUBLE_EQ(0.0, actualDistance);
            }
         }

    }

//---------------------------------------------------------------------------------------
// Syntax: DATEFROMSTRING (Str) : DateTime
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct DateFromStringECSqlFunction : ECSqlFunction, ScalarFunction::IScalar
    {
    private:
        virtual void _ComputeScalar(ScalarFunction::Context* ctx, int nArgs, DbValue* args) override
            {
            if (nArgs != 1)
                {
                ctx->SetResultError("Wrong number of arguments for function DATEFROMSTRING.", -1);
                return;
                }

            if (args[0].IsNull())
                {
                ctx->SetResultError("Argument to DISTANCE must not be NULL", -1);
                return;
                }

            Utf8CP dateStr = args[0].GetValueText();
            DateTime dt;
            if (SUCCESS != DateTime::FromString(dt, dateStr))
                ctx->SetResultError("Invalid date string format.", -1);
            else
                {
                double jd = -1.0;
                ctx->SetResultDouble(dt.ToJulianDay(jd));
                }
            }

    public:
        DateFromStringECSqlFunction() : ECSqlFunction("DATEFROMSTRING", 1, ECN::PrimitiveType::PRIMITIVETYPE_DateTime, this) {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_DateECSqlFunction)
    {
    auto& ecdb = SetUp("dateecsqlfunctiontest.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), 0);

    DateTime dt(2015, 3, 24);
    Utf8String dtStr = dt.ToUtf8String();
    //insert test data
        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "INSERT INTO ecsql.P (DtUtc, S) VALUES (?,?)"));
        stmt.BindDateTime(1, dt);
        stmt.BindText(2, dtStr.c_str(), IECSqlBinder::MakeCopy::No);

        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());

        ecdb.SaveChanges();
        }


    DateFromStringECSqlFunction ecsqlFunc;
    ASSERT_EQ(SUCCESS, ecdb.AddECSqlFunction(ecsqlFunc));

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT DtUtc, DATEFROMSTRING(S) FROM ecsql.P"));

        while (stmt.Step() == ECSqlStepStatus::HasRow)
            {
            DateTime expectedDt = stmt.GetValueDateTime(0);
            DateTime actualDt = stmt.GetValueDateTime(1);
            ASSERT_TRUE(expectedDt.GetYear() == actualDt.GetYear());
            ASSERT_TRUE(expectedDt.GetMonth() == actualDt.GetMonth());
            ASSERT_TRUE(expectedDt.GetDay() == actualDt.GetDay());
            ASSERT_TRUE(expectedDt.GetHour() == actualDt.GetHour());
            ASSERT_TRUE(expectedDt.GetMinute() == actualDt.GetMinute());
            ASSERT_TRUE(expectedDt.GetSecond() == actualDt.GetSecond());
            }
        }
    }
END_ECDBUNITTESTS_NAMESPACE