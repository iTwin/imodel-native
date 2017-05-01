using System;
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
    class IndexECPluginUSGSTests
        {
        IUSGSDataFetcher m_usgsDataFetcherMock;
        IECSchema m_schema;
        IInstanceCacheManager m_instanceCacheManager;
        MockRepository m_mock;
        JObject m_scienceBaseJson;
        XmlDocument m_doc;
        USGSRequestBundle m_testBundle;
        //ECQuery m_query;

        [SetUp]
        public void SetUp ()
            {
            m_mock = new MockRepository();

            CreateFetcherMock();

            m_instanceCacheManager = (IInstanceCacheManager) m_mock.StrictMock(typeof(IInstanceCacheManager));

            m_schema = SetupHelpers.PrepareSchema();
            }

        private RepositoryConnection CreateRepositoryConnection (ECSession session, RepositoryConnectionService connectionService)
            {
            RepositoryIdentifier repositoryIdentifier = new RepositoryIdentifier("IndexECPlugin", "Test", "", "", null);

            return connectionService.Open(session, repositoryIdentifier, null, null);
            }

        private IECSchema GetFirstSchema (RepositoryConnection repositoryConnection, PersistenceService persistenceService)
            {
            IList<IECSchema> listOfSchema = persistenceService.GetConnectionSchemas(repositoryConnection);

            IECSchema schema = listOfSchema[0];
            return schema;
            }

        private void CreateBasicInstanceCacheManager()
            {
            //MockRepository mock = new MockRepository();
            m_instanceCacheManager = (IInstanceCacheManager) m_mock.StrictMock(typeof(IInstanceCacheManager));

            //using ( mock.Record() )
                //{
                //Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Return(instanceList);
                //SetupResult.For(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).IgnoreArguments().Return(instanceList);
                //SetupResult.For(m_instanceCacheManager.QuerySpatialInstancesFromCache(null, null, null, null, null)).IgnoreArguments().Return(instanceList);
                //Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Return(instanceList);
                //Expect.Call(delegate
                    //{
                        //m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                        //LastCall.On(m_instanceCacheManager).Repeat.Any();
                    //});
                //SetupResult.For(delegate
                //    {
                //        m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                //    });
                //}
            }

        private IECInstance CreateParentSEWDV()
            {
            IECInstance instance = m_schema.GetClass("SpatialEntityWithDetailsView").CreateInstance();
            instance.InstanceId = "543e6b86e4b0fd76af69cf4c";

            instance["Id"].StringValue = "543e6b86e4b0fd76af69cf4c";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "4f552e93e4b018de15819c51");
            instance.ExtendedDataValueSetter.Add("Complete", true);

            return instance;
            }

        private IECInstance CreateParentSED ()
            {
            IECInstance instance = m_schema.GetClass("SpatialEntityDataset").CreateInstance();
            instance.InstanceId = "543e6b86e4b0fd76af69cf4c";

            instance["Id"].StringValue = "543e6b86e4b0fd76af69cf4c";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "4f552e93e4b018de15819c51");
            instance.ExtendedDataValueSetter.Add("Complete", true);

            return instance;
            }

        private JObject GetParentScienceBaseJson()
            {
            var scienceBaseJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonParent.txt"));

            string scienceBaseJsonString = scienceBaseJsonStreamReader.ReadToEnd();

            return JObject.Parse(scienceBaseJsonString);
            }

        private void CreateFetcherMock ()
            {

            m_usgsDataFetcherMock = (IUSGSDataFetcher) m_mock.StrictMock(typeof(IUSGSDataFetcher));

            List<UsgsAPICategory> categoryTable = new List<UsgsAPICategory> 
            { 
                new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1 meter DEM", Format = "IMG", Priority = 1, Classification = "Terrain", SbDatasetTag = "Digital Elevation Model (DEM) 1 meter"}
            };

            SetupResult.For(m_usgsDataFetcherMock.CategoryTable).Return(categoryTable);

            List<string> blacklist = new List<string>
            {
                "581d0533e4b08da350d5250b"
            };

            SetupResult.For(m_usgsDataFetcherMock.Blacklist).Return(blacklist);

            var jObject = JObject.Parse(@"{""items"":[{""title"":""USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015"",""sourceId"":""553690bfe4b0b22a15807df2"",""sourceName"":""ScienceBase"",""sourceOriginId"":""7085222"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/553690bfe4b0b22a15807df2"",""publicationDate"":""2015-03-19"",""lastUpdated"":""2016-04-02"",""dateCreated"":""2015-04-21"",""sizeInBytes"":262239745,""extent"":""10000 x 10000 meter"",""format"":""IMG"",""downloadURL"":""https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip"",""previewGraphicURL"":""https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg"",""urls"":{""downloadURL"":""https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip"",""previewGraphicURL"":""https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg"",""metaURL"":""https://www.sciencebase.gov/catalog/item/553690bfe4b0b22a15807df2""},""datasets"":[""Digital Elevation Model (DEM) 1 meter""],""boundingBox"":{""minX"":-90.1111928012935,""maxX"":-89.9874229095346,""minY"":41.32950370684,""maxY"":41.4227313251356},""bestFitIndex"":1.0,""prettyFileSize"":""250.09 MB""}]}");

            m_testBundle = new USGSRequestBundle()
            {
                jtokenList = jObject["items"], Classification = "Terrain", Dataset = "Digital Elevation Model (DEM) 1 meter", DatasetId = "543e6b86e4b0fd76af69cf4c"
            };

            using ( StreamReader scienceBaseJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED1m.txt")) )
                {
                string scienceBaseJsonString = scienceBaseJsonStreamReader.ReadToEnd();
                m_scienceBaseJson = JObject.Parse(scienceBaseJsonString);
                }

            m_doc = new XmlDocument();
            using ( StreamReader metadataFileStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_meta.xml")) )
                {
                m_doc.LoadXml(metadataFileStreamReader.ReadToEnd());
                }

            //using ( mock.Record() )
            //    {
                //Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Return(new List<USGSRequestBundle> { m_testBundle });
                //Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Return(m_scienceBaseJson);
                //Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Return(m_doc);
                //}


            }



        [Test]
        public void SpatialEntityWithDetailsViewTest ()
            {

            //This instance's purpose is to verify that instances that are cached but not returned by the fetcher are returned in the results.
            var instanceToAdd = SetupHelpers.CreateUsgsSEWDV(false, m_schema);
            instanceToAdd.InstanceId = "553690bfe4b0b22a15807aaa";
            instanceToAdd["Id"].StringValue = "553690bfe4b0b22a15807aaa";

            //This instance's purpose is to verify that instances that are cached and also returned by the fetcher are not duplicated in the results.
            var duplicateOfFetcherInstance = SetupHelpers.CreateUsgsSEWDV(false, m_schema);

            List<IECInstance> cachedInstanceList = new List<IECInstance>() { instanceToAdd, duplicateOfFetcherInstance };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Once().Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Never();
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Times(3);
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {

                IECClass sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                ECQuery query = new ECQuery(sewdvClass);
                query.SelectClause.SelectAllProperties = true;
                query.WhereClause = new WhereCriteria(new PropertyExpression(RelationalOperator.EQ, sewdvClass.Properties(true).First(p => p.Name == "Classification"), "Terrain"));
                //Except classification and date, the PropertyExpression Criteria should be ignored.
                query.WhereClause.Add(new PropertyExpression(RelationalOperator.EQ, sewdvClass.Properties(true).First(p => p.Name == "Name"), "TestName"));

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                var instanceFromFetcher = instanceList.First(inst => inst.InstanceId != "553690bfe4b0b22a15807aaa");

                //m_usgsDataFetcherMock.AssertWasCalled(x => x.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything));
                Assert.AreEqual(2, instanceList.Count(), "There should be only two instance.");
                Assert.AreEqual("553690bfe4b0b22a15807df2", instanceFromFetcher.GetPropertyValue("Id").StringValue);
                Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", instanceList.First().GetPropertyValue("Name").StringValue);
                Assert.AreEqual(true, (instanceFromFetcher.GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }")) ||
                                      (instanceFromFetcher.GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684]], \"coordinate_system\" : \"4326\" }")) ||
                                      (instanceFromFetcher.GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356]], \"coordinate_system\" : \"4326\" }")) ||
                                      (instanceFromFetcher.GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356]], \"coordinate_system\" : \"4326\" }")), "The content of the instance was not set properly.");
                Assert.AreEqual("https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg", instanceList.First().GetPropertyValue("ThumbnailURL").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https://www.sciencebase.gov/catalog/item/553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("MetadataURL").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https://www.sciencebase.gov/catalog/item/download/553690bfe4b0b22a15807df2?format=fgdc", instanceList.First().GetPropertyValue("RawMetadataURL").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("FGDC", instanceFromFetcher.GetPropertyValue("RawMetadataFormat").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("USGS", instanceFromFetcher.GetPropertyValue("SubAPI").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("USGS", instanceFromFetcher.GetPropertyValue("DataProvider").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("United States Geological Survey", instanceFromFetcher.GetPropertyValue("DataProviderName").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("2015-03-19T00:00:00", instanceFromFetcher.GetPropertyValue("Date").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("1.0x1.0", instanceFromFetcher.GetPropertyValue("ResolutionInMeters").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Terrain", instanceFromFetcher.GetPropertyValue("Classification").StringValue, "The content of the instance was not set properly.");
                Assert.IsFalse(instanceFromFetcher.ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                Assert.IsTrue(instanceList.Any(inst => inst.InstanceId == "553690bfe4b0b22a15807aaa"));
                }
            }

        [Test]
        public void SpatialEntityWithDetailsViewDateFilterTest ()
            {
            List<IECInstance> cachedInstanceList = new List<IECInstance>() { };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Once().Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Never();
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Times(3);
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                IECClass sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                ECQuery query = new ECQuery(sewdvClass);
                query.SelectClause.SelectAllProperties = true;
                query.WhereClause = new WhereCriteria(new PropertyExpression(RelationalOperator.EQ, sewdvClass.Properties(true).First(p => p.Name == "Classification"), "Terrain"));
                query.WhereClause.Add(new PropertyExpression(RelationalOperator.LTEQ, sewdvClass.Properties(true).First(p => p.Name == "Date"), "2015-03-20T00:00:00"));
                query.WhereClause.Add(new PropertyExpression(RelationalOperator.GTEQ, sewdvClass.Properties(true).First(p => p.Name == "Date"), "2015-03-18T00:00:00"));
                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                //m_usgsDataFetcherMock.AssertWasCalled(x => x.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything));
                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                var instance = instanceList.First();
                Assert.AreEqual("553690bfe4b0b22a15807df2", instance.GetPropertyValue("Id").StringValue);
                Assert.AreEqual("2015-03-19T00:00:00", instance.GetPropertyValue("Date").StringValue, "The content of the instance was not set properly.");
                }
            }

        [Test]
        public void SpatialEntityWithDetailsViewDateFilterEQTest ()
            {
            List<IECInstance> cachedInstanceList = new List<IECInstance>()
            {
            };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Once().Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Never();
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                IECClass sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                ECQuery query = new ECQuery(sewdvClass);
                query.SelectClause.SelectAllProperties = true;
                query.WhereClause = new WhereCriteria(new PropertyExpression(RelationalOperator.EQ, sewdvClass.Properties(true).First(p => p.Name == "Classification"), "Terrain"));
                query.WhereClause.Add(new PropertyExpression(RelationalOperator.EQ, sewdvClass.Properties(true).First(p => p.Name == "Date"), "2015-03-20T00:00:00"));
                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                //m_usgsDataFetcherMock.AssertWasCalled(x => x.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything));
                Assert.AreEqual(0, instanceList.Count(), "There should be no instance.");
                }
            }

        [Test]
        public void SEWDVUsgsFailureTest ()
            {
            List<IECInstance> cachedInstanceList = new List<IECInstance>(){SetupHelpers.CreateUsgsSEWDV(false, m_schema)};

            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Once().Throw(new OperationFailedException());//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Never();//Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Never();//Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }

            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityWithDetailsView"));
                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.IsTrue(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should come from the cache.");
                }
            }

        [Test]
        public void SingleSEWDVTest ()
            {
            //CreateBasicInstanceCacheManager(null);
            List<IECInstance> cachedInstanceList = new List<IECInstance>();

            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Times(3);
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }

            using ( m_mock.Playback() )
                {

                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityWithDetailsView"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();
                var firstInst = instanceList.First();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.AreEqual("553690bfe4b0b22a15807df2", firstInst.GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, (firstInst.GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]],\"coordinate_system\":\"4326\"}")) ||
                                      (firstInst.GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684]],\"coordinate_system\":\"4326\"}")) ||
                                      (firstInst.GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356]],\"coordinate_system\":\"4326\"}")) ||
                                      (firstInst.GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356]],\"coordinate_system\":\"4326\"}")), "The content of the instance was not set properly.");
                Assert.AreEqual(false, (firstInst.GetPropertyValue("ApproximateFootprint").NativeValue));
                Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", firstInst.GetPropertyValue("Name").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey", firstInst.GetPropertyValue("Legal").StringValue);
                Assert.AreEqual("https://www2.usgs.gov/laws/info_policies.html", firstInst.GetPropertyValue("TermsOfUse").StringValue);
                Assert.AreEqual("https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg", firstInst.GetPropertyValue("ThumbnailURL").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("IMG", firstInst.GetPropertyValue("DataSourceType").StringValue, "The content of the instance was not set properly.");
                //Assert.AreEqual("1.0m", instanceList.First().GetPropertyValue("AccuracyResolutionDensity").StringValue);
                Assert.AreEqual("USGS", firstInst.GetPropertyValue("DataProvider").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("United States Geological Survey", firstInst.GetPropertyValue("DataProviderName").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("Digital Elevation Model (DEM) 1 meter", firstInst.GetPropertyValue("Dataset").StringValue, "The content of the instance was not set properly.");
                //Assert.AreEqual("2015-03-19T00:00:00", instanceList.First().GetPropertyValue("Date").StringValue);
                Assert.IsTrue(firstInst.GetPropertyValue("Classification").IsNull, "The content of the instance was not set properly.");
                Assert.IsFalse(firstInst.ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void SpatialEntityTest ()
            {
            //CreateBasicInstanceCacheManager(null);
            List<IECInstance> cachedInstanceList = new List<IECInstance>();

            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Once();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }

            using ( m_mock.Playback() )
                {

                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]],\"coordinate_system\":\"4326\"}")) ||
                                      (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684]],\"coordinate_system\":\"4326\"}")) ||
                                      (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356]],\"coordinate_system\":\"4326\"}")) ||
                                      (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356]],\"coordinate_system\":\"4326\"}")), "The content of the instance was not set properly.");
                Assert.AreEqual(false, (instanceList.First().GetPropertyValue("ApproximateFootprint").NativeValue));
                Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", instanceList.First().GetPropertyValue("Name").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg", instanceList.First().GetPropertyValue("ThumbnailURL").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("IMG", instanceList.First().GetPropertyValue("DataSourceTypesAvailable").StringValue, "The content of the instance was not set properly.");
                //Assert.AreEqual("1.0m", instanceList.First().GetPropertyValue("AccuracyResolutionDensity").StringValue);
                Assert.AreEqual("USGS", instanceList.First().GetPropertyValue("DataProvider").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("United States Geological Survey", instanceList.First().GetPropertyValue("DataProviderName").StringValue, "The content of the instance was not set properly.");
                //Assert.AreEqual("2015-03-19T00:00:00", instanceList.First().GetPropertyValue("Date").StringValue);
                Assert.AreEqual("Terrain", instanceList.First().GetPropertyValue("Classification").StringValue, "The content of the instance was not set properly.");
                Assert.IsFalse(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void SpatialEntityAndRelatedMetadataTest ()
            {
            //CreateBasicInstanceCacheManager(null);
            List<IECInstance> cachedInstanceList = new List<IECInstance>();

            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Twice().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Twice();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }

            using ( m_mock.Playback() )
                {

                IECRelationshipClass metadataRelClass = m_schema.GetClass("SpatialEntityToMetadata") as IECRelationshipClass;
                IECClass metadataClass = m_schema.GetClass("Metadata");
                RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), false);


                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
                query.SelectClause.SelectAllProperties = true;
                query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);
                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");

                var spatialEntity = instanceList.First();

                Assert.AreEqual(1, spatialEntity.GetRelationshipInstances().Count(), "There should be only one related instance.");

                var firstMetadataRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition == metadataRelClass);
                var metadataEntity = firstMetadataRel.Target;

                Assert.AreEqual("553690bfe4b0b22a15807df2", spatialEntity.GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, (spatialEntity.GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]],\"coordinate_system\":\"4326\"}")) ||
                                      (spatialEntity.GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684]],\"coordinate_system\":\"4326\"}")) ||
                                      (spatialEntity.GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356]],\"coordinate_system\":\"4326\"}")) ||
                                      (spatialEntity.GetPropertyValue("Footprint").StringValue.Contains("{\"points\":[[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356]],\"coordinate_system\":\"4326\"}")), "The content of the instance was not set properly.");
                Assert.AreEqual(false, (spatialEntity.GetPropertyValue("ApproximateFootprint").NativeValue));
                Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", spatialEntity.GetPropertyValue("Name").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg", spatialEntity.GetPropertyValue("ThumbnailURL").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("IMG", spatialEntity.GetPropertyValue("DataSourceTypesAvailable").StringValue, "The content of the instance was not set properly.");
                //Assert.AreEqual("1.0m", spatialEntity.GetPropertyValue("AccuracyResolutionDensity").StringValue);
                Assert.AreEqual("USGS", spatialEntity.GetPropertyValue("DataProvider").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("United States Geological Survey", spatialEntity.GetPropertyValue("DataProviderName").StringValue, "The content of the instance was not set properly.");
                //Assert.AreEqual("2015-03-19T00:00:00", spatialEntity.GetPropertyValue("Date").StringValue);
                Assert.AreEqual("Terrain", spatialEntity.GetPropertyValue("Classification").StringValue, "The content of the instance was not set properly.");
                Assert.IsFalse(spatialEntity.ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");

                Assert.AreEqual("553690bfe4b0b22a15807df2", metadataEntity.GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, metadataEntity.GetPropertyValue("DisplayStyle").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("This is a tile of the National Elevation Dataset (NED) and is one meter resolution. Data in this layer represent a bare earth surface. The National Elevation Dataset (NED) serves as the elevation layer of The National Map, and provides basic elevation information for earth science studies and mapping applications in the United States. Scientists and resource managers use NED data for global change research, hydrologic modeling, resource monitoring, mapping and visualization, and many other applications. The NED is an elevation dataset that consists of seamless layers and non-seamless high resolution layers. Each of the seamless layers are composed of the best available raster elevation data of the conterminous United States, Alaska, [...]", metadataEntity.GetPropertyValue("Description").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, metadataEntity.GetPropertyValue("ContactInformation").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("elevation, Elevation, National Elevation Dataset, NED, Elevation, Light Detection and Ranging, LIDAR, High Resolution, Topographic Surface, Topography, Bare Earth, Hydro-Flattened, Terrain Elevation, Cartography, DEM, Digital Elevation Model, Digital Mapping, Digital Terrain Model, Geodata, GIS, Mapping, Raster, USGS, U.S. Geological Survey, 10,000 meter DEM, 1 meter DEM, Downloadable Data, Elevation, Digital Elevation Model (DEM) 1 meter, 10000 x 10000 meter, IMG, US, United States", metadataEntity.GetPropertyValue("Keywords").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey", metadataEntity.GetPropertyValue("Legal").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, metadataEntity.GetPropertyValue("Lineage").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, metadataEntity.GetPropertyValue("Provenance").IsNull, "The content of the instance was not set properly.");
                Assert.IsFalse(metadataEntity.ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void SpatialEntityAndRelatedMetadataTestFromCache ()
            {
            //CreateBasicInstanceCacheManager(null);
            List<IECInstance> cachedInstanceList = new List<IECInstance>() { SetupHelpers.CreateUsgsSE(true, m_schema) };
            List<IECInstance> cachedInstanceList2 = new List<IECInstance>() { SetupHelpers.CreateUsgsMetadata(true, m_schema)};

            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList2);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }

            using ( m_mock.Playback() )
                {

                IECRelationshipClass metadataRelClass = m_schema.GetClass("SpatialEntityToMetadata") as IECRelationshipClass;
                IECClass metadataClass = m_schema.GetClass("Metadata");
                RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), false);


                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
                query.SelectClause.SelectAllProperties = true;
                query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);
                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");

                var spatialEntity = instanceList.First();

                Assert.AreEqual(1, spatialEntity.GetRelationshipInstances().Count(), "There should be only one related instance.");

                var firstMetadataRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition == metadataRelClass);
                var metadataEntity = firstMetadataRel.Target;

                Assert.IsTrue(spatialEntity.ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should come from the cache.");

                Assert.IsTrue(metadataEntity.ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should come from the cache.");
                }
            }

        [Test]
        public void MetadataTest ()
            {

            List<IECInstance> cachedInstanceList = new List<IECInstance>();
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Once();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("Metadata"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("DisplayStyle").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("This is a tile of the National Elevation Dataset (NED) and is one meter resolution. Data in this layer represent a bare earth surface. The National Elevation Dataset (NED) serves as the elevation layer of The National Map, and provides basic elevation information for earth science studies and mapping applications in the United States. Scientists and resource managers use NED data for global change research, hydrologic modeling, resource monitoring, mapping and visualization, and many other applications. The NED is an elevation dataset that consists of seamless layers and non-seamless high resolution layers. Each of the seamless layers are composed of the best available raster elevation data of the conterminous United States, Alaska, [...]", instanceList.First().GetPropertyValue("Description").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("ContactInformation").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("elevation, Elevation, National Elevation Dataset, NED, Elevation, Light Detection and Ranging, LIDAR, High Resolution, Topographic Surface, Topography, Bare Earth, Hydro-Flattened, Terrain Elevation, Cartography, DEM, Digital Elevation Model, Digital Mapping, Digital Terrain Model, Geodata, GIS, Mapping, Raster, USGS, U.S. Geological Survey, 10,000 meter DEM, 1 meter DEM, Downloadable Data, Elevation, Digital Elevation Model (DEM) 1 meter, 10000 x 10000 meter, IMG, US, United States", instanceList.First().GetPropertyValue("Keywords").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey", instanceList.First().GetPropertyValue("Legal").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("Lineage").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("Provenance").IsNull, "The content of the instance was not set properly.");
                Assert.IsFalse(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void SpatialDataSourceTest ()
            {

            List<IECInstance> cachedInstanceList = new List<IECInstance>();
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Once();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialDataSource"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip", instanceList.First().GetPropertyValue("MainURL").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("USGS", instanceList.First().GetPropertyValue("CompoundType").StringValue, "The content of the instance was not set properly.");
                Assert.IsTrue(instanceList.First().GetPropertyValue("LocationInCompound").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("IMG", instanceList.First().GetPropertyValue("DataSourceType").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("SisterFiles").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("256093", instanceList.First().GetPropertyValue("FileSize").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https://www.sciencebase.gov/catalog/file/get/581d23cee4b08da350d5761d?f=__disk__97%2F4a%2Fd4%2F974ad4dfcb40c7ba11cc11ab940be62b48105195", instanceList.First().GetPropertyValue("Metadata").StringValue, "The content of the instance was not set properly.");
                Assert.IsFalse(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void ServerTest ()
            {

            List<IECInstance> cachedInstanceList = new List<IECInstance>();
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Once().Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Once();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("Server"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https", instanceList.First().GetPropertyValue("CommunicationProtocol").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("https://prd-tnm.s3.amazonaws.com", instanceList.First().GetPropertyValue("URL").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("Name").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("ServerContactInformation").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual("None. No fees are applicable for obtaining the data set.", instanceList.First().GetPropertyValue("Fees").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey", instanceList.First().GetPropertyValue("Legal").StringValue, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("AccessConstraints").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("Online").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("LastCheck").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("LastTimeOnline").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("Latency").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("MeanReachabilityStats").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("State").IsNull, "The content of the instance was not set properly.");
                Assert.AreEqual(true, instanceList.First().GetPropertyValue("Type").IsNull, "The content of the instance was not set properly.");
                Assert.IsFalse(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void SpatialEntityCompleteCacheTest ()
            {

            List<IECInstance> cachedInstanceList = new List<IECInstance>(){SetupHelpers.CreateUsgsSE(true, m_schema)};
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();


                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.IsTrue(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should come from the cache.");
                }
            }

        [Test]
        public void SpatialDataSourceCompleteCacheTest ()
            {

            List<IECInstance> cachedInstanceList = new List<IECInstance>() { SetupHelpers.CreateUsgsSDS(true, m_schema) };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialDataSource"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.IsTrue(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should come from the cache.");
                }
            }

        [Test]
        public void MetadataCompleteCacheTest ()
            {

            List<IECInstance> cachedInstanceList = new List<IECInstance>() { SetupHelpers.CreateUsgsMetadata(true, m_schema) };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("Metadata"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.IsTrue(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should come from the cache.");
                }
            }

        [Test]
        public void ServerCompleteCacheTest ()
            {
            List<IECInstance> cachedInstanceList = new List<IECInstance>() { SetupHelpers.CreateUsgsServer(true, m_schema) };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("Server"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.IsTrue(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should come from the cache.");
                }
            }

        [Test]
        public void SpatialEntityIncompleteCacheTest ()
            {
            List<IECInstance> cachedInstanceList = new List<IECInstance>() { SetupHelpers.CreateUsgsSE(false, m_schema) };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Once();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.IsFalse(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void SpatialDataSourceIncompleteCacheTest ()
            {

            List<IECInstance> cachedInstanceList = new List<IECInstance>() { SetupHelpers.CreateUsgsSDS(false, m_schema) };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Once();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialDataSource"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.IsFalse(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void MetadataIncompleteCacheTest ()
            {
            List<IECInstance> cachedInstanceList = new List<IECInstance>() { SetupHelpers.CreateUsgsMetadata(false, m_schema) };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Once();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("Metadata"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.IsFalse(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void ServerIncompleteCacheTest ()
            {

            List<IECInstance> cachedInstanceList = new List<IECInstance>() { SetupHelpers.CreateUsgsServer(false, m_schema) };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Return(m_scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Once().Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Once();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("Server"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.IsFalse(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should not come from the cache.");
                }
            }

        [Test]
        public void IncompleteCacheUSGSFailureTest ()
            {
            List<IECInstance> cachedInstanceList = new List<IECInstance>() { SetupHelpers.CreateUsgsSE(false, m_schema) };
            using ( m_mock.Record() )
                {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();//.Return(new List<USGSRequestBundle> { m_testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Repeat.Once().Throw(new OperationFailedException());
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Repeat.Never();//.Return(m_doc);

                Expect.Call(m_instanceCacheManager.QueryInstancesFromCache(Arg<IEnumerable<string>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything)).Repeat.Once().Return(cachedInstanceList);
                Expect.Call(m_instanceCacheManager.QuerySpatialInstancesFromCache(Arg<PolygonDescriptor>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<SelectCriteria>.Is.Anything, Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.PrepareCacheInsertStatement(Arg<IEnumerable<IECInstance>>.Is.Anything, Arg<IECClass>.Is.Anything, Arg<IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>>>.Is.Anything);
                }).Repeat.Never();
                Expect.Call(delegate
                {
                    m_instanceCacheManager.SendAllPreparedCacheInsertStatements();
                }).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
                query.SelectClause.SelectAllProperties = true;

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

                UsgsSubAPIQueryProvider usgsAPIQueryProvider = new UsgsSubAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock, m_instanceCacheManager, m_schema);

                var instanceList = usgsAPIQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, instanceList.Count(), "There should be only one instance.");
                Assert.IsTrue(instanceList.First().ExtendedData.ContainsKey("IsFromCacheTest"), "The instance should come from the cache.");
                }
            }
        }
    }
