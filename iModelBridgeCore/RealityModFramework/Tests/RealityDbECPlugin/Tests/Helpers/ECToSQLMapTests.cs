using System;
using System.Data;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.Exceptions;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class ECToSQLMapTests
        {

        [TestCase(RelationalOperator.EQ, "=")]
        [TestCase(RelationalOperator.GT, ">")]
        [TestCase(RelationalOperator.GTEQ, ">=")]
        [TestCase(RelationalOperator.IN, "IN")]
        [TestCase(RelationalOperator.LIKE, "LIKE")]
        [TestCase(RelationalOperator.LT, "<")]
        [TestCase(RelationalOperator.LTEQ, "<=")]
        [TestCase(RelationalOperator.NE, "<>")]
        [TestCase(RelationalOperator.NOTIN, "NOT IN")]
        [TestCase(RelationalOperator.NOTLIKE, "NOT LIKE")]
        [TestCase(RelationalOperator.ISNULL, "IS NULL")]
        [TestCase(RelationalOperator.ISNOTNULL, "IS NOT NULL")]
        public void ECRelationalOperatorToSQLTest (RelationalOperator relOp, string expectedString)
            {
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(relOp), Is.EqualTo(expectedString));
            }

        [Test]
        public void ECRelationalOperatorToSQLExceptionTest ()
            {
            Assert.That(() => ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.ISNOTNULL + 1), Throws.TypeOf<NotImplementedException>());
            }

        [Test]
        public void ECTypeToDbTypeTest ()
            {
            Assert.That(ECToSQLMap.ECTypeToDbType(Bentley.ECObjects.ECObjects.StringType), Is.EqualTo(DbType.String));
            Assert.That(ECToSQLMap.ECTypeToDbType(Bentley.ECObjects.ECObjects.DoubleType), Is.EqualTo(DbType.Double));
            Assert.That(ECToSQLMap.ECTypeToDbType(Bentley.ECObjects.ECObjects.BooleanType), Is.EqualTo(DbType.Boolean));
            Assert.That(ECToSQLMap.ECTypeToDbType(Bentley.ECObjects.ECObjects.IntegerType), Is.EqualTo(DbType.Int32));
            Assert.That(ECToSQLMap.ECTypeToDbType(Bentley.ECObjects.ECObjects.LongType), Is.EqualTo(DbType.Int64));
            Assert.That(ECToSQLMap.ECTypeToDbType(Bentley.ECObjects.ECObjects.DateTimeType), Is.EqualTo(DbType.DateTime));
            }

        [Test]
        public void ECTypeToDbTypeExceptionTest ()
            {
            Assert.That(() => ECToSQLMap.ECTypeToDbType(Bentley.ECObjects.ECObjects.Point2dType), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void ECTypeToSqlDbTypeTest ()
            {
            Assert.That(ECToSQLMap.ECTypeToSqlDbType(Bentley.ECObjects.ECObjects.StringType), Is.EqualTo(SqlDbType.NVarChar));
            Assert.That(ECToSQLMap.ECTypeToSqlDbType(Bentley.ECObjects.ECObjects.DoubleType), Is.EqualTo(SqlDbType.Float));
            Assert.That(ECToSQLMap.ECTypeToSqlDbType(Bentley.ECObjects.ECObjects.BooleanType), Is.EqualTo(SqlDbType.Bit));
            Assert.That(ECToSQLMap.ECTypeToSqlDbType(Bentley.ECObjects.ECObjects.IntegerType), Is.EqualTo(SqlDbType.Int));
            Assert.That(ECToSQLMap.ECTypeToSqlDbType(Bentley.ECObjects.ECObjects.LongType), Is.EqualTo(SqlDbType.BigInt));
            Assert.That(ECToSQLMap.ECTypeToSqlDbType(Bentley.ECObjects.ECObjects.DateTimeType), Is.EqualTo(SqlDbType.DateTime));
            }

        [Test]
        public void ECTypeToSqlDbTypeExceptionTest ()
            {
            Assert.That(() => ECToSQLMap.ECTypeToSqlDbType(Bentley.ECObjects.ECObjects.Point2dType), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void SQLReaderToECPropertyTest ()
            {
            const string returnedString = "abc123";
            const double returnedDouble = 42.42;
            const bool returnedBoolean = true;
            const int returnedInt = 42;
            const long returnedLong = 1337;
            DateTime returnedDateTime = new DateTime(2017, 5, 18);

            MockRepository mocks = new MockRepository();
            IDataReader dataReaderStub = mocks.Stub<IDataReader>();
            IECPropertyValue stringECPropertyValueStub = mocks.Stub<IECPropertyValue>();
            IECPropertyValue doubleECPropertyValueStub = mocks.Stub<IECPropertyValue>();
            IECPropertyValue booleanECPropertyValueStub = mocks.Stub<IECPropertyValue>();
            IECPropertyValue intECPropertyValueStub = mocks.Stub<IECPropertyValue>();
            IECPropertyValue longECPropertyValueStub = mocks.Stub<IECPropertyValue>();
            IECPropertyValue dateTimeECPropertyValueStub = mocks.Stub<IECPropertyValue>();

            using ( mocks.Record() )
                {
                SetupResult.For(dataReaderStub.GetString(Arg<int>.Is.Anything)).Return(returnedString);
                SetupResult.For(dataReaderStub.GetDouble(Arg<int>.Is.Anything)).Return(returnedDouble);
                SetupResult.For(dataReaderStub.GetBoolean(Arg<int>.Is.Anything)).Return(returnedBoolean);
                SetupResult.For(dataReaderStub.GetInt32(Arg<int>.Is.Anything)).Return(returnedInt);
                SetupResult.For(dataReaderStub.GetInt64(Arg<int>.Is.Anything)).Return(returnedLong);
                SetupResult.For(dataReaderStub.GetDateTime(Arg<int>.Is.Anything)).Return(returnedDateTime);

                SetupResult.For(stringECPropertyValueStub.Type).Return(Bentley.ECObjects.ECObjects.StringType);
                SetupResult.For(doubleECPropertyValueStub.Type).Return(Bentley.ECObjects.ECObjects.DoubleType);
                SetupResult.For(booleanECPropertyValueStub.Type).Return(Bentley.ECObjects.ECObjects.BooleanType);
                SetupResult.For(intECPropertyValueStub.Type).Return(Bentley.ECObjects.ECObjects.IntegerType);
                SetupResult.For(longECPropertyValueStub.Type).Return(Bentley.ECObjects.ECObjects.LongType);
                SetupResult.For(dateTimeECPropertyValueStub.Type).Return(Bentley.ECObjects.ECObjects.DateTimeType);
                }
            ECToSQLMap.SQLReaderToECProperty(stringECPropertyValueStub, dataReaderStub, 0);
            Assert.That(stringECPropertyValueStub.StringValue, Is.EqualTo(returnedString));

            ECToSQLMap.SQLReaderToECProperty(doubleECPropertyValueStub, dataReaderStub, 0);
            Assert.That(doubleECPropertyValueStub.DoubleValue, Is.EqualTo(returnedDouble));

            ECToSQLMap.SQLReaderToECProperty(booleanECPropertyValueStub, dataReaderStub, 0);
            Assert.That(booleanECPropertyValueStub.NativeValue, Is.EqualTo(returnedBoolean));

            ECToSQLMap.SQLReaderToECProperty(intECPropertyValueStub, dataReaderStub, 0);
            Assert.That(intECPropertyValueStub.IntValue, Is.EqualTo(returnedInt));

            ECToSQLMap.SQLReaderToECProperty(longECPropertyValueStub, dataReaderStub, 0);
            Assert.That(longECPropertyValueStub.NativeValue, Is.EqualTo(returnedLong));

            ECToSQLMap.SQLReaderToECProperty(dateTimeECPropertyValueStub, dataReaderStub, 0);
            Assert.That(dateTimeECPropertyValueStub.NativeValue, Is.EqualTo(returnedDateTime));
            }

        [Test]
        public void SQLReaderToECPropertyExceptionTest ()
            {
            MockRepository mocks = new MockRepository();
            IDataReader dataReaderStub = mocks.Stub<IDataReader>();
            IECPropertyValue point2DECPropertyValueStub = mocks.Stub<IECPropertyValue>();

            using ( mocks.Record() )
                {
                SetupResult.For(point2DECPropertyValueStub.Type).Return(Bentley.ECObjects.ECObjects.Point2dType);
                }
            Assert.That(() => ECToSQLMap.SQLReaderToECProperty(point2DECPropertyValueStub, dataReaderStub, 0), Throws.TypeOf<ProgrammerException>());
            }
        }
    }
