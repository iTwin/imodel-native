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

        static public IECInstance CreateSEWDV (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialEntityWithDetailsView");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "543e6b86e4b0fd76af69cf4c");
            instance.ExtendedDataValueSetter.Add("Complete", complete);

            return instance;
            }

        static public IECInstance CreateSEB (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialEntityBase");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["Footprint"].StringValue = "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }";
            instance["Name"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015";
            instance["Keywords"].StringValue = "elevation, Elevation, National Elevation Dataset, NED, Elevation, Light Detection and Ranging, LIDAR, High Resolution, Topographic Surface, Topography, Bare Earth, Hydro-Flattened, Terrain Elevation, Cartography, DEM, Digital Elevation Model, Digital Mapping, Digital Terrain Model, Geodata, GIS, Mapping, Raster, USGS, U.S. Geological Survey, 10,000 meter DEM, 1 meter DEM, Downloadable Data, Elevation, Digital Elevation Model (DEM) 1 meter, 10000 x 10000 meter, IMG, US, United States";
            instance["DataSourceTypesAvailable"].StringValue = "IMG";
            //instance["CloudCoverage"].DoubleValue = 0;
            instance["DataProvider"].StringValue = "USGS";
            instance["DataProviderName"].StringValue = "United States Geological Survey";
            instance["Classification"].StringValue = "Terrain";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "543e6b86e4b0fd76af69cf4c");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateSpatialEntity (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialEntity");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["Footprint"].StringValue = "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }";
            instance["Name"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015";
            instance["Keywords"].StringValue = "elevation, Elevation, National Elevation Dataset, NED, Elevation, Light Detection and Ranging, LIDAR, High Resolution, Topographic Surface, Topography, Bare Earth, Hydro-Flattened, Terrain Elevation, Cartography, DEM, Digital Elevation Model, Digital Mapping, Digital Terrain Model, Geodata, GIS, Mapping, Raster, USGS, U.S. Geological Survey, 10,000 meter DEM, 1 meter DEM, Downloadable Data, Elevation, Digital Elevation Model (DEM) 1 meter, 10000 x 10000 meter, IMG, US, United States";
            instance["DataSourceTypesAvailable"].StringValue = "IMG";
            //instance["CloudCoverage"].DoubleValue = 0;
            instance["DataProvider"].StringValue = "USGS";
            instance["DataProviderName"].StringValue = "United States Geological Survey";
            instance["Classification"].StringValue = "Terrain";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "543e6b86e4b0fd76af69cf4c");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateParentSED (IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialEntityDataset");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "543e6b86e4b0fd76af69cf4c";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "543e6b86e4b0fd76af69cf4c";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "4f552e93e4b018de15819c51");
            instance.ExtendedDataValueSetter.Add("Complete", true);

            return instance;
            }

        static public IECInstance CreateMetadata (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("Metadata");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["RawMetadataFormat"].StringValue = "FGDC";
            instance["Description"].StringValue = "CachedValueTest";
            instance["Keywords"].StringValue = "CachedValueTest";
            instance["Legal"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateThumbnail (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("Thumbnail");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["ThumbnailFormat"].StringValue = "jpg";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateSDS (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("SpatialDataSource");
            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = "553690bfe4b0b22a15807df2";
            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["MainURL"].StringValue = "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip";
            instance["CompoundType"].StringValue = "USGS";
            instance["LocationInCompound"].StringValue = "Unknown";
            instance["DataSourceType"].StringValue = "IMG";
            instance["FileSize"].StringValue = "256093";
            //instance["NoDataValue"].StringValue = "0";
            instance["Metadata"].StringValue = "https://www.sciencebase.gov/catalog/file/get/553690bfe4b0b22a15807df2?f=__disk__d0%2F20%2Fa5%2Fd020a57f42c6a948f52f567a25858aa87b4e7f50";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateOsmSource (IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("OsmSource");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["MainURL"].StringValue = "OSM_URL";
            instance["CompoundType"].StringValue = "OSM";
            instance["LocationInCompound"].StringValue = "";
            instance["DataSourceType"].StringValue = "OSM";
            instance["SisterFiles"].StringValue = "SisterFiles";
            instance["FileSize"].NativeValue = (long) 0;
            instance["Metadata"].StringValue = "Metadata";

            instance["AlternateURL1"].StringValue = "AlternateURL1";
            instance["AlternateURL2"].StringValue = "AlternateURL2";

            return instance;
            }

        static public IECInstance CreateMultibandSource (IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("MultibandSource");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["MainURL"].StringValue = "OSM_URL";
            instance["CompoundType"].StringValue = "OSM";
            instance["LocationInCompound"].StringValue = "";
            instance["DataSourceType"].StringValue = "OSM";
            instance["SisterFiles"].StringValue = "SisterFiles";
            instance["FileSize"].NativeValue = (long) 0;
            instance["Metadata"].StringValue = "Metadata";

            instance["RedBandURL"].StringValue = "RedBandURL";
            instance["GreenBandURL"].StringValue = "GreenBandURL";
            instance["BlueBandURL"].StringValue = "BlueBandURL";

            return instance;
            }

        static public IECInstance CreateWMSSource (IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("WMSSource");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["MainURL"].StringValue = "MainURL";
            instance["CompoundType"].StringValue = "CompoundType";
            instance["LocationInCompound"].StringValue = "LocationInCompound";
            instance["DataSourceType"].StringValue = "DataSourceType";
            instance["SisterFiles"].StringValue = "SisterFiles";
            instance["FileSize"].NativeValue = (long) 0;
            instance["Metadata"].StringValue = "Metadata";

            instance["Layers"].StringValue = "Layers";
            instance["Styles"].StringValue = "Styles";
            instance["Formats"].StringValue = "Formats";

            return instance;
            }

        static public IECInstance CreateServer (bool complete, IECSchema schema)
            {
            IECClass ecClass = schema.GetClass("Server");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["CommunicationProtocol"].StringValue = "ftp";
            instance["URL"].StringValue = "ftp://rockyftp.cr.usgs.gov";
            instance["Fees"].StringValue = "None. No fees are applicable for obtaining the data set.";
            instance["Legal"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateWMSServer (IECSchema schema)
            {

            IECClass ecClass = schema.GetClass("WMSServer");
            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            InitializePropertiesToNull(instance, ecClass);

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["CommunicationProtocol"].StringValue = "ftp";
            instance["URL"].StringValue = "ftp://rockyftp.cr.usgs.gov";
            instance["Fees"].StringValue = "None. No fees are applicable for obtaining the data set.";
            instance["Legal"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey";

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

        static public IECInstance CreateWMSLayer(IECSchema schema)
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

        static private void InitializePropertiesToNull (IECInstance instance, IECClass ecClass)
            {
            foreach ( IECProperty prop in ecClass )
                {
                instance[prop.Name].SetToNull();
                }
            }

        }
    }
