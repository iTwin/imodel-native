/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlCustomFunction_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementTestFixture.h"
#include <cmath> // for std::pow

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PowSqlFunction : ScalarFunction
    {
private:

    virtual void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
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
struct IntegerToBlobSqlFunction : ScalarFunction
    {
    private:

        virtual void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
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
struct ToBoolStrSqlFunction : ScalarFunction
    {
    private:
        virtual void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
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
 struct SumOfSquares : AggregateFunction
     {
     private:

         double runningSum = 0;
         int valueToSet = 0;
         virtual void _StepAggregate (AggregateFunction::Context& ctx, int nArgs, DbValue* args) override
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

         virtual void _FinishAggregate (AggregateFunction::Context& ctx) override
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
TEST_F(ECSqlStatementTestFixture, RegisterUnregisterCustomSqlFunction)
    {
    const auto perClassRowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly), perClassRowCount);

    PowSqlFunction func;
    ASSERT_EQ(0, ecdb.AddFunction(func));

    PowSqlFunction func2;
    ASSERT_EQ(0, ecdb.AddFunction(func2)) << "Adding same ECSQL function twice is expected to succeed.";

    ASSERT_EQ(0, ecdb.RemoveFunction(func));
    ASSERT_EQ(0, ecdb.RemoveFunction(func)) << "Removing an unregistered ECSQL function is expected to succeeed";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, CallUnregisteredSqlFunction)
    {
    const auto perClassRowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly), perClassRowCount);

    Utf8CP ecsql = "SELECT I,POW(I,2) FROM ecsql.P";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, ecsql)) << "ECSQL preparation expected to fail with unregistered custom ECSQL function";

    PowSqlFunction func;
    ASSERT_EQ(0, ecdb.AddFunction(func));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << "ECSQL preparation expected to succeed with registered custom ECSQL function";
    ASSERT_EQ(0, ecdb.RemoveFunction(func));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NumericSqlFunction)
    {
    const int perClassRowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly), perClassRowCount);

    //insert one more test row which has a NULL column
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        ecdb.SaveChanges();
        }

    PowSqlFunction func;
    ASSERT_EQ(0, ecdb.AddFunction(func));

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT I,POW(I,2) FROM ecsql.P WHERE I IS NOT NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        int rowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
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
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT I, POW(I,?) FROM ecsql.P WHERE I IS NOT NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

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
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT POW(I,NULL) FROM ecsql.P")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT POW(NULL,2) FROM ecsql.P")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT POW(NULL,NULL) FROM ecsql.P")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT POW(I,2) FROM ecsql.P WHERE I IS NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()) << "Step is expected to fail if function is called with NULL arg";
        }
    ASSERT_EQ(0, ecdb.RemoveFunction(func));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StringSqlFunction)
    {
    const int perClassRowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly), perClassRowCount);

    ToBoolStrSqlFunction func;
    ASSERT_EQ(0, ecdb.AddFunction(func));

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT TOBOOLSTR(1) FROM ecsql.P LIMIT 1")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        IECSqlValue const& val = stmt.GetValue(0);
        ASSERT_STREQ("true", val.GetText());
        ASSERT_EQ(ECN::PRIMITIVETYPE_String, val.GetColumnInfo().GetDataType().GetPrimitiveType());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BlobSqlFunction)
    {
    const int perClassRowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite), perClassRowCount);

    //insert one more test row which has a NULL column
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        ecdb.SaveChanges();
        }

        IntegerToBlobSqlFunction func;
        ASSERT_EQ(0, ecdb.AddFunction(func));

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT L,INTEGERTOBLOB(L) FROM ecsql.P WHERE L IS NOT NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        int rowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            rowCount++;
            int64_t expectedInt = stmt.GetValueInt64(0);
            int blobSize = -1;
            void const* actualBlob = stmt.GetValueBinary(1, &blobSize);

            ASSERT_EQ(sizeof(expectedInt), blobSize);
            int64_t actualInt = -1LL;
            memcpy(&actualInt, actualBlob, sizeof(actualInt));
            ASSERT_EQ(expectedInt, actualInt);
            }

        ASSERT_TRUE(rowCount > 0);
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT L,INTEGERTOBLOB(L) FROM ecsql.P WHERE L IS NULL")) << "ECSQL preparation expected to succeed with registered custom ECSQL function";

        int rowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            rowCount++;
            int blobSize = -1;
            void const* actualBlob = stmt.GetValueBinary(1, &blobSize);

            ASSERT_EQ(0, blobSize);
            ASSERT_TRUE(actualBlob == nullptr);
            }

        ASSERT_TRUE(rowCount > 0);
        }
    ASSERT_EQ(0, ecdb.RemoveFunction(func));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
 TEST_F (ECSqlStatementTestFixture, AggregateFunction)
     {
     SumOfSquares func;
     ECSqlStatement stmt;
     const int perClassRowCount = 3;
     ECDbR ecdb = SetupECDb("ecsqlfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite), perClassRowCount);

     ASSERT_EQ (ECSqlStatus::InvalidECSql, stmt.Prepare (ecdb, "SELECT SOS(D) FROM ecsql.P")) << "ECSql Preparetion expected to fail with unregistered ECSql function";
     stmt.Finalize ();

     ASSERT_EQ (0, ecdb.AddFunction (func));

     ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT SOS(D) FROM ecsql.P")) << "ECSql Preparetion expected to succeed after adding custom ECSql function to Db";
     ASSERT_TRUE (stmt.Step () == BE_SQLITE_ROW);
     double actualSumOfSquares = stmt.GetValueDouble (0);
     stmt.Finalize ();

     ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT D FROM ecsql.P"));
     double expectedSumOfSquares = 0;

     while (stmt.Step () != BE_SQLITE_DONE)
         {
         expectedSumOfSquares += std::pow (stmt.GetValueDouble (0), 2);
         }
     //Verify that the value returned by the Custom ECSql function is the same is expected.
     ASSERT_EQ (expectedSumOfSquares, actualSumOfSquares);
     stmt.Finalize ();

     ASSERT_EQ (0, ecdb.RemoveFunction (func));
     ASSERT_EQ (ECSqlStatus::InvalidECSql, stmt.Prepare (ecdb, "SELECT SOS(D) FROM ecsql.P ")) << "ECSql Preparation expected to fail when custom ECSql function is removed from Db";
     stmt.Finalize ();

     //Adding Custom ECSql function to the Db is expected to Succeed once it was removed 
     ASSERT_EQ (0, ecdb.AddFunction (func));

     ASSERT_EQ (ECSqlStatus::InvalidECSql, stmt.Prepare (ecdb, "SELECT SOS() FROM ecsql.P")) << "ECSql Prepration is expected to fail when no argument is passed to SOS";
     stmt.Finalize ();

     ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT SOS(NUll) FROM ecsql.P")) << "ECSql Prepration is expected to succeed when NULL is passed to SOS";
     ASSERT_EQ (BE_SQLITE_ERROR, stmt.Step())<< "Step is expected to fail if function is called with NULL argument";
     }

#ifdef NOT_NOW
//---------------------------------------------------------------------------------------
// Syntax: DATEFROMSTRING (Str) : DateTime
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct DateFromStringFunction : ScalarFunction, ScalarFunction::IScalar
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
        DateFromStringFunction() : ScalarFunction("DATEFROMSTRING", 1, ECN::PrimitiveType::PRIMITIVETYPE_DateTime, this) {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, DateECSqlFunction)
    {
    ECDbR ecdb = SetupECDb("ecsqlfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    DateTime dt(2015, 3, 24);
    Utf8String dtStr = dt.ToUtf8String();
    //insert test data
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.P (DtUtc, S) VALUES (?,?)"));
        stmt.BindDateTime(1, dt);
        stmt.BindText(2, dtStr.c_str(), IECSqlBinder::MakeCopy::No);

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

        ecdb.SaveChanges();
        }


    DateFromStringECSqlFunction ecsqlFunc;
    ASSERT_EQ(0, ecdb.AddScalarFunction(ecsqlFunc));

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT DtUtc, DATEFROMSTRING(S) FROM ecsql.P"));

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
            IGeometryPtr geom = BackDoor::IGeometryFlatBuffer::BytesToGeometry(byteVec);
            if (geom == nullptr)
                {
                ctx->SetResultError("Argument to GETGEOMETRYTYPE is not an IGeometry", -1);
                return;
                }

            IGeometry::GeometryType type = geom->GetGeometryType();
            Utf8CP geomTypeStr = GeometryTypeToString(type);
            ctx->SetResultText(geomTypeStr, -1, DbFunction::Context::CopyData::Yes);
            }

    public:
        GeometryTypeECSqlFunction() : ECSqlScalarFunction("GETGEOMETRYTYPE", 1, ECN::PrimitiveType::PRIMITIVETYPE_String, this) {}

        static Utf8CP GeometryTypeToString(IGeometry::GeometryType type)
            {
            switch (type)
                {
                    case IGeometry::GeometryType::BsplineSurface: return "BsplineSurface";
                    case IGeometry::GeometryType::CurvePrimitive: return "CurvePrimitive";
                    case IGeometry::GeometryType::CurveVector: return "CurveVector";
                    case IGeometry::GeometryType::Polyface: return "Polyface";
                    case IGeometry::GeometryType::SolidPrimitive: return "SolidPrimitive";
                    default: return "Unknown";
                }
            }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GeometryECSqlFunction)
    {
    ECDbR ecdb = SetupECDb("ecsqlfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    IGeometryPtr line = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    Utf8CP expectedGeomTypeStr = GeometryTypeECSqlFunction::GeometryTypeToString(line->GetGeometryType());
    //insert test data
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PASpatial (Geometry) VALUES (?)"));
        stmt.BindGeometry(1, *line);
        
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

        ecdb.SaveChanges();
        }

        GeometryTypeECSqlFunction ecsqlFunc;
        ASSERT_EQ(0, ecdb.AddScalarFunction(ecsqlFunc));

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT GETGEOMETRYTYPE(Geometry) FROM ecsql.PASpatial"));

        while (stmt.Step() == BE_SQLITE_ROW)
            {
            Utf8CP actualGeomTypeStr = stmt.GetValueText(0);
            ASSERT_STREQ(expectedGeomTypeStr, actualGeomTypeStr);
            }
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT GETGEOMETRYTYPE(?) FROM ecsql.PASpatial LIMIT 1"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry (1, *line));

        while (stmt.Step() == BE_SQLITE_ROW)
            {
            Utf8CP actualGeomTypeStr = stmt.GetValueText(0);
            ASSERT_STREQ(expectedGeomTypeStr, actualGeomTypeStr);
            }
        }
    }

#endif
END_ECDBUNITTESTS_NAMESPACE