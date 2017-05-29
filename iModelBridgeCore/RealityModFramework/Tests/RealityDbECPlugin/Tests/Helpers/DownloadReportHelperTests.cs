using System;
using System.Data;
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Reflection;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {

    [TestFixture]
    class DownloadReportHelperTests
        {
        private MockRepository mocks;
        private Stream memoryStream;
        private IDbConnectionCreator dbConnectionCreatorStub;
        private IDbConnection dbConnectionMock;
        private FakeDbCommand fakeDbCommand;
        private IDbDataParameter dbDataParameter0Stub;
        private IDbDataParameter dbDataParameter1Stub;
        private IDbDataParameter dbDataParameter2Stub;

        private const string packageName = "package";
        private const string connectionString = "connection string";
        private readonly byte[] fileBytes = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9 };

        [SetUp]
        public void SetUp ()
            {
            mocks = new MockRepository();
            dbConnectionCreatorStub = mocks.Stub<IDbConnectionCreator>();
            dbConnectionMock = mocks.DynamicMock<IDbConnection>();
            dbDataParameter0Stub = mocks.Stub<IDbDataParameter>();
            dbDataParameter1Stub = mocks.Stub<IDbDataParameter>();
            dbDataParameter2Stub = mocks.Stub<IDbDataParameter>();

            fakeDbCommand = new FakeDbCommand();

            memoryStream = new MemoryStream(fileBytes.Length);
            memoryStream.Write(fileBytes, 0, fileBytes.Length);
            }

        [TearDown]
        public void TearDown ()
            {
            memoryStream.Dispose();
            }

        [Test]
        public void InsertInDatabaseTest ()
            {
            using ( mocks.Record() )
                {
                SetSharedExpections();

                Expect.Call(dbConnectionMock.Close).Repeat.Once();
                }
            using ( mocks.Playback() )
                {
                DownloadReportHelper.InsertInDatabase(memoryStream, packageName, connectionString, dbConnectionCreatorStub);
                }

            Assert.That(fakeDbCommand.CommandText, Is.EqualTo("IF 0 = (SELECT COUNT(*) FROM dbo.DownloadReports WHERE Id = @param0)" +
                                                              " INSERT INTO dbo.DownloadReports (Id, CreationTime, ReportContent) VALUES " +
                                                              "(@param0, @param1, @param2)"));
            Assert.That(fakeDbCommand.CommandType, Is.EqualTo(CommandType.Text));
            Assert.That(fakeDbCommand.Parameters.Contains(dbDataParameter0Stub));
            Assert.That(fakeDbCommand.Parameters.Contains(dbDataParameter1Stub));
            Assert.That(fakeDbCommand.Parameters.Contains(dbDataParameter2Stub));

            Assert.That(dbDataParameter0Stub.DbType, Is.EqualTo(DbType.String));
            Assert.That(dbDataParameter0Stub.ParameterName, Is.EqualTo("@param0"));
            Assert.That(dbDataParameter0Stub.Value, Is.EqualTo(packageName));

            Assert.That(dbDataParameter1Stub.DbType, Is.EqualTo(DbType.DateTime));
            Assert.That(dbDataParameter1Stub.ParameterName, Is.EqualTo("@param1"));
            Assert.That(dbDataParameter1Stub.Value, Is.EqualTo(DateTime.UtcNow).Within(1).Seconds);

            Assert.That(dbDataParameter2Stub.DbType, Is.EqualTo(DbType.Binary));
            Assert.That(dbDataParameter2Stub.ParameterName, Is.EqualTo("@param2"));
            Assert.That(((byte[]) dbDataParameter2Stub.Value).SequenceEqual(fileBytes), Is.True);
            }

        [Test]
        public void InsertInDatabaseReportTooLargeExceptionTest ()
            {
            Stream streamStub = mocks.Stub<Stream>();

            using ( mocks.Record() )
                {
                SetSharedExpections();

                SetupResult.For(streamStub.Length).Return((long) int.MaxValue + 1);
                }
            using ( mocks.Playback() )
                {
                Assert.That(() => DownloadReportHelper.InsertInDatabase(streamStub, packageName, connectionString, dbConnectionCreatorStub),
                    Throws.TypeOf<UserFriendlyException>());
                }
            }

        [Test]
        public void InsertInDatabase547SqlExceptionTest ()
            {
            using ( mocks.Record() )
                {
                SetSharedExpections();

                fakeDbCommand.SetExecuteNonQueryBehavior(Throw547SqlException);
                }
            using ( mocks.Playback() )
                {
                Assert.That(() => DownloadReportHelper.InsertInDatabase(memoryStream, packageName, connectionString, dbConnectionCreatorStub),
                    Throws.TypeOf<UserFriendlyException>());
                }
            }

        [Test]
        public void InsertInDatabaseGenericSqlExceptionTest ()
            {
            using ( mocks.Record() )
                {
                SetSharedExpections();

                fakeDbCommand.SetExecuteNonQueryBehavior(ThrowGenericSqlException);
                }
            using ( mocks.Playback() )
                {
                Assert.That(() => DownloadReportHelper.InsertInDatabase(memoryStream, packageName, connectionString, dbConnectionCreatorStub),
                    Throws.TypeOf<SqlException>());
                }
            }

        private void SetSharedExpections ()
            {
            SetupResult.For(dbConnectionCreatorStub.CreateDbConnection(Arg<string>.Is.Anything)).Return(dbConnectionMock);
            Expect.Call(dbConnectionMock.Open).Repeat.Once();
            Expect.Call(dbConnectionMock.CreateCommand()).Repeat.Once().Return(fakeDbCommand);

            fakeDbCommand.DbDataParametersToCreate.Add(dbDataParameter0Stub);
            fakeDbCommand.DbDataParametersToCreate.Add(dbDataParameter1Stub);
            fakeDbCommand.DbDataParametersToCreate.Add(dbDataParameter2Stub);
            }

        private static int Throw547SqlException ()
            {
            ThrowSqlException(547);
            return 0;
            }

        private static int ThrowGenericSqlException ()
            {
            ThrowSqlException(0);
            return 0;
            }

        /* 
         * This black magic allows our mocks to throw a SqlException.
         * See http://blog.gauffin.org/2014/08/how-to-create-a-sqlexception/ for reference.
         */
        private static void ThrowSqlException (int errorNumber)
            {
            var collectionConstructor = typeof(SqlErrorCollection).GetConstructor(BindingFlags.NonPublic | BindingFlags.Instance, null, new Type[0], null);
            var addMethod = typeof(SqlErrorCollection).GetMethod("Add", BindingFlags.NonPublic | BindingFlags.Instance);
            var errorCollection = (SqlErrorCollection) collectionConstructor.Invoke(null);

            var errorConstructor = typeof(SqlError).GetConstructor(BindingFlags.NonPublic | BindingFlags.Instance, null,
                new[] { typeof(int), typeof(byte), typeof(byte), typeof(string), typeof(string), typeof(string), typeof(int), typeof(uint) },
                null);
            var error = errorConstructor.Invoke(new object[] { errorNumber, (byte) 0, (byte) 0, "server", "errMsg", "procedure", 100, (uint) 0 });

            addMethod.Invoke(errorCollection, new[] { error });

            var constructor = typeof(SqlException).GetConstructor(BindingFlags.NonPublic | BindingFlags.Instance, null,
                new[] { typeof(string), typeof(SqlErrorCollection), typeof(Exception), typeof(Guid) }, null);

            throw (SqlException) constructor.Invoke(new object[] { "Error message", errorCollection, new DataException(), Guid.NewGuid() });
            }
        }
    }
