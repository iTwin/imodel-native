using System;
using System.Collections.Generic;
using System.Linq;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
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
        public void SETest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateUsgsSE(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateUsgsSE(true, m_schema);


            string overridenFootprint = "OverridenFootprint";
            overrideInstance["Footprint"].StringValue = overridenFootprint;

            string overridenName = "OverridenName";
            overrideInstance["Name"].StringValue = overridenName;

            string overridenDTA = "OverridenDTA";
            overrideInstance["DataSourceTypesAvailable"].StringValue = overridenDTA;

            string overridenAIM = "OverridenAIM";
            overrideInstance["AccuracyInMeters"].StringValue = overridenAIM;

            string overridenResolutionInMeters = "OverridenResolutionInMeters";
            overrideInstance["ResolutionInMeters"].StringValue = overridenResolutionInMeters;

            string overridenDataProvider = "OverridenDataProvider";
            overrideInstance["DataProvider"].StringValue = overridenDataProvider;

            string overridenDataProviderName = "OverridenDataProviderName";
            overrideInstance["DataProviderName"].StringValue = overridenDataProviderName;

            string overridenDataset = "OverridenDataset";
            overrideInstance["Dataset"].StringValue = overridenDataset;

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
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

                Assert.AreEqual(overridenFootprint, modifiedInstance["Footprint"].StringValue);
                Assert.AreEqual(overridenName, modifiedInstance["Name"].StringValue);
                Assert.AreEqual(overridenDTA, modifiedInstance["DataSourceTypesAvailable"].StringValue);
                Assert.AreEqual(overridenAIM, modifiedInstance["AccuracyInMeters"].StringValue);
                Assert.AreEqual(overridenResolutionInMeters, modifiedInstance["ResolutionInMeters"].StringValue);
                Assert.AreEqual(overridenDataProvider, modifiedInstance["DataProvider"].StringValue);
                Assert.AreEqual(overridenDataProviderName, modifiedInstance["DataProviderName"].StringValue);
                Assert.AreEqual(overridenDataset, modifiedInstance["Dataset"].StringValue);
                Assert.AreEqual(overridenDate, modifiedInstance["Date"].NativeValue);
                Assert.AreEqual(overridenClassification, modifiedInstance["Classification"].StringValue);

                }
            }

        [Test]
        public void MetadataTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateUsgsMetadata(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateUsgsMetadata(true, m_schema);

            string overridenMetadataURL = "OverridenMetadataURL";
            overrideInstance["MetadataURL"].StringValue = overridenMetadataURL;

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

            string overridenTermsOfUse = "OverridenTermsOfUse";
            overrideInstance["TermsOfUse"].StringValue = overridenTermsOfUse;

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
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

                Assert.AreEqual(overridenDisplayStyle, modifiedInstance["DisplayStyle"].StringValue);
                Assert.AreEqual(overridenDescription, modifiedInstance["Description"].StringValue);
                Assert.AreEqual(overridenContactInformation, modifiedInstance["ContactInformation"].StringValue);
                Assert.AreEqual(overridenKeywords, modifiedInstance["Keywords"].StringValue);
                Assert.AreEqual(overridenLegal, modifiedInstance["Legal"].StringValue);
                Assert.AreEqual(overridenTermsOfUse, modifiedInstance["TermsOfUse"].NativeValue);
                Assert.AreEqual(overridenLineage, modifiedInstance["Lineage"].NativeValue);
                Assert.AreEqual(overridenProvenance, modifiedInstance["Provenance"].StringValue);

                }
            }

        [Test]
        public void OsmSourceTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);

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

            string overridenCS = "OverridenCS";
            overrideInstance["CoordinateSystem"].StringValue = overridenCS;

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
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

                Assert.AreEqual(overridenMainURL, modifiedInstance["MainURL"].StringValue);
                Assert.AreEqual(overridenCompoundType, modifiedInstance["CompoundType"].StringValue);
                Assert.AreEqual(overridenLocationInCompound, modifiedInstance["LocationInCompound"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenSisterFiles, modifiedInstance["SisterFiles"].StringValue);
                Assert.AreEqual(overridenNDV, modifiedInstance["NoDataValue"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);
                Assert.AreEqual(overridenCS, modifiedInstance["CoordinateSystem"].StringValue);

                Assert.AreEqual(overridenAlternateURL1, modifiedInstance["AlternateURL1"].StringValue);
                Assert.AreEqual(overridenAlternateURL2, modifiedInstance["AlternateURL2"].StringValue);

                }
            }

        [Test]
        public void MultibandSourceTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);

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

            string overridenCS = "OverridenCS";
            overrideInstance["CoordinateSystem"].StringValue = overridenCS;

            string overridenRedBandURL = "OverridenRedBandURL";
            overrideInstance["RedBandURL"].StringValue = overridenRedBandURL;

            string overridenGreenBandURL = "OverridenGreenBandURL";
            overrideInstance["GreenBandURL"].StringValue = overridenGreenBandURL;

            string overridenBlueBandURL = "OverridenBlueBandURL";
            overrideInstance["BlueBandURL"].StringValue = overridenBlueBandURL;

            string overridenPanchromaticBandURL = "OverridenPanchromaticBandURL";
            overrideInstance["PanchromaticBandURL"].StringValue = overridenPanchromaticBandURL;

            long overridenRedBandFileSize = 89275;
            overrideInstance["RedBandFileSize"].NativeValue = overridenRedBandFileSize;

            long overridenGreenBandFileSize = 89276;
            overrideInstance["GreenBandFileSize"].NativeValue = overridenGreenBandFileSize;

            long overridenBlueBandFileSize = 89277;
            overrideInstance["BlueBandFileSize"].NativeValue = overridenBlueBandFileSize;

            long overridenPanchromaticBandFileSize = 89278;
            overrideInstance["PanchromaticBandFileSize"].NativeValue = overridenPanchromaticBandFileSize;

            string overridenRedBandSisterFiles = "OverridenRedBandSisterFiles";
            overrideInstance["RedBandSisterFiles"].StringValue = overridenRedBandSisterFiles;

            string overridenGreenBandSisterFiles = "OverridenGreenBandSisterFiles";
            overrideInstance["GreenBandSisterFiles"].StringValue = overridenGreenBandSisterFiles;

            string overridenBlueBandSisterFiles = "OverridenBlueBandSisterFiles";
            overrideInstance["BlueBandSisterFiles"].StringValue = overridenBlueBandSisterFiles;

            string overridenPanchromaticBandSisterFiles = "OverridenPanchromaticBandSisterFiles";
            overrideInstance["PanchromaticBandSisterFiles"].StringValue = overridenPanchromaticBandSisterFiles;

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

                Assert.AreEqual(overridenMainURL, modifiedInstance["MainURL"].StringValue);
                Assert.AreEqual(overridenCompoundType, modifiedInstance["CompoundType"].StringValue);
                Assert.AreEqual(overridenLocationInCompound, modifiedInstance["LocationInCompound"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenSisterFiles, modifiedInstance["SisterFiles"].StringValue);
                Assert.AreEqual(overridenNDV, modifiedInstance["NoDataValue"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);
                Assert.AreEqual(overridenCS, modifiedInstance["CoordinateSystem"].StringValue);

                Assert.AreEqual(overridenRedBandURL, modifiedInstance["RedBandURL"].StringValue);
                Assert.AreEqual(overridenGreenBandURL, modifiedInstance["GreenBandURL"].StringValue);
                Assert.AreEqual(overridenBlueBandURL, modifiedInstance["BlueBandURL"].StringValue);
                Assert.AreEqual(overridenPanchromaticBandURL, modifiedInstance["PanchromaticBandURL"].StringValue);

                Assert.AreEqual(overridenRedBandFileSize, modifiedInstance["RedBandFileSize"].NativeValue);
                Assert.AreEqual(overridenGreenBandFileSize, modifiedInstance["GreenBandFileSize"].NativeValue);
                Assert.AreEqual(overridenBlueBandFileSize, modifiedInstance["BlueBandFileSize"].NativeValue);
                Assert.AreEqual(overridenPanchromaticBandFileSize, modifiedInstance["PanchromaticBandFileSize"].NativeValue);

                Assert.AreEqual(overridenRedBandSisterFiles, modifiedInstance["RedBandSisterFiles"].StringValue);
                Assert.AreEqual(overridenGreenBandSisterFiles, modifiedInstance["GreenBandSisterFiles"].StringValue);
                Assert.AreEqual(overridenBlueBandSisterFiles, modifiedInstance["BlueBandSisterFiles"].StringValue);
                Assert.AreEqual(overridenPanchromaticBandSisterFiles, modifiedInstance["PanchromaticBandSisterFiles"].StringValue);
                }
            }

        [Test]
        public void ServerTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateUsgsServer(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateUsgsServer(true, m_schema);


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
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {

                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

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

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateUsgsSDS(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateUsgsSDS(true, m_schema);


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

            string overridenCS = "OverridenCS";
            overrideInstance["CoordinateSystem"].StringValue = overridenCS;

            List<IECInstance> overrideInstances = new List<IECInstance>() { overrideInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {

                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

                Assert.AreEqual(overridenMainURL, modifiedInstance["MainURL"].StringValue);
                Assert.AreEqual(overridenCompoundType, modifiedInstance["CompoundType"].StringValue);
                Assert.AreEqual(overridenLocationInCompound, modifiedInstance["LocationInCompound"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenSisterFiles, modifiedInstance["SisterFiles"].StringValue);
                Assert.AreEqual(overridenNDV, modifiedInstance["NoDataValue"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);
                Assert.AreEqual(overridenCS, modifiedInstance["CoordinateSystem"].StringValue);
                }
            }

        [Test]
        public void RelatedInstancesTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);

            IECInstance sebInstance = SetupHelpers.CreateUsgsSE(true, m_schema);
            IECInstance metadataInstance = SetupHelpers.CreateUsgsMetadata(true, m_schema);
            ((IECRelationshipClass) m_schema.GetClass("SpatialEntityToMetadata")).CreateRelationship(sebInstance, metadataInstance);

            List<IECInstance> instances = new List<IECInstance>() { sebInstance };
            IECInstance sebOverrideInstance = SetupHelpers.CreateUsgsSE(true, m_schema);

            string sebOverridenName = "sebOverridenName";

            sebOverrideInstance["Name"].StringValue = sebOverridenName;


            IECInstance metadataOverrideInstance = SetupHelpers.CreateUsgsMetadata(true, m_schema);

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
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(sebOverrideInstances);

                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                              Arg<DataReadingHelper>.Is.Anything,
                                              Arg<IParamNameValueMap>.Is.Anything,
                                              Arg<IECClass>.Is.Equal(metadataInstance.ClassDefinition),
                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                              Arg<IEnumerable<string>>.Is.Anything,
                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(metadataOverrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();
                IECInstance modifiedRelInstance = modifiedInstance.GetRelationshipInstances().First().Target;

                instanceOverrider.Modify(instances, DataSource.USGS);

                Assert.AreEqual(sebOverridenName, modifiedInstance["Name"].StringValue);
                Assert.AreEqual(metadataOverridenLegal, modifiedRelInstance["Legal"].StringValue);

                }
            }

        [Test]
        public void WMSLayerTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);

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
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

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
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {

                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

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

            string overridenCS = "OverridenCS";
            overrideInstance["CoordinateSystem"].StringValue = overridenCS;

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
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {

                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

                Assert.AreEqual(overridenMainURL, modifiedInstance["MainURL"].StringValue);
                Assert.AreEqual(overridenCompoundType, modifiedInstance["CompoundType"].StringValue);
                Assert.AreEqual(overridenLocationInCompound, modifiedInstance["LocationInCompound"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenSisterFiles, modifiedInstance["SisterFiles"].StringValue);
                Assert.AreEqual(overridenNDV, modifiedInstance["NoDataValue"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);
                Assert.AreEqual(overridenCS, modifiedInstance["CoordinateSystem"].NativeValue);

                Assert.AreEqual(overridenLayers, modifiedInstance["Layers"].StringValue);
                Assert.AreEqual(overridenStyles, modifiedInstance["Styles"].StringValue);
                Assert.AreEqual(overridenFormats, modifiedInstance["Formats"].StringValue);
                }
            }

        [Test]
        public void SEWDVTest ()
            {

            InstanceOverrider instanceOverrider = new InstanceOverrider(m_querierMock);

            List<IECInstance> instances = new List<IECInstance>() { SetupHelpers.CreateUsgsSEWDV(true, m_schema) };
            IECInstance overrideInstance = SetupHelpers.CreateUsgsSEWDV(true, m_schema);

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

            string overridenTermsOfUse = "OverridenTermsOfUse";
            overrideInstance["TermsOfUse"].StringValue = overridenTermsOfUse;

            string overridenDataSourceType = "OverridenDataSourceType";
            overrideInstance["DataSourceType"].StringValue = overridenDataSourceType;

            string overridenAIM = "OverridenAIM";
            overrideInstance["AccuracyInMeters"].StringValue = overridenAIM;

            double overridenCC = 12.3;
            overrideInstance["Occlusion"].DoubleValue = overridenCC;

            string overridenResolutionInMeters = "ResolutionInMeters";
            overrideInstance["ResolutionInMeters"].StringValue = overridenResolutionInMeters;

            string overridenDataProvider = "DataProvider";
            overrideInstance["DataProvider"].StringValue = overridenDataProvider;

            string overridenDataProviderName = "DataProviderName";
            overrideInstance["DataProviderName"].StringValue = overridenDataProviderName;

            string overridenDataset = "Dataset";
            overrideInstance["Dataset"].StringValue = overridenDataset;

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
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(overrideInstances);
                }
            using ( m_mock.Playback() )
                {
                IECInstance modifiedInstance = instances.First();

                instanceOverrider.Modify(instances, DataSource.USGS);

                Assert.AreEqual(overridenFootprint, modifiedInstance["Footprint"].StringValue);
                Assert.AreEqual(overridenName, modifiedInstance["Name"].StringValue);
                Assert.AreEqual(overridenDescription, modifiedInstance["Description"].StringValue);
                Assert.AreEqual(overridenContactInformation, modifiedInstance["ContactInformation"].StringValue);
                Assert.AreEqual(overridenKeywords, modifiedInstance["Keywords"].StringValue);
                Assert.AreEqual(overridenLegal, modifiedInstance["Legal"].StringValue);
                Assert.AreEqual(overridenTermsOfUse, modifiedInstance["TermsOfUse"].StringValue);
                Assert.AreEqual(overridenDataSourceType, modifiedInstance["DataSourceType"].StringValue);
                Assert.AreEqual(overridenAIM, modifiedInstance["AccuracyInMeters"].StringValue);
                Assert.AreEqual(overridenCC, modifiedInstance["Occlusion"].DoubleValue);
                Assert.AreEqual(overridenResolutionInMeters, modifiedInstance["ResolutionInMeters"].StringValue);
                Assert.AreEqual(overridenDataProvider, modifiedInstance["DataProvider"].StringValue);
                Assert.AreEqual(overridenDataProviderName, modifiedInstance["DataProviderName"].StringValue);
                Assert.AreEqual(overridenDataset, modifiedInstance["Dataset"].StringValue);
                Assert.AreEqual(overridenDate, modifiedInstance["Date"].NativeValue);
                Assert.AreEqual(overridenClassification, modifiedInstance["Classification"].StringValue);
                Assert.AreEqual(overridenFileSize, modifiedInstance["FileSize"].NativeValue);

                }
            }
        }
    }
