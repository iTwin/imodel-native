using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using IndexECPlugin;
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

namespace IndexECPlugin.Tests
{
    [TestFixture]
    class IndexECPluginUSGSTests
    {
        IUSGSDataFetcher m_usgsDataFetcherMock;
        IECSchema m_schema;
        //ECQuery m_query;

        [SetUp]
        public void SetUp()
        {
            CreateFetcherMock();

            PrepareSchema();

        }

        private void PrepareSchema()
        {
            //m_query = new ECQuery()

            //ECSession session = SessionManager.CreateSession();
            //RepositoryConnectionService connectionService = RepositoryConnectionServiceFactory.GetService();
            //RepositoryConnection connection = CreateRepositoryConnection(session, connectionService);
            //PersistenceService persistenceService = PersistenceServiceFactory.GetService();
            ;

            ECSchemaXmlStreamReader schemaReader = new ECSchemaXmlStreamReader(Assembly.GetAssembly(typeof(IndexECPlugin.Source.IndexECPlugin)).GetManifestResourceStream("ECSchemaDB.xml"));
            m_schema = schemaReader.Deserialize();

            //m_schema = GetFirstSchema(connection, persistenceService);

            //connectionService.Close(connection, null);

        }

        private RepositoryConnection CreateRepositoryConnection(ECSession session, RepositoryConnectionService connectionService)
        {
            RepositoryIdentifier repositoryIdentifier = new RepositoryIdentifier("IndexECPlugin", "Test", "", "", null);

            return connectionService.Open(session, repositoryIdentifier, null, null);
        }

        private IECSchema GetFirstSchema(RepositoryConnection repositoryConnection, PersistenceService persistenceService)
        {
            IList<IECSchema> listOfSchema = persistenceService.GetConnectionSchemas(repositoryConnection);

            IECSchema schema = listOfSchema[0];
            return schema;
        }

        private void CreateFetcherMock()
        {
            MockRepository mock = new MockRepository();

            //var usgsDataFetcherMock = MockRepository.GenerateStub<IUSGSDataFetcher>();

            m_usgsDataFetcherMock = (IUSGSDataFetcher)mock.StrictMock(typeof(IUSGSDataFetcher));

            List<UsgsAPICategory> categoryTable = new List<UsgsAPICategory> 
            { 
                new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1 meter DEM", Format = "IMG", Priority = 1, Classification = "Terrain", SbDatasetTag = "Digital Elevation Model (DEM) 1 meter"}
            };

            SetupResult.For(m_usgsDataFetcherMock.CategoryTable).Return(categoryTable);

            var jObject = JObject.Parse(@"{""items"":[{""title"":""USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015"",""sourceId"":""553690bfe4b0b22a15807df2"",""sourceName"":""ScienceBase"",""sourceOriginId"":""7085222"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/553690bfe4b0b22a15807df2"",""publicationDate"":""2015-03-19"",""lastUpdated"":""2015-04-21"",""dateCreated"":""2015-04-21"",""sizeInBytes"":262239745,""extent"":""10000 x 10000 meter"",""format"":""IMG"",""downloadURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip"",""previewGraphicURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg"",""datasets"":[""Digital Elevation Model (DEM) 1 meter""],""boundingBox"":{""minX"":-90.1111928012935,""maxX"":-89.9874229095346,""minY"":41.32950370684,""maxY"":41.4227313251356},""bestFitIndex"":0.0,""prettyFileSize"":""250.09 MB""}]}");

            USGSRequestBundle testBundle = new USGSRequestBundle() { jtokenList = jObject["items"], Classification = "Terrain", Dataset = "Digital Elevation Model (DEM) 1 meter", DatasetId = "543e6b86e4b0fd76af69cf4c" };

            //usgsDataFetcherMock.Stub(x => x.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything))
            //    //.IgnoreArguments()
            //                   .Return(new List<USGSRequestBundle> { testBundle });

            var scienceBaseJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED.txt"));

