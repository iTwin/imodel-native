using System;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Bentley.EC.Persistence.Query;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    public class ECToSQLMapTests
        {

        [SetUp]
        public void SetUp ()
            {

            }

        [Test]
        public void ECRelationalOperatorToSQLTest ()
            {
            Assert.AreEqual("=", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.EQ));
            Assert.AreEqual(">", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.GT));
            Assert.AreEqual(">=", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.GTEQ));
            Assert.AreEqual("IN", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.IN));
            Assert.AreEqual("LIKE", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.LIKE));
            Assert.AreEqual("<", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.LT));
            Assert.AreEqual("<=", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.LTEQ));
            Assert.AreEqual("<>", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.NE));
            Assert.AreEqual("NOT IN", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.NOTIN));
            Assert.AreEqual("NOT LIKE", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.NOTLIKE));
            Assert.AreEqual("IS NULL", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.ISNULL));
            Assert.AreEqual("IS NOT NULL", ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.ISNOTNULL));

            Assert.Throws<NotImplementedException>(() => ECToSQLMap.ECRelationalOperatorToSQL(RelationalOperator.X));
            }
        }
    }
