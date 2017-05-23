using System;
using System.Data;
using System.Data.SqlClient;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class DbConnectionHelperTests
        {
        private IDbConnection dbConnectionMock;

        [Test]
        public void GetSqlInsertStatementBuilderTest ()
            {
            dbConnectionMock = MockRepository.GenerateMock<SqlConnection>();
            Assert.That(DbConnectionHelper.GetSqlInsertStatementBuilder(dbConnectionMock), Is.TypeOf<SQLServerInsertStatementBuilder>());
            }

        [Test]
        public void GetSqlInsertStatementBuilderExceptionTest ()
        {
            dbConnectionMock = MockRepository.GenerateMock<IDbConnection>();
            Assert.That(() => DbConnectionHelper.GetSqlInsertStatementBuilder(dbConnectionMock), Throws.TypeOf<NotImplementedException>());
        }
        }
    }