            //            string scienceBaseJsonString = @"{""link"":{""rel"":""self"",""url"":""https://www.sciencebase.gov/catalog/item/553690bfe4b0b22a15807df2""},""relatedItems"":{""link"":{""url"":""https://www.sciencebase.gov/catalog/itemLinks?itemId=553690bfe4b0b22a15807df2"",""rel"":""related""}},""id"":""553690bfe4b0b22a15807df2"",""identifiers"":[{""type"":""id"",""scheme"":""gda"",""key"":""7085222""},{""type"":null,""scheme"":""processingUrl"",""key"":""http://thor-f5.er.usgs.gov/ngtoc/metadata/waf/elevation/1_meter/img/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_meta.xml""}],""title"":""USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015"",""summary"":""This is a tile of the National Elevation Dataset (NED) and is one meter resolution. Data in this layer represent a bare earth surface. The National Elevation Dataset (NED) serves as the elevation layer of The National Map, and provides basic elevation information for earth science studies and mapping applications in the United States. Scientists and resource managers use NED data for global change research, hydrologic modeling, resource monitoring, mapping and visualization, and many other applications. The NED is an elevation dataset that consists of seamless layers and non-seamless high resolution layers. Each of the seamless layers are composed of the best available raster elevation data of the conterminous United States, Alaska, [...]"",""body"":""This is a tile of the National Elevation Dataset (NED) and is one meter resolution. Data in this layer represent a bare earth surface. The National Elevation Dataset (NED) serves as the elevation layer of The National Map, and provides basic elevation information for earth science studies and mapping applications in the United States. Scientists and resource managers use NED data for global change research, hydrologic modeling, resource monitoring, mapping and visualization, and many other applications. The NED is an elevation dataset that consists of seamless layers and non-seamless high resolution layers. Each of the seamless layers are composed of the best available raster elevation data of the conterminous United States, Alaska, Hawaii, territorial islands, Mexico and Canada. The NED is updated continually as new data become available. All NED data are in the public domain. The NED is derived from diverse source data that are processed to a common coordinate system and unit of vertical measure. The spatial reference used for tiles of the one meter layer within the conterminous United States (CONUS) is Universal Transverse Mercator (UTM) in units of meters, and in conformance with the North American Datum of 1983 (NAD83). All bare earth elevation values are in meters and are referenced to the North American Vertical Datum of 1988 (NAVD88). Each tile is distributed in the UTM Zone in which it lies. If a tile crosses two UTM Zones, it is delivered in both zones. Only source data of one meter resolution or finer are used to produce the NED standard one meter layer"",
            //                                           ""purpose"":""The NED serves as the elevation layer of The National Map, and provides basic elevation information for earth science studies and mapping applications in the United States. The data are utilized by the scientific and resource management communities for global change research, hydrologic modeling, resource monitoring, mapping and visualization, and many other applications."",""provenance"":{""html"":""Added to Sciencebase on Tue Apr 21 12:02:33 MDT 2015 by processing URL \n<b>USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_meta.xml<\u002fb> in item \n<a href=\""https://www.sciencebase.gov/catalog/item/543e6b86e4b0fd76af69cf4c\"">https://www.sciencebase.gov/catalog/item/543e6b86e4b0fd76af69cf4c<\u002fa>\n<br />"",""dataSource"":""Link Processing"",""linkProcess"":{""itemReference"":""543e6b86e4b0fd76af69cf4c"",""linkReference"":""http://thor-f5.er.usgs.gov/ngtoc/metadata/waf/elevation/1_meter/img/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_meta.xml"",""dateProcessed"":""2015-04-21T18:02:33Z"",""processedBy"":""wgillman@usgs.gov"",""processType"":""WebAccessibleFolder""},""dateCreated"":""2015-04-21T18:02:39Z"",""lastUpdated"":""2015-04-21T18:02:39Z""},""hasChildren"":false,""parentId"":""543e6b86e4b0fd76af69cf4c"",""contacts"":[{""name"":""U.S. Geological Survey"",""type"":""Distributor"",""hours"":""Monday through Friday 8:00 AM to 4:00 PM Eastern Time Zone USA"",""instructions"":""Please visit http://www.usgs.gov/ask/ to contact us."",""primaryLocation"":{""officePhone"":""1-888-ASK-USGS (1-888-275-8747)"",""mailAddress"":{""line1"":""USGS National Geospatial Program Office"",""line2"":""12201 Sunrise Valley Drive"",""city"":""Reston"",""state"":""VA"",""zip"":""20192"",""country"":""USA""}}},{""name"":""U.S. Geological Survey"",""type"":""Metadata Contact"",""hours"":""Monday through Friday 8:00 AM to 4:00 PM Eastern Time Zone USA"",""instructions"":""Please visit http://www.usgs.gov/ask/ to contact us."",""primaryLocation"":{""officePhone"":""1-888-ASK-USGS (1-888-275-8747)"",""mailAddress"":{""line1"":""USGS National Geospatial Program Office"",""line2"":""12201 Sunrise Valley Drive"",""city"":""Reston"",""state"":""VA"",""zip"":""20192"",""country"":""USA""}}},{""name"":""U.S. Geological Survey"",""type"":""Originator""}],""webLinks"":[{""type"":""Online Link"",""uri"":""http://ned.usgs.gov/"",""rel"":""related"",""hidden"":false},{""type"":""Online Link"",""uri"":""http://nationalmap.gov/viewer.html"",""rel"":""related"",""hidden"":false},{""type"":""browseImage"",""uri"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg"",""rel"":""related"",""title"":""Thumbnail JPG image"",""hidden"":false},
            //                                           {""type"":""download"",""uri"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip"",""rel"":""related"",""title"":""IMG"",""hidden"":false,""length"":262239745}],""tags"":[{""type"":""Theme"",""scheme"":""ISO 19115 Topic Category"",""name"":""elevation""},{""type"":""Theme"",""scheme"":""NGDA Portfolio Themes"",""name"":""Elevation""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""National Elevation Dataset""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""NED""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Elevation""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Light Detection and Ranging""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""LIDAR""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""High Resolution""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Topographic Surface""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Topography""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Bare Earth""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Hydro-Flattened""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Terrain Elevation""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Cartography""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""DEM""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Digital Elevation Model""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Digital Mapping""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Digital Terrain Model""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Geodata""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""GIS""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Mapping""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""Raster""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""USGS""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""U.S. Geological Survey""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""10,000 meter DEM""},{""type"":""Theme"",""scheme"":""National Elevation Dataset (NED)"",""name"":""1 meter DEM""},{""type"":""Theme"",""scheme"":""The National Map Type Thesaurus"",
            //                                           ""name"":""Downloadable Data""},{""type"":""Theme"",""scheme"":""The National Map Theme Thesaurus"",""name"":""Elevation""},{""type"":""Theme"",""scheme"":""The National Map Collection Thesaurus"",""name"":""Digital Elevation Model (DEM) 1 meter""},{""type"":""Theme"",""scheme"":""The National Map Product Extent Thesaurus"",""name"":""10000 x 10000 meter""},{""type"":""Theme"",""scheme"":""The National Map Product Format Thesaurus"",""name"":""IMG""},{""type"":""Place"",""scheme"":""Geographic Names Information System"",""name"":""US""},{""type"":""Place"",""scheme"":""Geographic Names Information System"",""name"":""United States""}],""dates"":[{""type"":""Publication"",""dateString"":""2015-03-19"",""label"":""Publication Date""},{""type"":""Start"",""dateString"":""2009-05-12"",""label"":""""},{""type"":""End"",""dateString"":""2009-05-12"",""label"":""""},{""type"":""Info"",""dateString"":""2015-04-21 09:51:00"",""label"":""File Modification Date""}],""spatial"":{""boundingBox"":{""minX"":-90.1111928012935,""maxX"":-89.9874229095346,""minY"":41.32950370684,""maxY"":41.4227313251356}},""files"":[{""name"":""USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_meta.xml"",""title"":null,""contentType"":""application/fgdc+xml"",""contentEncoding"":null,""pathOnDisk"":""__disk__74/bb/91/74bb9105fd15f0129423afefeae7144279edf5ec"",""processed"":null,""processToken"":null,""imageWidth"":null,""imageHeight"":null,""size"":13686,""dateUploaded"":""2015-04-21T18:02:39Z"",""originalMetadata"":true,""useForPreview"":null,""itemS3File"":null,""url"":""https://www.sciencebase.gov/catalog/file/get/553690bfe4b0b22a15807df2?f=__disk__74%2Fbb%2F91%2F74bb9105fd15f0129423afefeae7144279edf5ec""}],""distributionLinks"":[{""uri"":""https://www.sciencebase.gov/catalog/file/get/553690bfe4b0b22a15807df2"",""title"":""Download Attached Files"",""type"":""downloadLink"",""typeLabel"":""Download Link"",""rel"":""alternate"",""name"":""USGSNEDonemeter.zip"",""files"":[{""name"":""USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_meta.xml"",""title"":null,""contentType"":""application/fgdc+xml""}]}],""previewImage"":{""thumbnail"":{""uri"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg"",""title"":""Thumbnail JPG image""},""small"":{""uri"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg"",""title"":""Thumbnail JPG image""},""from"":""webLinks""}}";

