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

        [Test, Pairwise]
        public void ECRelationalOperatorToSQLTest ()/*[Values(RelationalOperator.EQ, RelationalOperator.GT)] RelationalOperator relOp,
            [Values("=", ">")] string expectedString)*/
            {
            //TODO: Clean this up !
            //Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(relOp), Is.EqualTo(expectedString));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.EQ), Is.EqualTo("="));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.GT), Is.EqualTo(">"));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.GTEQ), Is.EqualTo(">="));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.IN), Is.EqualTo("IN"));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.LIKE), Is.EqualTo("LIKE"));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.LT), Is.EqualTo("<"));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.LTEQ), Is.EqualTo("<="));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.NE), Is.EqualTo("<>"));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.NOTIN), Is.EqualTo("NOT IN"));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.NOTLIKE), Is.EqualTo("NOT LIKE"));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.ISNULL), Is.EqualTo("IS NULL"));
            Assert.That(ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.ISNOTNULL), Is.EqualTo("IS NOT NULL"));
            }

        [Test]
        public void ECRelationalOperatorToSQLExceptionTest ()
            {
            Assert.That(() => ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.X), Throws.TypeOf<NotImplementedException>());
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
