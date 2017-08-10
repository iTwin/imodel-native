using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECSystem.Configuration;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;
using Newtonsoft.Json.Linq;

namespace IndexECPlugin.Source.QueryProviders
    {
    /// <summary>
    /// Usgs specialization of the SubAPIQueryProvider class
    /// </summary>
    public class UsgsSubAPIQueryProvider : SubAPIQueryProvider
        {

        Tuple<string, JObject> jsonCache;

        IUSGSDataFetcher m_usgsDataFetcher;



        /// <summary>
        /// UsgsSubAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="usgsDataFetcher">The usgsDataFetcher that will be used to query USGS</param>
        /// <param name="cacheManager">The cache manager that will be used to access the cache in the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        public UsgsSubAPIQueryProvider(ECQuery query, ECQuerySettings querySettings, IUSGSDataFetcher usgsDataFetcher, IInstanceCacheManager cacheManager, IECSchema schema)
            : base(query, querySettings, cacheManager, schema, DataSource.USGS)
            {
            m_usgsDataFetcher = usgsDataFetcher;
            }

        /// <summary>
        /// UsgsSubAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="dbQuerier">The IDbQuerier object used to communicate with the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        public UsgsSubAPIQueryProvider(ECQuery query, ECQuerySettings querySettings, IDbQuerier dbQuerier, IECSchema schema)
            : base(query, querySettings, dbQuerier, schema, DataSource.USGS, true)
            {
            m_usgsDataFetcher = new USGSDataFetcher(query, new GenericHttpResponseGetter("USGS", new TimeSpan(0, 0, 15)));
            }

        /// <summary>
        /// Query a specific SpatialEntityWithDetailsView by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialEntityWithDetailsView instance</returns>
        override protected IECInstance QuerySingleSpatialEntityWithDetailsView (string sourceID)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            JObject json = GetJsonWithCache(sourceID);

            IECClass ecClass = Schema.GetClass("SpatialEntityWithDetailsView");

            IECInstance instance = ecClass.CreateInstance();

            IECInstance instanceSEB = QuerySingleSpatialEntity(sourceID);
            IECInstance instanceMetadata = QuerySingleMetadata(sourceID);
            IECInstance instanceSDS = QuerySingleSpatialDataSource(sourceID);

            instance.InitializePropertiesToNull();

            instance.InstanceId = sourceID;

            instance["Id"].StringValue = sourceID;

            instance.InstanceId = json.TryToGetString("id");

            instance["Name"].StringValue = json.TryToGetString("title");

            if ( instanceSEB["Footprint"] != null && !instanceSEB["Footprint"].IsNull )
                {
                instance["Footprint"].NativeValue = instanceSEB["Footprint"].NativeValue;
                }

            instance["ApproximateFootprint"].NativeValue = false;

            if ( instanceSDS["DataSourceType"] != null && !instanceSDS["DataSourceType"].IsNull )
                {
                instance["DataSourceType"].StringValue = instanceSDS["DataSourceType"].StringValue;
                }

            if ( instanceSEB["ThumbnailURL"] != null && !instanceSEB["ThumbnailURL"].IsNull )
                {
                instance["ThumbnailURL"].StringValue = instanceSEB["ThumbnailURL"].StringValue;
                }

            if ( instanceMetadata["Legal"] != null && !instanceMetadata["Legal"].IsNull )
                {
                instance["Legal"].StringValue = instanceMetadata["Legal"].StringValue;
                }

            instance["TermsOfUse"].StringValue = instanceMetadata["TermsOfUse"].StringValue;

            instance["MetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/" + instance.InstanceId;
            instance["RawMetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/download/" + instance.InstanceId + IndexConstants.UsgsRawMetadataURLEnding;
            instance["RawMetadataFormat"].StringValue = IndexConstants.UsgsRawMetadataFormatString;
            instance["SubAPI"].StringValue = IndexConstants.UsgsSubAPIString;

            if ( instanceSEB["DataProvider"] != null && !instanceSEB["DataProvider"].IsNull )
                {
                instance["DataProvider"].StringValue = instanceSEB["DataProvider"].StringValue;
                }

            if ( instanceSEB["DataProviderName"] != null && !instanceSEB["DataProviderName"].IsNull )
                {
                instance["DataProviderName"].StringValue = instanceSEB["DataProviderName"].StringValue;
                }
            if ( instanceSEB["Dataset"] != null && !instanceSEB["Dataset"].IsNull )
                {
                instance["Dataset"].StringValue = instanceSEB["Dataset"].StringValue;
                }

            instance["Streamed"].NativeValue = false;

            instance["SpatialDataSourceId"].StringValue = instance.InstanceId;

            return instance;

            }

        private JObject GetJsonWithCache (string sourceID)
            {
            JObject json;
            if ( jsonCache == null || jsonCache.Item1 != sourceID )
                {
                json = m_usgsDataFetcher.GetSciencebaseJson(sourceID);
                jsonCache = new Tuple<string, JObject>(sourceID, json);
                }
            else
                {
                json = jsonCache.Item2;
                }
            return json;
            }

        //private void InitializePropertiesToNull (IECInstance instance, IECClass ecClass)
        //    {
        //    foreach ( IECProperty prop in ecClass )
        //        {
        //        instance[prop.Name].SetToNull();
        //        }
        //    }

        /// <summary>
        /// Query a specific SpatialEntity by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialEntity instance</returns>
        override protected IECInstance QuerySingleSpatialEntity (string sourceID)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECClass ecClass = Schema.GetClass("SpatialEntity");

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            instance.InstanceId = sourceID;

            instance["Id"].StringValue = sourceID;

            //Footprint
            var spatial = json["spatial"] as JObject;
            if ( spatial != null )
                {
                var bbox = spatial["boundingBox"] as JObject;
                if ( bbox != null )
                    {
                    instance["Footprint"].StringValue = DbGeometryHelpers.CreateFootprintString(bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY"), 4326);
                    }
                }

            instance["ApproximateFootprint"].NativeValue = false;

            //Name

            instance["Name"].StringValue = json.TryToGetString("title");

            //Keywords

            string keywords = null;

            if ( json["tags"] != null )
                {
                keywords = "";
                foreach ( JObject tag in json["tags"] )
                    {
                    string name = tag.TryToGetString("name");
                    if ( name != null )
                        {
                        keywords += name + ", ";

                        string scheme = tag.TryToGetString("scheme");
                        if ( (scheme != null) && (scheme == "The National Map Collection Thesaurus") )
                            {
                            var cat = m_usgsDataFetcher.CategoryTable.FirstOrDefault(c => c.SbDatasetTag == name);
                            if ( cat != null )
                                {
                                instance["Dataset"].StringValue = cat.SbDatasetTag;
                                instance["Classification"].StringValue = cat.Classification;
                                IUSGSDataExtractor dataExtractor = ReturnDataExtractor(cat.SbDatasetTag);
                                DateTime? date;
                                string res;
                                string resMeter;
                                dataExtractor.ExtractDateAndResolutionSB(json, out date, out res, out resMeter);

                                if ( date.HasValue )
                                    {
                                    instance["Date"].NativeValue = date.Value;
                                    }

                                instance["ResolutionInMeters"].StringValue = resMeter;

                                }
                            }
                        }
                    }
                keywords = keywords.TrimEnd(' ', ',');
                }

            instance["AccuracyInMeters"].SetToNull();

            //DataSourceTypesAvailable

            //If all else fails, this is set to null beforehand
            //instance["DataSourceTypesAvailable"].SetToNull();

            string type = ExtractDataSourceType(json);

            if ( type == null )
                {
                instance["DataSourceTypesAvailable"].SetToNull();
                }
            else
                {
                instance["DataSourceTypesAvailable"].StringValue = type;
                }

            instance["DataProvider"].StringValue = IndexConstants.UsgsDataProviderString;
            instance["DataProviderName"].StringValue = IndexConstants.UsgsDataProviderNameString;

            instance["ThumbnailURL"].SetToNull();

            JObject previewImage = json["previewImage"] as JObject;

            if ( previewImage != null )
                {
                if ( previewImage["thumbnail"] != null )
                    {
                    instance["ThumbnailURL"].StringValue = previewImage["thumbnail"].TryToGetString("uri");
                    }
                else
                    {
                    //In case the thumbnail object is not there, we take the first url we find. 
                    //I don't know if it is possible for the thumbnail object to not be there, but let's not take any risk.
                    foreach ( var imageObject in previewImage )
                        {
                        if ( imageObject.Value["uri"] != null )
                            {
                            instance["ThumbnailURL"].StringValue = imageObject.Value["uri"].Value<string>();
                            break;
                            }
                        }
                    }
                }

            instance.ExtendedDataValueSetter.Add("Complete", true);

            PrepareInstanceForCaching(instance);

            return instance;
            }

        private IUSGSDataExtractor ReturnDataExtractor (string dataset)
            {
            switch ( dataset )
                {
                case "High Resolution Orthoimagery":
                    return new HRODataExtractor();
                case "USDA National Agriculture Imagery Program (NAIP)":
                    return new NAIPDataExtractor();
                case "Digital Elevation Model (DEM) 1 meter":
                //return DatasetFilter(tokenList, minDate, maxDate, new NEDDataExtractor(dataset));
                case "National Elevation Dataset (NED) 1/9 arc-second":
                case "National Elevation Dataset (NED) 1/3 arc-second":
                case "National Elevation Dataset (NED) 1 arc-second":
                    return new NEDDataExtractor(dataset);
                case "National Land Cover Database (NLCD) - 2001":
                case "National Land Cover Database (NLCD) - 2006":
                case "National Land Cover Database (NLCD) - 2011":
                    return new NLCDDataExtractor(dataset);
                case "National Elevation Dataset (NED) 1/3 arc-second - Contours":
                case "National Hydrography Dataset (NHD) Medium Resolution":
                case "National Hydrography Dataset (NHD) Best Resolution":
                case "National Watershed Boundary Dataset (WBD)":
                case "National Structures Dataset (NSD)":
                case "National Transportation Dataset (NTD)":
                case "Lidar Point Cloud (LPC)":
                default:
                    return new DefaultDataExtractor();
                }
            }

        private string ExtractDataSourceType (JObject json)
            {

            //The commentend code sets the DataSourceType to Raster, Vector or Point.
            //For now, what we need is the type of file (JPEG2000, TIFF, LAS, etc...)

            //JArray files = json["files"] as JArray;
            //if (files != null)
            //{
            //    foreach (JObject file in files)
            //    {
            //        if ((file["contentType"].Value<string>() == "application/fgdc+xml") && file["originalMetadata"].Value<bool>() == true)
            //        {
            //            string xmlString = GetHttpResponse(file["url"].Value<string>());
            //            //using (XmlReader xmlReader = XmlReader.Create(xmlString))
            //            //{

            //            //}
            //            XmlDocument xmlDoc = new XmlDocument();
            //            xmlDoc.LoadXml(xmlString);

            //            //var nodeList = xmlDoc.SelectSingleNode("/metadata/spdoinfo/direct/");
            //            if (xmlDoc.SelectSingleNode("metadata/spdoinfo/direct") != null)
            //            {
            //                return xmlDoc.SelectSingleNode("metadata/spdoinfo/direct").InnerText;
            //            }

            //        }
            //    }
            //}

            //JArray weblinks = json["webLinks"] as JArray;

            //if (weblinks != null)
            //{
            //    foreach (JObject weblink in weblinks)
            //    {
            //        if (weblink["title"] != null)
            //        {
            //            switch (weblink["title"].Value<string>())
            //            {
            //                case "JPEG2000":
            //                    return "Raster";
            //                case "TIFF":
            //                    return "Raster";
            //                case "IMG":
            //                    return "Raster";
            //                case "Shapefile":
            //                    return "Vector";
            //                case "LAS":
            //                    return "Point";
            //            }
            //        }
            //    }
            //}

            JArray weblinks = json["webLinks"] as JArray;

            if ( weblinks != null )
                {
                foreach ( JObject weblink in weblinks )
                    {
                    if ( weblink["title"] != null )
                        {
                        switch ( weblink["title"].Value<string>() )
                            {
                            case "JPEG2000":
                                return "JPEG2000";
                            case "TIFF":
                                return "TIF";
                            case "IMG":
                                return "IMG";
                            case "Shapefile":
                                return "Shapefile";
                            case "LAS":
                                return "LAS";
                            }
                        }
                    }
                }

            return null;
            }

        /// <summary>
        /// Query a specific Metadata by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The Metadata instance</returns>
        override protected IECInstance QuerySingleMetadata (string sourceID)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECClass ecClass = Schema.GetClass("Metadata");

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            instance.InstanceId = sourceID;

            instance["Id"].StringValue = sourceID;

            //DisplayStyle

            instance["DisplayStyle"].SetToNull();

            //Description

            instance["Description"].StringValue = json.TryToGetString("summary");

            //ContactInformation

            instance["ContactInformation"].SetToNull();

            //Keywords

            string keywords = null;

            if ( json["tags"] != null )
                {
                keywords = "";
                foreach ( JObject tag in json["tags"] )
                    {
                    string name = tag.TryToGetString("name");

                    if ( name != null )
                        {
                        keywords += name + ", ";
                        }
                    }
                keywords = keywords.TrimEnd(' ', ',');
                }
            instance["Keywords"].StringValue = keywords;

            //Legal

            if ( json["title"] != null )
                {
                instance["Legal"].StringValue = json["title"].Value<string>() + IndexConstants.UsgsLegalString;
                }

            instance["TermsOfUse"].StringValue = IndexConstants.UsgsTermsOfUse;

            //Lineage

            instance["Lineage"].SetToNull();

            //Provenance

            instance["Provenance"].SetToNull();

            instance.ExtendedDataValueSetter.Add("Complete", true);

            PrepareInstanceForCaching(instance);

            return instance;
            }

        /// <summary>
        /// Query a specific SpatialDataSource by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialDataSource instance</returns>
        override protected IECInstance QuerySingleSpatialDataSource (string sourceID)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECClass ecClass = Schema.GetClass("SpatialDataSource");

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();

            instance.InitializePropertiesToNull();

            instance.InstanceId = sourceID;

            instance["Id"].StringValue = sourceID;

            // MainURL
            // FileSize (in KB)
            JArray weblinks = json["webLinks"] as JArray;

            if ( weblinks != null )
                {
                foreach ( JObject weblink in weblinks )
                    {

                    string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
                    if ( (weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())) )
                        {
                        instance["MainURL"].StringValue = weblink.TryToGetString("uri");
                        if ( weblink["length"] != null )
                            {
                            long size;
                            if ( Int64.TryParse(weblink.TryToGetString("length"), out size) )
                                {
                                instance["FileSize"].NativeValue = (long) (size / 1024);
                                }
                            }
                        break;
                        }
                    }
                }

            //CompoundType

            instance["CompoundType"].StringValue = "USGS";

            //LocationInCompound

            //instance["LocationInCompound"].StringValue = null;

            //DataSourceType

            string type = ExtractDataSourceType(json);

            if ( type == null )
                {
                instance["DataSourceType"].SetToNull();
                }
            else
                {
                instance["DataSourceType"].StringValue = type;
                }

            //SisterFiles

            instance["SisterFiles"].SetToNull();

            //Metadata

            instance["Metadata"].SetToNull();

            JArray files = json["files"] as JArray;
            if ( files != null )
                {
                foreach ( JObject file in files )
                    {
                    if ( (file["contentType"].Value<string>() == "application/fgdc+xml") && file["originalMetadata"].Value<bool>() == true )
                        {
                        instance["Metadata"].StringValue = file["url"].Value<string>();

                        break;
                        }
                    }
                }

            instance["Streamed"].NativeValue = false;

            instance.ExtendedDataValueSetter.Add("Complete", true);

            PrepareInstanceForCaching(instance);

            return instance;
            }

        /// <summary>
        /// Query a specific Server by its ID.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The Server instance</returns>
        override protected IECInstance QuerySingleServer (string sourceID)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECClass ecClass = Schema.GetClass("Server");

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();

            instance.InstanceId = sourceID;

            instance.InitializePropertiesToNull();

            instance["Id"].StringValue = sourceID;

            // CommunicationProtocol + URL

            JArray weblinks = json["webLinks"] as JArray;

            if ( weblinks != null )
                {
                foreach ( JObject weblink in weblinks )
                    {

                    string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
                    if ( (weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())) )
                        {
                        string[] splitURL = weblink["uri"].Value<string>().Split(new char[] { '/' }, StringSplitOptions.RemoveEmptyEntries);

                        //The communication protocol is the first part of the uri (http or ftp, usually).
                        string communicationProtocol = splitURL[0].TrimEnd(':');
                        instance["CommunicationProtocol"].StringValue = communicationProtocol;

                        instance["URL"].StringValue = communicationProtocol + "://" + splitURL[1];
                        break;
                        }
                    }
                }

            instance["Streamed"].NativeValue = false;

            //Name

            instance["Name"].SetToNull();

            //ServerContactInformation

            instance["ServerContactInformation"].SetToNull();

            //Fees
            bool setToNull = true;
            JArray files = json["files"] as JArray;
            if ( files != null )
                {
                foreach ( JObject file in files )
                    {
                    if ( (file["contentType"].Value<string>() == "application/fgdc+xml") && file["originalMetadata"].Value<bool>() == true )
                        {
                        XmlDocument xmlDoc = m_usgsDataFetcher.GetXmlDocFromURL(file["url"].Value<string>());

                        var feesNode = xmlDoc.SelectSingleNode("metadata/distinfo/stdorder/fees");
                        if ( feesNode != null )
                            {
                            instance["Fees"].StringValue = feesNode.InnerText;

                            setToNull = false;
                            break;
                            }

                        }
                    }
                }
            if ( setToNull )
                {
                instance["Fees"].SetToNull();
                }
            //Legal

            if ( json["title"] != null )
                {
                instance["Legal"].StringValue = json["title"].Value<string>() + IndexConstants.UsgsLegalString;
                }

            //AccessConstraints

            instance["AccessConstraints"].SetToNull();

            //Online

            instance["Online"].SetToNull();

            //LastCheck

            instance["LastCheck"].SetToNull();

            //LastTimeOnline

            instance["LastTimeOnline"].SetToNull();

            //Latency

            instance["Latency"].SetToNull();

            //MeanReachabilityStats

            instance["MeanReachabilityStats"].SetToNull();

            //State

            instance["State"].SetToNull();

            //Type

            instance["Type"].SetToNull();

            instance.ExtendedDataValueSetter.Add("Complete", true);

            PrepareInstanceForCaching(instance);

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
            List<IECInstance> instanceList;

            IECClass ecClass = Schema.GetClass("SpatialEntityWithDetailsView");

            //We fetch beforehand the instances in the cache.
            PolygonModel model = DbGeometryHelpers.CreatePolygonModelFromJson(polygon);
            string polygonWKT = DbGeometryHelpers.CreateWktPolygonString(model.Points);

            PolygonDescriptor polyDesc = new PolygonDescriptor
            {
                WKT = polygonWKT,
                SRID = model.coordinate_system
            };

            var cacheCriteriaList = new List<SingleWhereCriteriaHolder>(criteriaList);
            cacheCriteriaList.Add(new SingleWhereCriteriaHolder()
            {
                Property = ecClass["Classification"], Operator = RelationalOperator.ISNOTNULL, Value = null
            });

            List<IECInstance> cachedinstanceList = InstanceCacheManager.QuerySpatialInstancesFromCache(polyDesc, ecClass, ecClass, Query.SelectClause, cacheCriteriaList);

            CompleteInstances(cachedinstanceList, ecClass);

            try
                {
                instanceList = new List<IECInstance>();
                foreach ( var bundle in m_usgsDataFetcher.GetNonFormattedUSGSResults(criteriaList) )
                    {
                    var FilteredList = SpecialFilteringAndExtracting(bundle.jtokenList, bundle.Dataset, criteriaList);

                    foreach ( var item in FilteredList )
                        {

                        IECInstance instance = ecClass.CreateInstance();

                        instance.InitializePropertiesToNull();

                        JToken jtoken = item.jToken;

                        instance.InstanceId = jtoken.TryToGetString("sourceId");
                        instance["Id"].StringValue = instance.InstanceId;

                        if ( m_usgsDataFetcher.Blacklist.Any(id => id == instance.InstanceId) )
                            {
                            //This Id is blacklisted. We don't add it to the instanceList.
                            continue;
                            }

                        string downloadURL = jtoken.TryToGetString("downloadURL");

                        if ( (downloadURL != null) && (downloadURL.Contains("arcgis")) && (downloadURL.Contains("rest")) && (downloadURL.Contains("services")) )
                            {
                            //This is a server entry. We don't add it to the instanceList.
                            continue;
                            }

                        CreateIncompleteCacheInstances(item, bundle.Classification, bundle.DatasetId, bundle.Dataset);

                        instance["SpatialDataSourceId"].StringValue = instance.InstanceId;

                        //instance["Name"].StringValue = jtoken.TryToGetString("title");
                        instance["Name"].StringValue = item.Title;

                        instance["Legal"].StringValue = item.Title + IndexConstants.UsgsLegalString;

                        instance["TermsOfUse"].StringValue = IndexConstants.UsgsTermsOfUse;

                        var bbox = jtoken["boundingBox"];

                        if ( bbox != null )
                            {
                            instance["Footprint"].StringValue = "{ \"points\" : " + String.Format("[[{0},{1}],[{2},{1}],[{2},{3}],[{0},{3}],[{0},{1}]]", bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY")) + ", \"coordinate_system\" : \"4326\" }";
                            }

                        instance["ApproximateFootprint"].NativeValue = false;

                        instance["DataSourceType"].StringValue = jtoken.TryToGetString("format");

                        var urls = jtoken["urls"];
                        if ( urls != null )
                            {
                            instance["ThumbnailURL"].StringValue = urls.TryToGetString("previewGraphicURL");
                            }
                        instance["MetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/" + instance.InstanceId;
                        instance["RawMetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/download/" + instance.InstanceId + IndexConstants.UsgsRawMetadataURLEnding;
                        instance["RawMetadataFormat"].StringValue = IndexConstants.UsgsRawMetadataFormatString;
                        instance["SubAPI"].StringValue = IndexConstants.UsgsSubAPIString;

                        instance["DataProvider"].StringValue = IndexConstants.UsgsDataProviderString;
                        instance["DataProviderName"].StringValue = IndexConstants.UsgsDataProviderNameString;
                        instance["Dataset"].StringValue = bundle.Dataset;

                        if ( item.Date.HasValue )
                            {
                            instance["Date"].NativeValue = item.Date.Value;
                            }
                        instance["AccuracyInMeters"].SetToNull();
                        instance["ResolutionInMeters"].StringValue = item.ResolutionInMeters;

                        instance["Classification"].StringValue = bundle.Classification;

                        if ( jtoken["sizeInBytes"] != null )
                            {
                            long size;
                            if ( Int64.TryParse(jtoken.TryToGetString("sizeInBytes"), out size) )
                                {
                                instance["FileSize"].NativeValue = (long) (size / 1024);
                                }
                            }

                        instance["Streamed"].NativeValue = false;

                        instanceList.Add(instance);
                        }
                    }
                foreach ( IECInstance cachedInstance in cachedinstanceList )
                    {
                    if ( !instanceList.Any(inst => inst.InstanceId == cachedInstance.InstanceId) && !m_usgsDataFetcher.Blacklist.Any(id => id == cachedInstance.InstanceId) )
                        {
                        instanceList.Add(cachedInstance);
                        }
                    }

                }
            catch ( EnvironmentalException )
                {
                //USGS returned an error. We will return the results contained in the cache.
                instanceList = cachedinstanceList.Where(inst => !m_usgsDataFetcher.Blacklist.Any(id => id == inst.InstanceId)).ToList();
                }
            catch ( System.AggregateException ex )
                {
                if ( ex.InnerExceptions.All(e => e.GetType() == typeof(EnvironmentalException)) )
                    {
                    //USGS returned an error. We will return the results contained in the cache.
                    instanceList = cachedinstanceList.Where(inst => !m_usgsDataFetcher.Blacklist.Any(id => id == inst.InstanceId)).ToList();
                    }
                else
                    {
                    throw;
                    }
                }

            return instanceList;
            }

        private IEnumerable<USGSExtractedResult> SpecialFilteringAndExtracting (IEnumerable<JToken> tokenList, string dataset, List<SingleWhereCriteriaHolder> criteriaList)
            {

            DateTime? minDate = null;
            DateTime? maxDate = null;

            foreach ( var criteria in criteriaList )
                {
                if ( criteria.Property.Name == "Date" )
                    {
                    DateTime givenDate;
                    bool success = DateTime.TryParse(criteria.Value, out givenDate);
                    if ( success )
                        {
                        if ( (criteria.Operator == RelationalOperator.LT) || (criteria.Operator == RelationalOperator.LTEQ) )
                            {
                            if ( !maxDate.HasValue || (maxDate.Value > givenDate) )
                                {
                                maxDate = givenDate;
                                }
                            }
                        if ( criteria.Operator == RelationalOperator.GT || (criteria.Operator == RelationalOperator.GTEQ) )
                            {
                            if ( !minDate.HasValue || (minDate.Value < givenDate) )
                                {
                                minDate = givenDate;
                                }
                            }
                        if ( criteria.Operator == RelationalOperator.EQ )
                            {
                            if ( !maxDate.HasValue || (maxDate.Value > givenDate) )
                                {
                                maxDate = givenDate;
                                }
                            if ( !minDate.HasValue || (minDate.Value < givenDate) )
                                {
                                minDate = givenDate;
                                }
                            }
                        }
                    }
                }

            IUSGSDataExtractor dataExtractor = ReturnDataExtractor(dataset);

            return DatasetFilter(tokenList, minDate, maxDate, dataExtractor);

            }

        /// <summary>
        /// Completes the fields that are not contained in the database, and are fixed by the sub api.
        /// </summary>
        /// <param name="cachedInstances">The instances to complete.</param>
        /// <param name="ecClass">The ECClass of the instances.</param>
        override protected void CompleteInstances (List<IECInstance> cachedInstances, IECClass ecClass)
            {
            switch ( ecClass.Name )
                {
                case "SpatialEntityWithDetailsView":
                        {
                        foreach ( IECInstance inst in cachedInstances )
                            {
                            inst["MetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/" + inst.InstanceId;
                            inst["RawMetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/download/" + inst.InstanceId + IndexConstants.UsgsRawMetadataURLEnding;
                            inst["RawMetadataFormat"].StringValue = IndexConstants.UsgsRawMetadataFormatString;
                            inst["SubAPI"].StringValue = IndexConstants.UsgsSubAPIString;
                            }
                        break;
                        }
                default:
                    //Do nothing
                    break;
                }
            }

        private void CreateIncompleteCacheInstances (USGSExtractedResult item, string classification, string parentId, string parentName)
            {
            IECInstance SEInstance = Schema.GetClass("SpatialEntity").CreateInstance();
            IECInstance metadataInstance = Schema.GetClass("Metadata").CreateInstance();
            IECInstance SDSInstance = Schema.GetClass("SpatialDataSource").CreateInstance();

            SEInstance.ExtendedDataValueSetter.Add("Complete", false);
            metadataInstance.ExtendedDataValueSetter.Add("Complete", false);
            SDSInstance.ExtendedDataValueSetter.Add("Complete", false);

            JToken jtoken = item.jToken;

            string instanceId = jtoken.TryToGetString("sourceId");

            SEInstance.InstanceId = instanceId;
            metadataInstance.InstanceId = instanceId;
            SDSInstance.InstanceId = instanceId;

            SEInstance["Id"].StringValue = instanceId;
            metadataInstance["Id"].StringValue = instanceId;
            SDSInstance["Id"].StringValue = instanceId;

            SEInstance["Name"].StringValue = item.Title;

            var bbox = jtoken["boundingBox"];

            if ( bbox != null )
                {
                SEInstance["Footprint"].StringValue = "{ \"points\" : " + String.Format("[[{0},{1}],[{2},{1}],[{2},{3}],[{0},{3}],[{0},{1}]]", bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY")) + ", \"coordinate_system\" : \"4326\" }";
                }

            SEInstance["ApproximateFootprint"].NativeValue = false;

            SDSInstance["DataSourceType"].StringValue = jtoken.TryToGetString("format");
            //SDSInstance["LocationInCompound"].StringValue = null;
            var urls = jtoken["urls"];
            if ( urls != null )
                {
                SDSInstance["MainURL"].StringValue = urls.TryToGetString("downloadURL");
                SEInstance["ThumbnailURL"].StringValue = urls.TryToGetString("previewGraphicURL");
                }

            SEInstance["DataSourceTypesAvailable"].StringValue = jtoken.TryToGetString("format");

            metadataInstance["Legal"].StringValue = item.Title + IndexConstants.UsgsLegalString;

            metadataInstance["TermsOfUse"].StringValue = IndexConstants.UsgsTermsOfUse;

            if ( item.Date.HasValue )
                {
                SEInstance["Date"].NativeValue = item.Date.Value;
                }
            SEInstance["AccuracyInMeters"].SetToNull();
            SEInstance["ResolutionInMeters"].StringValue = item.ResolutionInMeters;

            SEInstance["DataProvider"].StringValue = IndexConstants.UsgsDataProviderString;
            SEInstance["DataProviderName"].StringValue = IndexConstants.UsgsDataProviderNameString;

            SEInstance["Dataset"].StringValue = parentName;

            SEInstance["Classification"].StringValue = classification;

            if ( jtoken["sizeInBytes"] != null )
                {
                long size;
                if ( Int64.TryParse(jtoken.TryToGetString("sizeInBytes"), out size) )
                    {
                    SDSInstance["FileSize"].NativeValue = (long) (size / 1024);
                    }
                }

            PrepareInstanceForCaching(SEInstance);
            PrepareInstanceForCaching(metadataInstance);
            PrepareInstanceForCaching(SDSInstance);

            }

        /// <summary>
        /// Dataset special filtering. We eliminate duplicate
        /// entries (having the same title) and filter by date.
        /// </summary>
        /// <param name="tokenList">The complete list of json entries of the HRO dataset obtained from USGS api</param>
        /// <param name="minDate">Lower bound for the date filter</param>
        /// <param name="maxDate">Upper bound for the date filter</param>
        /// <param name="extractor">An implementation of IUSGSDataExtractor, which will extract the date and resolution according to the dataset</param>
        /// <returns>The list of entries, exempt from superfluous data</returns>
        private IEnumerable<USGSExtractedResult> DatasetFilter (IEnumerable<JToken> tokenList, DateTime? minDate, DateTime? maxDate, /*bool takeMostRecentOnly,*/ IUSGSDataExtractor extractor)
            {
            List<USGSExtractedResult> results = new List<USGSExtractedResult>();

            foreach ( JToken newToken in tokenList )
                {
                string title;
                DateTime? newTokenDate;
                string newTokenResolution;
                string newTokenResolutionInMeters;

                extractor.ExtractTitleDateAndResolution(newToken, out title, out newTokenDate, out newTokenResolution, out newTokenResolutionInMeters);

                //Check if date is between the range

                if ( (minDate.HasValue) && (newTokenDate.HasValue) && (newTokenDate < minDate.Value) )
                    {
                    //It is not in the requested time lapse. We skip this one.
                    continue;
                    }
                if ( (maxDate.HasValue) && (newTokenDate.HasValue) && (newTokenDate > maxDate.Value) )
                    {
                    //It is not in the requested time lapse. We skip this one.
                    continue;
                    }

                if ( (title != null) && (!results.Any(r => r.Title == title)) )
                    {
                    results.Add(new USGSExtractedResult
                    {
                        jToken = newToken, Title = title, Date = newTokenDate, Resolution = newTokenResolution, ResolutionInMeters = newTokenResolutionInMeters
                    });
                    }
                }
            //if(takeMostRecentOnly)
            //{
            //    //If we take only the most recent, all of the entries we select are already in the dictionary
            //    return bboxDictionary.Values;
            //}
            return results;
            }
        }
    }
