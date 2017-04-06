﻿using System;
using System.Collections.Generic;
using System.Linq;

using NUnit.Framework;
using Rhino.Mocks;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Source;
using Newtonsoft.Json.Linq;
using System.IO;
using System.Xml;
using Bentley.EC.Persistence.Query;
using IndexECPlugin.Source.QueryProviders;
using Bentley.ECSystem.Repository;
using Bentley.ECSystem.Session;
using Bentley.ECObjects.Schema;
using Bentley.EC.Persistence;
using System.Reflection;
using Bentley.ECObjects.XML;
using Bentley.ECObjects.Instance;
using Bentley.EC.Persistence.Operations;
using IndexECPlugin.Tests.Common;

namespace IndexECPlugin.Tests
    {
    [TestFixture]
    class IndexECPluginRDSTests
        {
        IRDSDataFetcher m_RDSDataFetcherMock;
        IECSchema m_schema;
        MockRepository m_mock;
        string m_rdsUrlBaseTest = "http://www.testurl.com/";


        JArray m_rdsArray;
        JObject m_rdsSingleObject;

        [SetUp]
        public void SetUp ()
            {
            m_mock = new MockRepository();

            CreateFetcherMock();

            m_schema = SetupHelpers.PrepareSchema();
            }

        private void CreateFetcherMock ()
            {

            m_RDSDataFetcherMock = (IRDSDataFetcher) m_mock.StrictMock(typeof(IRDSDataFetcher));

            m_rdsArray = JArray.Parse("[{\"instanceId\":\"\",\"schemaName\":\"S3MX\",\"className\":\"RealityData\",\"properties\":{\"Id\":\"05610e4c-79d4-43ef-a9e5-e02e6328d843\",\"Enterprise\":\"5e41126f-6875-400f-9f75-4492c99ee544\",\"ContainerName\":\"05610e4c-79d4-43ef-a9e5-e02e6328d843\",\"Name\":\"Salt Lake City\",\"Dataset\":\"Landsat 8\",\"Description\":\"Lorem ipsum dolor sit amet.\",\"RootDocument\":\"Graz/Scene/Production_Graz_3MX.3mx\",\"Size\":4418267,\"Classification\":\"Terrain\",\"Streamed\":true,\"Type\":\"3mx\",\"Footprint\":\"{ \\\"points\\\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \\\"coordinate_system\\\" : \\\"4326\\\" }\",\"ThumbnailDocument\":\"Testdir/TestFile\",\"MetadataURL\":null,\"Copyright\":\"TestCopyright\",\"TermsOfUse\":\"TestTermsOfUse\",\"AccuracyInMeters\":\"16.147\",\"ResolutionInMeters\":null,\"PublicAccess\":true,\"Listable\":true,\"ModifiedTimestamp\":\"2017-01-18T18:42:58.7622253Z\",\"CreatedTimestamp\":\"2016-12-13T15:23:54.1431052Z\",\"OwnedBy\":\"francis.boily@bentley.com\"},\"eTag\":\"\\\"eGIOVrqUoUuzm+NmmddfsE7VGkQ=\\\"\"}]");
            m_rdsSingleObject = (JObject) m_rdsArray.First();

            SetupResult.For(m_RDSDataFetcherMock.RdsUrlBase).Return(m_rdsUrlBaseTest);

            }

        [Test]
        public void SpatialEntityTest ()
            {

            using(m_mock.Record())
                {
                Expect.Call(m_RDSDataFetcherMock.GetDataBySpatialQuery(Arg<String>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(m_RDSDataFetcherMock.GetSingleData(Arg<String>.Is.Anything)).Repeat.Once().Return(m_rdsSingleObject);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "rds"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("05610e4c-79d4-43ef-a9e5-e02e6328d843"));

                RdsAPIQueryProvider rdsAPIQueryProvider = new RdsAPIQueryProvider(query, new ECQuerySettings(), m_RDSDataFetcherMock, null, m_schema);

                var instanceList = rdsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");

                var instance = instanceList.First();

                //[[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]]

                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.InstanceId, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instance.GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \"coordinate_system\" : \"4326\" }"), "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("ApproximateFootprint").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("Salt Lake City", instance.GetPropertyValue("Name").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("3mx", instance.GetPropertyValue("DataSourceTypesAvailable").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("16.147", instance.GetPropertyValue("AccuracyInMeters").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("ResolutionInMeters").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(m_rdsUrlBaseTest + "Document/" + instance.InstanceId + "~2f" + "Testdir~2fTestFile", instance.GetPropertyValue("ThumbnailURL").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("BentleyCONNECT", instance.GetPropertyValue("ThumbnailLoginKey").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("DataProvider").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("DataProviderName").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("Landsat 8", instance.GetPropertyValue("Dataset").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(DateTime.Parse("2016-12-13T15:23:54"), instance.GetPropertyValue("Date").NativeValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Terrain", instance.GetPropertyValue("Classification").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Occlusion").IsNull, "The content of the instance was not set properly.");
                }
            }

        [Test]
        public void MetadataTest ()
            {

            using ( m_mock.Record() )
                {
                Expect.Call(m_RDSDataFetcherMock.GetDataBySpatialQuery(Arg<String>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(m_RDSDataFetcherMock.GetSingleData(Arg<String>.Is.Anything)).Repeat.Once().Return(m_rdsSingleObject);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("Metadata"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "rds"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("05610e4c-79d4-43ef-a9e5-e02e6328d843"));

                RdsAPIQueryProvider rdsAPIQueryProvider = new RdsAPIQueryProvider(query, new ECQuerySettings(), m_RDSDataFetcherMock, null, m_schema);

                var instanceList = rdsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");

                var instance = instanceList.First();

                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.InstanceId, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("MetadataURL").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("DisplayStyle").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("Lorem ipsum dolor sit amet.", instance.GetPropertyValue("Description").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Owned by francis.boily@bentley.com", instance.GetPropertyValue("ContactInformation").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Keywords").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("TestCopyright", instance.GetPropertyValue("Legal").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("TestTermsOfUse", instance.GetPropertyValue("TermsOfUse").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Lineage").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Provenance").IsNull, "The content of the instance was not set properly.");
                }
            }

        [Test]
        public void SpatialDataSourceTest ()
            {

            using ( m_mock.Record() )
                {
                Expect.Call(m_RDSDataFetcherMock.GetDataBySpatialQuery(Arg<String>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(m_RDSDataFetcherMock.GetSingleData(Arg<String>.Is.Anything)).Repeat.Once().Return(m_rdsSingleObject);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialDataSource"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "rds"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("05610e4c-79d4-43ef-a9e5-e02e6328d843"));

                RdsAPIQueryProvider rdsAPIQueryProvider = new RdsAPIQueryProvider(query, new ECQuerySettings(), m_RDSDataFetcherMock, null, m_schema);

                var instanceList = rdsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");

                var instance = instanceList.First();

                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.InstanceId, "The content of the instance was not set properly.");
                Assert.AreEqual(m_rdsUrlBaseTest + "Document/" + instance.InstanceId + "~2f" + "Graz~2fScene~2fProduction_Graz_3MX.3mx", instance.GetPropertyValue("MainURL").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("ParameterizedURL").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("CompoundType").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("LocationInCompound").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("3mx", instance.GetPropertyValue("DataSourceType").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("SisterFiles").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("NoDataValue").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(4418267, (Int64) instance.GetPropertyValue("FileSize").NativeValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("CoordinateSystem").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue((bool) instance.GetPropertyValue("Streamed").NativeValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Metadata").IsNull, "The content of the instance was not set properly.");


                }
            }

        [Test]
        public void ServerTest ()
            {

            using ( m_mock.Record() )
                {
                Expect.Call(m_RDSDataFetcherMock.GetDataBySpatialQuery(Arg<String>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(m_RDSDataFetcherMock.GetSingleData(Arg<String>.Is.Anything)).Repeat.Once().Return(m_rdsSingleObject);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("Server"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "rds"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("05610e4c-79d4-43ef-a9e5-e02e6328d843"));

                RdsAPIQueryProvider rdsAPIQueryProvider = new RdsAPIQueryProvider(query, new ECQuerySettings(), m_RDSDataFetcherMock, null, m_schema);

                var instanceList = rdsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");

                var instance = instanceList.First();

                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.InstanceId, "The content of the instance was not set properly.");
                Assert.AreEqual(m_rdsUrlBaseTest.Split(':')[0], instance.GetPropertyValue("CommunicationProtocol").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue((bool) instance.GetPropertyValue("Streamed").NativeValue, "The content of the instance was not set properly.");
                Assert.AreEqual("BentleyCONNECT", instance.GetPropertyValue("LoginKey").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("CUSTOM", instance.GetPropertyValue("LoginMethod").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https://ims.bentley.com/IMS/Registration/", instance.GetPropertyValue("RegistrationPage").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https://www.bentley.com/", instance.GetPropertyValue("OrganisationPage").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("RealityDataService", instance.GetPropertyValue("Name").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(m_rdsUrlBaseTest, instance.GetPropertyValue("URL").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("ServerContactInformation").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Fees").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Legal").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("AccessConstraints").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Online").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("LastCheck").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("LastTimeOnline").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Latency").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("MeanReachabilityStats").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("State").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Type").IsNull, "The content of the instance was not set properly.");
                }
            }

        [Test]
        public void SpatialEntityWithDetailsViewTest ()
            {

            using ( m_mock.Record() )
                {
                Expect.Call(m_RDSDataFetcherMock.GetDataBySpatialQuery(Arg<String>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(m_RDSDataFetcherMock.GetSingleData(Arg<String>.Is.Anything)).Repeat.Once().Return(m_rdsSingleObject);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityWithDetailsView"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "rds"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("05610e4c-79d4-43ef-a9e5-e02e6328d843"));

                RdsAPIQueryProvider rdsAPIQueryProvider = new RdsAPIQueryProvider(query, new ECQuerySettings(), m_RDSDataFetcherMock, null, m_schema);

                var instanceList = rdsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");

                var instance = instanceList.First();

                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.InstanceId, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instance.GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \"coordinate_system\" : \"4326\" }"), "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("ApproximateFootprint").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("Salt Lake City", instance.GetPropertyValue("Name").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Lorem ipsum dolor sit amet.", instance.GetPropertyValue("Description").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Owned by francis.boily@bentley.com", instance.GetPropertyValue("ContactInformation").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Keywords").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("TestCopyright", instance.GetPropertyValue("Legal").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("TestTermsOfUse", instance.GetPropertyValue("TermsOfUse").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("3mx", instance.GetPropertyValue("DataSourceType").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("16.147", instance.GetPropertyValue("AccuracyInMeters").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("ResolutionInMeters").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(DateTime.Parse("2016-12-13T15:23:54"), instance.GetPropertyValue("Date").NativeValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Terrain", instance.GetPropertyValue("Classification").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(4418267, (Int64) instance.GetPropertyValue("FileSize").NativeValue, "The content of the instance was not set properly.");
                Assert.IsTrue((bool) instance.GetPropertyValue("Streamed").NativeValue, "The content of the instance was not set properly.");
                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.GetPropertyValue("SpatialDataSourceId").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(m_rdsUrlBaseTest + "Document/" + instance.InstanceId + "~2f" + "Testdir~2fTestFile", instance.GetPropertyValue("ThumbnailURL").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("DataProvider").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("DataProviderName").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("Landsat 8", instance.GetPropertyValue("Dataset").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Occlusion").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("MetadataURL").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("RawMetadataURL").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("RawMetadataFormat").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("RDS", instance.GetPropertyValue("SubAPI").StringValue, "The content of the instance was not set properly.");
                }
            }

        [Test]
        public void SpatialEntityWithDetailsViewSpatialTest ()
            {

            using ( m_mock.Record() )
                {
                Expect.Call(m_RDSDataFetcherMock.GetDataBySpatialQuery(Arg<String>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Once().Return(m_rdsArray);
                Expect.Call(m_RDSDataFetcherMock.GetSingleData(Arg<String>.Is.Anything)).Repeat.Never();
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityWithDetailsView"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \"coordinate_system\" : \"4326\" }");

                RdsAPIQueryProvider rdsAPIQueryProvider = new RdsAPIQueryProvider(query, new ECQuerySettings(), m_RDSDataFetcherMock, null, m_schema);

                var instanceList = rdsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");

                var instance = instanceList.First();

                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.InstanceId, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instance.GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \"coordinate_system\" : \"4326\" }"), "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("ApproximateFootprint").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("Salt Lake City", instance.GetPropertyValue("Name").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Lorem ipsum dolor sit amet.", instance.GetPropertyValue("Description").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Owned by francis.boily@bentley.com", instance.GetPropertyValue("ContactInformation").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Keywords").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("TestCopyright", instance.GetPropertyValue("Legal").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("TestTermsOfUse", instance.GetPropertyValue("TermsOfUse").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("3mx", instance.GetPropertyValue("DataSourceType").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("16.147", instance.GetPropertyValue("AccuracyInMeters").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("ResolutionInMeters").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(DateTime.Parse("2016-12-13T15:23:54"), instance.GetPropertyValue("Date").NativeValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Terrain", instance.GetPropertyValue("Classification").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(4418267, (Int64) instance.GetPropertyValue("FileSize").NativeValue, "The content of the instance was not set properly.");
                Assert.IsTrue((bool) instance.GetPropertyValue("Streamed").NativeValue, "The content of the instance was not set properly.");
                Assert.AreEqual("05610e4c-79d4-43ef-a9e5-e02e6328d843", instance.GetPropertyValue("SpatialDataSourceId").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(m_rdsUrlBaseTest + "Document/" + instance.InstanceId + "~2f" + "Testdir~2fTestFile", instance.GetPropertyValue("ThumbnailURL").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("DataProvider").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("DataProviderName").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("Landsat 8", instance.GetPropertyValue("Dataset").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("Occlusion").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("MetadataURL").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("RawMetadataURL").IsNull, "The content of the instance was not set properly.");
                Assert.IsTrue(instance.GetPropertyValue("RawMetadataFormat").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("RDS", instance.GetPropertyValue("SubAPI").StringValue, "The content of the instance was not set properly.");
                }
            }

        }
    }
