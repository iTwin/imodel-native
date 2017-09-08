using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Json;
using Bentley.ECObjects.Schema;
using Bentley.ECSystem.Configuration;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Source.QueryProviders;
using IndexECPlugin.Tests.Common;
using Newtonsoft.Json.Linq;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class UsgsEEApiQueryProviderTests
        {

        IECSchema m_schema;
        MockRepository m_mock;
        IUSGSEEDataFetcher m_fetcher;
        List<EERequestBundle> m_eeRequestBundlelist;
        List<UsgsEEDataset> m_eeDatasetlist;
        string m_xmlDocString;

        [SetUp]
        public void Setup()
            {
            m_mock = new MockRepository();
            m_schema = SetupHelpers.PrepareSchema();

            CreateFetcher();
            }

        private void CreateFetcher ()
            {
            m_fetcher = m_mock.StrictMock<IUSGSEEDataFetcher>();

            UsgsEEDataset eeDataset = new UsgsEEDataset()
            {
                DatasetName = "HIGH_RES_ORTHO", DatasetId = "3411", DataFormat = "GeoTIFF", Classification = "Imagery"
            };

            string jsonQueryResponseString;
            using ( StreamReader jsonQueryResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerSpatialQueryResponse.txt")) )
                {
                jsonQueryResponseString = jsonQueryResponseStreamReader.ReadToEnd();
                }

            var jobject = JObject.Parse(jsonQueryResponseString);

            m_eeDatasetlist = new List<UsgsEEDataset>() { eeDataset };
            m_eeRequestBundlelist = new List<EERequestBundle>() { new EERequestBundle() { Dataset = eeDataset, jtokenList = jobject["data"]["results"] } };

            using ( StreamReader xmlResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerXmlDocResponse.txt")) )
                {
                m_xmlDocString = xmlResponseStreamReader.ReadToEnd();
                }
            }

        private void SetErroneousDatasetList()
            {
            SetupResult.For(m_fetcher.DatasetList).Return(new List<UsgsEEDataset>());
            }

        [Test]
        public void CreateSETest()
            {
            
            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(m_xmlDocString);

            using(m_mock.Record())
                {
                SetupResult.For(m_fetcher.DatasetList).Return(m_eeDatasetlist);
                Expect.Call(m_fetcher.GetXmlDocForInstance(Arg<string>.Is.Equal("1160860_L5C2"), Arg<string>.Is.Equal("3411"))).Return(xmlDoc).Repeat.Once();
                }
            using(m_mock.Playback())
                {
                var resultList = usgsEEApiQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, resultList.Count(), "There was not exactly one result.");

                var instance = resultList.First();

                Assert.AreEqual(instance.InstanceId, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["Id"].StringValue, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["Name"].StringValue, "946127_A9");
                Assert.AreEqual(instance["Footprint"].StringValue, DbGeometryHelpers.CreateFootprintString("-84.355558576002977", "39.775409975403505", "-84.334504883972897", "39.786725461382638", 4326), "The Footprint was not set correctly.");
                Assert.AreEqual(instance["DataSourceTypesAvailable"].StringValue, m_eeRequestBundlelist.First().Dataset.DataFormat, "The dataSourceType was not set correctly.");

                double meterRes = 2*0.3048;
                string resInMetersString = meterRes.ToString("0.####");
                Assert.AreEqual(instance["ResolutionInMeters"].StringValue, resInMetersString + "x" + resInMetersString);
                Assert.AreEqual(instance["ThumbnailURL"].StringValue, "https://earthexplorer.usgs.gov/browse/hro/OH/2000/200003_dayton_oh_2ft_sp_clr/vol001/a-9.jpg", "The Thumbnail was not set correctly.");
                Assert.AreEqual(instance["DataProvider"].StringValue, IndexConstants.UsgsDataProviderString, "The DataProvider was not set correctly.");
                Assert.AreEqual(instance["DataProviderName"].StringValue, IndexConstants.UsgsDataProviderNameString, "The DataProviderName was not set correctly.");
                Assert.AreEqual(instance["Dataset"].StringValue, m_eeRequestBundlelist.First().Dataset.DatasetName, "The Dataset was not set correctly.");
                Assert.AreEqual(instance["Date"].NativeValue, new DateTime(2000, 3, 1), "The Date was not set correctly.");
                Assert.AreEqual(instance["Classification"].StringValue, m_eeRequestBundlelist.First().Dataset.Classification, "The Classification was not set correctly.");
                }
            }

        [Test]
        public void CreateSEWDVTest ()
            {

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityWithDetailsView"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(m_xmlDocString);

            using ( m_mock.Record() )
                {
                SetupResult.For(m_fetcher.DatasetList).Return(m_eeDatasetlist);
                Expect.Call(m_fetcher.GetXmlDocForInstance(Arg<string>.Is.Equal("1160860_L5C2"), Arg<string>.Is.Equal("3411"))).Return(xmlDoc).Repeat.Once();
                }
            using ( m_mock.Playback() )
                {
                var resultList = usgsEEApiQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, resultList.Count(), "There was not exactly one result.");

                var instance = resultList.First();

                Assert.AreEqual(instance.InstanceId, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["Id"].StringValue, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["Footprint"].StringValue, DbGeometryHelpers.CreateFootprintString("-84.355558576002977", "39.775409975403505", "-84.334504883972897", "39.786725461382638", 4326), "The Footprint was not set correctly.");

                Assert.AreEqual(instance["Name"].StringValue, "946127_A9");
                Assert.AreEqual(instance["Legal"].StringValue, IndexConstants.EELegalString, "The Legal was not set correctly.");
                Assert.AreEqual(instance["TermsOfUse"].StringValue, IndexConstants.EETermsOfUse, "The TermsOfUse were not set correctly.");
                Assert.AreEqual(instance["DataSourceType"].StringValue, m_eeRequestBundlelist.First().Dataset.DataFormat, "The dataSourceType was not set correctly.");
                Assert.AreEqual(instance["Date"].NativeValue, new DateTime(2000, 3, 1), "The Date was not set correctly.");
                Assert.AreEqual(instance["Classification"].StringValue, m_eeRequestBundlelist.First().Dataset.Classification, "The Classification was not set correctly.");
                Assert.AreEqual(instance["Streamed"].NativeValue, false, "The Streamed property was not set correctly.");
                Assert.AreEqual(instance["SpatialDataSourceId"].StringValue, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");

                double meterRes = 2 * 0.3048;
                string resInMetersString = meterRes.ToString("0.####");
                Assert.AreEqual(instance["ResolutionInMeters"].StringValue, resInMetersString + "x" + resInMetersString);
                Assert.AreEqual(instance["ThumbnailURL"].StringValue, "https://earthexplorer.usgs.gov/browse/hro/OH/2000/200003_dayton_oh_2ft_sp_clr/vol001/a-9.jpg", "The Thumbnail was not set correctly.");
                Assert.AreEqual(instance["DataProvider"].StringValue, IndexConstants.UsgsDataProviderString, "The DataProvider was not set correctly.");
                Assert.AreEqual(instance["DataProviderName"].StringValue, IndexConstants.UsgsDataProviderNameString, "The DataProviderName was not set correctly.");
                Assert.AreEqual(instance["Dataset"].StringValue, m_eeRequestBundlelist.First().Dataset.DatasetName, "The Dataset was not set correctly.");

                Assert.AreEqual(instance["MetadataURL"].StringValue, "https://earthexplorer.usgs.gov/fgdc/3411/1160860_L5C2/save_xml", "The MetadataURL was not set correctly.");
                Assert.AreEqual(instance["SubAPI"].StringValue, IndexConstants.EESubAPIString, "The SubAPI was not set correctly.");
                }
            }

        [Test]
        public void CreateSETest2 ()
            {

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            XmlDocument xmlDoc = new XmlDocument();

            xmlDoc.LoadXml(m_xmlDocString.Replace("FEET", "METER"));

            using ( m_mock.Record() )
                {
                SetupResult.For(m_fetcher.DatasetList).Return(m_eeDatasetlist);
                Expect.Call(m_fetcher.GetXmlDocForInstance(Arg<string>.Is.Equal("1160860_L5C2"), Arg<string>.Is.Equal("3411"))).Return(xmlDoc).Repeat.Once();
                }
            using ( m_mock.Playback() )
                {
                var resultList = usgsEEApiQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, resultList.Count(), "There was not exactly one result.");

                var instance = resultList.First();

                Assert.AreEqual(instance["ResolutionInMeters"].StringValue, "2x2");

                }
            }

        [Test]
        public void CreateSEWDVTest2 ()
            {

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityWithDetailsView"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            XmlDocument xmlDoc = new XmlDocument();

            xmlDoc.LoadXml(m_xmlDocString.Replace("FEET", "METER"));

            using ( m_mock.Record() )
                {
                SetupResult.For(m_fetcher.DatasetList).Return(m_eeDatasetlist);
                Expect.Call(m_fetcher.GetXmlDocForInstance(Arg<string>.Is.Equal("1160860_L5C2"), Arg<string>.Is.Equal("3411"))).Return(xmlDoc).Repeat.Once();
                }
            using ( m_mock.Playback() )
                {
                var resultList = usgsEEApiQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, resultList.Count(), "There was not exactly one result.");

                var instance = resultList.First();

                Assert.AreEqual(instance["ResolutionInMeters"].StringValue, "2x2");

                }
            }

        [Test]
        public void CreateMetadataTest()
            {

            ECQuery query = new ECQuery(m_schema.GetClass("Metadata"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(m_xmlDocString);

            using ( m_mock.Record() )
                {
                SetupResult.For(m_fetcher.DatasetList).Return(m_eeDatasetlist);
                Expect.Call(m_fetcher.GetXmlDocForInstance(Arg<string>.Is.Equal("1160860_L5C2"), Arg<string>.Is.Equal("3411"))).Return(xmlDoc).Repeat.Once();
                }
            using ( m_mock.Playback() )
                {
                var resultList = usgsEEApiQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, resultList.Count(), "There was not exactly one result.");

                var instance = resultList.First();

                Assert.AreEqual(instance.InstanceId, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["Id"].StringValue, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["MetadataURL"].StringValue, "https://earthexplorer.usgs.gov/fgdc/3411/1160860_L5C2/save_xml", "The MetadataURL was not set correctly.");
                Assert.AreEqual(instance["Legal"].StringValue, IndexConstants.EELegalString, "The Legal was not set correctly.");
                Assert.AreEqual(instance["TermsOfUse"].StringValue, IndexConstants.EETermsOfUse, "The TermsOfUse were not set correctly.");

                }
            }

        [Test]
        public void CreateSpatialDataSourceTest ()
            {

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialDataSource"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(m_xmlDocString);

            using ( m_mock.Record() )
                {
                SetupResult.For(m_fetcher.DatasetList).Return(m_eeDatasetlist);
                Expect.Call(m_fetcher.GetXmlDocForInstance(Arg<string>.Is.Equal("1160860_L5C2"), Arg<string>.Is.Equal("3411"))).Return(xmlDoc).Repeat.Once();
                }
            using ( m_mock.Playback() )
                {
                var resultList = usgsEEApiQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, resultList.Count(), "There was not exactly one result.");

                var instance = resultList.First();

                Assert.AreEqual(instance.InstanceId, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["Id"].StringValue, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["MainURL"].StringValue, "https://earthexplorer.usgs.gov/download/3411/1160860_L5C2/STANDARD/EE", "The MainURL was not set correctly.");
                Assert.AreEqual(instance["DataSourceType"].StringValue, m_eeRequestBundlelist.First().Dataset.DataFormat, "The DataSourceType was not set correctly.");

                }
            }

        [Test]
        public void CreateServerTest ()
            {

            ECQuery query = new ECQuery(m_schema.GetClass("Server"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(m_xmlDocString);

            using ( m_mock.Record() )
                {
                SetupResult.For(m_fetcher.DatasetList).Return(m_eeDatasetlist);
                Expect.Call(m_fetcher.GetXmlDocForInstance(Arg<string>.Is.Equal("1160860_L5C2"), Arg<string>.Is.Equal("3411"))).Return(xmlDoc).Repeat.Once();
                }
            using ( m_mock.Playback() )
                {
                var resultList = usgsEEApiQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, resultList.Count(), "There was not exactly one result.");

                var instance = resultList.First();

                Assert.AreEqual(instance.InstanceId, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["Id"].StringValue, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["CommunicationProtocol"].StringValue, "HTTPS", "The CommunicationProtocol was not set correctly.");
                Assert.AreEqual(instance["Streamed"].NativeValue, false, "The Streamed property was not set correctly.");
                Assert.AreEqual(instance["LoginKey"].StringValue, "EarthExplorer", "The LoginKey was not set correctly.");
                Assert.AreEqual(instance["LoginMethod"].StringValue, "CUSTOM", "The LoginMethod was not set correctly.");
                Assert.AreEqual(instance["RegistrationPage"].StringValue, IndexConstants.EERegistrationPage, "The RegistrationPage was not set correctly.");
                Assert.AreEqual(instance["OrganisationPage"].StringValue, IndexConstants.EEOrganisationPage, "The OrganisationPage was not set correctly.");
                Assert.AreEqual(instance["Name"].StringValue, IndexConstants.EEServerName, "The Name was not set correctly.");
                Assert.AreEqual(instance["URL"].StringValue, IndexConstants.EEServerURL, "The URL was not set correctly.");
                Assert.AreEqual(instance["Legal"].StringValue, IndexConstants.EELegalString, "The Legal was not set correctly.");

                }
            }

        [Test]
        public void CreateSEWDVSpatialQueryTest()
            {
            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityWithDetailsView"));
            query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            using ( m_mock.Record() )
                {
                SetupResult.For(m_fetcher.DatasetList).Return(m_eeDatasetlist);
                Expect.Call(m_fetcher.SpatialQuery()).Return(m_eeRequestBundlelist).Repeat.Once();
                }
            using (m_mock.Playback())
                {
                var resultList = usgsEEApiQueryProvider.CreateInstanceList();

                Assert.AreEqual(1, resultList.Count(), "There was not exactly one result.");

                var instance = resultList.First();

                Assert.AreEqual(instance.InstanceId, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["Id"].StringValue, "USGSEE__3411__1160860_L5C2", "The instanceId was not set correctly.");
                Assert.AreEqual(instance["Footprint"].StringValue, DbGeometryHelpers.CreateFootprintString("-73.983410994844", "40.863720236241", "-73.965222965647", "40.877526828118", 4326), "The Footprint was not set correctly.");
                Assert.AreEqual(instance["MetadataURL"].StringValue, String.Format(IndexConstants.EEFgdcMetadataURL, 3411, "1160860_L5C2"), "The MetadataURL was not set correctly");
                Assert.AreEqual(instance["Name"].StringValue, "1160860_L5C2", "The Name was not set correctly.");
                Assert.AreEqual(instance["Description"].StringValue, "Entity ID: 1160860_L5C2, Acquisition Date: 18-FEB-02, State: NJ", "The Description was not set correctly.");
                Assert.AreEqual(instance["Legal"].StringValue, IndexConstants.EELegalString, "The Legal was not set correctly.");
                Assert.AreEqual(instance["TermsOfUse"].StringValue, IndexConstants.EETermsOfUse, "The TermsOfUse were not set correctly.");
                Assert.AreEqual(instance["DataSourceType"].StringValue, m_eeRequestBundlelist.First().Dataset.DataFormat, "The dataSourceType was not set correctly.");
                Assert.AreEqual(instance["Date"].NativeValue, new DateTime(2002, 02, 18), "The Date was not set correctly.");
                Assert.AreEqual(instance["DataSourceType"].StringValue, m_eeRequestBundlelist.First().Dataset.DataFormat, "The dataSourceType was not set correctly.");
                Assert.AreEqual(instance["Classification"].StringValue, m_eeRequestBundlelist.First().Dataset.Classification, "The Classification was not set correctly.");
                Assert.AreEqual(instance["Streamed"].NativeValue, false, "The Streamed property was not set correctly.");
                Assert.AreEqual(instance["ThumbnailURL"].StringValue, "https://earthexplorer.usgs.gov/browse/hro/NJ/2002/200202_new_jersey_state_nj_1ft_sp_cir/vol272/l5c2.jpg", "The ThumbnailURL was not set correctly.");
                Assert.AreEqual(instance["DataProvider"].StringValue, IndexConstants.UsgsDataProviderString, "The DataProvider was not set correctly.");
                Assert.AreEqual(instance["DataProviderName"].StringValue, IndexConstants.UsgsDataProviderNameString, "The DataProviderName was not set correctly.");
                Assert.AreEqual(instance["Dataset"].StringValue, m_eeRequestBundlelist.First().Dataset.DatasetName, "The Dataset was not set correctly.");
                Assert.AreEqual(instance["SubAPI"].StringValue, IndexConstants.EESubAPIString, "The SubAPI was not set correctly.");

                }
            }

        [Test]
        public void CreateSEDatasetError()
            {
            SetErroneousDatasetList();

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            Assert.Throws<UserFriendlyException>(() => usgsEEApiQueryProvider.CreateInstanceList(), "An error should have been returned by CreateInstanceList due to the absent dataset.");
            }

        [Test]
        public void CreateSEWDVDatasetError ()
            {
            SetErroneousDatasetList();

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityWithDetailsView"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            Assert.Throws<UserFriendlyException>(() => usgsEEApiQueryProvider.CreateInstanceList(), "An error should have been returned by CreateInstanceList due to the absent dataset.");
            }

        [Test]
        public void CreateMetadataDatasetError ()
            {
            SetErroneousDatasetList();

            ECQuery query = new ECQuery(m_schema.GetClass("Metadata"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            Assert.Throws<UserFriendlyException>(() => usgsEEApiQueryProvider.CreateInstanceList(), "An error should have been returned by CreateInstanceList due to the absent dataset.");
            }

        [Test]
        public void CreateSpatialDataSourceDatasetError ()
            {
            SetErroneousDatasetList();

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialDataSource"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            Assert.Throws<UserFriendlyException>(() => usgsEEApiQueryProvider.CreateInstanceList(), "An error should have been returned by CreateInstanceList due to the absent dataset.");
            }

        [Test]
        public void CreateServerDatasetError ()
            {
            SetErroneousDatasetList();

            ECQuery query = new ECQuery(m_schema.GetClass("Server"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            Assert.Throws<UserFriendlyException>(() => usgsEEApiQueryProvider.CreateInstanceList(), "An error should have been returned by CreateInstanceList due to the absent dataset.");
            }

        [Test]
        public void CreateSEInstanceIdError ()
            {
            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntity"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("Hello_1160860_L5C2"));

            UsgsEEApiQueryProvider usgsEEApiQueryProvider = new UsgsEEApiQueryProvider(query, null, m_fetcher, null, m_schema);

            var instanceList = usgsEEApiQueryProvider.CreateInstanceList();
            Assert.AreEqual(0, instanceList.Count());
            }
        
        }
    }
