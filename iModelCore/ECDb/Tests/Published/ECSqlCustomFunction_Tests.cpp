/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlCustomFunction_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFixture.h"
#include <GeomSerialization/GeomSerializationApi.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PowECSqlFunction : ECSqlScalarFunction, ScalarFunction::IScalar
    {
private:

    virtual void _ComputeScalar(ScalarFunction::Context* ctx, int nArgs, DbValue* args) override
        {
        if (nArgs != GetNumArgs ())
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
    PowECSqlFunction() : ECSqlScalarFunction("POW", 2, ECN::PrimitiveType::PRIMITIVETYPE_Double, this) {}
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct SqrtSqlFunction : ScalarFunction, ScalarFunction::IScalar
    {
private:
    virtual void _ComputeScalar(ScalarFunction::Context* ctx, int nArgs, DbValue* args) override
        {
        if (nArgs != GetNumArgs())
            {
            ctx->SetResultError("Function SQRT expects 1 argument.", -1);
            return;
            }

        if (args[0].IsNull())
            {
            ctx->SetResultError("Argument to SQRT must not be NULL.", -1);
            return;
            }

        double operand = args[0].GetValueDouble();
        if (operand < 0.0)
            {
            ctx->SetResultError("Argument to SQRT must not be negative.", -1);
            return;
            }

        double res = std::sqrt(operand);
        ctx->SetResultDouble(res);
        }

public:
    SqrtSqlFunction() : ScalarFunction("SQRT", 1, this) {}
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
    ASSERT_EQ(0, ecdb.AddScalarFunction(ecsqlFunc));

    PowECSqlFunction ecsqlFunc2;
    ASSERT_EQ(0, ecdb.AddScalarFunction(ecsqlFunc2)) << "Adding same ECSQL function twice is expected to succeed.";

    ASSERT_EQ(0, ecdb.RemoveFunction(ecsqlFunc));
    ASSERT_EQ(0, ecdb.RemoveFunction(ecsqlFunc)) << "Removing an unregistered ECSQL function is expected to succeeed";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_CallUnregisteredFunction)
    {
    const auto perClassRowCount = 3;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlfunctiontest.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

    Utf8CP ecsql = "SELECT I,POW(I,2) FROM ecsql.P";

    ECSqlStatement stmt;
    ASSERT_EQ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare(ecdb, ecsql)) << "ECSQL preparation expected to fail with unregistered custom ECSQL function";

    PowECSqlFunction ecsqlFunc;
    ASSERT_EQ(0, ecdb.AddScalarFunction(ecsqlFunc));

    ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, ecsql)) << "ECSQL preparation expected to succeed with registered custom ECSQL function";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_RegisterInvalidFunction)
    {
    const auto perClassRowCount = 3;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlfunctiontest.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

    struct InvalidECSqlFunction : ECSqlScalarFunction
        {
        InvalidECSqlFunction() : ECSqlScalarFunction("INVALID", 2, ECN::PRIMITIVETYPE_Point2D) {}
        };

    InvalidECSqlFunction func;
    ASSERT_EQ((int) ERROR, ecdb.AddScalarFunction(func));
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_NumericECSqlFunction)
    {
    const int perClassRowCount = 3;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlfunctiontest.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

    //insert one more test row which has a NULL column
        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)"));
        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());
        ecdb.SaveChanges();
        }

    PowECSqlFunction ecsqlFunc;
    ASSERT_EQ(0, ecdb.AddScalarFunction(ecsqlFunc));

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT I,POW(I,2) FROM ecsql.P WHERE I IS NOT NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

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

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT I, POW(I,?) FROM ecsql.P WHERE I IS NOT NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        int exp = 3;
        stmt.BindInt(1, exp);

        while (stmt.Step() == ECSqlStepStatus::HasRow)
            {
            int base = stmt.GetValueInt(0);
            int actualFuncResult = stmt.GetValueInt(1);

            ASSERT_EQ(std::pow(base, exp), actualFuncResult);
            }

        stmt.Reset();
        stmt.ClearBindings();
        ASSERT_EQ((int) ECSqlStepStatus::Error, (int) stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT POW(I,NULL) FROM ecsql.P")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ((int) ECSqlStepStatus::Error, (int) stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT POW(NULL,2) FROM ecsql.P")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";
        ASSERT_EQ((int) ECSqlStepStatus::Error, (int) stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT POW(NULL,NULL) FROM ecsql.P")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ((int) ECSqlStepStatus::Error, (int) stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT POW(I,2) FROM ecsql.P WHERE I IS NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ((int) ECSqlStepStatus::Error, (int) stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_NumericBeSQliteFunction)
    {
    const auto perClassRowCount = 3;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlfunctiontest.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

    //insert one more test row which has a NULL column
        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)"));
        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());
        ecdb.SaveChanges();
        }

    SqrtSqlFunction besqliteFunc;
    ASSERT_EQ(0, ecdb.AddScalarFunction(besqliteFunc));

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT I,SQRT(I) FROM ecsql.P WHERE I IS NOT NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        int rowCount = 0;
        while (stmt.Step() == ECSqlStepStatus::HasRow)
            {
            rowCount++;

            int operand = stmt.GetValueInt(0);
            double actualSqrt = stmt.GetValueDouble(1);

            EXPECT_DOUBLE_EQ(std::sqrt(operand), actualSqrt);
            }

        ASSERT_EQ(perClassRowCount, rowCount);
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT SQRT(?) FROM ecsql.P")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        int operand = 16;
        stmt.BindInt(1, operand);

        while (stmt.Step() == ECSqlStepStatus::HasRow)
            {
            int actualSqrt = stmt.GetValueInt(0);
            ASSERT_EQ(4, actualSqrt);
            }

        stmt.Reset();
        stmt.ClearBindings();
        ASSERT_EQ((int) ECSqlStepStatus::Error, (int) stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT SQRT(NULL) FROM ecsql.P")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ((int) ECSqlStepStatus::Error, (int) stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT SQRT(I) FROM ecsql.P WHERE I IS NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ((int) ECSqlStepStatus::Error, (int) stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }
    }

