using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECObjects.XML;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests
    {
    [TestFixture]
    class InstanceOverriderTests
        {

        MockRepository m_mock;
        IECSchema m_schema;
        IDbQuerier m_querierMock;
        [SetUp]
        public void SetUp ()
            {
            m_mock = new MockRepository();

            m_querierMock = (IDbQuerier) m_mock.StrictMock(typeof(IDbQuerier));

            m_schema = SetupHelpers.PrepareSchema();
            }

        [Test]
        public void SEBTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateSEB(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateSEB(true, m_schema);


            string overridenFootprint = "OverridenFootprint";
            overrideInstance["Footprint"].StringValue = overridenFootprint;

            string overridenName = "OverridenName";
            overrideInstance["Name"].StringValue = overridenName;

            string overridenKeywords = "OverridenKeywords";
            overrideInstance["Keywords"].StringValue = overridenKeywords;

            string overridenAssociateFile = "OverridenAssociateFile";
            overrideInstance["AssociateFile"].StringValue = overridenAssociateFile;

            string overridenProcessingDescription = "OverridenProcessingDescription";
            overrideInstance["ProcessingDescription"].StringValue = overridenProcessingDescription;

            string overridenDTA = "OverridenDTA";
            overrideInstance["DataSourceTypesAvailable"].StringValue = overridenDTA;

            string overridenARD = "OverridenARD";
            overrideInstance["AccuracyResolutionDensity"].StringValue = overridenARD;

            double overridenCC = 12.3;
            overrideInstance["CloudCoverage"].DoubleValue = overridenCC;

            string overridenResolutionInMeters = "ResolutionInMeters";
            overrideInstance["ResolutionInMeters"].StringValue = overridenResolutionInMeters;

            string overridenDataProvider = "DataProvider";
            overrideInstance["DataProvider"].StringValue = overridenDataProvider;

            string overridenDataProviderName = "DataProviderName";
            overrideInstance["DataProviderName"].StringValue = overridenDataProviderName;

            DateTime overridenDate = new DateTime(2016, 10, 31);
            overrideInstance["Date"].NativeValue = overridenDate;

            string overridenClassification = "OverridenClassification";
            overrideInstance["Classification"].StringValue = overridenClassification;



            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenFootprint, modifiedInstance["Footprint"].StringValue);
                Assert.AreEqual(overridenName, modifiedInstance["Name"].StringValue);
                Assert.AreEqual(overridenKeywords, modifiedInstance["Keywords"].StringValue);
                Assert.AreEqual(overridenAssociateFile, modifiedInstance["AssociateFile"].StringValue);
                Assert.AreEqual(overridenProcessingDescription, modifiedInstance["ProcessingDescription"].StringValue);
                Assert.AreEqual(overridenDTA, modifiedInstance["DataSourceTypesAvailable"].StringValue);
                Assert.AreEqual(overridenARD, modifiedInstance["AccuracyResolutionDensity"].StringValue);
                Assert.AreEqual(overridenCC, modifiedInstance["CloudCoverage"].DoubleValue);
                Assert.AreEqual(overridenResolutionInMeters, modifiedInstance["ResolutionInMeters"].StringValue);
                Assert.AreEqual(overridenDataProvider, modifiedInstance["DataProvider"].StringValue);
                Assert.AreEqual(overridenDataProviderName, modifiedInstance["DataProviderName"].StringValue);
                Assert.AreEqual(overridenDate, modifiedInstance["Date"].NativeValue);
                Assert.AreEqual(overridenClassification, modifiedInstance["Classification"].StringValue);

                }
            }

        [Test]
        public void MetadataTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateMetadata(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateMetadata(true, m_schema);


            string overridenRawMetadata = "OverridenRawMetadata";
            overrideInstance["RawMetadata"].StringValue = overridenRawMetadata;

            string overridenRawMetadataFormat = "OverridenRawMetadataFormat";
            overrideInstance["RawMetadataFormat"].StringValue = overridenRawMetadataFormat;

            string overridenDisplayStyle = "OverridenDisplayStyle";
            overrideInstance["DisplayStyle"].StringValue = overridenDisplayStyle;

            string overridenDescription = "OverridenDescription";
            overrideInstance["Description"].StringValue = overridenDescription;

            string overridenContactInformation = "OverridenContactInformation";
            overrideInstance["ContactInformation"].StringValue = overridenContactInformation;

            string overridenKeywords = "OverridenKeywords";
            overrideInstance["Keywords"].StringValue = overridenKeywords;

            string overridenLegal = "OverridenLegal";
            overrideInstance["Legal"].StringValue = overridenLegal;

            string overridenLineage = "OverridenLineage";
            overrideInstance["Lineage"].NativeValue = overridenLineage;

            string overridenProvenance = "OverridenProvenance";
            overrideInstance["Provenance"].StringValue = overridenProvenance;



            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenRawMetadata, modifiedInstance["RawMetadata"].StringValue);
                Assert.AreEqual(overridenRawMetadataFormat, modifiedInstance["RawMetadataFormat"].StringValue);
                Assert.AreEqual(overridenDisplayStyle, modifiedInstance["DisplayStyle"].StringValue);
                Assert.AreEqual(overridenDescription, modifiedInstance["Description"].StringValue);
                Assert.AreEqual(overridenContactInformation, modifiedInstance["ContactInformation"].StringValue);
                Assert.AreEqual(overridenKeywords, modifiedInstance["Keywords"].StringValue);
                Assert.AreEqual(overridenLegal, modifiedInstance["Legal"].StringValue);
                Assert.AreEqual(overridenLineage, modifiedInstance["Lineage"].NativeValue);
                Assert.AreEqual(overridenProvenance, modifiedInstance["Provenance"].StringValue);

                }
            }

        [Test]
        public void OsmSourceTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();



            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateOsmSource(m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateOsmSource(m_schema);

            string overridenMainURL = "OverridenMainURL";
            overrideInstance["MainURL"].StringValue = overridenMainURL;

            string overridenCompoundType = "OverridenCompoundType";
            overrideInstance["CompoundType"].StringValue = overridenCompoundType;

            string overridenLocationInCompound = "OverridenLocationInCompound";
            overrideInstance["LocationInCompound"].StringValue = overridenLocationInCompound;

            string overridenDataSourceType = "OverridenDataSourceType";
            overrideInstance["DataSourceType"].StringValue = overridenDataSourceType;

            string overridenSisterFiles = "OverridenSisterFiles";
            overrideInstance["SisterFiles"].StringValue = overridenSisterFiles;

            string overridenNDV = "OverridenNDV";
            overrideInstance["NoDataValue"].StringValue = overridenNDV;

            long overridenFileSize = 89274;
            overrideInstance["FileSize"].NativeValue = overridenFileSize;

            string overridenAlternateURL1 = "OverridenAlternateURL1";
            overrideInstance["AlternateURL1"].StringValue = overridenAlternateURL1;

            string overridenAlternateURL2 = "OverridenAlternateURL2";
            overrideInstance["AlternateURL2"].StringValue = overridenAlternateURL2;

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenMainURL, modifiedInstance["MainURL"].StringValue);
                Assert.AreEqual(overridenCompoundType, modifiedInstance["CompoundType"].StringValue);
                Assert.AreEqual(overridenLocationInCompound, modifiedInstance["LocationInCompound"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenSisterFiles, modifiedInstance["SisterFiles"].StringValue);
                Assert.AreEqual(overridenNDV, modifiedInstance["NoDataValue"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);

                Assert.AreEqual(overridenAlternateURL1, modifiedInstance["AlternateURL1"].StringValue);
                Assert.AreEqual(overridenAlternateURL2, modifiedInstance["AlternateURL2"].StringValue);

                }
            }

        [Test]
        public void MultibandSourceTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();



            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateMultibandSource(m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateMultibandSource(m_schema);

            string overridenMainURL = "OverridenMainURL";
            overrideInstance["MainURL"].StringValue = overridenMainURL;

            string overridenCompoundType = "OverridenCompoundType";
            overrideInstance["CompoundType"].StringValue = overridenCompoundType;

            string overridenLocationInCompound = "OverridenLocationInCompound";
            overrideInstance["LocationInCompound"].StringValue = overridenLocationInCompound;

            string overridenDataSourceType = "OverridenDataSourceType";
            overrideInstance["DataSourceType"].StringValue = overridenDataSourceType;

            string overridenSisterFiles = "OverridenSisterFiles";
            overrideInstance["SisterFiles"].StringValue = overridenSisterFiles;

            string overridenNDV = "OverridenNDV";
            overrideInstance["NoDataValue"].StringValue = overridenNDV;

            long overridenFileSize = 89274;
            overrideInstance["FileSize"].NativeValue = overridenFileSize;

            string overridenRedBandURL = "OverridenRedBandURL";
            overrideInstance["RedBandURL"].StringValue = overridenRedBandURL;

            string overridenGreenBandURL = "OverridenGreenBandURL";
            overrideInstance["GreenBandURL"].StringValue = overridenGreenBandURL;

            string overridenBlueBandURL = "OverridenBlueBandURL";
            overrideInstance["BlueBandURL"].StringValue = overridenBlueBandURL;

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenMainURL, modifiedInstance["MainURL"].StringValue);
                Assert.AreEqual(overridenCompoundType, modifiedInstance["CompoundType"].StringValue);
                Assert.AreEqual(overridenLocationInCompound, modifiedInstance["LocationInCompound"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenSisterFiles, modifiedInstance["SisterFiles"].StringValue);
                Assert.AreEqual(overridenNDV, modifiedInstance["NoDataValue"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);

                Assert.AreEqual(overridenRedBandURL, modifiedInstance["RedBandURL"].StringValue);
                Assert.AreEqual(overridenGreenBandURL, modifiedInstance["GreenBandURL"].StringValue);
                Assert.AreEqual(overridenBlueBandURL, modifiedInstance["BlueBandURL"].StringValue);
                }
            }

        [Test]
        public void ServerTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();



            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateServer(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateServer(true, m_schema);


            string overridenCommunicationProtocol = "OverridenCommunicationProtocol";
            overrideInstance["CommunicationProtocol"].StringValue = overridenCommunicationProtocol;

            string overridenName = "OverridenName";
            overrideInstance["Name"].StringValue = overridenName;

            string overridenURL = "OverridenURL";
            overrideInstance["URL"].StringValue = overridenURL;

            string overridenSCI = "OverridenSCI";
            overrideInstance["ServerContactInformation"].StringValue = overridenSCI;

            string overridenFees = "OverridenFees";
            overrideInstance["Fees"].StringValue = overridenFees;

            string overridenLegal = "OverridenLegal";
            overrideInstance["Legal"].StringValue = overridenLegal;

            string overridenAccessConstraints = "OverridenAccessConstraints";
            overrideInstance["AccessConstraints"].StringValue = overridenAccessConstraints;

            bool overridenOnline = true;
            overrideInstance["Online"].NativeValue = overridenOnline;

            DateTime overridenLastCheck = new DateTime(2016, 10, 31);
            overrideInstance["LastCheck"].NativeValue = overridenLastCheck;

            DateTime overridenLastTimeOnline = new DateTime(2016, 10, 31);
            overrideInstance["LastTimeOnline"].NativeValue = overridenLastTimeOnline;

            double overridenLatency = 8254.3;
            overrideInstance["Latency"].DoubleValue = overridenLatency;

            int overridenMRS = 8294;
            overrideInstance["MeanReachabilityStats"].IntValue = overridenMRS;

            string overridenState = "OverridenState";
            overrideInstance["State"].StringValue = overridenState;

            string overridenType = "OverridenType";
            overrideInstance["Type"].StringValue = overridenType;

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {

                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenCommunicationProtocol, modifiedInstance["CommunicationProtocol"].StringValue);
                Assert.AreEqual(overridenName, modifiedInstance["Name"].StringValue);
                Assert.AreEqual(overridenURL, modifiedInstance["URL"].StringValue);
                Assert.AreEqual(overridenSCI, modifiedInstance["ServerContactInformation"].StringValue);
                Assert.AreEqual(overridenFees, modifiedInstance["Fees"].StringValue);
                Assert.AreEqual(overridenLegal, modifiedInstance["Legal"].StringValue);
                Assert.AreEqual(overridenAccessConstraints, modifiedInstance["AccessConstraints"].StringValue);
                Assert.AreEqual(overridenOnline, modifiedInstance["Online"].NativeValue);
                Assert.AreEqual(overridenLastCheck, modifiedInstance["LastCheck"].NativeValue);
                Assert.AreEqual(overridenLatency, modifiedInstance["Latency"].DoubleValue);
                Assert.AreEqual(overridenMRS, modifiedInstance["MeanReachabilityStats"].IntValue);
                Assert.AreEqual(overridenState, modifiedInstance["State"].StringValue);
                Assert.AreEqual(overridenType, modifiedInstance["Type"].StringValue);

                }
            }

        [Test]
        public void SDSTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();



            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateSDS(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateSDS(true, m_schema);


            string overridenMainURL = "OverridenMainURL";
            overrideInstance["MainURL"].StringValue = overridenMainURL;

            string overridenCompoundType = "OverridenCompoundType";
            overrideInstance["CompoundType"].StringValue = overridenCompoundType;

            string overridenLocationInCompound = "OverridenLocationInCompound";
            overrideInstance["LocationInCompound"].StringValue = overridenLocationInCompound;

            string overridenDataSourceType = "OverridenDataSourceType";
            overrideInstance["DataSourceType"].StringValue = overridenDataSourceType;

            string overridenSisterFiles = "OverridenSisterFiles";
            overrideInstance["SisterFiles"].StringValue = overridenSisterFiles;

            string overridenNDV = "OverridenNDV";
            overrideInstance["NoDataValue"].StringValue = overridenNDV;

            long overridenFileSize = 89274;
            overrideInstance["FileSize"].NativeValue = overridenFileSize;

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {

                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenMainURL, modifiedInstance["MainURL"].StringValue);
                Assert.AreEqual(overridenCompoundType, modifiedInstance["CompoundType"].StringValue);
                Assert.AreEqual(overridenLocationInCompound, modifiedInstance["LocationInCompound"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenSisterFiles, modifiedInstance["SisterFiles"].StringValue);
                Assert.AreEqual(overridenNDV, modifiedInstance["NoDataValue"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);
                }
            }

        [Test]
        public void RelatedInstancesTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            IECInstance sebInstance = SetupHelpers.CreateSEB(true, m_schema);
            IECInstance metadataInstance = SetupHelpers.CreateMetadata(true, m_schema);
            ((IECRelationshipClass) m_schema.GetClass("SpatialEntityBaseToMetadata")).CreateRelationship(sebInstance, metadataInstance);

            List<IECInstance> instances = new List<IECInstance>() { sebInstance };
            IECInstance sebOverrideInstance = SetupHelpers.CreateSEB(true, m_schema);

            string sebOverridenName = "sebOverridenName";

            sebOverrideInstance["Name"].StringValue = sebOverridenName;


            IECInstance metadataOverrideInstance = SetupHelpers.CreateMetadata(true, m_schema);

            string metadataOverridenLegal = "metadataOverridenName";

            metadataOverrideInstance["Legal"].StringValue = metadataOverridenLegal;

            List<IECInstance> sebOverrideInstances = new List<IECInstance>() { sebOverrideInstance };
            List<IECInstance> metadataOverrideInstances = new List<IECInstance>() { metadataOverrideInstance };
            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Equal(sebInstance.ClassDefinition),
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(sebOverrideInstances);

                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                              Arg<DataReadingHelper>.Is.Anything,
                                              Arg<IParamNameValueMap>.Is.Anything,
                                              Arg<IECClass>.Is.Equal(metadataInstance.ClassDefinition),
                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                              Arg<IDbConnection>.Is.Anything,
                                              Arg<IEnumerable<string>>.Is.Anything,
                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(metadataOverrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();
                IECInstance modifiedRelInstance = modifiedInstance.GetRelationshipInstances().First().Target;

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(sebOverridenName, modifiedInstance["Name"].StringValue);
                Assert.AreEqual(metadataOverridenLegal, modifiedRelInstance["Legal"].StringValue);

                }
            }

        [Test]
        public void SpatialEntityTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateSpatialEntity(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateSpatialEntity(true, m_schema);


            string overridenFootprint = "OverridenFootprint";
            overrideInstance["Footprint"].StringValue = overridenFootprint;

            string overridenName = "OverridenName";
            overrideInstance["Name"].StringValue = overridenName;

            string overridenKeywords = "OverridenKeywords";
            overrideInstance["Keywords"].StringValue = overridenKeywords;

            string overridenAssociateFile = "OverridenAssociateFile";
            overrideInstance["AssociateFile"].StringValue = overridenAssociateFile;

            string overridenProcessingDescription = "OverridenProcessingDescription";
            overrideInstance["ProcessingDescription"].StringValue = overridenProcessingDescription;

            string overridenDTA = "OverridenDTA";
            overrideInstance["DataSourceTypesAvailable"].StringValue = overridenDTA;

            string overridenARD = "OverridenARD";
            overrideInstance["AccuracyResolutionDensity"].StringValue = overridenARD;

            double overridenCC = 12.3;
            overrideInstance["CloudCoverage"].DoubleValue = overridenCC;

            string overridenResolutionInMeters = "ResolutionInMeters";
            overrideInstance["ResolutionInMeters"].StringValue = overridenResolutionInMeters;

            string overridenDataProvider = "DataProvider";
            overrideInstance["DataProvider"].StringValue = overridenDataProvider;

            string overridenDataProviderName = "DataProviderName";
            overrideInstance["DataProviderName"].StringValue = overridenDataProviderName;

            DateTime overridenDate = new DateTime(2016, 10, 31);
            overrideInstance["Date"].NativeValue = overridenDate;

            string overridenClassification = "OverridenClassification";
            overrideInstance["Classification"].StringValue = overridenClassification;



            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenFootprint, modifiedInstance["Footprint"].StringValue);
                Assert.AreEqual(overridenName, modifiedInstance["Name"].StringValue);
                Assert.AreEqual(overridenKeywords, modifiedInstance["Keywords"].StringValue);
                Assert.AreEqual(overridenAssociateFile, modifiedInstance["AssociateFile"].StringValue);
                Assert.AreEqual(overridenProcessingDescription, modifiedInstance["ProcessingDescription"].StringValue);
                Assert.AreEqual(overridenDTA, modifiedInstance["DataSourceTypesAvailable"].StringValue);
                Assert.AreEqual(overridenARD, modifiedInstance["AccuracyResolutionDensity"].StringValue);
                Assert.AreEqual(overridenCC, modifiedInstance["CloudCoverage"].DoubleValue);
                Assert.AreEqual(overridenResolutionInMeters, modifiedInstance["ResolutionInMeters"].StringValue);
                Assert.AreEqual(overridenDataProvider, modifiedInstance["DataProvider"].StringValue);
                Assert.AreEqual(overridenDataProviderName, modifiedInstance["DataProviderName"].StringValue);
                Assert.AreEqual(overridenDate, modifiedInstance["Date"].NativeValue);
                Assert.AreEqual(overridenClassification, modifiedInstance["Classification"].StringValue);

                }
            }

        [Test]
        public void SpatialEntityDatasetTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateParentSED(m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateParentSED(m_schema);


            string overridenFootprint = "OverridenFootprint";
            overrideInstance["Footprint"].StringValue = overridenFootprint;

            string overridenName = "OverridenName";
            overrideInstance["Name"].StringValue = overridenName;

            string overridenKeywords = "OverridenKeywords";
            overrideInstance["Keywords"].StringValue = overridenKeywords;

            string overridenAssociateFile = "OverridenAssociateFile";
            overrideInstance["AssociateFile"].StringValue = overridenAssociateFile;

            string overridenProcessingDescription = "OverridenProcessingDescription";
            overrideInstance["ProcessingDescription"].StringValue = overridenProcessingDescription;

            string overridenDTA = "OverridenDTA";
            overrideInstance["DataSourceTypesAvailable"].StringValue = overridenDTA;

            string overridenARD = "OverridenARD";
            overrideInstance["AccuracyResolutionDensity"].StringValue = overridenARD;

            double overridenCC = 12.3;
            overrideInstance["CloudCoverage"].DoubleValue = overridenCC;

            string overridenResolutionInMeters = "ResolutionInMeters";
            overrideInstance["ResolutionInMeters"].StringValue = overridenResolutionInMeters;

            string overridenDataProvider = "DataProvider";
            overrideInstance["DataProvider"].StringValue = overridenDataProvider;

            string overridenDataProviderName = "DataProviderName";
            overrideInstance["DataProviderName"].StringValue = overridenDataProviderName;

            DateTime overridenDate = new DateTime(2016, 10, 31);
            overrideInstance["Date"].NativeValue = overridenDate;

            string overridenClassification = "OverridenClassification";
            overrideInstance["Classification"].StringValue = overridenClassification;

            bool overridenProcessable = true;
            overrideInstance["Processable"].NativeValue = overridenProcessable;


            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenFootprint, modifiedInstance["Footprint"].StringValue, "Footprints are not equal");
                Assert.AreEqual(overridenName, modifiedInstance["Name"].StringValue, "Names are not equal");
                Assert.AreEqual(overridenKeywords, modifiedInstance["Keywords"].StringValue, "Keywords are not equal");
                Assert.AreEqual(overridenAssociateFile, modifiedInstance["AssociateFile"].StringValue, "AssociateFiles are not equal");
                Assert.AreEqual(overridenProcessingDescription, modifiedInstance["ProcessingDescription"].StringValue, "ProcessingDescriptions are not equal");
                Assert.AreEqual(overridenDTA, modifiedInstance["DataSourceTypesAvailable"].StringValue, "DataSourceTypesAvailables are not equal");
                Assert.AreEqual(overridenARD, modifiedInstance["AccuracyResolutionDensity"].StringValue, "AccuracyResolutionDensities are not equal");
                Assert.AreEqual(overridenCC, modifiedInstance["CloudCoverage"].DoubleValue);
                Assert.AreEqual(overridenResolutionInMeters, modifiedInstance["ResolutionInMeters"].StringValue);
                Assert.AreEqual(overridenDataProvider, modifiedInstance["DataProvider"].StringValue);
                Assert.AreEqual(overridenDataProviderName, modifiedInstance["DataProviderName"].StringValue);
                Assert.AreEqual(overridenDate, modifiedInstance["Date"].NativeValue, "Date are not equal");
                Assert.AreEqual(overridenClassification, modifiedInstance["Classification"].StringValue, "Classification are not equal");
                Assert.AreEqual(overridenProcessable, modifiedInstance["Processable"].NativeValue, "Processable are not equal");
                }
            }

        [Test]
        public void ThumbnailTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateThumbnail(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateThumbnail(true, m_schema);


            string overridenProvenance = "OverridenProvenance";
            overrideInstance["ThumbnailProvenance"].StringValue = overridenProvenance;

            string overridenFormat = "OverridenFormat";
            overrideInstance["ThumbnailFormat"].StringValue = overridenFormat;

            string overridenWidth = "OverridenWidth";
            overrideInstance["ThumbnailWidth"].StringValue = overridenWidth;

            string overridenHeight = "OverridenHeight";
            overrideInstance["ThumbnailHeight"].StringValue = overridenHeight;

            string overridenStamp = "OverridenStamp";
            overrideInstance["ThumbnailStamp"].StringValue = overridenStamp;

            string overridenGenerationDetails = "OverridenGenerationDetails";
            overrideInstance["ThumbnailGenerationDetails"].StringValue = overridenGenerationDetails;

            Stream originalStream = new MemoryStream();
            byte[] originalBytes = System.Text.Encoding.UTF8.GetBytes("OriginalStream");
            originalStream.Write(originalBytes, 0, originalBytes.Length);
            Stream overridenStream = new MemoryStream();
            string overridenStreamString = "OverridenStream";
            byte[] overridenBytes = System.Text.Encoding.UTF8.GetBytes(overridenStreamString);
            overridenStream.Write(overridenBytes, 0, overridenBytes.Length);

            StreamBackedDescriptor originalSBD = new StreamBackedDescriptor(originalStream);
            StreamBackedDescriptorAccessor.SetIn(instances.First(), originalSBD);

            StreamBackedDescriptor overridenSBD = new StreamBackedDescriptor(overridenStream);
            StreamBackedDescriptorAccessor.SetIn(overrideInstance, overridenSBD);

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenProvenance, modifiedInstance["ThumbnailProvenance"].StringValue, "ThumbnailProvenances are not equal");
                Assert.AreEqual(overridenFormat, modifiedInstance["ThumbnailFormat"].StringValue, "ThumbnailFormats are not equal");
                Assert.AreEqual(overridenWidth, modifiedInstance["ThumbnailWidth"].StringValue, "ThumbnailWidths are not equal");
                Assert.AreEqual(overridenHeight, modifiedInstance["ThumbnailHeight"].StringValue, "ThumbnailHeights are not equal");
                Assert.AreEqual(overridenStamp, modifiedInstance["ThumbnailStamp"].StringValue, "ThumbnailStamps are not equal");
                Assert.AreEqual(overridenGenerationDetails, modifiedInstance["ThumbnailGenerationDetails"].StringValue, "ThumbnailGenerationDetails are not equal");

                StreamBackedDescriptor testSBD = StreamBackedDescriptorAccessor.GetFrom(modifiedInstance);
                StreamReader reader = new StreamReader(testSBD.Stream, System.Text.Encoding.UTF8);
                testSBD.Stream.Position = 0;
                string testStreamString = reader.ReadToEnd();
                Assert.AreEqual(overridenStreamString, testStreamString, "Thumbnail Datas are not equal.");
                }
            }

        [Test]
        public void WMSLayerTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateWMSLayer(m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateWMSLayer(m_schema);


            string overridenName = "OverridenName";
            overrideInstance["Name"].StringValue = overridenName;

            string overridenTitle = "OverridenTitle";
            overrideInstance["Title"].StringValue = overridenTitle;

            string overridenDescription = "OverridenDescription";
            overrideInstance["Description"].StringValue = overridenDescription;

            string overridenBoundingBox = "OverridenBoundingBox";
            overrideInstance["BoundingBox"].StringValue = overridenBoundingBox;

            string overridenCoordinateSystems = "OverridenCoordinateSystems";
            overrideInstance["CoordinateSystems"].StringValue = overridenCoordinateSystems;


            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenName, modifiedInstance["Name"].StringValue, "Names are not equal");
                Assert.AreEqual(overridenTitle, modifiedInstance["Title"].StringValue, "Titles are not equal");
                Assert.AreEqual(overridenDescription, modifiedInstance["Description"].StringValue, "Descriptions are not equal");
                Assert.AreEqual(overridenBoundingBox, modifiedInstance["BoundingBox"].StringValue, "BoundingBoxes are not equal");
                Assert.AreEqual(overridenCoordinateSystems, modifiedInstance["CoordinateSystems"].StringValue, "CoordinateSystems are not equal");

                }
            }

        [Test]
        public void WMSServerTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();



            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateWMSServer(m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateWMSServer(m_schema);


            string overridenCommunicationProtocol = "OverridenCommunicationProtocol";
            overrideInstance["CommunicationProtocol"].StringValue = overridenCommunicationProtocol;

            string overridenName = "OverridenName";
            overrideInstance["Name"].StringValue = overridenName;

            string overridenURL = "OverridenURL";
            overrideInstance["URL"].StringValue = overridenURL;

            string overridenSCI = "OverridenSCI";
            overrideInstance["ServerContactInformation"].StringValue = overridenSCI;

            string overridenFees = "OverridenFees";
            overrideInstance["Fees"].StringValue = overridenFees;

            string overridenLegal = "OverridenLegal";
            overrideInstance["Legal"].StringValue = overridenLegal;

            string overridenAccessConstraints = "OverridenAccessConstraints";
            overrideInstance["AccessConstraints"].StringValue = overridenAccessConstraints;

            bool overridenOnline = true;
            overrideInstance["Online"].NativeValue = overridenOnline;

            DateTime overridenLastCheck = new DateTime(2016, 10, 31);
            overrideInstance["LastCheck"].NativeValue = overridenLastCheck;

            DateTime overridenLastTimeOnline = new DateTime(2016, 10, 31);
            overrideInstance["LastTimeOnline"].NativeValue = overridenLastTimeOnline;

            double overridenLatency = 8254.3;
            overrideInstance["Latency"].DoubleValue = overridenLatency;

            int overridenMRS = 8294;
            overrideInstance["MeanReachabilityStats"].IntValue = overridenMRS;

            string overridenState = "OverridenState";
            overrideInstance["State"].StringValue = overridenState;

            string overridenType = "OverridenType";
            overrideInstance["Type"].StringValue = overridenType;

            string overridenTitle = "OverridenTitle";
            overrideInstance["Title"].StringValue = overridenTitle;

            string overridenDescription = "OverridenDescription";
            overrideInstance["Description"].StringValue = overridenDescription;

            int overridenNumLayers = 835654;
            overrideInstance["NumLayers"].IntValue = overridenNumLayers;

            string overridenVersion = "OverridenVersion";
            overrideInstance["Version"].StringValue = overridenVersion;

            string overridenGetCapabilitiesURL = "OverridenGetCapabilitiesURL";
            overrideInstance["GetCapabilitiesURL"].StringValue = overridenGetCapabilitiesURL;

            string overridenSupportedFormats = "OverridenSupportedFormats";
            overrideInstance["SupportedFormats"].StringValue = overridenSupportedFormats;

            string overridenGetMapURL = "OverridenGetMapURL";
            overrideInstance["GetMapURL"].StringValue = overridenGetMapURL;

            string overridenGetMapURLQuery = "OverridenGetMapURLQuery";
            overrideInstance["GetMapURLQuery"].StringValue = overridenGetMapURLQuery;

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {

                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenCommunicationProtocol, modifiedInstance["CommunicationProtocol"].StringValue);
                Assert.AreEqual(overridenName, modifiedInstance["Name"].StringValue);
                Assert.AreEqual(overridenURL, modifiedInstance["URL"].StringValue);
                Assert.AreEqual(overridenSCI, modifiedInstance["ServerContactInformation"].StringValue);
                Assert.AreEqual(overridenFees, modifiedInstance["Fees"].StringValue);
                Assert.AreEqual(overridenLegal, modifiedInstance["Legal"].StringValue);
                Assert.AreEqual(overridenAccessConstraints, modifiedInstance["AccessConstraints"].StringValue);
                Assert.AreEqual(overridenOnline, modifiedInstance["Online"].NativeValue);
                Assert.AreEqual(overridenLastCheck, modifiedInstance["LastCheck"].NativeValue);
                Assert.AreEqual(overridenLatency, modifiedInstance["Latency"].DoubleValue);
                Assert.AreEqual(overridenMRS, modifiedInstance["MeanReachabilityStats"].IntValue);
                Assert.AreEqual(overridenState, modifiedInstance["State"].StringValue);
                Assert.AreEqual(overridenType, modifiedInstance["Type"].StringValue);

                Assert.AreEqual(overridenTitle, modifiedInstance["Title"].StringValue);
                Assert.AreEqual(overridenDescription, modifiedInstance["Description"].StringValue);
                Assert.AreEqual(overridenNumLayers, modifiedInstance["NumLayers"].IntValue);
                Assert.AreEqual(overridenVersion, modifiedInstance["Version"].StringValue);
                Assert.AreEqual(overridenGetCapabilitiesURL, modifiedInstance["GetCapabilitiesURL"].StringValue);
                Assert.AreEqual(overridenSupportedFormats, modifiedInstance["SupportedFormats"].StringValue);
                Assert.AreEqual(overridenGetMapURL, modifiedInstance["GetMapURL"].StringValue);
                Assert.AreEqual(overridenGetMapURLQuery, modifiedInstance["GetMapURLQuery"].StringValue);

                }
            }

        [Test]
        public void WMSSourceTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();



            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateWMSSource(m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateWMSSource(m_schema);


            string overridenMainURL = "OverridenMainURL";
            overrideInstance["MainURL"].StringValue = overridenMainURL;

            string overridenCompoundType = "OverridenCompoundType";
            overrideInstance["CompoundType"].StringValue = overridenCompoundType;

            string overridenLocationInCompound = "OverridenLocationInCompound";
            overrideInstance["LocationInCompound"].StringValue = overridenLocationInCompound;

            string overridenDataSourceType = "OverridenDataSourceType";
            overrideInstance["DataSourceType"].StringValue = overridenDataSourceType;

            string overridenSisterFiles = "OverridenSisterFiles";
            overrideInstance["SisterFiles"].StringValue = overridenSisterFiles;

            string overridenNDV = "OverridenNDV";
            overrideInstance["NoDataValue"].StringValue = overridenNDV;

            long overridenFileSize = 89274;
            overrideInstance["FileSize"].NativeValue = overridenFileSize;

            string overridenLayers = "OverridenLayers";
            overrideInstance["Layers"].StringValue = overridenLayers;

            string overridenStyles = "OverridenStyles";
            overrideInstance["Styles"].StringValue = overridenStyles;

            string overridenFormats = "OverridenFormats";
            overrideInstance["Formats"].StringValue = overridenFormats;

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {

                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenMainURL, modifiedInstance["MainURL"].StringValue);
                Assert.AreEqual(overridenCompoundType, modifiedInstance["CompoundType"].StringValue);
                Assert.AreEqual(overridenLocationInCompound, modifiedInstance["LocationInCompound"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenSisterFiles, modifiedInstance["SisterFiles"].StringValue);
                Assert.AreEqual(overridenNDV, modifiedInstance["NoDataValue"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);

                Assert.AreEqual(overridenLayers, modifiedInstance["Layers"].StringValue);
                Assert.AreEqual(overridenStyles, modifiedInstance["Styles"].StringValue);
                Assert.AreEqual(overridenFormats, modifiedInstance["Formats"].StringValue);
                }
            }

        [Test]
        public void SEWDVTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateSEWDV(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateSEWDV(true, m_schema);


            string overridenFootprint = "OverridenFootprint";
            overrideInstance["Footprint"].StringValue = overridenFootprint;

            string overridenName = "OverridenName";
            overrideInstance["Name"].StringValue = overridenName;

            string overridenDescription = "OverridenDescription";
            overrideInstance["Description"].StringValue = overridenDescription;

            string overridenContactInformation = "OverridenContactInformation";
            overrideInstance["ContactInformation"].StringValue = overridenContactInformation;

            string overridenKeywords = "OverridenKeywords";
            overrideInstance["Keywords"].StringValue = overridenKeywords;

            string overridenLegal = "OverridenLegal";
            overrideInstance["Legal"].StringValue = overridenLegal;

            string overridenProcessingDescription = "OverridenProcessingDescription";
            overrideInstance["ProcessingDescription"].StringValue = overridenProcessingDescription;

            string overridenDataSourceType = "OverridenDataSourceType";
            overrideInstance["DataSourceType"].StringValue = overridenDataSourceType;

            string overridenARD = "OverridenARD";
            overrideInstance["AccuracyResolutionDensity"].StringValue = overridenARD;

            string overridenResolutionInMeters = "ResolutionInMeters";
            overrideInstance["ResolutionInMeters"].StringValue = overridenResolutionInMeters;

            string overridenDataProvider = "DataProvider";
            overrideInstance["DataProvider"].StringValue = overridenDataProvider;

            string overridenDataProviderName = "DataProviderName";
            overrideInstance["DataProviderName"].StringValue = overridenDataProviderName;

            DateTime overridenDate = new DateTime(2016, 10, 31);
            overrideInstance["Date"].NativeValue = overridenDate;

            string overridenClassification = "OverridenClassification";
            overrideInstance["Classification"].StringValue = overridenClassification;

            long overridenFileSize = 89274;
            overrideInstance["FileSize"].NativeValue = overridenFileSize;

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(overridenFootprint, modifiedInstance["Footprint"].StringValue);
                Assert.AreEqual(overridenName, modifiedInstance["Name"].StringValue);
                Assert.AreEqual(overridenDescription, modifiedInstance["Description"].StringValue);
                Assert.AreEqual(overridenContactInformation, modifiedInstance["ContactInformation"].StringValue);
                Assert.AreEqual(overridenKeywords, modifiedInstance["Keywords"].StringValue);
                Assert.AreEqual(overridenLegal, modifiedInstance["Legal"].StringValue);
                Assert.AreEqual(overridenProcessingDescription, modifiedInstance["ProcessingDescription"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenARD, modifiedInstance["AccuracyResolutionDensity"].StringValue);
                Assert.AreEqual(overridenResolutionInMeters, modifiedInstance["ResolutionInMeters"].StringValue);
                Assert.AreEqual(overridenDataProvider, modifiedInstance["DataProvider"].StringValue);
                Assert.AreEqual(overridenDataProviderName, modifiedInstance["DataProviderName"].StringValue);
                Assert.AreEqual(overridenDate, modifiedInstance["Date"].NativeValue);
                Assert.AreEqual(overridenClassification, modifiedInstance["Classification"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);

                }
            }
        }
    }
