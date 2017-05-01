using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECObjects.XML;

namespace IndexECPlugin.Tests.Common
    {
    internal static class SetupHelpers
        {
        static public IECSchema PrepareSchema()
            {
            ECSchemaXmlStreamReader schemaReader = new ECSchemaXmlStreamReader(Assembly.GetAssembly(typeof(IndexECPlugin.Source.IndexECPlugin)).GetManifestResourceStream("ECSchemaDB.xml"));
            return schemaReader.Deserialize();
            }

        static public IECInstance CreateUsgsSEWDV (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialEntityWithDetailsView");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["Footprint"].StringValue = "{ \"points\" : [[-180,-90],[-180,90],[180,90],[180,-90],[-180,-90]], \"coordinate_system\" : \"4326\" }";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);

            return instance;
            }

        static public IECInstance CreateIndexSEWDV (IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialEntityWithDetailsView");
            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = "1";
            instance["Id"].StringValue = "1";
            instance["Footprint"].StringValue = "{ \"points\" : [[-180,-90],[-180,90],[180,90],[180,-90],[-180,-90]], \"coordinate_system\" : \"4326\" }";
            instance["ApproximateFootprint"].NativeValue = false;
            instance["Name"].StringValue = "TestEntity";
            instance["Description"].StringValue = "Description";
            instance["ContactInformation"].StringValue = "ContactInformation";
            instance["Keywords"].StringValue = "Keywords";
            instance["Legal"].StringValue = "Legal";
            instance["TermsOfUse"].StringValue = "TermsOfUse";
            instance["DataSourceType"].StringValue = "tif";
            instance["AccuracyInMeters"].StringValue = "TestAccuracy";
            instance["ResolutionInMeters"].StringValue = "TestResolution";
            instance["ThumbnailURL"].StringValue = "ThumbnailURL";
            instance["DataProvider"].StringValue = "DataProvider";
            instance["DataProviderName"].StringValue = "DataProviderName";
            instance["Dataset"].StringValue = "DatasetVal";
            instance["Date"].NativeValue = new DateTime(2012, 01, 01);
            instance["Classification"].StringValue = "Terrain";
            instance["FileSize"].NativeValue = (long) 123;
            instance["Streamed"].NativeValue = false;
            instance["SpatialDataSourceId"].StringValue = "1";
            instance["Occlusion"].DoubleValue = 1.23;

            return instance;
            }

        static public IECInstance CreateUsgsSE (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialEntity");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["Footprint"].StringValue = "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }";
            instance["ApproximateFootprint"].NativeValue = false;
            instance["Name"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015";
            instance["DataSourceTypesAvailable"].StringValue = "IMG";
            instance["DataProvider"].StringValue = "USGS";
            instance["DataProviderName"].StringValue = "United States Geological Survey";
            instance["Classification"].StringValue = "Terrain";
            instance["ThumbnailURL"].StringValue = "https://prd-tnm.s3.amazonaws.com/StagedProducts/Elevation/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg";
            instance["Dataset"].StringValue = "Digital Elevation Model (DEM) 1 meter";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

			static public IECInstance CreateIndexSE(IECSchema schema)
			{
			IECClass ecClass = schema.GetClass("SpatialEntity");
            IECInstance instance = ecClass.CreateInstance();
			
			InitializePropertiesToNull(instance, ecClass);
			
			instance.InstanceId = "1";
            instance["Id"].StringValue = "1";
            instance["Footprint"].StringValue = "{ \"points\" : [[-180,-90],[-180,90],[180,90],[180,-90],[-180,-90]], \"coordinate_system\" : \"4326\" }";
            instance["ApproximateFootprint"].NativeValue = false;
            instance["Name"].StringValue = "TestEntity";
            instance["DataSourceTypesAvailable"].StringValue = "tif";
            instance["AccuracyInMeters"].StringValue = "TestAccuracy";
            instance["ResolutionInMeters"].StringValue = "TestResolution";
            instance["ThumbnailURL"].StringValue = "ThumbnailURL";
            instance["ThumbnailLoginKey"].StringValue = "ThumbnailLoginKey";
            instance["DataProvider"].StringValue = "DataProvider";
            instance["DataProviderName"].StringValue = "DataProviderName";
            instance["Dataset"].StringValue = "DatasetVal";
            instance["Date"].NativeValue = new DateTime(2012, 01, 01);
            instance["Classification"].StringValue = "Terrain";
            instance["Occlusion"].DoubleValue = 1.23;

			    return instance;
			}
			
        static public IECInstance CreateUsgsMetadata (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("Metadata");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["Description"].StringValue = "CachedValueTest";
            instance["Keywords"].StringValue = "CachedValueTest";
            instance["Legal"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey";
            instance["TermsOfUse"].StringValue = "https://www2.usgs.gov/laws/info_policies.html";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

		static public IECInstance CreateIndexMetadata (IECSchema schema)
		{
            IECClass ecClass = schema.GetClass("Metadata");
            IECInstance instance = ecClass.CreateInstance();
			
			InitializePropertiesToNull(instance, ecClass);
			
            instance.InstanceId = "1";
            instance["Id"].StringValue = "1";
            instance["MetadataURL"].StringValue = "MetadataURL";
            instance["DisplayStyle"].StringValue = "DisplayStyle";
            instance["Description"].StringValue = "Description";
            instance["ContactInformation"].StringValue = "ContactInformation";
            instance["Keywords"].StringValue = "Keywords";
            instance["Legal"].StringValue = "Legal";
            instance["TermsOfUse"].StringValue = "TermsOfUse";
            instance["Lineage"].StringValue = "Lineage";
            instance["Provenance"].StringValue = "Provenance";

		    return instance;
		}
			
        static public IECInstance CreateUsgsSDS (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialDataSource");
            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = "553690bfe4b0b22a15807df2";
            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["MainURL"].StringValue = "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip";
            instance["CompoundType"].StringValue = "USGS";
            instance["DataSourceType"].StringValue = "IMG";
            instance["FileSize"].StringValue = "256093";
            instance["CoordinateSystem"].StringValue = "EPSG:4326";
            //instance["NoDataValue"].StringValue = "0";
            instance["Metadata"].StringValue = "https://www.sciencebase.gov/catalog/file/get/553690bfe4b0b22a15807df2?f=__disk__d0%2F20%2Fa5%2Fd020a57f42c6a948f52f567a25858aa87b4e7f50";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateIndexSDS (IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialDataSource");
            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);
			
            instance.InstanceId = "1";
            instance["Id"].StringValue = "1";
            instance["MainURL"].StringValue = "www.test.com?$(MINLONG)&$(MAXLONG)&$(MINLAT)&$(MAXLAT)&$(EMAIL_ADDRESS)&$(TARGET_GCS)";
            instance["ParameterizedURL"].NativeValue = true;
			instance["CompoundType"].StringValue = "Zip";
			instance["LocationInCompound"].StringValue = "Location/In/Compound";
			instance["DataSourceType"].StringValue = "tif";
			instance["SisterFiles"].StringValue = "SisterFiles";
			instance["NoDataValue"].StringValue = "0255385";
			instance["FileSize"].NativeValue = (long)6723523;
			instance["CoordinateSystem"].StringValue = "EPSG:1234";
			instance["Streamed"].NativeValue = false;

                return instance;

            }
			
        static public IECInstance CreateOsmSource (IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("OsmSource");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = "1";
            instance["Id"].StringValue = "1";
            instance["MainURL"].StringValue = "www.test.com";
            instance["ParameterizedURL"].NativeValue = true;
            instance["CompoundType"].StringValue = "Zip";
            instance["LocationInCompound"].StringValue = "Location/In/Compound";
            instance["DataSourceType"].StringValue = "OSM";
            instance["SisterFiles"].StringValue = "SisterFilesVal";
            instance["NoDataValue"].StringValue = "0255385";
            instance["FileSize"].NativeValue = (long) 6723523;
            instance["CoordinateSystem"].StringValue = "EPSG:1234";
            instance["Streamed"].NativeValue = false;

            instance["AlternateURL1"].StringValue = "www.AlternateURL1.com";
            instance["AlternateURL2"].StringValue = "www.AlternateURL2.com";

            return instance;
            }

        static public IECInstance CreateMultibandSource(IECSchema schema)
            {
                IECClass ecClass = schema.GetClass("MultibandSource");
                IECInstance instance = ecClass.CreateInstance();
                instance.InstanceId = "1";

                InitializePropertiesToNull(instance, ecClass);

                instance.InstanceId = "1";
                instance["Id"].StringValue = "1";
                instance["MainURL"].StringValue = "www.mainurl.com";
                //instance["ParameterizedURL"].StringValue = "1";
                instance["CompoundType"].StringValue = "Zip";
                instance["LocationInCompound"].StringValue = "Location/In/Compound";
                instance["DataSourceType"].StringValue = "tif";
                instance["SisterFiles"].StringValue = "SisterFilesVal";
                instance["NoDataValue"].StringValue = "0255385";
                instance["FileSize"].NativeValue = (long) 6723523;
                instance["CoordinateSystem"].StringValue = "EPSG:1234";
                instance["Streamed"].NativeValue = false;

                instance["RedBandURL"].StringValue = "RedBandURLVal";
                instance["RedBandFileSize"].NativeValue = (long) 6723524;
                instance["RedBandSisterFiles"].StringValue = "RedBandSisterFilesVal";
                instance["GreenBandURL"].StringValue = "GreenBandURLVal";
                instance["GreenBandFileSize"].NativeValue = (long) 6723525;
                instance["GreenBandSisterFiles"].StringValue = "GreenBandSisterFilesVal";
                instance["BlueBandURL"].StringValue = "BlueBandURLVal";
                instance["BlueBandFileSize"].NativeValue = (long) 6723526;
                instance["BlueBandSisterFiles"].StringValue = "BlueBandSisterFilesVal";
                instance["PanchromaticBandURL"].StringValue = "PanchromaticBandURLVal";
                instance["PanchromaticBandFileSize"].NativeValue = (long) 6723527;
                instance["PanchromaticBandSisterFiles"].StringValue = "PanchromaticBandSisterFilesVal";

                return instance;
            }

            static public IECInstance CreateWMSSource(IECSchema schema)
                {
                IECClass ecClass = schema.GetClass("WMSSource");
                IECInstance instance = ecClass.CreateInstance();
                instance.InstanceId = "1";

                InitializePropertiesToNull(instance, ecClass);

                instance.InstanceId = "1";
                instance["Id"].StringValue = "1";
                instance["MainURL"].StringValue = "www.mainurl.com";
                //instance["ParameterizedURL"].StringValue = "1";
                instance["CompoundType"].StringValue = "Zip";
                instance["LocationInCompound"].StringValue = "Location/In/Compound";
                instance["DataSourceType"].StringValue = "tif";
                instance["SisterFiles"].StringValue = "SisterFiles";
                instance["NoDataValue"].StringValue = "0255385";
                instance["FileSize"].NativeValue = (long) 6723523;
                instance["CoordinateSystem"].StringValue = "EPSG:1234";
                instance["Streamed"].NativeValue = false;

                instance["Layers"].StringValue = "Layers";
                instance["Styles"].StringValue = "Styles";
                instance["Formats"].StringValue = "Formats";

                return instance;
                }

            static public IECInstance CreateUsgsServer(bool complete, IECSchema schema)
                {
                IECClass ecClass = schema.GetClass("Server");
                IECInstance instance = ecClass.CreateInstance();
                instance.InstanceId = "553690bfe4b0b22a15807df2";

                InitializePropertiesToNull(instance, ecClass);

                instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
                instance["CommunicationProtocol"].StringValue = "ftp";
                instance["URL"].StringValue = "ftp://rockyftp.cr.usgs.gov";
                instance["Fees"].StringValue = "None. No fees are applicable for obtaining the data set.";
                instance["Legal"].StringValue =
                    "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey";

                instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
                instance.ExtendedDataValueSetter.Add("Complete", complete);
                return instance;
                }

            static public IECInstance CreateIndexServer (IECSchema schema)
		        {
            IECClass ecClass = schema.GetClass("Server");
            IECInstance instance = ecClass.CreateInstance();
			
			InitializePropertiesToNull(instance, ecClass);
			
            instance.InstanceId = "1";
            instance["Id"].StringValue = "1";
            instance["CommunicationProtocol"].StringValue = "http";
            instance["Streamed"].NativeValue = false;
            instance["LoginKey"].StringValue = "KeyVal";
            instance["LoginMethod"].StringValue = "MethodVal";
            instance["RegistrationPage"].StringValue = "www.registrationpage.com";
            instance["OrganisationPage"].StringValue = "www.organisationpage.com";
            instance["Name"].StringValue = "NameVal";
            instance["URL"].StringValue = "www.server.com";
            instance["ServerContactInformation"].StringValue = "ServerContactInfo@Bob.com";
            instance["Fees"].StringValue = "FeesVal";
            instance["Legal"].StringValue = "LegalVal";
            instance["AccessConstraints"].StringValue = "ConstraintsVal";
            instance["Online"].NativeValue = false;
            instance["LastCheck"].NativeValue = new DateTime(2012, 01, 01);
            instance["LastTimeOnline"].NativeValue = new DateTime(2012, 01, 01);
            instance["Latency"].DoubleValue = 1.234;
            instance["MeanReachabilityStats"].IntValue = 1235;
            instance["State"].StringValue = "StateVal";
            instance["Type"].StringValue = "TypeVal";

		    return instance;

		        }
			
        static public IECInstance CreateWMSServer (IECSchema schema)
            {

            IECClass ecClass = schema.GetClass("WMSServer");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "1";

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = "1";
            instance["Id"].StringValue = "1";
            instance["CommunicationProtocol"].StringValue = "http";
            instance["Streamed"].NativeValue = false;
            instance["LoginKey"].StringValue = "Key";
            instance["LoginMethod"].StringValue = "Method";
            instance["RegistrationPage"].StringValue = "www.registrationpage.com";
            instance["OrganisationPage"].StringValue = "www.organisationpage.com";
            instance["Name"].StringValue = "Name";
            instance["URL"].StringValue = "www.server.com";
            instance["ServerContactInformation"].StringValue = "ServerContactInfo@Bob.com";
            instance["Fees"].StringValue = "Fees";
            instance["Legal"].StringValue = "Legal";
            instance["AccessConstraints"].StringValue = "Constraints";
            instance["Online"].NativeValue = false;
            instance["LastCheck"].NativeValue = new DateTime(2012, 01, 01);
            instance["LastTimeOnline"].NativeValue = new DateTime(2012, 01, 01);
            instance["Latency"].DoubleValue = 1.234;
            instance["MeanReachabilityStats"].IntValue = 1235;
            instance["State"].StringValue = "State";
            instance["Type"].StringValue = "Type";

            instance["Title"].StringValue = "Title";
            instance["Description"].StringValue = "Description";
            instance["NumLayers"].IntValue = 1;
            instance["Version"].StringValue = "Version";
            instance["GetCapabilitiesURL"].StringValue = "GetCapabilitiesURL";
            instance["SupportedFormats"].StringValue = "SupportedFormats";
            instance["GetMapURL"].StringValue = "GetMapURL";
            instance["GetMapURLQuery"].StringValue = "GetMapURLQuery";

            return instance;
            }

        static public IECInstance CreateWMSLayer (IECSchema schema)
            {

            IECClass ecClass = schema.GetClass("WMSLayer");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["Name"].StringValue = "Name";
            instance["Title"].StringValue = "Title";
            instance["Description"].StringValue = "Description";
            instance["BoundingBox"].StringValue = "BoundingBox";
            instance["CoordinateSystems"].StringValue = "CoordinateSystems";

            return instance;
            }

        static public void CreateSE_MetadataRel(IECInstance seInst, IECInstance metadataInst, IECSchema schema)
            {
            IECRelationshipClass relClass = schema.GetClass("SpatialEntityToMetadata") as IECRelationshipClass;

            relClass.CreateRelationship(seInst, metadataInst);
            }

        static public void CreateSE_SDSRel (IECInstance seInst, IECInstance sdsInst, IECSchema schema)
            {
            IECRelationshipClass relClass = schema.GetClass("SpatialEntityToSpatialDataSource") as IECRelationshipClass;

            relClass.CreateRelationship(seInst, sdsInst);
            }

        static public void CreateServer_SDSRel (IECInstance serverInst, IECInstance sdsInst, IECSchema schema)
            {
            IECRelationshipClass relClass = schema.GetClass("ServerToSpatialDataSource") as IECRelationshipClass;

            relClass.CreateRelationship(serverInst, sdsInst);
            }

        static private void InitializePropertiesToNull (IECInstance instance, IECClass ecClass)
            {
            foreach ( IECProperty prop in ecClass )
                {
                instance[prop.Name].SetToNull();
                }
            }

        }
    }
