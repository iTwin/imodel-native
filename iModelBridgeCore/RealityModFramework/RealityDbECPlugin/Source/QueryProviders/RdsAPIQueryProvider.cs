using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
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
    /// RdsAPIQueryProvider's purpose is to transform Reality Data Server data into ECInstances for our plugin.
    /// </summary>
    public class RdsAPIQueryProvider : SubAPIQueryProvider 
        {
        Tuple<string, JObject> m_jsonCache;
        IRDSDataFetcher m_rdsDataFetcher;
        bool m_packageRequest;
        /// <summary>
        /// RdsAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="rdsDataFetcher">The rdsDataFetcher that will be used to query rds</param>
        /// <param name="cacheManager">The cache manager that will be used to access the cache in the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        public RdsAPIQueryProvider(ECQuery query, ECQuerySettings querySettings, IRDSDataFetcher rdsDataFetcher, IInstanceCacheManager cacheManager, IECSchema schema)
            :base(query, querySettings, cacheManager, schema, DataSource.RDS)
            {
            m_rdsDataFetcher = rdsDataFetcher;
            m_packageRequest = (query.ExtendedData.ContainsKey(IndexConstants.PRExtendedDataName) && (bool) query.ExtendedData[IndexConstants.PRExtendedDataName]);
            }

        /// <summary>
        /// RdsAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="dbQuerier">The IDbQuerier object used to communicate</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        /// <param name="base64token">The token (base64) used to communicate with rds.</param>
        public RdsAPIQueryProvider (ECQuery query, ECQuerySettings querySettings, IDbQuerier dbQuerier, IECSchema schema, string base64token)
            : base(query, querySettings, dbQuerier, schema, DataSource.RDS, false)
            {
            m_rdsDataFetcher = new RDSDataFetcher(new RdsHttpResponseGetter(base64token));
            m_packageRequest = (query.ExtendedData.ContainsKey("PackageRequest") && (bool) query.ExtendedData["PackageRequest"] == true);
            }

        /// <summary>
        /// Query a specific SpatialEntityWithDetailsView by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialEntityWithDetailsView instance</returns>
        override protected IECInstance QuerySingleSpatialEntityWithDetailsView (string sourceID)
            {
            JObject json = GetJsonWithCache(sourceID);

            return CreateSEWDVFromJObject(json);
            }

        private IECInstance CreateSEWDVFromJObject (JObject jsonInstance)
            {
            JObject properties = jsonInstance["properties"] as JObject;
            if ( properties == null )
                {
                throw new OperationFailedException("The instance doesn't have any properties");
                }

            IECClass ecClass = Schema.GetClass("SpatialEntityWithDetailsView");
            IECInstance instance = ecClass.CreateInstance();
            instance.InitializePropertiesToNull();

            instance.InstanceId = properties.TryToGetString("Id");
            instance["Id"].StringValue = instance.InstanceId;

            JObject footprint = properties["Footprint"] as JObject;
            if ( footprint != null )
                {
                JArray coordinates = footprint["Coordinates"] as JArray;
                if ( coordinates != null )
                    {
                    string footprintStr = ExtractFootprintFromRds(coordinates);
                    if ( footprintStr != null )
                        {
                        instance["Footprint"].StringValue = footprintStr;
                        }
                    }
                }

            //instance["ApproximateFootprint"]
            instance["Name"].StringValue = properties.TryToGetString("Name");
            instance["Description"].StringValue = properties.TryToGetString("Description");
            string owner = properties.TryToGetString("OwnedBy");
            if ( owner != null )
                {
                instance["ContactInformation"].StringValue = "Owned by " + owner;
                }
            //instance["Keywords"]
            instance["Legal"].StringValue = properties.TryToGetString("Copyright");
            instance["TermsOfUse"].StringValue = properties.TryToGetString("TermsOfUse");
            instance["DataSourceType"].StringValue = properties.TryToGetString("Type");
            instance["AccuracyInMeters"].StringValue = properties.TryToGetString("AccuracyInMeters");
            DateTime date;
            if ( DateTime.TryParse(properties.TryToGetString("CreatedTimestamp"), out date) )
                {
                instance["Date"].NativeValue = date;
                }
            instance["Classification"].StringValue = properties.TryToGetString("Classification");
            Int64 filesize;
            if ( Int64.TryParse(properties.TryToGetString("Size"), out filesize) )
                {
                instance["FileSize"].NativeValue = filesize;
                }
            if(properties["Streamed"] != null)
                {
                instance["Streamed"].NativeValue = properties.Value<bool>("Streamed");
                }
            

            instance["SpatialDataSourceId"].StringValue = instance.InstanceId;

            instance["ResolutionInMeters"].StringValue = properties.TryToGetString("ResolutionInMeters");

            string thumbnailDocument = properties.TryToGetString("ThumbnailDocument");
            if ( !String.IsNullOrWhiteSpace(thumbnailDocument) )
                {
                instance["ThumbnailURL"].StringValue = m_rdsDataFetcher.RdsUrlBase + IndexConstants.RdsDocumentClass + instance.InstanceId + "~2f" + thumbnailDocument.Replace("/", "~2f");
                }

            //DataProvider
            //DataProviderName


            instance["Dataset"].StringValue = properties.TryToGetString("Dataset");

            //Occlusion
            //MetadataURL
            //RawMetadataURL
            //RawMetadataFormat

            instance["SubAPI"].StringValue = IndexConstants.RdsSubAPIString;

            return instance;
            }

        /// <summary>
        /// Query a specific SpatialEntity by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialEntity instance</returns>
        override protected IECInstance QuerySingleSpatialEntity (string sourceID)
            {
            JObject json = GetJsonWithCache(sourceID);

            IECClass ecClass = Schema.GetClass("SpatialEntity");

            IECInstance instance = ecClass.CreateInstance();
            instance.InitializePropertiesToNull();

            JObject properties = json["properties"] as JObject;
            if ( properties == null )
                {
                throw new OperationFailedException("The instance doesn't have any properties");
                }

            instance.InstanceId = sourceID;
            instance["Id"].StringValue = sourceID;

            JObject footprint = properties["Footprint"] as JObject;
            if ( footprint != null )
                {
                JArray coordinates = footprint["Coordinates"] as JArray;
                if ( coordinates != null )
                    {
                    string footprintStr = ExtractFootprintFromRds(coordinates);
                    if ( footprintStr != null )
                        {
                        instance["Footprint"].StringValue = footprintStr;
                        }
                    }
                }

            //instance["ApproximateFootprint"]
            instance["Name"].StringValue = properties.TryToGetString("Name");
            instance["DataSourceTypesAvailable"].StringValue = properties.TryToGetString("Type");
            instance["AccuracyInMeters"].StringValue = properties.TryToGetString("AccuracyInMeters");
            instance["ResolutionInMeters"].StringValue = properties.TryToGetString("ResolutionInMeters");
            string thumbnailDocument = properties.TryToGetString("ThumbnailDocument");
            if ( !String.IsNullOrWhiteSpace(thumbnailDocument) )
                {
                instance["ThumbnailURL"].StringValue = m_rdsDataFetcher.RdsUrlBase + IndexConstants.RdsDocumentClass + sourceID + "~2f" + thumbnailDocument.Replace("/", "~2f");
                }
            instance["ThumbnailLoginKey"].StringValue = IndexConstants.RdsLoginKey;
            //DataProvider
            //DataProviderName

            instance["Dataset"].StringValue = properties.TryToGetString("Dataset");

            DateTime date;
            if ( DateTime.TryParse(properties.TryToGetString("CreatedTimestamp"), out date) )
                {
                instance["Date"].NativeValue = date;
                }

            instance["Classification"].StringValue = properties.TryToGetString("Classification");

            //Occlusion

            return instance;

            }

        /// <summary>
        /// Query a specific Metadata by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The Metadata instance</returns>
        override protected IECInstance QuerySingleMetadata (string sourceID)
            {
            JObject json = GetJsonWithCache(sourceID);

            IECClass ecClass = Schema.GetClass("Metadata");

            IECInstance instance = ecClass.CreateInstance();
            instance.InitializePropertiesToNull();

            JObject properties = json["properties"] as JObject;
            if ( properties == null )
                {
                throw new OperationFailedException("The instance doesn't have any properties");
                }

            instance.InstanceId = sourceID;
            instance["Id"].StringValue = sourceID;

            //MetadataURL
            //DisplayStyle
            instance["Description"].StringValue = properties.TryToGetString("Description");
            string owner = properties.TryToGetString("OwnedBy");
            if(owner != null)
                {
                instance["ContactInformation"].StringValue = "Owned by " + owner;
                }
            //Keywords
            instance["Legal"].StringValue = properties.TryToGetString("Copyright");
            instance["TermsOfUse"].StringValue = properties.TryToGetString("TermsOfUse");
            //Lineage
            //Provenance

            return instance;
            }

        /// <summary>
        /// Query a specific SpatialDataSource by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialDataSource instance</returns>
        override protected IECInstance QuerySingleSpatialDataSource (string sourceID)
            {
            JObject json = GetJsonWithCache(sourceID);

            IECClass ecClass = Schema.GetClass("SpatialDataSource");

            IECInstance instance = ecClass.CreateInstance();
            instance.InitializePropertiesToNull();

            JObject properties = json["properties"] as JObject;
            if ( properties == null )
                {
                throw new OperationFailedException("The instance doesn't have any properties");
                }

            instance.InstanceId = sourceID;
            instance["Id"].StringValue = sourceID;

            //ParameterizedURL
            //CompoundType
            //LocationInCompound

            instance["DataSourceType"].StringValue = properties.TryToGetString("Type");

            //SisterFiles
            //NoDataValue

            Int64 filesize;
            if ( Int64.TryParse(properties.TryToGetString("Size"), out filesize) )
                {
                instance["FileSize"].NativeValue = filesize;
                }

            bool streamed = true;

            if ( properties["Streamed"] != null )
                {
                streamed = properties.Value<bool>("Streamed");
                instance["Streamed"].NativeValue = streamed;
                }

            string visibility = properties.TryToGetString("Visibility");

            if ( m_packageRequest && visibility == "PUBLIC" && !streamed )
                {
                JObject jsonRAAT = m_rdsDataFetcher.GetReadAccessAzureToken(sourceID);

                JObject propertiesRAAT = jsonRAAT["properties"] as JObject;
                if ( propertiesRAAT == null )
                    {
                    throw new OperationFailedException("The instance doesn't have any properties");
                    }
                string url = propertiesRAAT.TryToGetString("Url");
                if(!String.IsNullOrWhiteSpace(url))
                    {
                    instance["MainURL"].StringValue = url;
                    }
                }
            else
                {
                string rootDocument = properties.TryToGetString("RootDocument");
                if ( !String.IsNullOrWhiteSpace(rootDocument) )
                    {
                    instance["MainURL"].StringValue = m_rdsDataFetcher.RdsUrlBase + IndexConstants.RdsDocumentClass + sourceID + "~2f" + rootDocument.Replace("/", "~2f");
                    }
                }
            //Metadata

            return instance;
            }

        /// <summary>
        /// Query a specific Server by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The Server instance</returns>
        override protected IECInstance QuerySingleServer (string sourceID)
            {
            JObject json = GetJsonWithCache(sourceID);

            IECClass ecClass = Schema.GetClass("Server");

            IECInstance instance = ecClass.CreateInstance();
            instance.InitializePropertiesToNull();

            JObject properties = json["properties"] as JObject;
            if ( properties == null )
                {
                throw new OperationFailedException("The instance doesn't have any properties");
                }

            instance.InstanceId = sourceID;
            instance["Id"].StringValue = sourceID;

            instance["CommunicationProtocol"].StringValue = m_rdsDataFetcher.RdsUrlBase.Split(':')[0];
            if ( properties["Streamed"] != null )
                {
                instance["Streamed"].NativeValue = properties.Value<bool>("Streamed");
                }
            instance["LoginKey"].StringValue = IndexConstants.RdsLoginKey;
            instance["LoginMethod"].StringValue = IndexConstants.RdsLoginMethod;
            instance["RegistrationPage"].StringValue = IndexConstants.RdsRegistrationPage;
            instance["OrganisationPage"].StringValue = IndexConstants.RdsOrganisationPage;
            instance["Name"].StringValue = IndexConstants.RdsSourceName;
            instance["URL"].StringValue = m_rdsDataFetcher.RdsUrlBase;
            //ServerContactInformation
            //Fees
            //Legal
            //AccessConstraints
            //Online
            //LastCheck
            //LastTimeOnline
            //Latency
            //MeanReachabilityStats
            //State
            //Type

            return instance;
            }

        /// <summary>
        /// Query SpatialEntityWithDetailsView instances in a given polygon.
        /// </summary>
        /// <param name="polygon">The polygon</param>
        /// <param name="criteriaList">List of criteria extracted from the query</param>
        /// <returns>The Server instance</returns>
        override protected List<IECInstance> QuerySpatialEntitiesWithDetailsViewByPolygon (string polygon, List<SingleWhereCriteriaHolder> criteriaList)
            {
            JArray jsonInstances = m_rdsDataFetcher.GetDataBySpatialQuery(polygon, criteriaList);

            List<IECInstance> instanceList = new List<IECInstance>();

            foreach ( JObject jsonInstance in jsonInstances )
                {
                instanceList.Add(CreateSEWDVFromJObject(jsonInstance));
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

        private JObject GetJsonWithCache (string sourceID)
            {
            JObject json;
            if ( m_jsonCache == null || m_jsonCache.Item1 != sourceID )
                {
                json = m_rdsDataFetcher.GetSingleData(sourceID);
                m_jsonCache = new Tuple<string, JObject>(sourceID, json);
                }
            else
                {
                json = m_jsonCache.Item2;
                }
            return json;
            }

        private string ExtractFootprintFromRds (JArray coordinates)
            {
            try
                {
                using ( StringWriter stringWriter = new StringWriter() )
                    {
                    stringWriter.Write("{\"points\":[");
                    List<string> coordList = new List<string>();
                    foreach ( JObject coordinate in coordinates )
                        {
                        string lon = coordinate["Longitude"].Value<string>();
                        string lat = coordinate["Latitude"].Value<string>();
                        coordList.Add("[" + lon + "," + lat + "]");
                        }
                    stringWriter.Write(String.Join(",", coordList));
                    stringWriter.Write("],\"coordinate_system\":\"4326\"}");
                    return stringWriter.ToString();
                    }
                }
            catch ( Exception )
                {
                //The footprint is malformed. We'll set it to null.
                return null;
                }
            }

        }
    }