            string scienceBaseJsonString = scienceBaseJsonStreamReader.ReadToEnd();

            var scienceBaseJson = JObject.Parse(scienceBaseJsonString);

            //usgsDataFetcherMock.Stub(x => x.GetSciencebaseJson(Arg<String>.Is.Equal("553690bfe4b0b22a15807df2")))
            //usgsDataFetcherMock.Stub(x => x.GetSciencebaseJson(Arg<String>.Is.Anything))
            //    //.IgnoreArguments()
            //                   .Return(scienceBaseJson);

            XmlDocument doc = new XmlDocument();
            using (StreamReader metadataFileStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_meta.xml")))
            {
                doc.LoadXml(metadataFileStreamReader.ReadToEnd());
            }

            //usgsDataFetcherMock.Stub(x => x.GetXmlDocFromURL(Arg<String>.Is.Equal(@"http://thor-f5.er.usgs.gov/ngtoc/metadata/waf/elevation/1_meter/img/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_meta.xml")))
            //usgsDataFetcherMock.Stub(x => x.GetXmlDocFromURL(Arg<String>.Is.Anything))
            //                   .Return(doc);

            using (mock.Record())
            {
                Expect.Call(m_usgsDataFetcherMock.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything)).Return(new List<USGSRequestBundle> { testBundle });
                Expect.Call(m_usgsDataFetcherMock.GetSciencebaseJson(Arg<String>.Is.Anything)).Return(scienceBaseJson);
                Expect.Call(m_usgsDataFetcherMock.GetXmlDocFromURL(Arg<String>.Is.Anything)).Return(doc);
            }


        }

        [Test]
        public void SpatialEntityWithDetailsViewTest()
        {

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityWithDetailsView"));
            query.SelectClause.SelectAllProperties = true;

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
            UsgsAPIQueryProvider usgsAPIQueryProvider = new UsgsAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock);

            var instanceList = usgsAPIQueryProvider.CreateInstanceList();

            //m_usgsDataFetcherMock.AssertWasCalled(x => x.GetNonFormattedUSGSResults(Arg<List<SingleWhereCriteriaHolder>>.Is.Anything));
            Assert.AreEqual(1, instanceList.Count());
            Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue);
            Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", instanceList.First().GetPropertyValue("Name").StringValue);
            Assert.AreEqual(true, (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }")) ||
                                  (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684]], \"coordinate_system\" : \"4326\" }")) ||
                                  (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356]], \"coordinate_system\" : \"4326\" }")) ||
                                  (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356]], \"coordinate_system\" : \"4326\" }")));
            Assert.AreEqual("ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg", instanceList.First().GetPropertyValue("ThumbnailURL").StringValue);
            Assert.AreEqual("https://www.sciencebase.gov/catalog/item/553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("MetadataURL").StringValue);
            Assert.AreEqual("https://www.sciencebase.gov/catalog/item/download/553690bfe4b0b22a15807df2?format=fgdc", instanceList.First().GetPropertyValue("RawMetadataURL").StringValue);
            Assert.AreEqual("FGDC", instanceList.First().GetPropertyValue("RawMetadataFormat").StringValue);
            Assert.AreEqual("USGS", instanceList.First().GetPropertyValue("SubAPI").StringValue);
            Assert.AreEqual("USGS", instanceList.First().GetPropertyValue("DataProvider").StringValue);
            Assert.AreEqual("United States Geological Survey", instanceList.First().GetPropertyValue("DataProviderName").StringValue);
            Assert.AreEqual("2015-03-19T00:00:00", instanceList.First().GetPropertyValue("Date").StringValue);
            Assert.AreEqual("1.0m", instanceList.First().GetPropertyValue("AccuracyResolutionDensity").StringValue);
            Assert.AreEqual("1.0x1.0", instanceList.First().GetPropertyValue("ResolutionInMeters").StringValue);
            Assert.AreEqual("Terrain", instanceList.First().GetPropertyValue("Classification").StringValue);

        }

        [Test]
        public void SpatialEntityBaseTest()
        {

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialEntityBase"));
            query.SelectClause.SelectAllProperties = true;

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

            UsgsAPIQueryProvider usgsAPIQueryProvider = new UsgsAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock);

            var instanceList = usgsAPIQueryProvider.CreateInstanceList();

            Assert.AreEqual(1, instanceList.Count());
            Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue);
            Assert.AreEqual(true, (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }")) ||
                                  (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684]], \"coordinate_system\" : \"4326\" }")) ||
                                  (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356]], \"coordinate_system\" : \"4326\" }")) ||
                                  (instanceList.First().GetPropertyValue("Footprint").StringValue.Contains("{ \"points\" : [[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356]], \"coordinate_system\" : \"4326\" }")));
            Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", instanceList.First().GetPropertyValue("Name").StringValue);
            Assert.AreEqual("elevation, Elevation, National Elevation Dataset, NED, Elevation, Light Detection and Ranging, LIDAR, High Resolution, Topographic Surface, Topography, Bare Earth, Hydro-Flattened, Terrain Elevation, Cartography, DEM, Digital Elevation Model, Digital Mapping, Digital Terrain Model, Geodata, GIS, Mapping, Raster, USGS, U.S. Geological Survey, 10,000 meter DEM, 1 meter DEM, Downloadable Data, Elevation, Digital Elevation Model (DEM) 1 meter, 10000 x 10000 meter, IMG, US, United States", instanceList.First().GetPropertyValue("Keywords").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("AssociateFile").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("ProcessingDescription").IsNull);
            Assert.AreEqual("IMG", instanceList.First().GetPropertyValue("DataSourceTypesAvailable").StringValue);
            //Assert.AreEqual("1.0m", instanceList.First().GetPropertyValue("AccuracyResolutionDensity").StringValue);
            Assert.AreEqual("USGS", instanceList.First().GetPropertyValue("DataProvider").StringValue);
            Assert.AreEqual("United States Geological Survey", instanceList.First().GetPropertyValue("DataProviderName").StringValue);
            //Assert.AreEqual("2015-03-19T00:00:00", instanceList.First().GetPropertyValue("Date").StringValue);
            Assert.AreEqual("Terrain", instanceList.First().GetPropertyValue("Classification").StringValue);


        }

        [Test]
        public void ThumbnailTest()
        {

            ECQuery query = new ECQuery(m_schema.GetClass("Thumbnail"));
            query.SelectClause.SelectAllProperties = true;

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

            UsgsAPIQueryProvider usgsAPIQueryProvider = new UsgsAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock);

            var instanceList = usgsAPIQueryProvider.CreateInstanceList();

            Assert.AreEqual(1, instanceList.Count());
            Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("ThumbnailProvenance").IsNull);
            Assert.AreEqual("jpg", instanceList.First().GetPropertyValue("ThumbnailFormat").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("ThumbnailWidth").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("ThumbnailHeight").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("ThumbnailStamp").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("ThumbnailGenerationDetails").IsNull);
        }

        [Test]
        public void MetadataTest()
        {

            ECQuery query = new ECQuery(m_schema.GetClass("Metadata"));
            query.SelectClause.SelectAllProperties = true;

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

            UsgsAPIQueryProvider usgsAPIQueryProvider = new UsgsAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock);

            var instanceList = usgsAPIQueryProvider.CreateInstanceList();

            Assert.AreEqual(1, instanceList.Count());
            Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("RawMetadata").IsNull);
            Assert.AreEqual("FGDC", instanceList.First().GetPropertyValue("RawMetadataFormat").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("DisplayStyle").IsNull);
            Assert.AreEqual("This is a tile of the National Elevation Dataset (NED) and is one meter resolution. Data in this layer represent a bare earth surface. The National Elevation Dataset (NED) serves as the elevation layer of The National Map, and provides basic elevation information for earth science studies and mapping applications in the United States. Scientists and resource managers use NED data for global change research, hydrologic modeling, resource monitoring, mapping and visualization, and many other applications. The NED is an elevation dataset that consists of seamless layers and non-seamless high resolution layers. Each of the seamless layers are composed of the best available raster elevation data of the conterminous United States, Alaska, [...]", instanceList.First().GetPropertyValue("Description").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("ContactInformation").IsNull);
            Assert.AreEqual("elevation, Elevation, National Elevation Dataset, NED, Elevation, Light Detection and Ranging, LIDAR, High Resolution, Topographic Surface, Topography, Bare Earth, Hydro-Flattened, Terrain Elevation, Cartography, DEM, Digital Elevation Model, Digital Mapping, Digital Terrain Model, Geodata, GIS, Mapping, Raster, USGS, U.S. Geological Survey, 10,000 meter DEM, 1 meter DEM, Downloadable Data, Elevation, Digital Elevation Model (DEM) 1 meter, 10000 x 10000 meter, IMG, US, United States", instanceList.First().GetPropertyValue("Keywords").StringValue);
            Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey", instanceList.First().GetPropertyValue("Legal").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("Lineage").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("Provenance").IsNull);
        }

        [Test]
        public void SpatialDataSourceTest()
        {

            ECQuery query = new ECQuery(m_schema.GetClass("SpatialDataSource"));
            query.SelectClause.SelectAllProperties = true;

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

            UsgsAPIQueryProvider usgsAPIQueryProvider = new UsgsAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock);

            var instanceList = usgsAPIQueryProvider.CreateInstanceList();

            Assert.AreEqual(1, instanceList.Count());
            Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue);
            Assert.AreEqual("ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip", instanceList.First().GetPropertyValue("MainURL").StringValue);
            Assert.AreEqual("USGS", instanceList.First().GetPropertyValue("CompoundType").StringValue);
            Assert.AreEqual("Unknown", instanceList.First().GetPropertyValue("LocationInCompound").StringValue);
            Assert.AreEqual("IMG", instanceList.First().GetPropertyValue("DataSourceType").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("SisterFiles").IsNull);
            Assert.AreEqual("256093", instanceList.First().GetPropertyValue("FileSize").StringValue);
            Assert.AreEqual("https://www.sciencebase.gov/catalog/file/get/553690bfe4b0b22a15807df2?f=__disk__74%2Fbb%2F91%2F74bb9105fd15f0129423afefeae7144279edf5ec", instanceList.First().GetPropertyValue("Metadata").StringValue);
        }

        [Test]
        public void ServerTest()
        {

            ECQuery query = new ECQuery(m_schema.GetClass("Server"));
            query.SelectClause.SelectAllProperties = true;

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("553690bfe4b0b22a15807df2"));

            UsgsAPIQueryProvider usgsAPIQueryProvider = new UsgsAPIQueryProvider(query, new ECQuerySettings(), m_usgsDataFetcherMock);

            var instanceList = usgsAPIQueryProvider.CreateInstanceList();

            Assert.AreEqual(1, instanceList.Count());
            Assert.AreEqual("553690bfe4b0b22a15807df2", instanceList.First().GetPropertyValue("Id").StringValue);
            Assert.AreEqual("ftp", instanceList.First().GetPropertyValue("CommunicationProtocol").StringValue);
            Assert.AreEqual("ftp://rockyftp.cr.usgs.gov", instanceList.First().GetPropertyValue("URL").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("Name").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("ServerContactInformation").IsNull);
            Assert.AreEqual("None. No fees are applicable for obtaining the data set.", instanceList.First().GetPropertyValue("Fees").StringValue);
            Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey", instanceList.First().GetPropertyValue("Legal").StringValue);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("AccessConstraints").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("Online").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("LastCheck").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("LastTimeOnline").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("Latency").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("MeanReachabilityStats").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("State").IsNull);
            Assert.AreEqual(true, instanceList.First().GetPropertyValue("Type").IsNull);
        }
    }
}
