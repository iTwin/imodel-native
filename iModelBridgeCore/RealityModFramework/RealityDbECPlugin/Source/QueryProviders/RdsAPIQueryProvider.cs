using System;
using System.Collections.Generic;
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
            }

        /// <summary>
        /// RdsAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="connectionString">The connection string that will be used to access the cache in the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        /// <param name="token">The token used to communicate with rds.</param>
        public RdsAPIQueryProvider (ECQuery query, ECQuerySettings querySettings, string connectionString, IECSchema schema, string token)
            : base(query, querySettings, connectionString, schema, DataSource.RDS)
            {
            m_rdsDataFetcher = new RDSDataFetcher(token);
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
            instance["Footprint"].StringValue = properties.TryToGetString("Footprint");
            //instance["ApproximateFootprint"]
            instance["Name"].StringValue = properties.TryToGetString("Name");
            instance["Description"].StringValue = properties.TryToGetString("Description");
            string owner = properties.TryToGetString("OwnedBy");
            if ( owner != null )
                {
                instance["ContactInformation"].StringValue = "Owned by " + owner;
                }
            //instance["Keywords"]
            //instance["Legal"]
            //instance["TermsOfUse"]
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
            instance["Streamed"].NativeValue = true;

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
            instance["Footprint"].StringValue = properties.TryToGetString("Footprint");
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
            //ThumbnailLoginKey
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
            //Legal
            //TermsOfUse
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

            string rootDocument = properties.TryToGetString("RootDocument");
            if ( !String.IsNullOrWhiteSpace(rootDocument) )
                {
                instance["MainURL"].StringValue = m_rdsDataFetcher.RdsUrlBase + IndexConstants.RdsDocumentClass + sourceID + "~2f" + rootDocument.Replace("/", "~2f");
                }

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

            instance["Streamed"].NativeValue = true;

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
            instance["Streamed"].NativeValue = true; 
            //loginKey
            instance["LoginMethod"].StringValue = "IMS";
            //RegistrationPage
            //OrganisationPage
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

        }
    }
