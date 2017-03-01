using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using IndexECPlugin.Source.Helpers;
using Newtonsoft.Json.Linq;

namespace IndexECPlugin.Source.QueryProviders
    {
    /// <summary>
    /// AustraliaAPIQueryProvider's purpose is to transform Australian API data into ECInstances for our plugin.
    /// </summary>
    public class AustraliaAPIQueryProvider : SubAPIQueryProvider
        {
        Tuple<string, JObject, AustraliaLayer> m_jsonCache;
        IAustraliaDataFetcher m_AustraliaDataFetcher;

        /// <summary>
        /// AustraliaAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="australiaDataFetcher">The IAustraliaDataFetcher that will be used to query the API</param>
        /// <param name="cacheManager">The cache manager that will be used to access the cache in the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        public AustraliaAPIQueryProvider (ECQuery query, ECQuerySettings querySettings, IAustraliaDataFetcher australiaDataFetcher, IInstanceCacheManager cacheManager, IECSchema schema)
            : base(query, querySettings, cacheManager, schema, DataSource.AU)
            {
            m_AustraliaDataFetcher = australiaDataFetcher;
            }

        /// <summary>
        /// AustraliaAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="connectionString">The connection string that will be used to access the cache in the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        public AustraliaAPIQueryProvider (ECQuery query, ECQuerySettings querySettings, string connectionString, IECSchema schema)
            : base(query, querySettings, connectionString, schema, DataSource.AU, false)
            {
            m_AustraliaDataFetcher = new AustraliaDataFetcher();
            }

        /// <summary>
        /// Query a specific SpatialEntityWithDetailsView by its ID. This method has the responsibility to prepare the instances (SpatialEntity, Metadata and SpatialDataSource) for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialEntityWithDetailsView instance</returns>
        override protected IECInstance QuerySingleSpatialEntityWithDetailsView (string sourceID)
            {
            AustraliaLayer layer;

            JObject json = GetJsonWithCache(sourceID, out layer);
            JObject feature = ExtractFirstFeature(json);

            return CreateSEWDV(sourceID, layer, json, feature);
            }

        private IECInstance CreateSEWDV (string sourceID, AustraliaLayer layer, JObject json, JObject feature)
            
            {
            IECClass ecClass = Schema.GetClass("SpatialEntityWithDetailsView");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();


            JToken attributes = feature["attributes"];
            if ( attributes == null )
                {
                throw new OperationFailedException("An entry in the Geoscience Australia API is corrupted.");
                }

            instance.InstanceId = sourceID;

            instance["Id"].StringValue = sourceID;

            string wkid = json["spatialReference"] == null ? null : json["spatialReference"].TryToGetString("latestWkid");

            try
                {
                if ( json.TryToGetString("geometryType") == "esriGeometryPolygon" && wkid != null )
                    {
                    JToken geometry = feature["geometry"];
                    instance["Footprint"].StringValue = ExtractEsriBBox(geometry, wkid);
                    }
                }
            catch (Exception)
                {
                //This part could fail if the geometry is too big. In that case, I prefer to prevent crashes and let the footprint be null
                }
            instance["ApproximateFootprint"].NativeValue = false;

            KeyValuePair<string, string> nameField = layer.FieldMap.FirstOrDefault(f => f.Key == "Name");
            if ( !nameField.Equals(default(KeyValuePair<string, string>)) )
                {
                instance["Name"].StringValue = attributes.TryToGetString(nameField.Value);
                }

            //instance["Description"].StringValue = 
            //instance["ContactInformation"].StringValue = 
            //instance["Keywords"].StringValue = 
            instance["Legal"].StringValue = layer.CopyrightText;
            //instance["TermsOfUse"].StringValue = 
            instance["DataSourceType"].StringValue = layer.Type;
            //instance["AccuracyInMeters"].StringValue = 
            //instance["Date"].NativeValue = 
            instance["Classification"].StringValue = layer.Classification;

            string mainURL = null;
            KeyValuePair<string, string> mainURLField = layer.FieldMap.FirstOrDefault(f => f.Key == "MainURL");
            if ( !mainURLField.Equals(default(KeyValuePair<string, string>)) )
                {
                mainURL = attributes.TryToGetString(mainURLField.Value);
                }

            KeyValuePair<string, string> filesizeField = layer.FieldMap.FirstOrDefault(f => f.Key == "Filesize");
            if ( !filesizeField.Equals(default(KeyValuePair<string, string>)) )
                {
                string sizeString = attributes.TryToGetString(filesizeField.Value);
                if ( sizeString != null )
                    {
                    long size;
                    if ( Int64.TryParse(sizeString, out size) )
                        {
                        instance["FileSize"].NativeValue = size / 1024;
                        }
                    }
                }

            try
                {
                if ( instance["FileSize"].IsNull && !String.IsNullOrWhiteSpace(mainURL) )
                    {
                    HttpClient client = new HttpClient();
                    HttpRequestMessage message = new HttpRequestMessage(HttpMethod.Head, mainURL);
                    HttpResponseMessage response = client.SendAsync(message, HttpCompletionOption.ResponseHeadersRead).Result;
                    long? filesize = response.Content.Headers.ContentLength;
                    if ( filesize.HasValue )
                        {
                        instance["FileSize"].NativeValue = filesize / 1024;
                        }
                    }
                }
            catch ( Exception )
                {
                //We failed to get information about the filesize. We simply continue.
                }

            instance["Streamed"].NativeValue = false;
            instance["SpatialDataSourceId"].StringValue = sourceID;
            //instance["ResolutionInMeters"].StringValue = 
            //instance["ThumbnailURL"].StringValue = 
            instance["DataProvider"].StringValue = IndexConstants.AUDataProviderString;
            instance["DataProviderName"].StringValue = IndexConstants.AUDataProviderNameString;
            instance["Dataset"].StringValue = layer.Dataset;
            //instance["Occlusion"].NativeValue = 
            //instance["MetadataURL"].StringValue = 
            //instance["RawMetadataURL"].StringValue = 
            //instance["RawMetadataFormat"].StringValue = 
            instance["SubAPI"].StringValue = IndexConstants.AUSubAPIString;

            return instance;
            }

        string ExtractEsriBBox(JToken geometry, string coordSys)
            {
            string rings = geometry["rings"].ToString();
            if(rings == null)
                {
                return null;
                }
            string[] ringsArray = rings.Split(new char[]{'[', ']', ',', ' ', '\r', '\n'}, StringSplitOptions.RemoveEmptyEntries);
            
            if(ringsArray.Length % 2 != 0 || ringsArray.Length == 0)
                {
                return null;
                }
            double minX = double.MaxValue;
            double maxX = double.MinValue;
            double minY = double.MaxValue;
            double maxY = double.MinValue;
            double xValue;
            double yValue;
            for(int i = 0; i < ringsArray.Length; i = i+2)
                {
                xValue = double.Parse(ringsArray[i]);
                if ( xValue < minX )
                    minX = xValue;
                if ( xValue > maxX )
                    maxX = xValue;
                yValue = double.Parse(ringsArray[i+1]);
                if ( yValue < minY )
                    minY = yValue;
                if ( yValue > maxY )
                    maxY = yValue;
                }
            int intCS = Int32.Parse(coordSys);
            return DbGeometryHelpers.CreateFootprintString(minX.ToString(), minY.ToString(), maxX.ToString(), maxY.ToString(), intCS); 
            }

        /// <summary>
        /// Query a specific SpatialEntity by its ID. This method has the responsibility to prepare the instance for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialEntity instance</returns>
        override protected IECInstance QuerySingleSpatialEntity (string sourceID)
            {
            IECClass ecClass = Schema.GetClass("SpatialEntity");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            AustraliaLayer layer;

            JObject json = GetJsonWithCache(sourceID, out layer);

            JObject feature = ExtractFirstFeature(json);

            JToken attributes = feature["attributes"];
            if ( attributes == null )
                {
                throw new OperationFailedException("An entry in the Geoscience Australia API is corrupted.");
                }

            instance.InstanceId = sourceID;

            instance["Id"].StringValue = sourceID;
            
            string wkid = json["spatialReference"] == null ? null : json["spatialReference"].TryToGetString("latestWkid");

            try
                {
                if ( json.TryToGetString("geometryType") == "esriGeometryPolygon" && wkid != null )
                    {
                    JToken geometry = feature["geometry"];
                    instance["Footprint"].StringValue = ExtractEsriBBox(geometry, wkid);
                    }
                }
            catch ( Exception )
                {
                //This part could fail if the geometry is too big. In that case, I prefer to prevent crashes and let the footprint be null
                }

            instance["ApproximateFootprint"].NativeValue = false;
            
            KeyValuePair<string, string> nameField = layer.FieldMap.FirstOrDefault(f => f.Key == "Name");
            if ( !nameField.Equals(default(KeyValuePair<string, string>)) )
                {
                instance["Name"].StringValue = attributes.TryToGetString(nameField.Value);
                }

            instance["DataSourceTypesAvailable"].StringValue = layer.Type;
            //instance["AccuracyInMeters"].StringValue = 
            //instance["ResolutionInMeters"].StringValue = 
            //instance["ThumbnailURL"].StringValue = 
            //instance["ThumbnailLoginKey"].StringValue = 
            instance["DataProvider"].StringValue = IndexConstants.AUDataProviderString;
            instance["DataProviderName"].StringValue = IndexConstants.AUDataProviderNameString;
            instance["Dataset"].StringValue = layer.Dataset;
            //instance["Date"].StringValue = 
            instance["Classification"].StringValue = layer.Classification;
            //instance["Occlusion"].StringValue = 

            return instance;
            }

        /// <summary>
        /// Query a specific Metadata by its ID. This method has the responsibility to prepare the instance for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The Metadata instance</returns>
        override protected IECInstance QuerySingleMetadata (string sourceID)
            {
            IECClass ecClass = Schema.GetClass("Metadata");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            AustraliaLayer layer;

            JObject json = GetJsonWithCache(sourceID, out layer);

            JObject feature = ExtractFirstFeature(json);

            JToken attributes = feature["attributes"];
            if ( attributes == null )
                {
                throw new OperationFailedException("An entry in the Geoscience Australia API is corrupted.");
                }

            instance.InstanceId = sourceID;

            instance["Id"].StringValue = sourceID;
            //instance["MetadataURL"].StringValue
            //instance["DisplayStyle"].StringValue
            //instance["Description"].StringValue
            //instance["ContactInformation"].StringValue
            //instance["Keywords"].StringValue
            instance["Legal"].StringValue = layer.CopyrightText;
            //instance["TermsOfUse"].StringValue
            //instance["Lineage"].StringValue
            //instance["Provenance"].StringValue

            return instance;
            }

        /// <summary>
        /// Query a specific SpatialDataSource by its ID. This method has the responsibility to prepare the instance for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialDataSource instance</returns>
        override protected IECInstance QuerySingleSpatialDataSource (string sourceID)
            {
            IECClass ecClass = Schema.GetClass("SpatialDataSource");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            AustraliaLayer layer;

            JObject json = GetJsonWithCache(sourceID, out layer);

            JObject feature = ExtractFirstFeature(json);

            JToken attributes = feature["attributes"];
            if ( attributes == null )
                {
                throw new OperationFailedException("An entry in the Geoscience Australia API is corrupted.");
                }

            instance.InstanceId = sourceID;

            instance["Id"].StringValue = sourceID;

            KeyValuePair<string, string> mainURLField = layer.FieldMap.FirstOrDefault(f => f.Key == "MainURL");
            if ( !mainURLField.Equals(default(KeyValuePair<string, string>)) )
                {
                instance["MainURL"].StringValue = attributes.TryToGetString(mainURLField.Value);
                }
            //instance["ParameterizedURL"].StringValue = 
            //instance["CompoundType"].StringValue = 
            //instance["LocationInCompound"].StringValue = 
            instance["DataSourceType"].StringValue = layer.Type;
            //instance["SisterFiles"].StringValue = 
            //instance["NoDataValue"].StringValue = 

            KeyValuePair<string, string> filesizeField = layer.FieldMap.FirstOrDefault(f => f.Key == "Filesize");
            if ( !filesizeField.Equals(default(KeyValuePair<string, string>)) )
                {
                string sizeString = attributes.TryToGetString(filesizeField.Value);
                if ( sizeString != null )
                    {
                    long size;
                    if ( Int64.TryParse(sizeString, out size) )
                        {
                        instance["FileSize"].NativeValue = size / 1024;
                        }
                    }
                }
            try
                {
                if ( instance["FileSize"].IsNull && !instance["MainURL"].IsNull )
                    {
                    HttpClient client = new HttpClient();
                    HttpRequestMessage message = new HttpRequestMessage(HttpMethod.Head, instance["MainURL"].StringValue);
                    HttpResponseMessage response = client.SendAsync(message, HttpCompletionOption.ResponseHeadersRead).Result;
                    long? filesize = response.Content.Headers.ContentLength;
                    if ( filesize.HasValue )
                        {
                        instance["FileSize"].NativeValue = filesize / 1024;
                        }
                    }
                }
            catch(Exception)
                {
                //We failed to get information about the filesize. We simply continue.
                }

            instance["CoordinateSystem"].StringValue = json["spatialReference"] == null ? null : json["spatialReference"].TryToGetString("latestWkid");
            instance["Streamed"].NativeValue = false;
            //instance["Metadata"].StringValue = 

            return instance;
            }

        /// <summary>
        /// Query a specific Server by its ID. This method has the responsibility to prepare the instance for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The Server instance</returns>
        override protected IECInstance QuerySingleServer (string sourceID)
            {
            IECClass ecClass = Schema.GetClass("Server");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            AustraliaLayer layer;

            JObject json = GetJsonWithCache(sourceID, out layer);

            JObject feature = ExtractFirstFeature(json);

            JToken attributes = feature["attributes"];
            if ( attributes == null )
                {
                throw new OperationFailedException("An entry in the Geoscience Australia API is corrupted.");
                }

            instance.InstanceId = sourceID;

            instance["Id"].StringValue = sourceID;
            instance["CommunicationProtocol"].StringValue = IndexConstants.AUBaseUrl.Split(':')[0];
            instance["Streamed"].NativeValue = false;
            //instance["LoginKey"].StringValue = 
            //instance["LoginMethod"].StringValue = 
            //instance["RegistrationPage"].StringValue = 
            //instance["OrganisationPage"].StringValue = ;
            instance["Name"].StringValue = IndexConstants.AUServerName;
            instance["URL"].StringValue = IndexConstants.AUBaseUrl;
            //instance["ServerContactInformation"].StringValue = ;
            //instance["Fees"].StringValue = ;
            //instance["Legal"].StringValue = ;
            //instance["AccessConstraints"].StringValue = ;
            //instance["Online"].StringValue = ;
            //instance["LastCheck"].StringValue = ;
            //instance["LastTimeOnline"].StringValue = ;
            //instance["Latency"].StringValue = ;
            //instance["MeanReachabilityStats"].StringValue = ;
            //instance["State"].StringValue = ;
            //instance["Type"].StringValue = ;

            return instance;
            }

        /// <summary>
        /// Query SpatialEntityWithDetailsView instances in a given polygon. This method has the responsibility to prepare the instances (SpatialEntity, Metadata and SpatialDataSource) for caching.
        /// Also has the responsibility to query the cache for cached instances in the case the subAPI doesn't work.
        /// </summary>
        /// <param name="polygon">The polygon</param>
        /// <param name="criteriaList">List of criteria extracted from the query</param>
        /// <returns>The Server instance</returns>
        override protected List<IECInstance> QuerySpatialEntitiesWithDetailsViewByPolygon (string polygon, List<SingleWhereCriteriaHolder> criteriaList)
            {
            List<IECInstance> instanceList = new List<IECInstance>();
            IEnumerable<AURequestBundle> bundleList = m_AustraliaDataFetcher.GetDataBySpatialQuery(polygon, criteriaList);

            foreach ( AURequestBundle bundle in bundleList )
                {
                foreach ( JObject feature in bundle.LayerContent["features"] as IList<JToken> )
                    {
                    JToken attributes = feature["attributes"];
                    if ( attributes == null )
                        {
                        //throw new OperationFailedException("An entry in the Geoscience Australia API is corrupted.");
                        continue;
                        }

                    string instanceInternalID = attributes.TryToGetString(bundle.Layer.IdField);
                    if ( String.IsNullOrWhiteSpace(instanceInternalID) )
                        {
                        continue;
                        }
                    string sourceID = "AU_" + bundle.Layer.Id + "_" + instanceInternalID;
                    instanceList.Add(CreateSEWDV(sourceID, bundle.Layer, bundle.LayerContent, feature));
                    }
                }
            return instanceList;
            }

        /// <summary>
        /// Completes the fields that are not contained in the database, and are fixed by the sub api.
        /// </summary>
        /// <param name="cachedInstances">The instances to complete.</param>
        /// <param name="ecClass">The ECClass of the instances.</param>
        override protected void CompleteInstances (List<IECInstance> cachedInstances, IECClass ecClass)
            {
            //Does nothing yet, unnecessary until cache is implemented.
            }

        private JObject ExtractFirstFeature(JObject rootJson)
            {
            if ( rootJson["features"].Count() < 1 )
                {
                throw new OperationFailedException("The requested instance doesn't exist.");
                }

            return rootJson["features"].First() as JObject;

            }

        private JObject GetJsonWithCache (string sourceID, out AustraliaLayer layer)
            {
            JObject json;
            if ( m_jsonCache == null || m_jsonCache.Item1 != sourceID )
                {
                json = m_AustraliaDataFetcher.GetSingleData(sourceID, out layer);
                m_jsonCache = new Tuple<string, JObject, AustraliaLayer>(sourceID, json, layer);
                }
            else
                {
                json = m_jsonCache.Item2;
                layer = m_jsonCache.Item3;
                }
            return json;
            }

        }
    }
