using Bentley.EC.Persistence;
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

    internal class UsgsAPIQueryProvider : IECQueryProvider
    {

        //TODO : Add outputFormat=JSON in WebReq???
        const string WebReqCategories = "http://viewer.nationalmap.gov/tnmaccess/api/searchCategories?";
        const string WebReq = "http://viewer.nationalmap.gov/tnmaccess/api/searchProducts?bbox=_bbox&q=&start=&end=&dateType=&datasets=_datasets&prodFormats=_prodFormats&prodExtents=&polyCode=&polyType=&max=200&offset=0";
        //const int USGSIdLenght = 24;
        private readonly List<UsgsAPICategory> CategoryTable = new List<UsgsAPICategory> 
        { //new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "2 arc-second DEM - Alaska", Format = "none", Priority = 0, Type = ""},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1 meter DEM", Format = "IMG", Priority = 1, Type = "Terrain"},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1/9 arc-second DEM", Format = "IMG", Priority = 2, Type = "Terrain"},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1/3 arc-second DEM", Format = "IMG", Priority = 3, Type = "Terrain"},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1 arc-second DEM", Format = "IMG", Priority = 4, Type = "Terrain"},
          //new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1/3 arc-second Contours", Format = "Shapefile", Priority = 6, Type = "Terrain"},
          //new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "DEM Source (OPR)", Format = "none", Priority = 0, Type = ""},
          //new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "Ifsar Digital Surface Model (DSM)", Format = "none", Priority = 0, Type = ""},
          //new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "Ifsar Orthorectified Radar Image (ORI)", Format = "none", Priority = 0, Type = ""},
          //new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "Lidar Point Cloud (LPC)", Format = "LAS", Priority = 1, Type = "PointCloud"},
          //new UsgsAPICategory(){Title = "Hydrography (NHD) and Watersheds (WBD)", SubTitle = "National Hydrography Dataset (NHD) Best Resolution", Format = "Shapefile", Priority = 1, Type = "WaterBody"},
          //new UsgsAPICategory(){Title = "Hydrography (NHD) and Watersheds (WBD)", SubTitle = "National Hydrography Dataset (NHD) Medium Resolution", Format = "Shapefile", Priority = 2, Type = "WaterBody"},
          //new UsgsAPICategory(){Title = "National Land Cover Database (NLCD)", SubTitle = "National Land Cover Database (NLCD) - 2011", Format = "GeoTIFF", Priority = 1, Type = "Imagery"},
          //new UsgsAPICategory(){Title = "National Land Cover Database (NLCD)", SubTitle = "National Land Cover Database (NLCD) - 2006", Format = "GeoTIFF", Priority = 2, Type = "Imagery"},
          //new UsgsAPICategory(){Title = "National Land Cover Database (NLCD)", SubTitle = "National Land Cover Database (NLCD) - 2001", Format = "GeoTIFF", Priority = 3, Type = "Imagery"},
          //new UsgsAPICategory(){Title = "Hydrography (NHD) and Watersheds (WBD)", SubTitle = "National Watershed Boundary Dataset (WBD)", Format = "Shapefile", Priority = 1, Type = "WaterBody"},
          //new UsgsAPICategory(){Title = "Boundaries - National Boundary Dataset", SubTitle = "Boundaries - National Boundary Dataset", Format = "none", Priority = 0, Type = ""},
          new UsgsAPICategory(){Title = "Imagery - 1 foot (HRO)", SubTitle = "Imagery - 1 foot (HRO)", Format = "JPEG2000", Priority = 1, Type = "Imagery"},
          new UsgsAPICategory(){Title = "Imagery - 1 meter (NAIP)", SubTitle = "Imagery - 1 meter (NAIP)", Format = "JPEG2000", Priority = 1, Type = "Imagery"},
          //new UsgsAPICategory(){Title = "Historical Topographic Maps", SubTitle = "Historical Topographic Maps", Format = "none", Priority = 0, Type = ""},
          //new UsgsAPICategory(){Title = "Map Indices", SubTitle = "Map Indices", Format = "none", Priority = 0, Type = ""},
          //new UsgsAPICategory(){Title = "Names - Geographic Names Information System (GNIS)", SubTitle = "Names - Geographic Names Information System (GNIS)", Format = "none", Priority = 0, Type = ""},
          //new UsgsAPICategory(){Title = "Small-scale Datasets", SubTitle = "Small-scale Datasets", Format = "none", Priority = 0, Type = ""},
          //new UsgsAPICategory(){Title = "Structures - National Structures Dataset", SubTitle = "Structures - National Structures Dataset", Format = "Shapefile", Priority = 2, Type = "Bridge,Building"},
          //new UsgsAPICategory(){Title = "Transportation - National Transportation Dataset", SubTitle = "Transportation - National Transportation Dataset", Format = "Shapefile", Priority = 1, Type = "Roadway"},
          //new UsgsAPICategory(){Title = "US Topo", SubTitle = "US Topo", Format = "none", Priority = 0, Type = ""},
          //new UsgsAPICategory(){Title = "Woodland Tint", SubTitle = "Woodland Tint", Format = "none", Priority = 0, Type = ""}
        };
        
        ECQuery m_query;
        ECQuerySettings m_querySettings;

        Dictionary<string, IECInstance> m_storedParents;

        /// <summary>
        /// UsgsAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        public UsgsAPIQueryProvider(ECQuery query, ECQuerySettings querySettings)
        {
            m_query = query;
            m_querySettings = querySettings;
        }

        /// <summary>
        /// This method returns the instances according to the ECQuery received in the constructor.
        /// The instances are created from the results of a query of the USGS database
        /// </summary>
        /// <returns>An IEnumerable containing the instances requested in the ECQuery received in the constructor</returns>
        public IEnumerable<IECInstance> CreateInstanceList()
        {
            //This method will map to other methods, depending on the search class in the query

            List<IECInstance> instanceList = null;

            string className = m_query.SearchClasses.First().Class.Name;

            int i = 0;
            while ((i < m_query.WhereClause.Count) && (instanceList == null))
            {
                //We'll take the first ECInstanceIdExpression criterion. Normally, there is only one, non embedded criterion of this sort when queried by WSG.
                if (m_query.WhereClause[i] is ECInstanceIdExpression)
                {
                    instanceList = new List<IECInstance>();

                    ECInstanceIdExpression instanceIDExpression = (ECInstanceIdExpression)m_query.WhereClause[i];

                    if (instanceIDExpression.InstanceIdSet.Count() == 0)
                    {
                        throw new ProgrammerException("The array of IDs in the ECInstanceIdExpression is empty.");
                    }

                    //We create the requested instances
                    foreach(string sourceID in instanceIDExpression.InstanceIdSet)
                    {
                        IECInstance instance = CreateInstanceFromID(className, sourceID);

                        if (instance != null)
                        {
                            instanceList.Add(instance);
                        }

                    }

                    //return instanceList;
                }
                i++;
            }

            if (instanceList == null)
            {
                // If we're here, that's because there was a polygon parameter in the extended data
                //TODO : Implement this verification

                switch (className)
                {
                    //case "USGSEntity": //To be removed
                    //    return QueryUSGSEntitiesByPolygon();
                    //case "USGSThumbnail":
                    //    return QueryUSGSThumbnail();
                    case "SpatialEntityWithDetailsView":
                        instanceList = QuerySpatialEntitiesWithDetailsViewByPolygon();
                        break;
                    default:
                        throw new UserFriendlyException("It is impossible to query instances of the class \"" + className + "\" ");
                }
            }

            //Related instances of each instance found before
            foreach (var instance in instanceList)
            {
                foreach (RelatedInstanceSelectCriteria crit in m_query.SelectClause.SelectedRelatedInstances)
                {
                    IECClass relatedClass = crit.RelatedClassSpecifier.RelatedClass;

                    IECRelationshipClass relationshipClass = crit.RelatedClassSpecifier.RelationshipClass;

                    //We do not use SpatialEntityDatasetToAlternateDataset for USGS data. DetailsViewToChildren is done in QuerySpatialEntitiesWithDetailsViewByPolygon
                    if ((relationshipClass.Name == "SpatialEntityDatasetToAlternateDataset") || (relationshipClass.Name == "DetailsViewToChildren"))
                    {
                        continue;
                    }

                    string relInstID;
                    if (/*(relationshipClass.Name != "DetailsViewToChildren") && */(relationshipClass.Name != "SpatialEntityDatasetToSpatialEntityBase"))
                    {
                        relInstID = instance.InstanceId;
                    }
                    else
                    {

                        if(crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Forward)
                        {
                            //TODO : See if it is possible and useful to go in the direction parent->children
                            continue;
                        }
                        else
                        {
                            //Here, we are sure to be in a SpatialEntityBase or SpatialEntityWithDetailsView, which should contain a ParentDatasetIdStr attribute 

                            relInstID = instance["ParentDatasetIdStr"].StringValue;
                        }
                        
                    }
                    
                    IECInstance relInst = CreateInstanceFromID(relatedClass.Name, relInstID);
                    if (relInst != null)
                    {
                        IECRelationshipInstance relationshipInst;
                        if (crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Forward)
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
                }
            }

            return instanceList;

        }

        private IECInstance CreateInstanceFromID(string className, string sourceID)
        {

            switch (className)
            {
                case "SpatialEntityBase":
                case "SpatialEntity":
                    return QuerySingleSpatialEntityBase(sourceID, m_query.SearchClasses.First().Class);
                //case "SpatialEntityWithDetailsView":
                //    return QuerySingleSpatialEntityWithDetailsView();
                case "SpatialEntityDataset":
                    return QuerySingleSpatialEntityDataset(sourceID, m_query.SearchClasses.First().Class);
                case "Thumbnail":
                    return QuerySingleThumbnail(sourceID, m_query.SearchClasses.First().Class);
                case "Metadata":
                    return QuerySingleMetadata(sourceID, m_query.SearchClasses.First().Class);
                case "SpatialDataSource":
                case "OtherSource":
                    return QuerySingleSpatialDataSource(sourceID, m_query.SearchClasses.First().Class);
                case "Server":
                    return QuerySingleServer(sourceID, m_query.SearchClasses.First().Class);
                default:
                    throw new UserFriendlyException("It is impossible to query instances of the class \"" + className + "\" ");
            }
        }

        private IECInstance QuerySingleSpatialEntityWithDetailsView(string sourceID, IECClass ecClass)
        {
            if (sourceID.Length != IndexConstants.USGSIdLenght)
            {
                return null;
            }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json = getSciencebaseJson(sourceID);

            if (json == null)
            {
                return null;
            }

            instance["Id"].StringValue = sourceID;

            instance.InstanceId = json.TryToGetString("id");

            instance["Name"].StringValue = json.TryToGetString("title");


            instance["Footprint"].SetToNull();

            var spatialObject = json["spatial"] as JObject;

            if (spatialObject != null)
            {
                var bbox = spatialObject["boundingBox"] as JObject;

                if (bbox != null)
                {
                    instance["Footprint"].StringValue = String.Format(@"{{ ""points"" : [[{2},{3}],[{2},{1}],[{0},{1}],[{0},{3}],[{2},{3}]], ""coordinate_system"" : ""4326"" }}", bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY"));
                }
            }

            instance["ThumbnailURL"].SetToNull();

            JObject previewImage = json["previewImage"] as JObject;

            if (previewImage != null)
            {
                if (previewImage["thumbnail"] != null)
                {
                    instance["ThumbnailURL"].StringValue = previewImage["thumbnail"].TryToGetString("uri");
                }
                else
                {
                    //In case the thumbnail object is not there, we take the first url we find. 
                    //I don't know if it is possible for the thumbnail object to not be there, but let's not take any risk.
                    foreach (var imageObject in previewImage)
                    {
                        if (imageObject.Value["uri"] != null)
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
            //throw new Exception("testing");
            return instance;

        }

        private IECInstance QuerySingleSpatialEntityDataset(string sourceID, IECClass ecClass)
        {
            IECInstance dataset = QuerySingleSpatialEntityBase(sourceID, ecClass);

            dataset["Processable"].SetToNull();

            return dataset;
        }

        private IECInstance QuerySingleServer(string sourceID, IECClass ecClass)
        {
            if (sourceID.Length != IndexConstants.USGSIdLenght)
            {
                return null;
            }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json = getSciencebaseJson(sourceID);

            if (json == null)
            {
                return null;
            }

            instance["Id"].StringValue = sourceID;

            // CommunicationProtocol + URL

            JArray weblinks = json["webLinks"] as JArray;

            if (weblinks != null)
            {
                foreach (JObject weblink in weblinks)
                {

                    string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
                    if ((weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())))
                    {
                        string[] splitURL = weblink["uri"].Value<string>().Split(new char[] {'/'}, StringSplitOptions.RemoveEmptyEntries);

                        //The communication protocol is the first part of the uri (http or ftp, usually).
                        instance["CommunicationProtocol"].StringValue = splitURL[0];

                        instance["URL"].StringValue = splitURL[0] + "://" + splitURL[1];
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
            if (files != null)
            {
                foreach (JObject file in files)
                {
                    if ((file["contentType"].Value<string>() == "application/fgdc+xml") && file["originalMetadata"].Value<bool>() == true)
                    {
                        string xmlString = GetHttpResponse(file["url"].Value<string>());
                        //using (XmlReader xmlReader = XmlReader.Create(xmlString))
                        //{

                        //}
                        XmlDocument xmlDoc = new XmlDocument();
                        xmlDoc.LoadXml(xmlString);

                        var feesNode = xmlDoc.SelectSingleNode("metadata/distinfo/stdorder/fees");
                        if (feesNode != null)
                        {
                            instance["Fees"].StringValue = feesNode.InnerText;

                            setToNull = false;
                            break;
                        }

                    }
                }
            }
            if (setToNull)
            {
                instance["Fees"].SetToNull();
            }
            //Legal

            if (json["title"] != null)
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

        private IECInstance QuerySingleSpatialDataSource(string sourceID, IECClass ecClass)
        {
            if (sourceID.Length != IndexConstants.USGSIdLenght)
            {
                return null;
            }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json = getSciencebaseJson(sourceID);

            if (json == null)
            {
                return null;
            }

            instance["Id"].StringValue = sourceID;

            // MainURL
            JArray weblinks = json["webLinks"] as JArray;

            if (weblinks != null)
            {
                foreach (JObject weblink in weblinks)
                {

                    string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
                    if ((weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())))
                    {
                        instance["MainURL"].StringValue = weblink["uri"].Value<string>();
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

            if (type == null)
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
            if (files != null)
            {
                foreach (JObject file in files)
                {
                    if ((file["contentType"].Value<string>() == "application/fgdc+xml") && file["originalMetadata"].Value<bool>() == true)
                    {
                        instance["Metadata"].StringValue = file["url"].Value<string>();

                        break;
                    }
                }
            }

            return instance;
        }

        private IECInstance QuerySingleMetadata(string sourceID, IECClass ecClass)
        {
            if (sourceID.Length != IndexConstants.USGSIdLenght)
            {
                return null;
            }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json = getSciencebaseJson(sourceID);

            if (json == null)
            {
                return null;
            }

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
            foreach (JObject tag in json["tags"])
            {
                string name = tag.TryToGetString("name");

                if (name != null)
                {
                    keywords += name + ", ";
                }
            }

            instance["Keywords"].StringValue = keywords.TrimEnd(' ', ',');

            //Legal

            if (json["title"] != null)
            {
                instance["Legal"].StringValue = json["title"].Value<string>() + " courtesy of the U.S. Geological Survey";
            }

            //Lineage

            instance["Lineage"].SetToNull();

            //Provenance

            instance["Provenance"].SetToNull();

            return instance;

        }

        private IECInstance QuerySingleThumbnail(string sourceID, IECClass ecClass)
        {
            if (sourceID.Length != IndexConstants.USGSIdLenght)
            {
                return null;
            }

            IECInstance instance = ecClass.CreateInstance();
            instance.InstanceId = sourceID;

            JObject json = getSciencebaseJson(sourceID);

            if (json == null)
            {
                return null;
            }

            instance["Id"].StringValue = sourceID;

            //previewImage is not always there, we have to check that it is not null
            JObject previewImage = json["previewImage"] as JObject;

            if(previewImage == null)
            {
                return null;
            }

            //ThumbnailProvenance
            instance["ThumbnailProvenance"].SetToNull();

            //ThumbnailURI

            string thumbnailURI = null;

            if (previewImage["thumbnail"] != null)
            {
                thumbnailURI = previewImage["thumbnail"].TryToGetString("uri");
            }
            else
            {
                //In case the thumbnail object is not there, we take the first uri we find. 
                //I don't know if it is possible for the thumbnail object to not be there, but let's not take any risk.
                foreach (var imageObject in previewImage)
                {
                    if(imageObject.Value["uri"] != null)
                    {
                        thumbnailURI = imageObject.Value["uri"].Value<string>();
                        break;
                    }
                }
            }


            if(thumbnailURI == null)
            {
                throw new UserFriendlyException("We have encountered a problem processing the order for the thumbnail of ID " + sourceID);
            }

            //The next part enables downloading the thumbnail

            if ((m_querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None)
            {
                MemoryStream thumbnailStream = DownloadThumbnail(thumbnailURI);

                StreamBackedDescriptor streamDescriptor = new StreamBackedDescriptor(thumbnailStream, ExtractNameFromURI(thumbnailURI), thumbnailStream.Length, DateTime.Now);
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

        private string ExtractFormatFromURI(string uri)
        {
            string[] splitURI = uri.Split('.');

            return splitURI[splitURI.Length - 1];
        }

        private string ExtractNameFromURI(string uri)
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
        private IECInstance QuerySingleSpatialEntityBase(string sourceID, IECClass ecClass)
        {
            if (sourceID.Length != IndexConstants.USGSIdLenght)
            {
                return null;
            }

            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = sourceID;

            JObject json = getSciencebaseJson(sourceID);

            if (json == null)
            {
                return null;
            }

            instance["Id"].StringValue = sourceID;

            //Footprint
            var bbox = json["spatial"]["boundingBox"] as JObject;


            if (bbox != null)
            {
                instance["Footprint"].StringValue = String.Format(@"{{ ""points"" : [[{2},{3}],[{2},{1}],[{0},{1}],[{0},{3}],[{2},{3}]], ""coordinate_system"" : ""4326"" }}", bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY"));
            }

            //Name

            instance["Name"].StringValue = json.TryToGetString("title");

            //Keywords

            string keywords = "";
            foreach(JObject tag in json["tags"])
            {
                string name = tag.TryToGetString("name");
                if (name != null)
                {
                    keywords += name + ", ";
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

            if (type == null)
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

            instance["ParentDatasetIdStr"].StringValue = json.TryToGetString("parentId");

            return instance;
        }

        private string ExtractDataSourceType(JObject json)
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

            if (weblinks != null)
            {
                foreach (JObject weblink in weblinks)
                {
                    if (weblink["title"] != null)
                    {
                        switch (weblink["title"].Value<string>())
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

        private string GetHttpResponse(string url)
        {
            using (HttpClient client = new HttpClient())
            {
                using (HttpResponseMessage response = client.GetAsync(url).Result)
                {
                    if (response.IsSuccessStatusCode)
                    {
                        using (HttpContent content = response.Content)
                        {
                            return content.ReadAsStringAsync().Result;
                        }
                    }
                }
            }
            return null;
        }

        /// <summary>
        /// Returns the json of the item of specified source ID in the sciencebase catalog
        /// </summary>
        /// <param name="sourceID">The source ID of the item</param>
        /// <returns>The JObject read if the request was successful, null otherwise</returns>
        private JObject getSciencebaseJson(string sourceID)
        {
            string url = "https://www.sciencebase.gov/catalog/item/" + sourceID + "?format=json";

            string jsonString = GetHttpResponse(url);
            if (jsonString != null)
            {
                return JObject.Parse(jsonString) as JObject;
            }

            return null;
        }

        private List<IECInstance> QuerySpatialEntitiesWithDetailsViewByPolygon()
        {
            List<IECInstance> instanceList = new List<IECInstance>();

            //This part is an optimization only for queries of SpatialEntitiesWithDetailsView and their parents
            var relCrit = m_query.SelectClause.SelectedRelatedInstances.FirstOrDefault(crit => (crit.RelatedClassSpecifier.RelationshipClass.Name == "DetailsViewToChildren") 
                                                                                       && (crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Backward));
            if(relCrit != null)
            {
                m_storedParents = new Dictionary<string,IECInstance>();
            }

            foreach (var USGSResult in GetNonFormattedUSGSResults())
            {
                IECClass ecClass = m_query.SearchClasses.First().Class;
                IECInstance instance = ecClass.CreateInstance();

                JToken item = USGSResult.jtoken;

                InitializePropertiesToNull(instance, ecClass);

                Random rnd = new Random();

                ////TODO : Erase this when the id problem is fixed **************
                //int id;
                //do
                //{
                //    id = rnd.Next(40000, 50000);
                //}while (instanceList.Any( inst => inst["Id"].IntValue == id));
                //instance["Id"].IntValue = id;
                ////*************************************************************

                instance.InstanceId = item.TryToGetString("sourceId");
                instance["Id"].StringValue = item.TryToGetString("sourceId");

                instance["Name"].StringValue = item.TryToGetString("title");

                var bbox = item["boundingBox"];

                if (bbox != null)
                {
                    instance["Footprint"].StringValue = "{ \"points\" : " + String.Format("[[{0},{1}],[{0},{3}],[{2},{3}],[{2},{1}],[{0},{1}]]", bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY")) + ", \"coordinate_system\" : \"4326\" }";
                }
                instance["ThumbnailURL"].StringValue = item.TryToGetString("previewGraphicURL");
                instance["MetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/" + instance.InstanceId;
                instance["RawMetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/download/" + instance.InstanceId + "?format=fgdc";
                instance["RawMetadataFormat"].StringValue = "FGDC";
                instance["SubAPI"].StringValue = "USGS";

                instance["DataProvider"].StringValue = "USGS";
                instance["DataProviderName"].StringValue = "United States Geological Survey";



                //instance["ParentDatasetIdStr"].StringValue = tuple.Item2;

                if (m_storedParents != null)
                {
                    string relInstId = USGSResult.DatasetId;
                    if (!m_storedParents.ContainsKey(relInstId))
                    {
                        m_storedParents.Add(relInstId, QuerySingleSpatialEntityWithDetailsView(relInstId, ecClass));
                    }
                    if (m_storedParents[relInstId] != null)
                    {
                        var relationshipInst = relCrit.RelatedClassSpecifier.RelationshipClass.CreateRelationship(m_storedParents[relInstId], instance);

                        //relationshipInst.InstanceId = "test";
                        instance.GetRelationshipInstances().Add(relationshipInst);
                    }

                }



                instanceList.Add(instance);
            }

            return instanceList;
        }

        /// <summary>
        /// Queries USGS for data located in the selected bbox and in the predetermined datasets (see CategoryTable)
        /// </summary>
        /// <returns>Non formatted data (still in JSON form), along with the ID of the parent dataset. This ID was added there to prevent
        /// having to query USGS for every single data</returns>
        private IEnumerable<USGSRequestBundle> GetNonFormattedUSGSResults()
        {

            List<USGSRequestBundle> instanceList = new List<USGSRequestBundle>();

            string bbox = ExtractBboxFromQuery();

            List<string> formatList = ExtractFormatList();
            bool selectAllFormats = (formatList == null || formatList.Count == 0);

            List<UsgsRequest> reqList = new List<UsgsRequest>();

            using (HttpClient client = new HttpClient())
            {
                
                client.Timeout = new TimeSpan(100000000);
                using (HttpResponseMessage response = client.GetAsync(WebReqCategories).Result)
                {
                    if (!response.IsSuccessStatusCode)
                    {
                        throw new Exception("USGS request timed out.");
                    }
                    using (HttpContent content = response.Content)
                    {
                        string responseString = content.ReadAsStringAsync().Result;

                        JArray json = JArray.Parse(responseString);

                        foreach (var entry in json)
                        {
                            if (entry["tags"].HasValues)
                            {

                                foreach (JProperty subEntry in entry["tags"])
                                {
                                    System.Console.Out.Write(subEntry.Value["title"]);
                                    foreach (var category in CategoryTable)
                                    {
                                        if (entry["title"].Value<string>() == category.Title &&
                                           subEntry.Value["title"].Value<string>() == category.SubTitle &&
                                           category.Priority != 0 &&
                                           (selectAllFormats || formatList.Any(f => f.ToLower() == category.Format.ToLower())))
                                        {
                                            UsgsRequest req = new UsgsRequest()
                                            {
                                                Dataset = subEntry.Value["sbDatasetTag"].Value<string>(),
                                                DatasetID = subEntry.Value["id"].Value<string>(),
                                                Format = category.Format,
                                                Category = category.Title,
                                                Priority = category.Priority
                                            };

                                            reqList.Add(req);
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                foreach (var category in CategoryTable)
                                {
                                    if (entry["title"].Value<string>() == category.Title &&
                                       entry["title"].Value<string>() == category.SubTitle &&
                                       category.Priority != 0)
                                    {
                                        UsgsRequest req = new UsgsRequest()
                                        {
                                            Dataset = entry["sbDatasetTag"].Value<string>(),
                                            DatasetID = entry["id"].Value<string>(),
                                            Format = category.Format,
                                            Category = category.Title,
                                            Priority = category.Priority
                                        };
                                        reqList.Add(req);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            reqList.Sort();

            //string curCat = "";

            object locker = new object();



            //TODO : We have deactivated the category feature for the parallel foreach we could reimplement it by storing all results in a temporary list of list, 
            //       then process it to have the same list we would have had before
            Parallel.ForEach (reqList, req =>
            {
                //if (curCat == req.Category)
                //{
                //    continue;
                //}

                string readyToSend = WebReq.Replace("_bbox", bbox).Replace("_datasets", req.Dataset).Replace("_prodFormats", req.Format).Replace(' ', '+');

                using (HttpClient client = new HttpClient())
                {

                    using (HttpResponseMessage response = client.GetAsync(readyToSend).Result)
                    {
                        if (response.IsSuccessStatusCode)
                        {
                            using (HttpContent content = response.Content)
                            {
                                string responseString = content.ReadAsStringAsync().Result;

                                JObject jsonResp = JObject.Parse(responseString);

                                //IEnumerable<JToken> tempContainer = SpecialFiltering(jsonResp["items"], req);

                                lock (locker)
                                {
                                    instanceList.AddRange(jsonResp["items"].Select(item => new USGSRequestBundle { jtoken = item, DatasetId = req.DatasetID }));
                                    //foreach (var item in jsonResp["items"] as JArray)
                                    //{
                                    //    instanceList.Add(item);

                                    //}
                                }

                            }
                        }
                    }
                }
            });

            return instanceList;
        }

        private IEnumerable<JToken> SpecialFiltering(IEnumerable<JToken> tokenList, UsgsRequest req)
        {
            switch(req.Dataset)
            {
                case "High Resolution Orthoimagery" :
                    //Would it be a good idea to create a hierarchy of classes of filters?
                    //For now, we only go to private functions...
                    return HROFilter(tokenList);
                default :
                    return tokenList;
            }
            
        }

        /// <summary>
        /// High resolution orthoimagery dataset special filtering. We need to eliminate superfluous
        /// entries in this dataset. There can be copies of the same entry and data for multiple dates
        /// for the same location. For now, we filter all of this by taking only the most recent. 
        /// The date is retrieved from the name (title) of the data.
        /// </summary>
        /// <param name="tokenList">The complete list of json entries of the HRO dataset obtained from USGS api</param>
        /// <returns>The list of entries, exempt from superfluous data</returns>
        private IEnumerable<JToken> HROFilter(IEnumerable<JToken> tokenList)
        {
            //Filter by bounding box
            Dictionary<string, JToken> bboxDictionary = new Dictionary<string, JToken>();

            foreach(JToken newToken in tokenList)
            {
                var bboxString = newToken["boundingBox"].ToString();
                if (bboxDictionary.ContainsKey(bboxString))
                {
                    var firstToken = bboxDictionary[bboxString];

                    string firstTokenName = firstToken.TryToGetString("title");
                    string[] firstTitleSplit = firstTokenName.Split(' ');

                    //In all the data we've seen, the date is always the fourth last string in the title.
                    string firstDateString = firstTitleSplit[firstTitleSplit.Length - 4];

                    //The date is always in the format YYYYMM, making it possible to convert it to an int and then comparing it
                    //directly

                    int firstDate = 0;
                    bool problemParsing1 = int.TryParse(firstDateString, out firstDate);

                    string newTokenName = newToken.TryToGetString("title");
                    string[] newTitleSplit = newTokenName.Split(' ');

                    //In all the data we've seen, the date is always the fourth last string in the title.
                    string newDateString = newTitleSplit[newTitleSplit.Length - 4];

                    //The date is always in the format YYYYMM, making it possible to convert it to an int and then comparing it
                    //directly
                    int newDate = 0;
                    bool problemParsing2 = int.TryParse(newDateString, out newDate);

                    if(problemParsing1 || problemParsing2)
                    {
                        //It seems we haven't got the right date string...
                        throw new Bentley.EC.Persistence.Operations.OperationFailedException("Error while filtering results.");
                    }
                    if(newDate > firstDate)
                    {
                        bboxDictionary[bboxString] = newToken;
                    }
                }
                else
                {
                    bboxDictionary[bboxString] = newToken;
                }
            }
            return bboxDictionary.Values;
        }

        private void InitializePropertiesToNull(IECInstance instance, IECClass ecClass)
        {
            foreach (IECProperty prop in ecClass)
            {
                instance[prop.Name].SetToNull();
            }
        }

        /// <summary>
        /// Extracts all formats from the extended data key "format". 
        /// </summary>
        /// <returns>Null if there is no format key, otherwise list of all formats requested</returns>
        private List<string> ExtractFormatList()
        {
            if(!m_query.ExtendedData.ContainsKey("format"))
            {
                return null;
            }
            string formatString = m_query.ExtendedData["format"].ToString();

            string[] formatArray = formatString.Split(new char[] {','}, StringSplitOptions.RemoveEmptyEntries);
            List<string> formatList = new List<string>();
            
            foreach(var format in formatArray)
            {
                formatList.Add(format.Trim());
            }

            return formatList;
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

        private string ExtractBboxFromQuery()
        {
            if (!m_query.ExtendedData.ContainsKey("polygon"))
            {
                throw new UserFriendlyException("This request must contain a \"polygon\" parameter in the form of a WKT polygon string.");
            }

            string polygonString = m_query.ExtendedData["polygon"].ToString();
            PolygonModel model;
            try
            {
                model = JsonConvert.DeserializeObject<PolygonModel>(polygonString);
            }
            catch (JsonSerializationException)
            {
                throw new UserFriendlyException("The polygon format is not valid.");
            }

            int polygonSRID;
            if (!int.TryParse(model.coordinate_system, out polygonSRID))
            {
                throw new UserFriendlyException("The polygon format is not valid.");
            }

            string polygonWKT = DbGeometryHelpers.CreateWktPolygonString(model.points);

            PolygonDescriptor polyDesc = new PolygonDescriptor
            {
                WKT = polygonWKT,
                SRID = polygonSRID
            };

            //We should now extract a bbox from this wkt

            return DbGeometryHelpers.ExtractBboxFromWKTPolygon(polyDesc.WKT);

        }

        //This is an helper method we could put in another file. It belonged in the now unused USGSThumbnailRetrievalController
        private MemoryStream DownloadThumbnail(string thumbnailUri)
        {
            if (thumbnailUri.StartsWith("ftp"))
            {
                //FtpWebRequest request = FtpWebRequest.Create(thumbnailUri) as FtpWebRequest;
                //request.Method = WebRequestMethods.Ftp.GetFileSize;

                //FtpWebResponse response = (FtpWebResponse)request.GetResponse();
                //contentLength = response.ContentLength;

                //request = FtpWebRequest.Create(thumbnailUri) as FtpWebRequest;
                //request.Method = WebRequestMethods.Ftp.DownloadFile;

                //response = (FtpWebResponse)request.GetResponse();

                //return response.GetResponseStream();
                using (WebClient ftpClient = new WebClient())
                {

                    //string tempPath = Path.GetTempPath();
                    //string tempFilePath = Path.Combine(tempPath, Guid.NewGuid().ToString());
                    //using (Stream fstream = File.Create(tempFilePath))
                    //{
                    //    ftpClient.OpenRead(thumbnailUri).CopyTo(fstream);
                    //}
                    //return File.Open(tempFilePath, FileMode.Open);

                    using (Stream image = ftpClient.OpenRead(thumbnailUri))
                    {
                        MemoryStream imageInMemory = new MemoryStream();

                        image.CopyTo(imageInMemory);

                        return imageInMemory;
                    }
                }

            }
            if (thumbnailUri.StartsWith("http"))
            {
                using (HttpClient client = new HttpClient())
                {
                    using (HttpResponseMessage response = client.GetAsync(thumbnailUri).Result)
                    {
                        using (HttpContent content = response.Content)
                        {

                            //contentLength = (content.Headers.ContentLength.HasValue ? content.Headers.ContentLength.Value : 0);

                            using (Stream image = content.ReadAsStreamAsync().Result)
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

    }
}
