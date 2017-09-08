using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;
using Newtonsoft.Json.Linq;

namespace IndexECPlugin.Source.QueryProviders
    {

    /// <summary>
    /// Usgs Earth Explorer specialization of the SubAPIQueryProvider class
    /// </summary>
    public class UsgsEEApiQueryProvider : SubAPIQueryProvider
        {
        IUSGSEEDataFetcher m_dataFetcher;

        /// <summary>
        /// UsgsEEApiQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="dataFetcher">The dataFetcher that will be used to query USGS EE</param>
        /// <param name="cacheManager">The cache manager that will be used to access the cache in the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        public UsgsEEApiQueryProvider (ECQuery query, ECQuerySettings querySettings, IUSGSEEDataFetcher dataFetcher, IInstanceCacheManager cacheManager, IECSchema schema)
            : base(query,querySettings,cacheManager,schema,DataSource.USGSEE)
            {
            m_dataFetcher = dataFetcher;
            }

        /// <summary>
        /// UsgsEEApiQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="dbQuerier">The IDbQuerier object used to communicate with the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        public UsgsEEApiQueryProvider (ECQuery query, ECQuerySettings querySettings, IDbQuerier dbQuerier, IECSchema schema)
            : base(query, querySettings, dbQuerier, schema, DataSource.USGSEE, false)
            {
            m_dataFetcher = new UsgsEEDataFetcher(query, new GenericHttpResponseGetter(SourceStringMap.SourceToString(DataSource.USGSEE), new TimeSpan(0, 0, 15)));
            }

        /// <summary>
        /// Query a specific SpatialEntityWithDetailsView by its ID.
        /// </summary>
        /// <param name="sourceId">The Id. Should have the following form: USGSEE__datasetId__entityId</param>
        /// <returns>The SpatialEntityWithDetailsView instance</returns>
        override protected IECInstance QuerySingleSpatialEntityWithDetailsView (string sourceId)
            {
            IECClass ecClass = Schema.GetClass("SpatialEntityWithDetailsView");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            string datasetId;
            string entityId;
            SplitSourceId(sourceId, out datasetId, out entityId);

            UsgsEEDataset dataset = m_dataFetcher.DatasetList.FirstOrDefault(d => d.DatasetId == datasetId);
            if(dataset == null)
            {
                throw new UserFriendlyException("This entity is not indexed.");
            }

            var xmlDoc = m_dataFetcher.GetXmlDocForInstance(entityId, datasetId);

            instance.InstanceId = sourceId;
            instance["Id"].StringValue = sourceId;

            XmlNamespaceManager xmlnsm = new XmlNamespaceManager(xmlDoc.NameTable);
            xmlnsm.AddNamespace("eemetadata", "http://earthexplorer.usgs.gov/eemetadata.xsd");
            XmlNode metadataFieldsNode = xmlDoc.SelectSingleNode("eemetadata:scene/eemetadata:metadataFields", xmlnsm);

            string minX = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='SW Corner Long dec']/eemetadata:metadataValue", xmlnsm);
            string minY = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='SW Corner Lat dec']/eemetadata:metadataValue", xmlnsm);
            string maxX = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='NE Corner Long dec']/eemetadata:metadataValue", xmlnsm);
            string maxY = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='NE Corner Lat dec']/eemetadata:metadataValue", xmlnsm);

            if(minX != null && minY != null && maxX != null && maxY != null)
                instance["Footprint"].StringValue = DbGeometryHelpers.CreateFootprintString(minX, minY, maxX, maxY, 4326);

            instance["Name"].StringValue = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='Entity ID']/eemetadata:metadataValue", xmlnsm);
            instance["Legal"].StringValue = IndexConstants.EELegalString;
            instance["TermsOfUse"].StringValue = IndexConstants.EETermsOfUse;
            instance["DataSourceType"].StringValue = dataset.DataFormat;
            //It doesn't work for @name='Beginning Date', for an unknown reason
            string dateString = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@link='https://lta.cr.usgs.gov/high_res_ortho.html#begin_date']/eemetadata:metadataValue", xmlnsm);
            DateTime date;
            if(dateString != null && DateTime.TryParse(dateString, out date))
                {
                instance["Date"].NativeValue = date;
                }
            instance["Classification"].StringValue = dataset.Classification;
            instance["Streamed"].NativeValue = false;
            instance["SpatialDataSourceId"].StringValue = instance.InstanceId;
            string unit = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='Units of Resolution']/eemetadata:metadataValue", xmlnsm);
            string resolutionString = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='Resolution']/eemetadata:metadataValue", xmlnsm);
            double resolution = 0;
            
            if((unit != null) && (resolutionString != null) && ((unit.ToLower() == "feet") || (unit.ToLower() == "foot")))
                {
                if ( Double.TryParse(resolutionString, out resolution) )
                    {
                    resolution *= 0.3048;
                    string truncatedRes = resolution.ToString("0.####");
                    instance["ResolutionInMeters"].StringValue = truncatedRes + "x" + truncatedRes;
                    }
                }
            else if((unit != null) && (resolutionString != null) && ((unit.ToLower() == "meter") || (unit.ToLower() == "meters")))
                {
                //This is simply a verification
                if ( Double.TryParse(resolutionString, out resolution) )
                    {
                    instance["ResolutionInMeters"].StringValue = resolutionString + "x" + resolutionString;
                    }
                }

            var thumbnailNode = xmlDoc.SelectSingleNode("eemetadata:scene/eemetadata:browseLinks", xmlnsm);

            if(thumbnailNode != null)
                {
                instance["ThumbnailURL"].StringValue = thumbnailNode.TrySelectSingleNodeInnerText("eemetadata:browse[@id='BROWSE_STANDARD_PATH']/eemetadata:browseLink", xmlnsm);
                }

            instance["DataProvider"].StringValue = IndexConstants.UsgsDataProviderString;
            instance["DataProviderName"].StringValue = IndexConstants.UsgsDataProviderNameString;

            instance["Dataset"].StringValue = dataset.DatasetName;
            instance["MetadataURL"].StringValue = String.Format(IndexConstants.EEFgdcMetadataURL, datasetId, entityId);
            instance["SubAPI"].StringValue = IndexConstants.EESubAPIString;

            return instance;
            }

            private static void SplitSourceId(string sourceId, out string datasetId, out string entityId)
            {
            string[] splitSourceId = sourceId.Split(new string[] { "__" }, StringSplitOptions.None);
                if (splitSourceId[0] != "USGSEE" || splitSourceId.Length < 3)
                {
                    throw new ProgrammerException("The sourceId does not have the required form.");
                }

                datasetId = splitSourceId[1];
                int splitSourceIdLength = splitSourceId.Length;
                string[] splitEntityId = new string[splitSourceIdLength - 2];

                //This joins all split strings past the third one, since only the first two had to be split.
                Array.Copy(splitSourceId, 2, splitEntityId, 0, splitSourceIdLength - 2);
                entityId = String.Join("__", splitEntityId);
            }

            /// <summary>
        /// Query a specific SpatialEntity by its ID.
        /// </summary>
        /// <param name="sourceId">The Id</param>
        /// <returns>The SpatialEntity instance</returns>
        override protected IECInstance QuerySingleSpatialEntity (string sourceId)
            {
            IECClass ecClass = Schema.GetClass("SpatialEntity");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            string datasetId;
            string entityId;
            SplitSourceId(sourceId, out datasetId, out entityId);

            UsgsEEDataset dataset = m_dataFetcher.DatasetList.FirstOrDefault(d => d.DatasetId == datasetId);
            if ( dataset == null )
                {
                throw new UserFriendlyException("This entity is not indexed.");
                }

            var xmlDoc = m_dataFetcher.GetXmlDocForInstance(entityId, datasetId);

            instance.InstanceId = sourceId;
            instance["Id"].StringValue = sourceId;

            XmlNamespaceManager xmlnsm = new XmlNamespaceManager(xmlDoc.NameTable);
            xmlnsm.AddNamespace("eemetadata", "http://earthexplorer.usgs.gov/eemetadata.xsd");
            XmlNode metadataFieldsNode = xmlDoc.SelectSingleNode("eemetadata:scene/eemetadata:metadataFields", xmlnsm);

            string minX = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='SW Corner Long dec']/eemetadata:metadataValue", xmlnsm);
            string minY = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='SW Corner Lat dec']/eemetadata:metadataValue", xmlnsm);
            string maxX = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='NE Corner Long dec']/eemetadata:metadataValue", xmlnsm);
            string maxY = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='NE Corner Lat dec']/eemetadata:metadataValue", xmlnsm);

            if ( minX != null && minY != null && maxX != null && maxY != null )
                instance["Footprint"].StringValue = DbGeometryHelpers.CreateFootprintString(minX, minY, maxX, maxY, 4326);

            instance["Name"].StringValue = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='Entity ID']/eemetadata:metadataValue", xmlnsm);
            instance["DataSourceTypesAvailable"].StringValue = dataset.DataFormat;

            string unit = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='Units of Resolution']/eemetadata:metadataValue", xmlnsm);
            string resolutionString = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@name='Resolution']/eemetadata:metadataValue", xmlnsm);
            double resolution = 0;

            if ( (unit != null) && (resolutionString != null) && ((unit.ToLower() == "feet") || (unit.ToLower() == "foot")) )
                {
                if ( Double.TryParse(resolutionString, out resolution) )
                    {
                    resolution *= 0.3048;
                    string truncatedRes = resolution.ToString("0.####");
                    instance["ResolutionInMeters"].StringValue = truncatedRes + "x" + truncatedRes;
                    }
                }
            else if ( (unit != null) && (resolutionString != null) && ((unit.ToLower() == "meter") || (unit.ToLower() == "meters")) )
                {
                //This is simply a verification
                if ( Double.TryParse(resolutionString, out resolution) )
                    {
                    instance["ResolutionInMeters"].StringValue = resolutionString + "x" + resolutionString;
                    }
                }

            var thumbnailNode = xmlDoc.SelectSingleNode("eemetadata:scene/eemetadata:browseLinks", xmlnsm);

            if ( thumbnailNode != null )
                {
                instance["ThumbnailURL"].StringValue = thumbnailNode.TrySelectSingleNodeInnerText("eemetadata:browse[@id='BROWSE_STANDARD_PATH']/eemetadata:browseLink", xmlnsm);
                }

            instance["DataProvider"].StringValue = IndexConstants.UsgsDataProviderString;
            instance["DataProviderName"].StringValue = IndexConstants.UsgsDataProviderNameString;

            instance["Dataset"].StringValue = dataset.DatasetName;

            //It doesn't work for @name='Beginning Date', for an unknown reason
            string dateString = metadataFieldsNode.TrySelectSingleNodeInnerText("eemetadata:metadataField[@link='https://lta.cr.usgs.gov/high_res_ortho.html#begin_date']/eemetadata:metadataValue", xmlnsm);
            DateTime date;
            if ( dateString != null && DateTime.TryParse(dateString, out date) )
                {
                instance["Date"].NativeValue = date;
                }
            instance["Classification"].StringValue = dataset.Classification;

            return instance;
            }

        /// <summary>
        /// Query a specific Metadata by its ID.
        /// </summary>
        /// <param name="sourceId">The Id</param>
        /// <returns>The Metadata instance</returns>
        override protected IECInstance QuerySingleMetadata (string sourceId)
            {
            IECClass ecClass = Schema.GetClass("Metadata");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            string datasetId;
            string entityId;
            SplitSourceId(sourceId, out datasetId, out entityId);

            UsgsEEDataset dataset = m_dataFetcher.DatasetList.FirstOrDefault(d => d.DatasetId == datasetId);
            if ( dataset == null )
                {
                throw new UserFriendlyException("This entity is not indexed.");
                }

            //We do this to validate that the entity really exists.
            m_dataFetcher.GetXmlDocForInstance(entityId, datasetId);

            instance.InstanceId = sourceId;
            instance["Id"].StringValue = sourceId;

            instance["MetadataURL"].StringValue = String.Format(IndexConstants.EEFgdcMetadataURL, datasetId, entityId);
            instance["Legal"].StringValue = IndexConstants.EELegalString;
            instance["TermsOfUse"].StringValue = IndexConstants.EETermsOfUse;

            return instance;

            }

        /// <summary>
        /// Query a specific SpatialDataSource by its ID.
        /// </summary>
        /// <param name="sourceId">The Id</param>
        /// <returns>The SpatialDataSource instance</returns>
        override protected IECInstance QuerySingleSpatialDataSource (string sourceId)
            {
            IECClass ecClass = Schema.GetClass("SpatialDataSource");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            string datasetId;
            string entityId;
            SplitSourceId(sourceId, out datasetId, out entityId);

            UsgsEEDataset dataset = m_dataFetcher.DatasetList.FirstOrDefault(d => d.DatasetId == datasetId);
            if ( dataset == null )
                {
                throw new UserFriendlyException("This entity is not indexed.");
                }

            //We do this to validate that the entity really exists.
            var xmlDoc = m_dataFetcher.GetXmlDocForInstance(entityId, datasetId);

            instance.InstanceId = sourceId;
            instance["Id"].StringValue = sourceId;

            instance["MainURL"].StringValue = String.Format(IndexConstants.EEDownloadURL, datasetId, entityId);
            instance["DataSourceType"].StringValue = dataset.DataFormat;

            return instance;
            }

        /// <summary>
        /// Query a specific Server by its ID.
        /// </summary>
        /// <param name="sourceId">The Id</param>
        /// <returns>The Server instance</returns>
        override protected IECInstance QuerySingleServer (string sourceId)
            {
            IECClass ecClass = Schema.GetClass("Server");
            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            string datasetId;
            string entityId;
            SplitSourceId(sourceId, out datasetId, out entityId);

            UsgsEEDataset dataset = m_dataFetcher.DatasetList.FirstOrDefault(d => d.DatasetId == datasetId);
            if ( dataset == null )
                {
                throw new UserFriendlyException("This entity is not indexed.");
                }

            //We do this to validate that the entity really exists.
            var xmlDoc = m_dataFetcher.GetXmlDocForInstance(entityId, datasetId);

            instance.InstanceId = sourceId;
            instance["Id"].StringValue = sourceId;
            instance["CommunicationProtocol"].StringValue = "HTTPS";
            instance["Streamed"].NativeValue = false;
            instance["LoginKey"].StringValue = "EarthExplorer";
            instance["LoginMethod"].StringValue = "CUSTOM";
            instance["RegistrationPage"].StringValue = IndexConstants.EERegistrationPage;
            instance["OrganisationPage"].StringValue = IndexConstants.EEOrganisationPage;
            instance["Name"].StringValue = IndexConstants.EEServerName;
            instance["URL"].StringValue = IndexConstants.EEServerURL;
            instance["Legal"].StringValue = IndexConstants.EELegalString;

            return instance;
            }

        /// <summary>
        /// Query SpatialEntityWithDetailsView instances in a given polygon.
        /// </summary>
        /// <param name="polygon">The polygon</param>
        /// <param name="criteriaList">List of criteria extracted from the query</param>
        /// <returns>The list of instances</returns>
        override protected List<IECInstance> QuerySpatialEntitiesWithDetailsViewByPolygon (string polygon, List<SingleWhereCriteriaHolder> criteriaList)
            {
            IECClass ecClass = Schema.GetClass("SpatialEntityWithDetailsView");
            List<IECInstance> instanceList = new List<IECInstance>();
            foreach ( EERequestBundle requestBundle in m_dataFetcher.SpatialQuery() )
                {
                foreach (JToken token in requestBundle.jtokenList)
                    {
                    IECInstance instance = ecClass.CreateInstance();

                    instance.InitializePropertiesToNull();

                    string sceneBounds = token.TryToGetString("sceneBounds");
                    string[] coords = sceneBounds.Split(',');
                    double minX;
                    double minY;
                    double maxX;
                    double maxY;
                    if ( Double.TryParse(coords[0], out minX) && Double.TryParse(coords[1], out minY) && Double.TryParse(coords[2], out maxX) && Double.TryParse(coords[3], out maxY) )
                        {
                        instance["Footprint"].StringValue = DbGeometryHelpers.CreateFootprintString(coords[0], coords[1], coords[2], coords[3], 4326);
                        }
                    if(token["entityId"] != null)
                        {
                        string entityId = token.TryToGetString("entityId");
                        instance.InstanceId = "USGSEE__" + requestBundle.Dataset.DatasetId + "__" + entityId;
                        instance["Id"].StringValue = instance.InstanceId;
                        instance["MetadataURL"].StringValue = String.Format(IndexConstants.EEFgdcMetadataURL, requestBundle.Dataset.DatasetId, entityId);
                        }

                    instance["Name"].StringValue = token.TryToGetString("entityId");
                    instance["Description"].StringValue = token.TryToGetString("summary");
                    //ContactInfo
                    //Keywords
                    instance["Legal"].StringValue = IndexConstants.EELegalString;
                    instance["TermsOfUse"].StringValue = IndexConstants.EETermsOfUse;
                    instance["DataSourceType"].StringValue = requestBundle.Dataset.DataFormat;
                    //AccuracyInMeters
                    DateTime date;
                    if((token["startTime"] != null) && DateTime.TryParse(token.TryToGetString("startTime"), out date))
                        {
                        instance["Date"].NativeValue = date;
                        }
                    instance["Classification"].StringValue = requestBundle.Dataset.Classification;
                    //filesize
                    instance["Streamed"].NativeValue = false;
                    //ResolutionInMeters => Could be extracted from browseUrl for HRO??? Use the same DataExtractor structure than USGS???
                    instance["ThumbnailURL"].StringValue = token.TryToGetString("browseUrl");
                    instance["DataProvider"].StringValue = IndexConstants.UsgsDataProviderString;
                    instance["DataProviderName"].StringValue = IndexConstants.UsgsDataProviderNameString;
                    instance["Dataset"].StringValue = requestBundle.Dataset.DatasetName;
                    //occlusion
                    instance["SubAPI"].StringValue = IndexConstants.EESubAPIString;

                    instanceList.Add(instance);
                    }
                }

            return instanceList;
            }

        /// <summary>
        /// Completes the fields that are not contained in the database, and are fixed by the sub api.
        /// </summary>
        /// <param name="cachedInstances">The instances to complete.</param>
        /// <param name="ecClass">The ECClass of the instances.</param>
        override protected void CompleteInstances(List<IECInstance> cachedInstances, IECClass ecClass)
            {
                throw new NotImplementedException();
            }
        }
    }
