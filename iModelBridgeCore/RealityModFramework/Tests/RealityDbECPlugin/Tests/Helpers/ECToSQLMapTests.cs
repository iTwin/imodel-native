using System;
using System.Data;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Bentley.EC.Persistence.Query;
using Bentley.Exceptions;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class ECToSQLMapTests
        {

        [SetUp]
        public void SetUp ()
            {

            }

        [TearDown]
        public void TearDown ()
            {
            
            }

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

            }

        [Test]
        public void SQLReaderToECPropertyExceptionTest ()
            {

            }
        }
    }
