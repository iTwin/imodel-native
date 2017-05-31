using System;
using System.Data;
using System.Data.Common;
using System.IO;
using System.Linq;
using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Operations;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {

    [TestFixture]
    class RetrievalControllerTests
        {
        private IECSchema schema;
        private IECInstance instance;

        private MockRepository mocks;
        private IDbConnectionCreator dbConnectionCreatorStub;
        private IDbConnection dbConnectionMock;
        private IDbCommand dbCommandMock;
        private DbParameterCollection dbDataParameterCollection;
        private IDbDataParameter dbDataParameterStub;
        private DataReaderStub dataReaderStub;

        private const string connectionString = "connection string";
        private const int numBytesInStream = 8192;
        private readonly byte[] bytes = Enumerable.Repeat((byte) 0xA5, numBytesInStream).ToArray();
        private object[][] records;

        [SetUp]
        public void SetUp ()
            {
            schema = SetupHelpers.PrepareSchema();
            instance = SetupHelpers.CreateIndexSEWDV(schema);

            mocks = new MockRepository();
            dbConnectionCreatorStub = mocks.Stub<IDbConnectionCreator>();
            dbConnectionMock = mocks.DynamicMock<IDbConnection>();
            dbCommandMock = mocks.DynamicMock<IDbCommand>();
            dbDataParameterCollection = mocks.Stub<DbParameterCollection>();
            dbDataParameterStub = mocks.Stub<IDbDataParameter>();

            records = new object[1][];
            records[0] = new object[] { bytes };
            }

        [Test]
        public void RetrievePackageTest ()
            {
            dataReaderStub = new DataReaderStub(records);

            SetExpectations();

            using ( mocks.Playback() )
                {
                RetrievalController.RetrievePackage(instance, connectionString, dbConnectionCreatorStub);
                Assert.That(dbCommandMock.CommandText, Is.EqualTo("SELECT FileContent FROM dbo.Packages WHERE Name = @param0"));
                }

            VerifyAssertions();
            }

        [Test]
        public void RetrieveDownloadReportTest ()
            {
            dataReaderStub = new DataReaderStub(records);

            SetExpectations();

            using ( mocks.Playback() )
                {
                RetrievalController.RetrieveDownloadReport(instance, connectionString, dbConnectionCreatorStub);
                Assert.That(dbCommandMock.CommandText, Is.EqualTo("SELECT ReportContent FROM dbo.DownloadReports WHERE Id = @param0"));
                }

            VerifyAssertions();
            }

        [Test]
        public void InstanceDoesNotExistExceptionTest ()
            {
            dataReaderStub = new DataReaderStub(null);

            SetExpectations();

            using ( mocks.Playback() )
                {
                Assert.That(() => RetrievalController.RetrievePackage(instance, connectionString, dbConnectionCreatorStub),
                    Throws.TypeOf<InstanceDoesNotExistException>().With.Message.EqualTo("There is no such Package."));
                }
            }


        private void SetExpectations ()
            {
            using ( mocks.Record() )
                {
                SetupResult.For(dbConnectionCreatorStub.CreateDbConnection(Arg<string>.Is.Anything)).Return(dbConnectionMock);
                Expect.Call(dbConnectionMock.Open).Repeat.Once();
                Expect.Call(dbConnectionMock.CreateCommand()).Repeat.Once().Return(dbCommandMock);

                Expect.Call(dbCommandMock.CommandText).PropertyBehavior();
                dbCommandMock.CommandText = "";
                Expect.Call(dbCommandMock.Parameters).Return(dbDataParameterCollection);
                Expect.Call(dbCommandMock.CreateParameter()).Return(dbDataParameterStub).Repeat.Once();
                Expect.Call(dbCommandMock.ExecuteReader(Arg<CommandBehavior>.Is.Equal(CommandBehavior.SequentialAccess)))
                    .Repeat.Once().Return(dataReaderStub);
                }
            }

        private void VerifyAssertions ()
            {
            StreamBackedDescriptor descriptor;

            Assert.That(dbDataParameterStub.DbType, Is.EqualTo(DbType.String));
            Assert.That(dbDataParameterStub.ParameterName, Is.EqualTo("@param0"));
            Assert.That(dbDataParameterStub.Value, Is.EqualTo(instance.InstanceId));

            Assert.That(StreamBackedDescriptorAccessor.TryGetFrom(instance, out descriptor), Is.True);

            Assert.That(((MemoryStream) descriptor.Stream).ToArray().SequenceEqual(bytes), Is.True);
            Assert.That(descriptor.FileName, Is.EqualTo(instance.InstanceId));
            Assert.That(descriptor.ExpectedSize, Is.EqualTo(numBytesInStream));
            Assert.That(descriptor.LastUpdateTime, Is.EqualTo(DateTime.UtcNow).Within(100).Milliseconds);
            }
        }
    }
