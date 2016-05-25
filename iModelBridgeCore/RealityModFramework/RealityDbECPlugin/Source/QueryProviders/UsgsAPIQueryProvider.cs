using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source.FileRetrievalControllers;
using IndexECPlugin.Source.Helpers;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using System.Xml;


namespace IndexECPlugin.Source.QueryProviders
    {
    /// <summary>
    /// UsgsAPIQueryProvider's purpose is to transform USGS data into ECInstances for our plugin.
    /// </summary>
    public class UsgsAPIQueryProvider : IECQueryProvider
        {

        ECQuery m_query;
        ECQuerySettings m_querySettings;
        IUSGSDataFetcher m_usgsDataFetcher;

        Tuple<string, JObject> jsonCache;

        Dictionary<string, IECInstance> m_storedParents;

        /// <summary>
        /// UsgsAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="usgsDataFetcher">The usgsDataFetcher that will be used to query USGS</param>
        public UsgsAPIQueryProvider (ECQuery query, ECQuerySettings querySettings, IUSGSDataFetcher usgsDataFetcher)
            {
            m_query = query;
            m_querySettings = querySettings;
            m_usgsDataFetcher = usgsDataFetcher;
            }

        /// <summary>
        /// UsgsAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        public UsgsAPIQueryProvider (ECQuery query, ECQuerySettings querySettings)
            {
            m_query = query;
            m_querySettings = querySettings;
            m_usgsDataFetcher = new USGSDataFetcher(query);
            }

        /// <summary>
        /// This method returns the instances according to the ECQuery received in the constructor.
        /// The instances are created from the results of a query of the USGS database
        /// </summary>
        /// <returns>An IEnumerable containing the instances requested in the ECQuery received in the constructor</returns>
        public IEnumerable<IECInstance> CreateInstanceList ()
            {
            //This method will map to other methods, depending on the search class in the query

            Log.Logger.info("Fetching USGS results for query " + m_query.ID);

            List<IECInstance> instanceList = null;

            IECClass ecClass = m_query.SearchClasses.First().Class;

            int i = 0;
            while ( (i < m_query.WhereClause.Count) && (instanceList == null) )
                {
                //We'll take the first ECInstanceIdExpression criterion. Normally, there is only one, non embedded criterion of this sort when queried by WSG.
                if ( m_query.WhereClause[i] is ECInstanceIdExpression )
                    {
                    instanceList = new List<IECInstance>();

                    ECInstanceIdExpression instanceIDExpression = (ECInstanceIdExpression) m_query.WhereClause[i];

                    if ( instanceIDExpression.InstanceIdSet.Count() == 0 )
                        {
                        throw new UserFriendlyException("Please specify at least one ID in this type of where clause");
                        }

                    //We create the requested instances
                    foreach ( string sourceID in instanceIDExpression.InstanceIdSet )
                        {
                        IECInstance instance = CreateInstanceFromID(ecClass, sourceID);
                        CreateRelatedInstance(instance, m_query.SelectClause.SelectedRelatedInstances);
                        if ( instance != null )
                            {
                            instanceList.Add(instance);
                            }

                        }

                    //return instanceList;
                    }
                i++;
                }

            if ( instanceList == null )
                {
                // If we're here, that's because there was a polygon parameter in the extended data
                //TODO : Implement this verification

                switch ( ecClass.Name )
                    {
                    //case "USGSEntity": //To be removed
                    //    return QueryUSGSEntitiesByPolygon();
                    //case "USGSThumbnail":
                    //    return QueryUSGSThumbnail();
                    case "SpatialEntityWithDetailsView":
                        instanceList = QuerySpatialEntitiesWithDetailsViewByPolygon();
                        break;
                    default:
                        throw new UserFriendlyException("It is impossible to query instances of the class \"" + ecClass.Name + "\" in a USGS spatial request. The only class allowed is \"SpatialEntityWithDetailsView\".");
                    }
                }

            //Related instances of each instance found before
            //foreach ( var instance in instanceList )
            //    {
            //    CreateRelatedInstance(instance, m_query.SelectClause.SelectedRelatedInstances);
            //    }

            return instanceList;

            }

        private void CreateRelatedInstance (IECInstance instance, List<RelatedInstanceSelectCriteria> relatedCriteriaList)
            {
            foreach ( RelatedInstanceSelectCriteria crit in relatedCriteriaList )
                {
                IECClass relatedClass = crit.RelatedClassSpecifier.RelatedClass;

                IECRelationshipClass relationshipClass = crit.RelatedClassSpecifier.RelationshipClass;

                //We do not use SpatialEntityDatasetToAlternateDataset for USGS data. DetailsViewToChildren is done in QuerySpatialEntitiesWithDetailsViewByPolygon
                if ( (relationshipClass.Name == "SpatialEntityDatasetToAlternateDataset") || (relationshipClass.Name == "DetailsViewToChildren") )
                    {
                    continue;
                    }

                string relInstID;
                if (/*(relationshipClass.Name != "DetailsViewToChildren") && */(relationshipClass.Name != "SpatialEntityDatasetToSpatialEntityBase") )
                    {
                    relInstID = instance.InstanceId;
                    }
                else
                    {

                    if ( crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Forward )
                        {
                        //TODO : See if it is possible and useful to go in the direction parent->children
                        continue;
                        }
                    else
                        {
                        //Here, we are sure to be in a SpatialEntityBase, which should contain a ParentDatasetIdStr attribute 

                        //relInstID = instance["ParentDatasetIdStr"].StringValue;
                        relInstID = instance.ExtendedData["ParentDatasetIdStr"] as string;
                        instance.ExtendedData.Remove("ParentDatasetIdStr");
                        }

                    }

                IECInstance relInst = CreateInstanceFromID(relatedClass, relInstID);
                if ( relInst != null )
                    {
                    IECRelationshipInstance relationshipInst;
                    if ( crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Forward )
                        {
                        relationshipInst = relationshipClass.CreateRelationship(instance, relInst);
                        }
                    else
                        {
                        relationshipInst = relationshipClass.CreateRelationship(relInst, instance);
                        }
                    //relationshipInst.InstanceId = "test";
                    instance.GetRelationshipInstances().Add(relationshipInst);
                    }
                CreateRelatedInstance(relInst, crit.SelectedRelatedInstances);
                }
            }

        private IECInstance CreateInstanceFromID (IECClass ecClass, string sourceID)
            {

            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }
            switch ( ecClass.Name )
                {
                case "SpatialEntityBase":
                case "SpatialEntity":
                    return QuerySingleSpatialEntityBase(sourceID, ecClass);
                //case "SpatialEntityWithDetailsView":
                //    return QuerySingleSpatialEntityWithDetailsView();
                case "SpatialEntityDataset":
                    return QuerySingleSpatialEntityDataset(sourceID, ecClass);
                case "Thumbnail":
                    return QuerySingleThumbnail(sourceID, ecClass);
                case "Metadata":
                    return QuerySingleMetadata(sourceID, ecClass);
                case "SpatialDataSource":
                case "OtherSource":
                    return QuerySingleSpatialDataSource(sourceID, ecClass);
                case "Server":
                    return QuerySingleServer(sourceID, ecClass);
                default:
                    throw new UserFriendlyException("It is impossible to query instances of the class \"" + ecClass.Name + "\" in a USGS request by ID.");
                }
            }

        private IECInstance QuerySingleSpatialEntityWithDetailsView (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json = GetJsonWithCache(sourceID);

            instance["Id"].StringValue = sourceID;

            instance.InstanceId = json.TryToGetString("id");

            instance["Name"].StringValue = json.TryToGetString("title");


            instance["Footprint"].SetToNull();

            var spatialObject = json["spatial"] as JObject;

            if ( spatialObject != null )
                {
                var bbox = spatialObject["boundingBox"] as JObject;

                if ( bbox != null )
                    {
                    instance["Footprint"].StringValue = String.Format(@"{{ ""points"" : [[{0},{1}],[{2},{1}],[{2},{3}],[{0},{3}],[{0},{1}]], ""coordinate_system"" : ""4326"" }}", bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY"));
                    }
                }

            string type = ExtractDataSourceType(json);

            if ( type == null )
                {
                instance["DataSourceType"].SetToNull();
                }
            else
                {
                instance["DataSourceType"].StringValue = type;
                }

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

            instance["MetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/" + instance.InstanceId;
            instance["RawMetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/download/" + instance.InstanceId + "?format=fgdc";
            instance["RawMetadataFormat"].StringValue = "FGDC";
            instance["SubAPI"].StringValue = "USGS";

            instance["DataProvider"].StringValue = "USGS";
            instance["DataProviderName"].StringValue = "United States Geological Survey";

            //instance["ParentDatasetIdStr"].StringValue = json["parentId"].Value<string>();
            return instance;

            }

        private IECInstance QuerySingleSpatialEntityDataset (string sourceID, IECClass ecClass)
            {
            IECInstance dataset = QuerySingleSpatialEntityBase(sourceID, ecClass);

            dataset["Processable"].SetToNull();

            return dataset;
            }

        private IECInstance QuerySingleServer (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json;
            json = GetJsonWithCache(sourceID);

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
                instance["Legal"].StringValue = json["title"].Value<string>() + " courtesy of the U.S. Geological Survey";
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

        private IECInstance QuerySingleSpatialDataSource (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json = GetJsonWithCache(sourceID);

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

            instance["LocationInCompound"].StringValue = "Unknown";

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

            return instance;
            }

        private IECInstance QuerySingleMetadata (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json = GetJsonWithCache(sourceID);

            instance["Id"].StringValue = sourceID;

            //RawMetadata

            instance["RawMetadata"].SetToNull();

            //RawMetadataFormat

            instance["RawMetadataFormat"].StringValue = "FGDC";

            //DisplayStyle

            instance["DisplayStyle"].SetToNull();

            //Description

            instance["Description"].StringValue = json.TryToGetString("summary");

            //ContactInformation

            instance["ContactInformation"].SetToNull();

            //Keywords

            string keywords = "";
            foreach ( JObject tag in json["tags"] )
                {
                string name = tag.TryToGetString("name");

                if ( name != null )
                    {
                    keywords += name + ", ";
                    }
                }

            instance["Keywords"].StringValue = keywords.TrimEnd(' ', ',');

            //Legal

            if ( json["title"] != null )
                {
                instance["Legal"].StringValue = json["title"].Value<string>() + " courtesy of the U.S. Geological Survey";
                }

            //Lineage

            instance["Lineage"].SetToNull();

            //Provenance

            instance["Provenance"].SetToNull();

            return instance;

            }

        private IECInstance QuerySingleThumbnail (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json = GetJsonWithCache(sourceID);

            instance["Id"].StringValue = sourceID;

            //previewImage is not always there, we have to check that it is not null
            JObject previewImage = json["previewImage"] as JObject;

            if ( previewImage == null )
                {
                return null;
                }

            //ThumbnailProvenance
            instance["ThumbnailProvenance"].SetToNull();

            //ThumbnailURI

            string thumbnailURI = null;

            if ( previewImage["thumbnail"] != null )
                {
                thumbnailURI = previewImage["thumbnail"].TryToGetString("uri");
                }
            else
                {
                //In case the thumbnail object is not there, we take the first uri we find. 
                //I don't know if it is possible for the thumbnail object to not be there, but let's not take any risk.
                foreach ( var imageObject in previewImage )
                    {
                    if ( imageObject.Value["uri"] != null )
                        {
                        thumbnailURI = imageObject.Value["uri"].Value<string>();
                        break;
                        }
                    }
                }


            if ( thumbnailURI == null )
                {
                throw new OperationFailedException("We have encountered a problem processing the order for the thumbnail of ID " + sourceID);
                }

            //The next part enables downloading the thumbnail

            if ( (m_querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None )
                {
                MemoryStream thumbnailStream = DownloadThumbnail(thumbnailURI);

                StreamBackedDescriptor streamDescriptor = new StreamBackedDescriptor(thumbnailStream, ExtractNameFromURI(thumbnailURI), thumbnailStream.Length, DateTime.UtcNow);
                StreamBackedDescriptorAccessor.SetIn(instance, streamDescriptor);
                }


            //ThumbnailFormat

            instance["ThumbnailFormat"].StringValue = ExtractFormatFromURI(thumbnailURI);

            //ThumbnailWidth

            instance["ThumbnailWidth"].SetToNull();

            //ThumbnailHeight

            instance["ThumbnailHeight"].SetToNull();

            //ThumbnailStamp

            instance["ThumbnailStamp"].SetToNull();

            //ThumbnailGenerationDetails

            instance["ThumbnailGenerationDetails"].SetToNull();

            return instance;
            }

        private string ExtractFormatFromURI (string uri)
            {
            string[] splitURI = uri.Split('.');
            string extension = splitURI[splitURI.Length - 1].ToLower();
            switch ( extension )
                {
                case "jpeg":
                case "jpg":
                case "png":
                case "gif":
                case "jp2":
                    return extension;
                default:
                    return "Unknown";
                }
            //return splitURI[splitURI.Length - 1];
            }

        private string ExtractNameFromURI (string uri)
            {
            string[] splitURI = uri.Split('/');

            return splitURI[splitURI.Length - 1];
            }

        /// <summary>
        /// Returns the informations of a file of specified source ID on sciencebase.gov (USGS)
        /// </summary>
        /// <param name="sourceID">The source ID of the item in USGS (sciencebase)</param>
        /// <param name="ecClass">The class to instanciate</param>
        /// <returns></returns>
        private IECInstance QuerySingleSpatialEntityBase (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = sourceID;

            JObject json = GetJsonWithCache(sourceID);

            instance["Id"].StringValue = sourceID;

            //Footprint
            var spatial = json["spatial"] as JObject;
            if ( spatial != null )
                {
                var bbox = spatial["boundingBox"] as JObject;
                if ( bbox != null )
                    {
                    instance["Footprint"].StringValue = String.Format(@"{{ ""points"" : [[{0},{1}],[{2},{1}],[{2},{3}],[{0},{3}],[{0},{1}]], ""coordinate_system"" : ""4326"" }}", bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY"));
                    }
                }
            //Name

            instance["Name"].StringValue = json.TryToGetString("title");

            //Keywords

            string keywords = "";
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
                            instance["Classification"].StringValue = cat.Classification;
                            }
                        }
                    }
                }

            instance["Keywords"].StringValue = keywords.TrimEnd(' ', ',');

            //AssociateFile

            instance["AssociateFile"].SetToNull();

            //ProcessingDescription

            instance["ProcessingDescription"].SetToNull();

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

            //AccuracyResolutionDensity

            instance["AccuracyResolutionDensity"].SetToNull();

            instance["DataProvider"].StringValue = "USGS";
            instance["DataProviderName"].StringValue = "United States Geological Survey";

            //instance["ParentDatasetIdStr"].StringValue = json.TryToGetString("parentId");

            //This happens when we request the parent of the SpatialEntityBase
            if ( m_query.SelectClause.SelectedRelatedInstances.Any(relCrit => relCrit.RelatedClassSpecifier.RelationshipClass.Name == "SpatialEntityDatasetToSpatialEntityBase" &&
                                                                              relCrit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Backward) )
                {
                instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", json.TryToGetString("parentId"));
                }


            return instance;
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
                                return "TIFF";
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

        private List<IECInstance> QuerySpatialEntitiesWithDetailsViewByPolygon ()
            {
            List<IECInstance> instanceList = new List<IECInstance>();

            //This part is an optimization only for queries of SpatialEntitiesWithDetailsView and their parents
            var relCrit = m_query.SelectClause.SelectedRelatedInstances.FirstOrDefault(crit => (crit.RelatedClassSpecifier.RelationshipClass.Name == "DetailsViewToChildren")
                                                                                       && (crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Backward));
            if ( relCrit != null )
                {
                m_storedParents = new Dictionary<string, IECInstance>();
                }

            var criteriaList = ExtractPropertyWhereClauses();

            foreach ( var bundle in m_usgsDataFetcher.GetNonFormattedUSGSResults(criteriaList) )
                {



                var FilteredList = SpecialFilteringAndExtracting(bundle.jtokenList, bundle.Dataset, criteriaList);



                foreach ( var item in FilteredList )
                    {
                    IECClass ecClass = m_query.SearchClasses.First().Class;
                    IECInstance instance = ecClass.CreateInstance();

                    InitializePropertiesToNull(instance, ecClass);

                    JToken jtoken = item.jToken;

                    instance.InstanceId = jtoken.TryToGetString("sourceId");
                    instance["Id"].StringValue = instance.InstanceId;
                    instance["SpatialDataSourceId"].StringValue = instance.InstanceId;

                    //instance["Name"].StringValue = jtoken.TryToGetString("title");
                    instance["Name"].StringValue = item.Title;

                    var bbox = jtoken["boundingBox"];

                    if ( bbox != null )
                        {
                        instance["Footprint"].StringValue = "{ \"points\" : " + String.Format("[[{0},{1}],[{2},{1}],[{2},{3}],[{0},{3}],[{0},{1}]]", bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY")) + ", \"coordinate_system\" : \"4326\" }";
                        }
                    instance["DataSourceType"].StringValue = jtoken.TryToGetString("format");
                    instance["ThumbnailURL"].StringValue = jtoken.TryToGetString("previewGraphicURL");
                    instance["MetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/" + instance.InstanceId;
                    instance["RawMetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/download/" + instance.InstanceId + "?format=fgdc";
                    instance["RawMetadataFormat"].StringValue = "FGDC";
                    instance["SubAPI"].StringValue = "USGS";

                    instance["DataProvider"].StringValue = "USGS";
                    instance["DataProviderName"].StringValue = "United States Geological Survey";

                    if ( item.Date.HasValue )
                        {
                        instance["Date"].NativeValue = item.Date.Value;
                        }
                    instance["AccuracyResolutionDensity"].StringValue = item.Resolution;
                    instance["ResolutionInMeters"].StringValue = item.ResolutionInMeters;

                    instance["Classification"].StringValue = bundle.Classification;

                    //instance["ParentDatasetIdStr"].StringValue = tuple.Item2;

                    if ( m_storedParents != null )
                        {
                        string relInstId = bundle.DatasetId;
                        if ( !m_storedParents.ContainsKey(relInstId) )
                            {
                            m_storedParents.Add(relInstId, QuerySingleSpatialEntityWithDetailsView(relInstId, ecClass));
                            }
                        if ( m_storedParents[relInstId] != null )
                            {
                            var relationshipInst = relCrit.RelatedClassSpecifier.RelationshipClass.CreateRelationship(m_storedParents[relInstId], instance);

                            //relationshipInst.InstanceId = "test";
                            instance.GetRelationshipInstances().Add(relationshipInst);
                            }

                        }

                    if ( jtoken["sizeInBytes"] != null )
                        {
                        long size;
                        if ( Int64.TryParse(jtoken.TryToGetString("sizeInBytes"), out size) )
                            {
                            instance["FileSize"].NativeValue = (long) (size / 1024);
                            }
                        }

                    instanceList.Add(instance);
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

            //if(minDate.HasValue && maxDate.HasValue && minDate.Value > maxDate.Value)
            //{
            //    throw new UserFriendlyException("The date criteria is invalid.");
            //}

            //if(m_query.ExtendedData.ContainsKey("minDate"))
            //{
            //    string minDateString = m_query.ExtendedData["minDate"].ToString();
            //    minDate = DateTime.ParseExact(minDateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
            //}
            //if(m_query.ExtendedData.ContainsKey("maxDate"))
            //{
            //    string maxDateString = m_query.ExtendedData["maxDate"].ToString();
            //    maxDate = DateTime.ParseExact(maxDateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
            //}

            //bool takeMostRecentOnly = false;

            //if (m_query.ExtendedData.ContainsKey("mostRecent"))
            //{
            //    string mostRecentString = m_query.ExtendedData["mostRecent"].ToString();
            //    if (mostRecentString.ToLower() == "true")
            //    {
            //        takeMostRecentOnly = true;
            //    }


            //}

            switch ( dataset )
                {
                case "High Resolution Orthoimagery":
                    return DatasetFilter(tokenList, minDate, maxDate, /*takeMostRecentOnly,*/ new HRODataExtractor());
                case "USDA National Agriculture Imagery Program (NAIP)":
                    return DatasetFilter(tokenList, minDate, maxDate, /*takeMostRecentOnly,*/ new NAIPDataExtractor());
                case "Digital Elevation Model (DEM) 1 meter":
                //return DatasetFilter(tokenList, minDate, maxDate, new NEDDataExtractor(dataset));
                case "National Elevation Dataset (NED) 1/9 arc-second":
                case "National Elevation Dataset (NED) 1/3 arc-second":
                case "National Elevation Dataset (NED) 1 arc-second":
                    return DatasetFilter(tokenList, minDate, maxDate, new NEDDataExtractor(dataset));
                case "National Elevation Dataset (NED) 1/3 arc-second - Contours":
                case "National Hydrography Dataset (NHD) Medium Resolution":
                case "National Hydrography Dataset (NHD) Best Resolution":
                case "National Watershed Boundary Dataset (WBD)":
                case "National Structures Dataset (NSD)":
                case "National Transportation Dataset (NTD)":
                case "Lidar Point Cloud (LPC)":
                    return DatasetFilter(tokenList, minDate, maxDate, new DefaultDataExtractor());
                case "National Land Cover Database (NLCD) - 2001":
                case "National Land Cover Database (NLCD) - 2006":
                case "National Land Cover Database (NLCD) - 2011":
                    return DatasetFilter(tokenList, minDate, maxDate, new NLCDDataExtractor(dataset));
                default:
                    return tokenList.Select(token => new USGSExtractedResult
                    {
                        jToken = token, Date = null, Resolution = null
                    });
                }

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
            //Filter by bounding box
            //Dictionary<string, USGSExtractedResult> bboxDictionary = new Dictionary<string, USGSExtractedResult>();
            List<USGSExtractedResult> results = new List<USGSExtractedResult>();
            //if(!takeMostRecentOnly)
            //{
            //    results = new List<USGSExtractedResult>();
            //}

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

                var bboxString = newToken["boundingBox"].ToString();
                //if (bboxDictionary.ContainsKey(bboxString))
                //{

                //    //var firstTokenObject = bboxDictionary[bboxString];

                //    string firstTokenName = firstTokenObject.jToken.TryToGetString("title");
                //    string newTokenName = newToken.TryToGetString("title");

                //    if (firstTokenName == newTokenName)
                //    {
                //        //The title of the two entries is the same. It is a duplicate entry, so we skip
                //        continue;
                //    }
                //    //if (takeMostRecentOnly)
                //    //{
                //    //    string[] firstTitleSplit = firstTokenName.Split(' ');

                //    //    if (newTokenDate > firstTokenObject.Date)
                //    //    {
                //    //        bboxDictionary[bboxString] = new USGSExtractedResult{jToken = newToken, Date = newTokenDate, Resolution = newTokenResolution};
                //    //    }
                //    //}
                //    //else
                //    //{
                //        //If we do not take only the most recent, we can add it to the results directly
                //        results.Add(new USGSExtractedResult { jToken = newToken, Title = title, Date = newTokenDate, Resolution = newTokenResolution });
                //    //}
                //}
                if ( (title != null) && (!results.Any(r => r.Title == title)) )
                    {
                    //There is no result having the same title


                    //bboxDictionary[bboxString] = new USGSExtractedResult { jToken = newToken, Date = newTokenDate, Resolution = newTokenResolution };
                    //if (!takeMostRecentOnly)
                    //{
                    //    //If we do not take only the most recent, we can add it to the results directly
                    results.Add(new USGSExtractedResult
                    {
                        jToken = newToken, Title = title, Date = newTokenDate, Resolution = newTokenResolution, ResolutionInMeters = newTokenResolutionInMeters
                    });
                    //}
                    }
                }
            //if(takeMostRecentOnly)
            //{
            //    //If we take only the most recent, all of the entries we select are already in the dictionary
            //    return bboxDictionary.Values;
            //}
            return results;
            }

        private void InitializePropertiesToNull (IECInstance instance, IECClass ecClass)
            {
            foreach ( IECProperty prop in ecClass )
                {
                instance[prop.Name].SetToNull();
                }
            }

        //private IEnumerable<IECInstance> QueryUSGSThumbnail()
        //{
        //    List<IECInstance> instanceList = new List<IECInstance>();

        //    //Byte[] bytePayload = ExtractPayload();

        //    //string jsonString = Encoding.ASCII.GetString(bytePayload);

        //    //USGSPayload payload = JsonConvert.DeserializeObject<USGSPayload>(jsonString);

        //    IECClass ecClass = m_query.SearchClasses.First().Class;
        //    IECInstance instance = ecClass.CreateInstance();
        //    //instance["ThumbnailURL"].StringValue = payload.PreviewLink;

        //    instance.InstanceId = "531f256fe4b0193009ddaf19";

        //    USGSThumbnailRetrievalController c = new USGSThumbnailRetrievalController(instance);
        //    c.processThumbnailRetrieval();

        //    //using (HttpClient client = new HttpClient())
        //    //{
        //    //    using (HttpResponseMessage response = client.GetAsync(payload.PreviewLink).Result)
        //    //    {
        //    //        using (HttpContent content = response.Content)
        //    //        {
        //    //            ((ECBinaryValue)instance["ThumbnailData"]).BinaryValue = content.ReadAsByteArrayAsync().Result;
        //    //        }
        //    //    }
        //    //}



        //    instanceList.Add(instance);

        //    return instanceList;
        //}

        //private byte[] ExtractPayload()
        //{
        //    if (!m_query.ExtendedData.ContainsKey("payload"))
        //    {
        //        throw new UserFriendlyException("This request must contain a \"payload\" parameter in the form of a WKT polygon string.");
        //    }
        //    //throw new Exception(m_query.ExtendedData["payload"].GetType().FullName);
        //    string hexString = m_query.ExtendedData["payload"].ToString();

        //    int hexLength = hexString.Length;
        //    byte[] bytes = new byte[hexLength / 2];
        //    for (int i = 0; i < hexLength; i += 2)
        //        bytes[i / 2] = Convert.ToByte(hexString.Substring(i, 2), 16);
        //    return bytes;

        //}

        //This is an helper method we could put in another file. It belonged in the now unused USGSThumbnailRetrievalController
        private MemoryStream DownloadThumbnail (string thumbnailUri)
            {
            if ( thumbnailUri.StartsWith("ftp") )
                {
                //FtpWebRequest request = FtpWebRequest.Create(thumbnailUri) as FtpWebRequest;
                //request.Method = WebRequestMethods.Ftp.GetFileSize;

                //FtpWebResponse response = (FtpWebResponse)request.GetResponse();
                //contentLength = response.ContentLength;

                //request = FtpWebRequest.Create(thumbnailUri) as FtpWebRequest;
                //request.Method = WebRequestMethods.Ftp.DownloadFile;

                //response = (FtpWebResponse)request.GetResponse();

                //return response.GetResponseStream();
                using ( WebClient ftpClient = new WebClient() )
                    {

                    //string tempPath = Path.GetTempPath();
                    //string tempFilePath = Path.Combine(tempPath, Guid.NewGuid().ToString());
                    //using (Stream fstream = File.Create(tempFilePath))
                    //{
                    //    ftpClient.OpenRead(thumbnailUri).CopyTo(fstream);
                    //}
                    //return File.Open(tempFilePath, FileMode.Open);

                    using ( Stream image = ftpClient.OpenRead(thumbnailUri) )
                        {
                        MemoryStream imageInMemory = new MemoryStream();

                        image.CopyTo(imageInMemory);

                        return imageInMemory;
                        }
                    }

                }
            if ( thumbnailUri.StartsWith("http") )
                {
                using ( HttpClient client = new HttpClient() )
                    {
                    using ( HttpResponseMessage response = client.GetAsync(thumbnailUri).Result )
                        {
                        using ( HttpContent content = response.Content )
                            {

                            //contentLength = (content.Headers.ContentLength.HasValue ? content.Headers.ContentLength.Value : 0);

                            using ( Stream image = content.ReadAsStreamAsync().Result )
                                {
                                MemoryStream imageInMemory = new MemoryStream();
                                image.CopyTo(imageInMemory);
                                return imageInMemory;
                                }
                            }
                        }
                    }
                }
            else
                {
                throw new NotImplementedException("The download of the thumbnail located at " + thumbnailUri + " is not implemented yet.");
                }
            }

        private List<SingleWhereCriteriaHolder> ExtractPropertyWhereClauses ()
            {
            List<SingleWhereCriteriaHolder> singleWhereCriteriaList = new List<SingleWhereCriteriaHolder>();
            var whereCriteria = m_query.WhereClause;
            for ( int i = 0; i < whereCriteria.Count; i++ )
                {
                if ( i > 0 )
                    {
                    if ( whereCriteria.GetLogicalOperatorBefore(i) != LogicalOperator.AND )
                        {
                        throw new UserFriendlyException("Please use only AND operator to filter the USGS requests");
                        }
                    }
                WhereCriterion criterion = whereCriteria[i];
                if ( criterion is PropertyExpression )
                    {
                    PropertyExpression propertyExpression = (PropertyExpression) criterion;

                    singleWhereCriteriaList.Add(new SingleWhereCriteriaHolder
                    {
                        Property = propertyExpression.LeftSideProperty, Operator = propertyExpression.Operator, Value = propertyExpression.RightSideString
                    });
                    }
                if ( criterion is RelatedCriterion )
                    {
                    throw new UserFriendlyException("Related Criterions are not allowed on USGS requests");
                    }
                if ( criterion is WhereCriteria )
                    {
                    throw new UserFriendlyException("Complex expressions are not allowed on USGS requests");
                    }
                }

            return singleWhereCriteriaList;
            }

        }
    }
