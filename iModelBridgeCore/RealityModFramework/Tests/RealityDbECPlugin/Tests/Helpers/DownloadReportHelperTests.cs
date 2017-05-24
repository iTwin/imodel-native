using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;
using Rhino.Mocks;
using Rhino.Mocks.Constraints;
using Is = NUnit.Framework.Is;

namespace IndexECPlugin.Tests.Tests.Helpers
    {

    [TestFixture]
    class DownloadReportHelperTests
        {
        private MockRepository mocks;
        private Stream memoryStream;
        private IDbConnectionCreator dbConnectionCreatorStub;
        private IDbConnection dbConnectionMock;
        private IDbCommand dbCommandMock;
        private DbParameterCollection dbDataParameterCollection;
        private IDbDataParameter dbDataParameter0Stub;
        private IDbDataParameter dbDataParameter1Stub;
        private IDbDataParameter dbDataParameter2Stub;

        private const string packageName = "package";
        private const string connectionString = "connection string";
        private byte[] fileBytes = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9 };

        [SetUp]
        public void SetUp ()
            {
            mocks = new MockRepository();
            dbConnectionCreatorStub = mocks.Stub<IDbConnectionCreator>();
            dbConnectionMock = mocks.DynamicMock<IDbConnection>();
            dbCommandMock = mocks.DynamicMock<IDbCommand>();
            dbDataParameterCollection = mocks.Stub<DbParameterCollection>();
            dbDataParameter0Stub = mocks.Stub<IDbDataParameter>();
            dbDataParameter1Stub = mocks.Stub<IDbDataParameter>();
            dbDataParameter2Stub = mocks.Stub<IDbDataParameter>();

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
                SetupResult.For(dbConnectionCreatorStub.CreateDbConnection(Arg<string>.Is.Anything)).Return(dbConnectionMock);
                Expect.Call(dbConnectionMock.Open).Repeat.Once();
                Expect.Call(dbConnectionMock.CreateCommand()).Repeat.Once().Return(dbCommandMock);

                Expect.Call(dbCommandMock.CommandText).PropertyBehavior();
                dbCommandMock.CommandText = "";
                Expect.Call(dbCommandMock.CommandType).PropertyBehavior();
                dbCommandMock.CommandType = 0;
                Expect.Call(dbCommandMock.Parameters).Return(dbDataParameterCollection);
                Expect.Call(dbCommandMock.CreateParameter()).Return(dbDataParameter0Stub).Repeat.Once();
                Expect.Call(dbCommandMock.CreateParameter()).Return(dbDataParameter1Stub).Repeat.Once();
                Expect.Call(dbCommandMock.CreateParameter()).Return(dbDataParameter2Stub).Repeat.Once();
                Expect.Call(dbDataParameterCollection.Add(dbDataParameter0Stub)).Repeat.Once();
                Expect.Call(dbDataParameterCollection.Add(dbDataParameter1Stub)).Repeat.Once();
                Expect.Call(dbDataParameterCollection.Add(dbDataParameter2Stub)).Repeat.Once();

                Expect.Call(dbConnectionMock.Close).Repeat.Once();
                }
            using ( mocks.Playback() )
                {
                DownloadReportHelper.InsertInDatabase(memoryStream, packageName, connectionString, dbConnectionCreatorStub);
                Assert.That(dbCommandMock.CommandText, Is.EqualTo("IF 0 = (SELECT COUNT(*) FROM dbo.DownloadReports WHERE Id = @param0)" +
                                                                  " INSERT INTO dbo.DownloadReports (Id, CreationTime, ReportContent) VALUES " +
                                                                  "(@param0, @param1, @param2)"));
                }
            Assert.That(dbCommandMock.CommandType, Is.EqualTo(CommandType.Text));
            Assert.That(dbDataParameter0Stub.DbType, Is.EqualTo(DbType.String));
            Assert.That(dbDataParameter0Stub.ParameterName, Is.EqualTo("@param0"));
            Assert.That(dbDataParameter0Stub.Value, Is.EqualTo(packageName));

            Assert.That(dbDataParameter1Stub.DbType, Is.EqualTo(DbType.DateTime));
            Assert.That(dbDataParameter1Stub.ParameterName, Is.EqualTo("@param1"));
            Assert.That(dbDataParameter1Stub.Value, Is.EqualTo(DateTime.UtcNow).Within(1).Seconds);

            Assert.That(dbDataParameter2Stub.DbType, Is.EqualTo(DbType.Binary));
            Assert.That(dbDataParameter2Stub.ParameterName, Is.EqualTo("@param2"));
            Assert.That(((byte[])dbDataParameter2Stub.Value).SequenceEqual(fileBytes), Is.True);
            }

        [Test]
        public void InsertInDatabaseReportTooLargeExceptionTest ()
            {
            Stream streamStub = mocks.Stub<Stream>();
            //FakeRead fakeReadMethod = new FakeRead(FakeReadMethod); 
            //byte[] buffer = new byte[fileBytes.Length];
            //Expect.Call(memoryStream.Read(Arg<byte[]>.Is.NotNull, Arg<int>.Is.Equal(0), Arg<int>.Is.Equal(10))).Do(fakeReadMethod);
            //SetupResult.For(memoryStream.Read(array, 0, 10)).Do(new Action<byte[]>(x => array = fileBytes)).Return(10);    //TODO: fill array Arg<byte[]>.Out(new byte[10]).Dummy
            using ( mocks.Record() )
                {
                SetupResult.For(dbConnectionCreatorStub.CreateDbConnection(Arg<string>.Is.Anything)).Return(dbConnectionMock);
                Expect.Call(dbConnectionMock.Open).Repeat.Once();
                Expect.Call(dbConnectionMock.CreateCommand()).Repeat.Once().Return(dbCommandMock);

                Expect.Call(dbCommandMock.CommandText).PropertyBehavior();
                dbCommandMock.CommandText = "";
                Expect.Call(dbCommandMock.CommandType).PropertyBehavior();
                dbCommandMock.CommandType = 0;
                Expect.Call(dbCommandMock.Parameters).Return(dbDataParameterCollection);
                Expect.Call(dbCommandMock.CreateParameter()).Return(dbDataParameter0Stub).Repeat.Once();
                Expect.Call(dbCommandMock.CreateParameter()).Return(dbDataParameter1Stub).Repeat.Once();
                Expect.Call(dbDataParameterCollection.Add(dbDataParameter0Stub)).Repeat.Once();
                Expect.Call(dbDataParameterCollection.Add(dbDataParameter1Stub)).Repeat.Once();

                SetupResult.For(streamStub.Length).Return((long)int.MaxValue + 1);
                }
            using (mocks.Playback())
                {
                Assert.That(() => DownloadReportHelper.InsertInDatabase(streamStub, packageName, connectionString, dbConnectionCreatorStub),
                    Throws.TypeOf<UserFriendlyException>());
                }
            }

        [Test]
        public void InsertInDatabaseInvalidPackageIdExceptionTest ()
            {
            
            }
        }
    }
