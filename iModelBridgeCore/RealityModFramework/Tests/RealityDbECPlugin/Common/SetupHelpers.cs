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
            IECInstance instance = schema.GetClass("SpatialEntityWithDetailsView").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            foreach ( IECProperty prop in schema.GetClass("SpatialEntityWithDetailsView") )
                {
                instance[prop.Name].SetToNull();
                }

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "543e6b86e4b0fd76af69cf4c");
            instance.ExtendedDataValueSetter.Add("Complete", complete);

            return instance;
            }

        static public IECInstance CreateSEB (bool complete, IECSchema schema)
            {
            IECInstance instance = schema.GetClass("SpatialEntityBase").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";
            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["Footprint"].StringValue = "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }";
            instance["Name"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015";
            instance["Keywords"].StringValue = "elevation, Elevation, National Elevation Dataset, NED, Elevation, Light Detection and Ranging, LIDAR, High Resolution, Topographic Surface, Topography, Bare Earth, Hydro-Flattened, Terrain Elevation, Cartography, DEM, Digital Elevation Model, Digital Mapping, Digital Terrain Model, Geodata, GIS, Mapping, Raster, USGS, U.S. Geological Survey, 10,000 meter DEM, 1 meter DEM, Downloadable Data, Elevation, Digital Elevation Model (DEM) 1 meter, 10000 x 10000 meter, IMG, US, United States";
            instance["AssociateFile"].SetToNull();
            instance["ProcessingDescription"].SetToNull();
            instance["DataSourceTypesAvailable"].StringValue = "IMG";
            instance["AccuracyResolutionDensity"].SetToNull();
            instance["ResolutionInMeters"].SetToNull();
            instance["DataProvider"].StringValue = "USGS";
            instance["DataProviderName"].StringValue = "United States Geological Survey";
            instance["Date"].SetToNull();
            instance["Classification"].StringValue = "Terrain";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "543e6b86e4b0fd76af69cf4c");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateSpatialEntity (bool complete, IECSchema schema)
            {
            IECInstance instance = schema.GetClass("SpatialEntity").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";
            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["Footprint"].StringValue = "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }";
            instance["Name"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015";
            instance["Keywords"].StringValue = "elevation, Elevation, National Elevation Dataset, NED, Elevation, Light Detection and Ranging, LIDAR, High Resolution, Topographic Surface, Topography, Bare Earth, Hydro-Flattened, Terrain Elevation, Cartography, DEM, Digital Elevation Model, Digital Mapping, Digital Terrain Model, Geodata, GIS, Mapping, Raster, USGS, U.S. Geological Survey, 10,000 meter DEM, 1 meter DEM, Downloadable Data, Elevation, Digital Elevation Model (DEM) 1 meter, 10000 x 10000 meter, IMG, US, United States";
            instance["AssociateFile"].SetToNull();
            instance["ProcessingDescription"].SetToNull();
            instance["DataSourceTypesAvailable"].StringValue = "IMG";
            instance["AccuracyResolutionDensity"].SetToNull();
            instance["ResolutionInMeters"].SetToNull();
            instance["DataProvider"].StringValue = "USGS";
            instance["DataProviderName"].StringValue = "United States Geological Survey";
            instance["Date"].SetToNull();
            instance["Classification"].StringValue = "Terrain";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "543e6b86e4b0fd76af69cf4c");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateParentSED (IECSchema schema)
            {
            IECInstance instance = schema.GetClass("SpatialEntityDataset").CreateInstance();
            instance.InstanceId = "543e6b86e4b0fd76af69cf4c";
            instance["Id"].StringValue = "543e6b86e4b0fd76af69cf4c";

            instance["Footprint"].SetToNull();
            instance["Name"].SetToNull();
            instance["Keywords"].SetToNull();
            instance["AssociateFile"].SetToNull();
            instance["ProcessingDescription"].SetToNull();
            instance["DataSourceTypesAvailable"].SetToNull();
            instance["AccuracyResolutionDensity"].SetToNull();
            instance["ResolutionInMeters"].SetToNull();
            instance["DataProvider"].SetToNull();
            instance["DataProviderName"].SetToNull();
            instance["Date"].SetToNull();
            instance["Classification"].SetToNull();
            instance["Processable"].SetToNull();

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", "4f552e93e4b018de15819c51");
            instance.ExtendedDataValueSetter.Add("Complete", true);

            return instance;
            }

        static public IECInstance CreateMetadata (bool complete, IECSchema schema)
            {
            IECInstance instance = schema.GetClass("Metadata").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";
            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["RawMetadata"].SetToNull();
            instance["RawMetadataFormat"].StringValue = "FGDC";
            instance["DisplayStyle"].SetToNull();
            instance["Description"].StringValue = "CachedValueTest";
            instance["ContactInformation"].SetToNull();
            instance["Keywords"].StringValue = "CachedValueTest";
            instance["Legal"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey";
            instance["Lineage"].SetToNull();
            instance["Provenance"].SetToNull();

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateThumbnail (bool complete, IECSchema schema)
            {
            IECInstance instance = schema.GetClass("Thumbnail").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";
            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["ThumbnailProvenance"].SetToNull();
            instance["ThumbnailFormat"].StringValue = "jpg";
            instance["ThumbnailWidth"].SetToNull();
            instance["ThumbnailHeight"].SetToNull();
            instance["ThumbnailStamp"].SetToNull();
            instance["ThumbnailGenerationDetails"].SetToNull();

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateSDS (bool complete, IECSchema schema)
            {
            IECInstance instance = schema.GetClass("SpatialDataSource").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";
            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["MainURL"].StringValue = "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip";
            instance["CompoundType"].StringValue = "USGS";
            instance["LocationInCompound"].StringValue = "Unknown";
            instance["DataSourceType"].StringValue = "IMG";
            instance["SisterFiles"].SetToNull();
            instance["FileSize"].StringValue = "256093";
            instance["Metadata"].StringValue = "https://www.sciencebase.gov/catalog/file/get/553690bfe4b0b22a15807df2?f=__disk__d0%2F20%2Fa5%2Fd020a57f42c6a948f52f567a25858aa87b4e7f50";

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateOsmSource (IECSchema schema)
            {
            IECInstance instance = schema.GetClass("OsmSource").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

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

        static public IECInstance CreateWMSSource (IECSchema schema)
            {
            IECInstance instance = schema.GetClass("WMSSource").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";
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
            IECInstance instance = schema.GetClass("Server").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";
            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["CommunicationProtocol"].StringValue = "ftp";
            instance["Name"].SetToNull();
            instance["URL"].StringValue = "ftp://rockyftp.cr.usgs.gov";
            instance["ServerContactInformation"].SetToNull();
            instance["Fees"].StringValue = "None. No fees are applicable for obtaining the data set.";
            instance["Legal"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey";
            instance["AccessConstraints"].SetToNull();
            instance["Online"].SetToNull();
            instance["LastCheck"].SetToNull();
            instance["LastTimeOnline"].SetToNull();
            instance["Latency"].SetToNull();
            instance["MeanReachabilityStats"].SetToNull();
            instance["State"].SetToNull();
            instance["Type"].SetToNull();

            instance.ExtendedDataValueSetter.Add("IsFromCacheTest", "IsFromCacheTest");
            instance.ExtendedDataValueSetter.Add("Complete", complete);
            return instance;
            }

        static public IECInstance CreateWMSServer (IECSchema schema)
            {
            IECInstance instance = schema.GetClass("WMSServer").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";
            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["CommunicationProtocol"].StringValue = "ftp";
            instance["Name"].SetToNull();
            instance["URL"].StringValue = "ftp://rockyftp.cr.usgs.gov";
            instance["ServerContactInformation"].SetToNull();
            instance["Fees"].StringValue = "None. No fees are applicable for obtaining the data set.";
            instance["Legal"].StringValue = "USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015 courtesy of the U.S. Geological Survey";
            instance["AccessConstraints"].SetToNull();
            instance["Online"].SetToNull();
            instance["LastCheck"].SetToNull();
            instance["LastTimeOnline"].SetToNull();
            instance["Latency"].SetToNull();
            instance["MeanReachabilityStats"].SetToNull();
            instance["State"].SetToNull();
            instance["Type"].SetToNull();

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
            IECInstance instance = schema.GetClass("WMSLayer").CreateInstance();
            instance.InstanceId = "553690bfe4b0b22a15807df2";

            instance["Id"].StringValue = "553690bfe4b0b22a15807df2";
            instance["Name"].StringValue = "Name";
            instance["Title"].StringValue = "Title";
            instance["Description"].StringValue = "Description";
            instance["BoundingBox"].StringValue = "BoundingBox";
            instance["CoordinateSystems"].StringValue = "CoordinateSystems";

            return instance;
            }

        }
    }