//---------------------------------------------------------------------------------------
// Syntax: DATEFROMSTRING (Str) : DateTime
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct DateFromStringECSqlFunction : ECSqlScalarFunction, ScalarFunction::IScalar
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
                ctx->SetResultError("Argument to DATEFROMSTRING must not be NULL", -1);
                return;
                }

            Utf8CP dateStr = args[0].GetValueText();
            DateTime dt;
            if (SUCCESS != DateTime::FromString(dt, dateStr))
                ctx->SetResultError("Invalid date string format.", -1);
            else
                {
                double jd = -1.0;
                dt.ToJulianDay(jd);
                ctx->SetResultDouble(jd);
                }
            }

    public:
        DateFromStringECSqlFunction() : ECSqlScalarFunction("DATEFROMSTRING", 1, ECN::PrimitiveType::PRIMITIVETYPE_DateTime, this) {}
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
    ASSERT_EQ(0, ecdb.AddScalarFunction(ecsqlFunc));

        {
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "SELECT DtUtc, DATEFROMSTRING(S) FROM ecsql.P"));

        while (stmt.Step() == ECSqlStepStatus::HasRow)
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
    }

//---------------------------------------------------------------------------------------
// Syntax: GETGEOMETRYTYPE (IGeometry) : String
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct GeometryTypeECSqlFunction : ECSqlScalarFunction, ScalarFunction::IScalar
    {
    private:
        virtual void _ComputeScalar(ScalarFunction::Context* ctx, int nArgs, DbValue* args) override
            {
            if (nArgs != 1)
                {
                ctx->SetResultError("Wrong number of arguments for function GETGEOMETRYTYPE.", -1);
                return;
                }

            if (args[0].IsNull())
                {
                ctx->SetResultError("Argument to GETGEOMETRYTYPE must not be NULL", -1);
                return;
                }

            Byte const* geomBlob = static_cast<Byte const*> (args[0].GetValueBlob());
            int blobSize = args[0].GetValueBytes();
            BeAssert(blobSize > 0);
            const size_t blobSizeU = (size_t) blobSize;
            bvector<Byte> byteVec;
            byteVec.reserve(blobSizeU);
            byteVec.assign(geomBlob, geomBlob + blobSizeU);
            IGeometryPtr geom = BentleyGeometryFlatBuffer::BytesToGeometry(byteVec);
            if (geom == nullptr)
                {
                ctx->SetResultError("Argument to GETGEOMETRYTYPE is not an IGeometry", -1);
                return;
                }

            Utf8CP geomTypeStr = nullptr;
            IGeometry::GeometryType type = geom->GetGeometryType();
            switch (type)
                {
                    case IGeometry::GeometryType::BsplineSurface: geomTypeStr = "BsplineSurface"; break;
                    case IGeometry::GeometryType::CurvePrimitive: geomTypeStr = "CurvePrimitive"; break;
                    case IGeometry::GeometryType::CurveVector: geomTypeStr = "CurveVector"; break;
                    case IGeometry::GeometryType::Polyface: geomTypeStr = "Polyface"; break;
                    case IGeometry::GeometryType::SolidPrimitive: geomTypeStr = "SolidPrimitive"; break;
                    default: geomTypeStr = "Unknown"; break;
                }

            ctx->SetResultText(geomTypeStr, -1, DbFunction::Context::CopyData::Yes);
            }

    public:
        GeometryTypeECSqlFunction() : ECSqlScalarFunction("GETGEOMETRYTYPE", 1, ECN::PrimitiveType::PRIMITIVETYPE_String, this) {}
    };


END_ECDBUNITTESTS_NAMESPACE