using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Reflection;
using Bentley.Collections;
using Bentley.EC.Persistence.Query;
using Bentley.EC.PluginBuilder;
using Bentley.EC.PluginBuilder.Modules;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECSystem.Repository;
using Bentley.ECSystem.Session;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class PackagerTests
        {
        MockRepository m_mock;
        IECSchema m_schema;
        private IDbQuerier m_querierMock;
        ECPlugin m_plugin;
        IECInstance m_packageRequest;
        RepositoryConnection m_repositoryConnection;

        ECQuery query;
        IDbConnectionCreator dbConnectionCreatorStub;
        IDbConnection dbConnectionMock;
        FakeDbCommand fakeDbCommand;
        IDbDataParameter dataParameterStub1;
        IDbDataParameter dataParameterStub2;
        List<IECInstance> returnedList;
        ECProperty creationTimeProperty = new ECProperty("CreationTime", Bentley.ECObjects.ECObjects.StringType);

        [SetUp]
        public void SetUp ()
            {
            m_mock = new MockRepository();
            m_querierMock = (IDbQuerier) m_mock.StrictMock(typeof(IDbQuerier));
            m_schema = SetupHelpers.PrepareSchema();

            m_plugin = new IndexECPlugin.Source.IndexECPlugin().Plugin;

            IECClass packageRequestClass = m_schema["PackageRequest"];
            m_packageRequest = packageRequestClass.CreateInstance();

            IECClass requestedEntityClass = m_schema["RequestedEntity"];

            //packageRequest.InstanceId = Guid.NewGuid().ToString();
            m_packageRequest["CoordinateSystem"].StringValue = "EPSG:4326";
            m_packageRequest["Polygon"].StringValue = "[[18,51],[18,48],[11,48],[11,51],[18,51]]";
            m_packageRequest["OSM"].NativeValue = false;

            ECSession session = SessionManager.CreateSession();

            var tokenReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TokenExample.txt"));
            string token = tokenReader.ReadToEnd();

            ConnectionInfo connectionInfo = new ConnectionInfo
            {
                Fields = new ConnectionFieldInfo[] 
                { 
                    new ConnectionFieldInfo("Token", token)
                }
            };
            RepositoryIdentifier repositoryIdentifier = new RepositoryIdentifier("IndexECPlugin", @"Server", "", "", null);
            m_repositoryConnection = m_plugin.ConnectionModule.Open(session, repositoryIdentifier, connectionInfo, null);

            }

        [Test]
        public void IndexPackaging ()
            {
            IECInstance m_spatialEntity = SetupHelpers.CreateIndexSE(m_schema);
            IECInstance m_metadata = SetupHelpers.CreateIndexMetadata(m_schema);
            IECInstance m_spatialDataSource = SetupHelpers.CreateIndexSDS(m_schema);
            IECInstance m_server = SetupHelpers.CreateIndexServer(m_schema);

            SetupHelpers.CreateSE_SDSRel(m_spatialEntity, m_spatialDataSource, m_schema);
            SetupHelpers.CreateSE_MetadataRel(m_spatialEntity, m_metadata, m_schema);
            SetupHelpers.CreateServer_SDSRel(m_server, m_spatialDataSource, m_schema);

            var requestedEntitiesArray = m_packageRequest["RequestedEntities"] as IECArrayValue;
            IECStructValue requestedEntity = requestedEntitiesArray[0] as IECStructValue;

            requestedEntity["ID"].StringValue = "1";

            EnumerableBasedQueryHandler queryHandlerMock = delegate(QueryModule sender, RepositoryConnection connection, ECQuery query, ECQuerySettings querySettings)
            {
                return new List<IECInstance>() { m_spatialEntity };
            };

            Packager packager = new Packager(m_querierMock, queryHandlerMock);


            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.ExecuteNonQueryInDb(Arg<String>.Is.Anything, Arg<IParamNameValueMap>.Is.Anything)).Repeat.Once().Return(0);
                }
            using ( m_mock.Playback() )
                {
                string packagePath = null;
                try
                    {
                    packagePath = packager.InsertPackageRequest(m_schema, m_repositoryConnection, m_packageRequest, m_plugin.QueryModule, 2, 0, "UnitTest", "2.0");

                    string packageContent = File.ReadAllText(packagePath);

                    Assert.IsTrue(packageContent.Contains(m_spatialEntity.InstanceId));
                    Assert.IsTrue(packageContent.Contains(m_spatialEntity["Name"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_spatialEntity["DataProvider"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_spatialEntity["Dataset"].StringValue));

                    Assert.IsTrue(packageContent.Contains(m_metadata["Legal"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_metadata["TermsOfUse"].StringValue));

                    Assert.IsTrue(packageContent.Contains("www.test.com?11&amp;18&amp;48&amp;51&amp;bcc_user16@mailinator.com&amp;EPSG:4326#Location/In/Compound"));
                    Assert.IsTrue(packageContent.Contains(m_spatialDataSource["DataSourceType"].StringValue));
                    Assert.IsTrue(packageContent.Contains(((long) m_spatialDataSource["FileSize"].NativeValue).ToString()));
                    Assert.IsTrue(packageContent.Contains(m_spatialDataSource["CoordinateSystem"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_spatialDataSource["NoDataValue"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_spatialDataSource["SisterFiles"].StringValue));

                    Assert.IsTrue(packageContent.Contains(m_server["LoginKey"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_server["LoginMethod"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_server["RegistrationPage"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_server["OrganisationPage"].StringValue));
                    }
                finally
                    {
                    if ( packagePath != null && File.Exists(packagePath) )
                        {
                        File.Delete(packagePath);
                        }
                    }
                }
            }

        [Test]
        public void IndexPackagingMultiband ()
            {
            IECInstance m_spatialEntity = SetupHelpers.CreateIndexSE(m_schema);
            IECInstance m_metadata = SetupHelpers.CreateIndexMetadata(m_schema);
            IECInstance m_spatialDataSource = SetupHelpers.CreateIndexSDS(m_schema);
            IECInstance m_server = SetupHelpers.CreateIndexServer(m_schema);
            IECInstance m_multibandSource = SetupHelpers.CreateMultibandSource(m_schema);

            m_spatialDataSource["MainURL"].StringValue = "www.mainurl.com";

            SetupHelpers.CreateSE_SDSRel(m_spatialEntity, m_spatialDataSource, m_schema);
            SetupHelpers.CreateSE_MetadataRel(m_spatialEntity, m_metadata, m_schema);
            SetupHelpers.CreateServer_SDSRel(m_server, m_spatialDataSource, m_schema);
            SetupHelpers.CreateSE_SDSRel(m_spatialEntity, m_multibandSource, m_schema);

            var requestedEntitiesArray = m_packageRequest["RequestedEntities"] as IECArrayValue;
            IECStructValue requestedEntity = requestedEntitiesArray[0] as IECStructValue;

            requestedEntity["ID"].StringValue = "1";

            EnumerableBasedQueryHandler queryHandlerMock = delegate(QueryModule sender, RepositoryConnection connection, ECQuery query, ECQuerySettings querySettings)
            {
                return new List<IECInstance>() { m_spatialEntity };
            };

            Packager packager = new Packager(m_querierMock, queryHandlerMock);


            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.ExecuteNonQueryInDb(Arg<String>.Is.Anything, Arg<IParamNameValueMap>.Is.Anything)).Repeat.Once().Return(0);
                }
            using ( m_mock.Playback() )
                {
                string packagePath = null;
                try
                    {
                    packagePath = packager.InsertPackageRequest(m_schema, m_repositoryConnection, m_packageRequest, m_plugin.QueryModule, 2, 0, "UnitTest", "2.0");
                    string packageContent = File.ReadAllText(packagePath);

                    Assert.IsTrue(packageContent.Contains(m_spatialEntity.InstanceId));
                    Assert.IsTrue(packageContent.Contains(m_spatialEntity["Name"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_spatialEntity["DataProvider"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_spatialEntity["Dataset"].StringValue));

                    Assert.IsTrue(packageContent.Contains(m_metadata["Legal"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_metadata["TermsOfUse"].StringValue));

                    Assert.IsTrue(packageContent.Contains(m_spatialDataSource["MainURL"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_spatialDataSource["DataSourceType"].StringValue));
                    Assert.IsTrue(packageContent.Contains(((long) m_spatialDataSource["FileSize"].NativeValue).ToString()));
                    Assert.IsTrue(packageContent.Contains(m_spatialDataSource["CoordinateSystem"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_spatialDataSource["NoDataValue"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_spatialDataSource["SisterFiles"].StringValue));

                    Assert.IsTrue(packageContent.Contains(m_server["LoginKey"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_server["LoginMethod"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_server["RegistrationPage"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_server["OrganisationPage"].StringValue));

                    Assert.IsTrue(packageContent.Contains(m_multibandSource["RedBandURL"].StringValue));
                    Assert.IsTrue(packageContent.Contains(((long) m_multibandSource["RedBandFileSize"].NativeValue).ToString()));
                    Assert.IsTrue(packageContent.Contains(m_multibandSource["RedBandSisterFiles"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_multibandSource["GreenBandURL"].StringValue));
                    Assert.IsTrue(packageContent.Contains(((long) m_multibandSource["GreenBandFileSize"].NativeValue).ToString()));
                    Assert.IsTrue(packageContent.Contains(m_multibandSource["GreenBandSisterFiles"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_multibandSource["BlueBandURL"].StringValue));
                    Assert.IsTrue(packageContent.Contains(((long) m_multibandSource["BlueBandFileSize"].NativeValue).ToString()));
                    Assert.IsTrue(packageContent.Contains(m_multibandSource["BlueBandSisterFiles"].StringValue));
                    Assert.IsTrue(packageContent.Contains(m_multibandSource["PanchromaticBandURL"].StringValue));
                    Assert.IsTrue(packageContent.Contains(((long) m_multibandSource["PanchromaticBandFileSize"].NativeValue).ToString()));
                    Assert.IsTrue(packageContent.Contains(m_multibandSource["PanchromaticBandSisterFiles"].StringValue));
                    }
                finally
                    {
                    if ( packagePath != null && File.Exists(packagePath) )
                        {
                        File.Delete(packagePath);
                        }
                    }
                }
            }
        [Test]
        public void IndexOSMPackaging ()
            {
            IECInstance m_spatialEntity = SetupHelpers.CreateIndexSE(m_schema);
            IECInstance m_metadata = SetupHelpers.CreateIndexMetadata(m_schema);
            IECInstance m_osmSource = SetupHelpers.CreateOsmSource(m_schema);

            SetupHelpers.CreateSE_MetadataRel(m_spatialEntity, m_metadata, m_schema);
            SetupHelpers.CreateSE_SDSRel(m_spatialEntity, m_osmSource, m_schema);

            m_packageRequest["OSM"].NativeValue = true;

            var requestedEntitiesArray = m_packageRequest["RequestedEntities"] as IECArrayValue;

            EnumerableBasedQueryHandler queryHandlerMock = delegate(QueryModule sender, RepositoryConnection connection, ECQuery query, ECQuerySettings querySettings)
            {
                return new List<IECInstance>() { m_spatialEntity };
            };

            Packager packager = new Packager(m_querierMock, queryHandlerMock);


            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.ExecuteNonQueryInDb(Arg<String>.Is.Anything, Arg<IParamNameValueMap>.Is.Anything)).Repeat.Once().Return(0);
                }
            using ( m_mock.Playback() )
                {
                string packagePath = packager.InsertPackageRequest(m_schema, m_repositoryConnection, m_packageRequest, m_plugin.QueryModule, 2, 0, "UnitTest", "2.0");

                string packageContent = File.ReadAllText(packagePath);

                Assert.IsTrue(packageContent.Contains(m_spatialEntity.InstanceId));
                Assert.IsTrue(packageContent.Contains(m_spatialEntity["Name"].StringValue));
                //Assert.IsTrue(packageContent.Contains(m_spatialEntity["DataProvider"].StringValue));
                //Assert.IsTrue(packageContent.Contains(m_spatialEntity["Dataset"].StringValue));

                Assert.IsTrue(packageContent.Contains(m_metadata["Legal"].StringValue));
                //Assert.IsTrue(packageContent.Contains(m_metadata["TermsOfUse"].StringValue));

                Assert.IsTrue(packageContent.Contains(m_osmSource["MainURL"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_osmSource["CoordinateSystem"].StringValue));
                Assert.IsTrue(packageContent.Contains("type=\"osm\""));

                }
            }

        [Test]
        public void IndexUsgsPackaging ()
            {
            IECInstance m_spatialEntity = SetupHelpers.CreateUsgsSE(true, m_schema);
            IECInstance m_metadata = SetupHelpers.CreateUsgsMetadata(true, m_schema);
            IECInstance m_spatialDataSource = SetupHelpers.CreateUsgsSDS(true, m_schema);
            IECInstance m_server = SetupHelpers.CreateUsgsServer(true, m_schema);

            SetupHelpers.CreateSE_SDSRel(m_spatialEntity, m_spatialDataSource, m_schema);
            SetupHelpers.CreateSE_MetadataRel(m_spatialEntity, m_metadata, m_schema);
            SetupHelpers.CreateServer_SDSRel(m_server, m_spatialDataSource, m_schema);

            var requestedEntitiesArray = m_packageRequest["RequestedEntities"] as IECArrayValue;
            IECStructValue requestedEntity = requestedEntitiesArray[0] as IECStructValue;

            requestedEntity["ID"].StringValue = "553690bfe4b0b22a15807df2";

            EnumerableBasedQueryHandler queryHandlerMock = delegate(QueryModule sender, RepositoryConnection connection, ECQuery query, ECQuerySettings querySettings)
            {
                return new List<IECInstance>() { m_spatialEntity };
            };

            Packager packager = new Packager(m_querierMock, queryHandlerMock);


            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.ExecuteNonQueryInDb(Arg<String>.Is.Anything, Arg<IParamNameValueMap>.Is.Anything)).Repeat.Once().Return(0);
                }
            using ( m_mock.Playback() )
                {
                string packagePath = packager.InsertPackageRequest(m_schema, m_repositoryConnection, m_packageRequest, m_plugin.QueryModule, 2, 0, "UnitTest", "2.0");

                string packageContent = File.ReadAllText(packagePath);

                Assert.IsTrue(packageContent.Contains(m_spatialEntity.InstanceId));
                Assert.IsTrue(packageContent.Contains(m_spatialEntity["Name"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_spatialEntity["DataProvider"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_spatialEntity["Dataset"].StringValue));

                Assert.IsTrue(packageContent.Contains(m_metadata["Legal"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_metadata["TermsOfUse"].StringValue));

                Assert.IsTrue(packageContent.Contains(m_spatialDataSource["MainURL"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_spatialDataSource["DataSourceType"].StringValue));
                Assert.IsTrue(packageContent.Contains(((long) m_spatialDataSource["FileSize"].NativeValue).ToString()));
                Assert.IsTrue(packageContent.Contains(m_spatialDataSource["CoordinateSystem"].StringValue));

                }
            }

        [Test]
        public void IndexWMSPackaging ()
            {
            IECInstance m_spatialEntity = SetupHelpers.CreateIndexSE(m_schema);
            IECInstance m_metadata = SetupHelpers.CreateIndexMetadata(m_schema);
            IECInstance m_spatialDataSource = SetupHelpers.CreateIndexSDS(m_schema);
            IECInstance m_server = SetupHelpers.CreateIndexServer(m_schema);
            IECInstance m_wmsDataSource = SetupHelpers.CreateWMSSource(m_schema);
            IECInstance m_wmsserver = SetupHelpers.CreateWMSServer(m_schema);

            m_spatialDataSource["MainURL"].StringValue = "www.mainurl.com";

            SetupHelpers.CreateSE_SDSRel(m_spatialEntity, m_spatialDataSource, m_schema);
            SetupHelpers.CreateServer_SDSRel(m_server, m_spatialDataSource, m_schema);
            SetupHelpers.CreateSE_SDSRel(m_spatialEntity, m_wmsDataSource, m_schema);
            SetupHelpers.CreateSE_MetadataRel(m_spatialEntity, m_metadata, m_schema);
            SetupHelpers.CreateServer_SDSRel(m_wmsserver, m_wmsDataSource, m_schema);

            var requestedEntitiesArray = m_packageRequest["RequestedEntities"] as IECArrayValue;
            IECStructValue requestedEntity = requestedEntitiesArray[0] as IECStructValue;

            requestedEntity["ID"].StringValue = "1";
            requestedEntity["SelectedFormat"].StringValue = "default";
            requestedEntity["SelectedStyle"].StringValue = "default";

            EnumerableBasedQueryHandler queryHandlerMock = delegate(QueryModule sender, RepositoryConnection connection, ECQuery query, ECQuerySettings querySettings)
            {
                return new List<IECInstance>() { m_spatialEntity };
            };

            Packager packager = new Packager(m_querierMock, queryHandlerMock);


            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.ExecuteNonQueryInDb(Arg<String>.Is.Anything, Arg<IParamNameValueMap>.Is.Anything)).Repeat.Once().Return(0);
                }
            using ( m_mock.Playback() )
                {
                string packagePath = packager.InsertPackageRequest(m_schema, m_repositoryConnection, m_packageRequest, m_plugin.QueryModule, 2, 0, "UnitTest", "2.0");

                string packageContent = File.ReadAllText(packagePath);

                Assert.IsTrue(packageContent.Contains(m_spatialEntity.InstanceId));
                Assert.IsTrue(packageContent.Contains(m_spatialEntity["Name"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_spatialEntity["DataProvider"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_spatialEntity["Dataset"].StringValue));

                Assert.IsTrue(packageContent.Contains(m_spatialDataSource["MainURL"].StringValue));
                Assert.IsTrue(packageContent.Contains("type=\"wms\""));

                Assert.IsTrue(packageContent.Contains(m_metadata["Legal"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_metadata["TermsOfUse"].StringValue));


                Assert.IsTrue(packageContent.Contains(((long) m_wmsDataSource["FileSize"].NativeValue).ToString()));
                Assert.IsTrue(packageContent.Contains(m_wmsDataSource["CoordinateSystem"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_wmsDataSource["NoDataValue"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_wmsDataSource["SisterFiles"].StringValue));

                Assert.IsTrue(packageContent.Contains(m_wmsserver["LoginKey"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_wmsserver["LoginMethod"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_wmsserver["RegistrationPage"].StringValue));
                Assert.IsTrue(packageContent.Contains(m_wmsserver["OrganisationPage"].StringValue));

                }
            }

        [Test]
        public void ExtractStatsNonNullValuesTest ()
            {
            object[][] records = new object[2][];
            records[0] = new object[] { "package 1", "polygon 1", new DateTime(), "user id 1" };
            records[1] = new object[] { "package 2", "polygon 2", new DateTime(), "user id 2" };

            SetFakes(records);

            SetExpectations();
            ExtractStats();

            Assert.That(fakeDbCommand.CommandText, Is.EqualTo("SELECT t.Name, t.BoundingPolygon, t.CreationTime, t.UserId FROM dbo.Packages AS" +
                                                              " t WHERE t.CreationTime > @startTime AND t.CreationTime < @endTime AND" +
                                                              " t.BentleyInternal = 0 "));
            Assert.That(fakeDbCommand.CommandType, Is.EqualTo(CommandType.Text));
            Assert.That(fakeDbCommand.Connection, Is.EqualTo(dbConnectionMock));
            Assert.That(fakeDbCommand.Parameters.Contains(dataParameterStub1));
            Assert.That(fakeDbCommand.Parameters.Contains(dataParameterStub2));

            Assert.That(dataParameterStub1.DbType, Is.EqualTo(DbType.DateTime));
            Assert.That(dataParameterStub1.ParameterName, Is.EqualTo("@startTime"));
            Assert.That(dataParameterStub1.Value, Is.EqualTo(new DateTime(DateTime.Now.Year, DateTime.Now.Month, 1).AddMonths(-1)));

            Assert.That(dataParameterStub2.DbType, Is.EqualTo(DbType.DateTime));
            Assert.That(dataParameterStub2.ParameterName, Is.EqualTo("@endTime"));
            Assert.That(dataParameterStub2.Value, Is.EqualTo(new DateTime(DateTime.Now.Year, DateTime.Now.Month, 1)));

            Assert.That(returnedList.Count, Is.EqualTo(records.Length));
            for ( int i = 0; i < returnedList.Count; i++ )
                {
                Assert.That(returnedList[i]["Name"].StringValue, Is.EqualTo(records[i][0]));
                Assert.That(returnedList[i]["BoundingPolygon"].StringValue, Is.EqualTo(records[i][1]));
                Assert.That(returnedList[i]["CreationTime"].NativeValue, Is.EqualTo(records[i][2]));
                Assert.That(returnedList[i]["UserId"].StringValue, Is.EqualTo(records[i][3]));
                }
            }

        [Test]
        public void ExtractStatsWithNullValuesTest ()
            {
            object[][] records = new object[1][];
            records[0] = new object[] { null, null, null, null };

            SetFakes(records);

            SetExpectations();
            ExtractStats();

            Assert.That(returnedList[0]["Name"].IsNull);
            Assert.That(returnedList[0]["BoundingPolygon"].IsNull);
            Assert.That(returnedList[0]["CreationTime"].IsNull);
            Assert.That(returnedList[0]["UserId"].IsNull);
            }

        [Test]
        public void ExtractStatsWithWhereClauseTest ()
            {
            DateTime startTime = new DateTime(2017, 1, 1);
            DateTime endTime = new DateTime();

            WhereCriteria whereClause = new WhereCriteria();
            whereClause.Add(new PropertyExpression(RelationalOperator.LT, creationTimeProperty, endTime));
            whereClause.Add(new PropertyExpression(RelationalOperator.GTEQ, creationTimeProperty, startTime));

            object[][] records = new object[2][];
            records[0] = new object[] { "package 1", "polygon 1", new DateTime(), "user id 1" };
            records[1] = new object[] { "package 2", "polygon 2", new DateTime(), "user id 2" };

            SetFakes(records);

            query.WhereClause = whereClause;

            SetExpectations();
            ExtractStats();

            Assert.That(dataParameterStub1.Value, Is.EqualTo(startTime));
            Assert.That(dataParameterStub2.Value, Is.EqualTo(endTime));
            }

        [Test]
        public void ExtractStatsQueryContainsBentleyInternalStatement ()
            {
            SetFakes(null);
            query.ExtendedDataValueSetter.Add("includebentleyinternal", true);

            SetExpectations();
            ExtractStats();

            Assert.That(fakeDbCommand.CommandText, Is.EqualTo("SELECT t.Name, t.BoundingPolygon, t.CreationTime, t.UserId FROM dbo.Packages AS" +
                                                              " t WHERE t.CreationTime > @startTime AND t.CreationTime < @endTime"));
            }

        [Test]
        public void ExtractStatsOrLogicalOperatorExceptionTest ()
            {
            query = new ECQuery();
            IECSchema schemaStub = m_mock.Stub<IECSchema>();
            dbConnectionCreatorStub = m_mock.Stub<IDbConnectionCreator>();

            WhereCriteria whereClause = new WhereCriteria();
            whereClause.Add(new PropertyExpression(RelationalOperator.LT, creationTimeProperty, new DateTime()));
            whereClause.Add(new PropertyExpression(RelationalOperator.GTEQ, creationTimeProperty, new DateTime(2017, 1, 1)));
            whereClause.SetLogicalOperatorAfter(0, LogicalOperator.OR);

            query.WhereClause = whereClause;

            Assert.That(() => Packager.ExtractStats(query, "connection string", schemaStub, dbConnectionCreatorStub),
                Throws.TypeOf<UserFriendlyException>().With.Message.EqualTo("This query only uses AND logical operators"));
            }

        [Test]
        public void ExtractStatsOnlyOneDateSpecifiedExceptionTest ()
            {
            query = new ECQuery();
            IECSchema schemaStub = m_mock.Stub<IECSchema>();
            dbConnectionCreatorStub = m_mock.Stub<IDbConnectionCreator>();

            WhereCriteria whereClause = new WhereCriteria();
            whereClause.Add(new PropertyExpression(RelationalOperator.LT, creationTimeProperty, new DateTime()));

            query.WhereClause = whereClause;

            Assert.That(() => Packager.ExtractStats(query, "connection string", schemaStub, dbConnectionCreatorStub),
                Throws.TypeOf<UserFriendlyException>().With.Message.EqualTo("Please specify both start and end times."));
            }

        private void SetFakes (object[][] dataReaderRecords)
            {
            query = new ECQuery();
            dbConnectionCreatorStub = m_mock.Stub<IDbConnectionCreator>();
            dbConnectionMock = m_mock.DynamicMock<IDbConnection>();
            fakeDbCommand = new FakeDbCommand();
            dataParameterStub1 = m_mock.Stub<IDbDataParameter>();
            dataParameterStub2 = m_mock.Stub<IDbDataParameter>();
            fakeDbCommand.DbDataParametersToCreate.Add(dataParameterStub1);
            fakeDbCommand.DbDataParametersToCreate.Add(dataParameterStub2);
            DataReaderStub dataReaderStub = new DataReaderStub(dataReaderRecords);
            fakeDbCommand.DataReaderStub = dataReaderStub;
            }

        private void SetExpectations ()
            {
            using ( m_mock.Record() )
                {
                SetupResult.For(dbConnectionCreatorStub.CreateDbConnection(Arg<string>.Is.Anything)).Return(dbConnectionMock);
                Expect.Call(dbConnectionMock.Open).Repeat.Once();
                Expect.Call(dbConnectionMock.CreateCommand()).Repeat.Once().Return(fakeDbCommand);
                }
            }

        private void ExtractStats ()
            {
            using ( m_mock.Playback() )
                {
                returnedList = Packager.ExtractStats(query, "connection string", m_schema, dbConnectionCreatorStub);
                }
            }
        }
    }
