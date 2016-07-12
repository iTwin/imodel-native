/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/QueryProviders/UsgsAPIQueryProvider.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

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
using System.Data;
using Bentley.ECSystem.Configuration;
using Bentley.EC.PluginBuilder.Modules;


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
        IInstanceCacheManager m_instanceCacheManager;
        IECSchema m_schema;

        Tuple<string, JObject> jsonCache;

        Dictionary<IECClass, List<IECInstance>> m_storageForCaching;
        Dictionary<string, IECInstance> m_storedParents;

        /// <summary>
        /// UsgsAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="usgsDataFetcher">The usgsDataFetcher that will be used to query USGS</param>
        /// <param name="cacheManager">The cache manager that will be used to access the cache in the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        public UsgsAPIQueryProvider (ECQuery query, ECQuerySettings querySettings, IUSGSDataFetcher usgsDataFetcher, IInstanceCacheManager cacheManager, IECSchema schema)
            {
            m_query = query;
            m_querySettings = querySettings;
            m_usgsDataFetcher = usgsDataFetcher;
            m_schema = schema;

            m_storageForCaching = new Dictionary<IECClass, List<IECInstance>>();

            int daysCacheIsValid;

            if( !Int32.TryParse(ConfigurationRoot.GetAppSetting("RECPDaysCacheIsValid"), out daysCacheIsValid) )
                {
                daysCacheIsValid = 10;
                }
            m_instanceCacheManager = cacheManager;
            }

        /// <summary>
        /// UsgsAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="dbConnection">The dbConnection that will be used to access the cache in the database</param>
        /// <param name="schemaModule">The schema of the ECPlugin</param>
        public UsgsAPIQueryProvider (ECQuery query, ECQuerySettings querySettings, IDbConnection dbConnection, IECSchema schemaModule)
            {
            m_query = query;
            m_querySettings = querySettings;
            m_usgsDataFetcher = new USGSDataFetcher(query);
            m_storageForCaching = new Dictionary<IECClass, List<IECInstance>>();
            m_schema = schemaModule;

            uint daysCacheIsValid;

            if( !UInt32.TryParse(ConfigurationRoot.GetAppSetting("RECPDaysCacheIsValid"), out daysCacheIsValid) )
                {
                daysCacheIsValid = 10;
                }
            m_instanceCacheManager = new InstanceCacheManager(DataSource.USGS, daysCacheIsValid, dbConnection, querySettings, new DbQuerier());

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
            //InstanceCacheManager instanceCacheManager = new InstanceCacheManager();

            IECClass ecClass = m_query.SearchClasses.First().Class;

            int i = 0;
            while ( (i < m_query.WhereClause.Count) && (instanceList == null) )
                {
                //We'll take the first ECInstanceIdExpression criterion. Normally, there is only one, non embedded criterion of this sort when queried by WSG.
                if ( m_query.WhereClause[i] is ECInstanceIdExpression )
                    {
                    //instanceList = new List<IECInstance>();

                    ECInstanceIdExpression instanceIDExpression = (ECInstanceIdExpression) m_query.WhereClause[i];

                    if ( instanceIDExpression.InstanceIdSet.Count() == 0 )
                        {
                        throw new UserFriendlyException("Please specify at least one ID in this type of where clause");
                        }

                    //We search in the cache for the instances of asked Ids

                    instanceList = CreateInstancesFromInstanceIdList(instanceIDExpression.InstanceIdSet, ecClass, m_query.SelectClause);

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

            LaunchCaching();

            return instanceList;

            }

        private List<IECInstance> CreateInstancesFromInstanceIdList(IEnumerable<string> instanceIdSet, IECClass ecClass, SelectCriteria selectClause)
            {
            List<IECInstance> instancesRequestingParent = null;
            List<IECInstance> instanceList = new List<IECInstance>();
            List<IECInstance> cachedInstances = m_instanceCacheManager.QueryInstancesFromCache(instanceIdSet, ecClass, GetCacheBaseClass(ecClass), selectClause);
            CompleteInstances(cachedInstances, ecClass);
            CreateCacheRelatedInstances(cachedInstances, selectClause.SelectedRelatedInstances);
            RelatedInstanceSelectCriteria parentCrit = null;
                    //We create the requested instances that were not in the cache
                    foreach ( string sourceID in instanceIdSet )
                        {
                        IECInstance instance = cachedInstances.FirstOrDefault(inst => inst.InstanceId == sourceID);
                        if ( instance == null || (bool) instance.ExtendedData["Complete"] == false )
                            {
                            try
                                {
                                instance = CreateInstanceFromID(ecClass, sourceID);
                                }
                            catch ( EnvironmentalException )
                                {
                                if ( instance == null )
                                    {
                                    //We had nothing in the cache and USGS returned an error. We cannot continue
                                    throw;
                                    }
                                }
                            }
                        InstanceCritTuple parentRequestingBundle = CreateRelatedInstance(instance, selectClause.SelectedRelatedInstances);
                        if(parentRequestingBundle != null)
                            {
                            if(instancesRequestingParent == null)
                                {
                                instancesRequestingParent = new List<IECInstance>();
                                }
                            instancesRequestingParent.Add(parentRequestingBundle.Instance);

                            if(parentCrit == null)
                                {
                                parentCrit = parentRequestingBundle.ParentRelatedCriteria;
                                }
                            else
                                {
                                if(parentCrit != parentRequestingBundle.ParentRelatedCriteria)
                                    {
                                    throw new UserFriendlyException();
                                    }
                                }
                            }

                        if ( instance != null )
                            {
                            instanceList.Add(instance);
                            }

                        }
                    //var parentCrit = selectClause.SelectedRelatedInstances.FirstOrDefault(crit => (crit.RelatedClassSpecifier.RelationshipClass.Name == "DetailsViewToChildren" ||
                    //                                                                                       crit.RelatedClassSpecifier.RelationshipClass.Name == "SpatialEntityDatasetToSpatialEntityBase")
                    //                 && (crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Backward));
                    if(parentCrit != null)
                        {
                        CreateParentRelationship(instancesRequestingParent, parentCrit, parentCrit.RelatedClassSpecifier.RelatedClass);
                        }
                    return instanceList;
            }

        /// <summary>
        /// This method completes the cached instances fields that are not in the cache tables in the database
        /// </summary>
        /// <param name="cachedInstances">The cached instances</param>
        /// <param name="ecClass">The class of the instances</param>
        private void CompleteInstances (List<IECInstance> cachedInstances, IECClass ecClass)
            {
            switch ( ecClass.Name )
                {
                case "SpatialEntityWithDetailsView":
                        {
                        foreach ( IECInstance inst in cachedInstances )
                            {
                            inst["MetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/" + inst.InstanceId;
                            inst["RawMetadataURL"].StringValue = "https://www.sciencebase.gov/catalog/item/download/" + inst.InstanceId + "?format=fgdc";
                            inst["RawMetadataFormat"].StringValue = "FGDC";
                            inst["SubAPI"].StringValue = "USGS";
                            }
                        break;
                        }
                default:
                    //Do nothing
                    break;
                }
            }

        //We should replace isComplete by an extended data parameter in each of the instances.
        private void LaunchCaching ()
            {
            foreach(KeyValuePair<IECClass, List<IECInstance>> instancesGroup in m_storageForCaching)
                {
                List<Tuple<string, IECType, Func<IECInstance, string>>> additionalColumns = null;
                if(instancesGroup.Key.Name == "SpatialEntityBase")
                    {
                    additionalColumns = new List<Tuple<string, IECType, Func<IECInstance, string>>>();
                    additionalColumns.Add(new Tuple<string, IECType, Func<IECInstance, string>>("ParentDatasetIdStr", Bentley.ECObjects.ECObjects.StringType, inst => ((string)inst.ExtendedData["ParentDatasetIdStr"])));
                    }
                m_instanceCacheManager.InsertInstancesInCache(instancesGroup.Value, instancesGroup.Key, additionalColumns);
                }
            }

        private void CreateCacheRelatedInstances (List<IECInstance> cachedInstances, List<RelatedInstanceSelectCriteria> relatedCriteriaList)
            {
            foreach ( RelatedInstanceSelectCriteria crit in relatedCriteriaList )
                {
                IECClass relatedClass = crit.RelatedClassSpecifier.RelatedClass;

                IECRelationshipClass relationshipClass = crit.RelatedClassSpecifier.RelationshipClass;
                bool useParentId = false;

                //We do not use SpatialEntityDatasetToAlternateDataset for USGS data. DetailsViewToChildren is done in QuerySpatialEntitiesWithDetailsViewByPolygon
                if ( (relationshipClass.Name == "SpatialEntityDatasetToAlternateDataset") || 
                    (relationshipClass.Name == "DetailsViewToChildren") ||
                    (relationshipClass.Name == "SpatialEntityDatasetToSpatialEntityBase") )
                    {
                    continue;
                    }

                List<string> relInstIDs = new List<string>();
                //if (/*(relationshipClass.Name != "DetailsViewToChildren") && */(relationshipClass.Name != "SpatialEntityDatasetToSpatialEntityBase") )
                //    {
                    foreach ( IECInstance instance in cachedInstances )
                        {
                        if ( !relInstIDs.Contains(instance.InstanceId) )
                            {
                            relInstIDs.Add(instance.InstanceId);
                            }
                        }
                    //}
                //else
                //    {

                //    if ( crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Forward )
                //        {
                //        //TODO : See if it is possible and useful to go in the direction parent->children
                //        continue;
                //        }
                //    else
                //        {
                //        useParentId = true;
                //        foreach ( IECInstance instance in cachedInstances )
                //            {
                //            string parentId = (string) instance.ExtendedData["ParentDatasetIdStr"];
                //            if ( !relInstIDs.Contains(parentId) )
                //                {
                //                relInstIDs.Add(parentId);
                //                }
                //            }
                //        }
                //    }

                List<IECInstance> relInstances = m_instanceCacheManager.QueryInstancesFromCache(relInstIDs, relatedClass, GetCacheBaseClass(relatedClass), crit);
                CompleteInstances(relInstances, relatedClass);
                CreateCacheRelatedInstances(relInstances, crit.SelectedRelatedInstances);
                foreach(IECInstance relInstance in relInstances)
                    {
                    IEnumerable<IECInstance> instances;
                    if ( useParentId )
                        {
                        instances = cachedInstances.Where(i => (string) i.ExtendedData["ParentDatasetIdStr"] == relInstance.InstanceId);
                        }
                    else
                        {
                        instances = cachedInstances.Where(i => i.InstanceId == relInstance.InstanceId);
                        }
                    foreach ( IECInstance instance in instances )
                        {
                        IECRelationshipInstance relationshipInst;
                        if ( crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Forward )
                            {
                            relationshipInst = relationshipClass.CreateRelationship(instance, relInstance);
                            }
                        else
                            {
                            relationshipInst = relationshipClass.CreateRelationship(relInstance, instance);
                            }
                        //relationshipInst.InstanceId = "test";
                        //instance.GetRelationshipInstances().Add(relationshipInst);
                        }
                    
                    }
                
                }

            }

        private IECClass GetCacheBaseClass (IECClass ecClass)
            {
            switch ( ecClass.Name )
                {
                case "SpatialEntityBase":
                case "SpatialEntityWithDetailsView":
                case "Thumbnail":
                case "Metadata":
                case "SpatialDataSource":
                case "Server":
                    return ecClass;
                case "SpatialEntity":
                case "SpatialEntityDataset":
                    return ecClass.BaseClasses.First(c => c.Name == "SpatialEntityBase");
                case "OtherSource":
                    return ecClass.BaseClasses.First(c => c.Name == "SpatialDataSource");
                
                default:
                    throw new ProgrammerException("It is impossible to cache instances of the class \"" + ecClass.Name + "\"");
                }
            }

        /// <summary>
        /// Creates the instances related to an instance, according to the RelatedInstanceSelectCriteria list. This method is recursive,
        /// meaning that this method is called on the related instances created before returning. The request for parent instances is
        /// not accomplished by this method for optimisation reasons. To help deferring this task, this method returns the instance for which
        /// a parent has been queried. A optimal request should not request the same instance's parent twice in a recursion. If this happens, this method will return an error.
        /// </summary>
        /// <param name="instance">The instance for which we want the related instances ()</param>
        /// <param name="relatedCriteriaList"></param>
        /// <returns>If a parent instance was requested in the SelectCriteria (directly or in the recursive criteria), 
        /// returns the instance for which we want to create the parent. Otherwise, null</returns>
        private InstanceCritTuple CreateRelatedInstance (IECInstance instance, List<RelatedInstanceSelectCriteria> relatedCriteriaList)
            {
            InstanceCritTuple childWithParentRequest = null;
            foreach ( RelatedInstanceSelectCriteria crit in relatedCriteriaList )
                {
                IECClass relatedClass = crit.RelatedClassSpecifier.RelatedClass;

                IECRelationshipClass relationshipClass = crit.RelatedClassSpecifier.RelationshipClass;

                Tuple<string, JObject> oldJsonCache = jsonCache;

                //We do not use SpatialEntityDatasetToAlternateDataset for USGS data.
                if ( (relationshipClass.Name == "SpatialEntityDatasetToAlternateDataset") /*|| 
                    (relationshipClass.Name == "DetailsViewToChildren") || 
                    (relationshipClass.Name == "SpatialEntityDatasetToSpatialEntityBase" )*/)
                    {
                    continue;
                    }
                if((relationshipClass.Name == "DetailsViewToChildren" || relationshipClass.Name == "SpatialEntityDatasetToSpatialEntityBase") && 
                   (crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Backward))
                    {
                    if ( childWithParentRequest == null )
                        {
                        childWithParentRequest = new InstanceCritTuple(instance, crit);
                        }
                    else
                        {
                        throw new UserFriendlyException("This request contains redundant related select criterias. Please simplify your request.");
                        }
                    continue;
                    }

                string relInstID = instance.InstanceId;
                //if (/*(relationshipClass.Name != "DetailsViewToChildren") && */(relationshipClass.Name != "SpatialEntityDatasetToSpatialEntityBase") )
                //    {
                //    relInstID = instance.InstanceId;
                //    }
                //else
                //    {

                //    if ( crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Forward )
                //        {
                //        //TODO : See if it is possible and useful to go in the direction parent->children
                //        continue;
                //        }
                //    else
                //        {
                //        //Here, we are sure to be in a SpatialEntityBase, which should contain a ParentDatasetIdStr attribute 

                //        //relInstID = instance["ParentDatasetIdStr"].StringValue;
                //        relInstID = instance.ExtendedData["ParentDatasetIdStr"] as string;
                //        //instance.ExtendedData.Remove("ParentDatasetIdStr");
                //        }

                //    }

                IECInstance relInst = null;
                bool mustAddRelation = false;

                relInst = instance.GetRelationshipInstances().GetRelatedInstances(relationshipClass, crit.RelatedClassSpecifier.RelatedDirection, relatedClass).FirstOrDefault();
                if(relInst == null)
                    {
                    mustAddRelation = true;
                    }
                if ( relInst == null || (bool) relInst.ExtendedData["Complete"] == false )
                    {
                    try
                        {
                        relInst = CreateInstanceFromID(relatedClass, relInstID);
                        }
                    catch(EnvironmentalException)
                        {
                        if(relInst == null)
                            {
                            //We had nothing in the cache and USGS returned an error. We cannot continue
                            throw;
                            }
                        }
                    //We reset the json cache to the old one, since it is more likely to be useful than the old one (which may contain the json of the parent, which we don't need)
                    jsonCache = oldJsonCache;
                    }
                if ( relInst != null && mustAddRelation)
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
                    //instance.GetRelationshipInstances().Add(relationshipInst);
                    }
                InstanceCritTuple tempTuple = CreateRelatedInstance(relInst, crit.SelectedRelatedInstances);
                if(tempTuple != null)
                    {
                    if ( childWithParentRequest != null )
                        {
                        throw new UserFriendlyException("This request contains redundant related select criterias. Please simplify your request.");
                        }
                    else
                        {
                        childWithParentRequest = tempTuple;
                        }
                    }
                }
            return childWithParentRequest;
            }

        private IECInstance CreateInstanceFromID (IECClass ecClass, string sourceID/*, bool allowSEWDV = false*/)
            {

            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }
            IECInstance instance;
            switch ( ecClass.Name )
                {
                case "SpatialEntityBase":
                case "SpatialEntity":
                    instance = QuerySingleSpatialEntityBase(sourceID, ecClass);
                    break;
                case "SpatialEntityWithDetailsView":
                    //if ( allowSEWDV )
                    //    {
                        return QuerySingleSpatialEntityWithDetailsView(sourceID, ecClass);
                    //    }
                    //else
                    //    {
                    //    throw new UserFriendlyException("It is impossible to query instances of the class \"" + ecClass.Name + "\" in a USGS request by ID.");
                    //    }
                case "SpatialEntityDataset":
                    instance = QuerySingleSpatialEntityDataset(sourceID, ecClass);
                    break;
                case "Thumbnail":
                    instance = QuerySingleThumbnail(sourceID, ecClass);
                    break;
                case "Metadata":
                    instance = QuerySingleMetadata(sourceID, ecClass);
                    break;
                case "SpatialDataSource":
                case "OtherSource":
                    instance = QuerySingleSpatialDataSource(sourceID, ecClass);
                    break;
                case "Server":
                    instance = QuerySingleServer(sourceID, ecClass);
                    break;
                default:
                    throw new UserFriendlyException("It is impossible to query instances of the class \"" + ecClass.Name + "\" in a USGS request by ID.");
                }
            PrepareInstanceForCaching(instance);

            return instance;
            }

        private void PrepareInstanceForCaching (IECInstance instance)
            {
            IECClass baseClass = GetCacheBaseClass(instance.ClassDefinition);
            if(!m_storageForCaching.Keys.Contains(baseClass))
                {
                m_storageForCaching.Add(baseClass, new List<IECInstance>());
                }
            m_storageForCaching[baseClass].Add(instance);
            }

        private IECInstance QuerySingleSpatialEntityWithDetailsView (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();
            IECInstance instanceSEB = QuerySingleSpatialEntityBase(sourceID, m_schema.GetClass("SpatialEntityBase"));
            PrepareInstanceForCaching(instanceSEB);

            IECInstance instanceMetadata = QuerySingleMetadata(sourceID, m_schema.GetClass("Metadata"));
            PrepareInstanceForCaching(instanceMetadata);
            
            IECInstance instanceSDS = QuerySingleSpatialDataSource(sourceID, m_schema.GetClass("SpatialDataSource"));
            PrepareInstanceForCaching(instanceSDS);


            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = sourceID;

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
            IECInstance instance = QuerySingleSpatialEntityBase(sourceID, ecClass);

            instance["Processable"].SetToNull();

            return instance;
            }

        private IECInstance QuerySingleServer (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();

            instance.InstanceId = sourceID;

            InitializePropertiesToNull(instance, ecClass);

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

            instance.ExtendedDataValueSetter.Add("Complete", true);

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

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);

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
            instance.ExtendedDataValueSetter.Add("Complete", true);

            return instance;
            }

        private IECInstance QuerySingleMetadata (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = sourceID;

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
                instance["Legal"].StringValue = json["title"].Value<string>() + " courtesy of the U.S. Geological Survey";
                }

            //Lineage

            instance["Lineage"].SetToNull();

            //Provenance

            instance["Provenance"].SetToNull();

            instance.ExtendedDataValueSetter.Add("Complete", true);

            return instance;
            }

        private IECInstance QuerySingleThumbnail (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = sourceID;

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
                instance.ExtendedDataValueSetter.Add("Complete", true);
                }
            else
                {
                instance.ExtendedDataValueSetter.Add("Complete", false);
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

        private IECInstance QuerySingleSpatialEntityBase (string sourceID, IECClass ecClass)
            {
            if ( sourceID.Length != IndexConstants.USGSIdLenght )
                {
                return null;
                }

            JObject json = GetJsonWithCache(sourceID);

            IECInstance instance = ecClass.CreateInstance();

            InitializePropertiesToNull(instance, ecClass);

            instance.InstanceId = sourceID;

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
                                instance["Classification"].StringValue = cat.Classification;
                                }
                            }
                        }
                    }
                keywords = keywords.TrimEnd(' ', ',');
                }

            instance["Keywords"].StringValue = keywords;

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

            instance["AccuracyResolutionDensity"].StringValue = "Unknown";
            instance["ResolutionInMeters"].StringValue = "Unknown";

            instance["DataProvider"].StringValue = "USGS";
            instance["DataProviderName"].StringValue = "United States Geological Survey";

            //instance["ParentDatasetIdStr"].StringValue = json.TryToGetString("parentId");

            //This happens when we request the parent of the SpatialEntityBase
            //if ( m_query.SelectClause.SelectedRelatedInstances.Any(relCrit => relCrit.RelatedClassSpecifier.RelationshipClass.Name == "SpatialEntityDatasetToSpatialEntityBase" &&
            //                                                                  relCrit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Backward) )
            //    {
            instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", json.TryToGetString("parentId"));
            //}

            instance.ExtendedDataValueSetter.Add("Complete", true);

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
            List<IECInstance> instanceList;

            //This part is an optimization only for queries of SpatialEntitiesWithDetailsView and their parents
            var relCrit = m_query.SelectClause.SelectedRelatedInstances.FirstOrDefault(crit => (crit.RelatedClassSpecifier.RelationshipClass.Name == "DetailsViewToChildren")
                                                                                       && (crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Backward));
            if ( relCrit != null )
                {
                m_storedParents = new Dictionary<string, IECInstance>();
                }

            IECClass ecClass = m_query.SearchClasses.First().Class;

            var criteriaList = ExtractPropertyWhereClauses();

            //We fetch beforehand the instances in the cache.
            string polygonString = m_query.ExtendedData["polygon"].ToString();
            PolygonModel model = DbGeometryHelpers.CreatePolygonModelFromJson(polygonString);
            string polygonWKT = DbGeometryHelpers.CreateWktPolygonString(model.points);

            PolygonDescriptor polyDesc = new PolygonDescriptor
            {
                WKT = polygonWKT,
                SRID = model.coordinate_system
            };

            List<IECInstance> cachedinstanceList = m_instanceCacheManager.QuerySpatialInstancesFromCache(polyDesc, ecClass, ecClass, m_query.SelectClause, criteriaList);
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

                        CreateIncompleteCacheInstances(item, bundle.Classification, bundle.DatasetId);

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
                        instance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", bundle.DatasetId);

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
                foreach (IECInstance cachedInstance in cachedinstanceList)
                    {
                    if(! instanceList.Any(inst => inst.InstanceId == cachedInstance.InstanceId))
                        {
                        instanceList.Add(cachedInstance);
                        }
                    }

                }
            catch ( EnvironmentalException )
                {
                //USGS returned an error. We will return the results contained in the cache.
                instanceList = cachedinstanceList;
                }
            catch ( System.AggregateException ex )
                {
                if (ex.InnerExceptions.All(e => e.GetType() == typeof(EnvironmentalException))) 
                    {
                    //USGS returned an error. We will return the results contained in the cache.
                    instanceList = cachedinstanceList;
                    }
                else
                    {
                    throw;
                    }
                }

            if ( relCrit != null )
                CreateParentRelationship(instanceList, relCrit, ecClass);

            return instanceList;
            }

        private void CreateParentRelationship (List<IECInstance> instanceList, RelatedInstanceSelectCriteria relCrit, IECClass ecClass)
            {

            List<string> parentIdList = new List<string>();
            foreach ( IECInstance instance in instanceList )
                {
                string parentId = (string) instance.ExtendedData["ParentDatasetIdStr"];

                if ( !parentIdList.Any(id => id == parentId) && parentId != null )
                    {
                    parentIdList.Add(parentId);
                    }
                }

            List<IECInstance> parentList = CreateInstancesFromInstanceIdList(parentIdList, ecClass, relCrit);
            foreach ( IECInstance instance in instanceList )
                {
                string parentId = (string) instance.ExtendedData["ParentDatasetIdStr"];
                if (parentId == null)
                    {
                    continue;
                    }
                IECInstance parentInst = parentList.FirstOrDefault(inst => inst.InstanceId == parentId);
                if ( parentInst == null )
                    {
                    throw new ProgrammerException("There should be a parent!");
                    }

                    var relationshipInst = relCrit.RelatedClassSpecifier.RelationshipClass.CreateRelationship(parentInst, instance);

                }
                
            }

        private void CreateIncompleteCacheInstances (USGSExtractedResult item, string classification, string parentId)
            {
            IECInstance SEBInstance = m_schema.GetClass("SpatialEntityBase").CreateInstance();
            IECInstance metadataInstance = m_schema.GetClass("Metadata").CreateInstance();
            IECInstance SDSInstance = m_schema.GetClass("SpatialDataSource").CreateInstance();

            SEBInstance.ExtendedDataValueSetter.Add("Complete", false);
            metadataInstance.ExtendedDataValueSetter.Add("Complete", false);
            SDSInstance.ExtendedDataValueSetter.Add("Complete", false);

            JToken jtoken = item.jToken;

            string instanceId = jtoken.TryToGetString("sourceId");

            SEBInstance.InstanceId = instanceId;
            metadataInstance.InstanceId = instanceId;
            SDSInstance.InstanceId = instanceId;

            SEBInstance["Id"].StringValue = instanceId;
            metadataInstance["Id"].StringValue = instanceId;
            SDSInstance["Id"].StringValue = instanceId;

            SEBInstance["Name"].StringValue = item.Title;

            var bbox = jtoken["boundingBox"];

            if ( bbox != null )
                {
                SEBInstance["Footprint"].StringValue = "{ \"points\" : " + String.Format("[[{0},{1}],[{2},{1}],[{2},{3}],[{0},{3}],[{0},{1}]]", bbox.TryToGetString("minX"), bbox.TryToGetString("minY"), bbox.TryToGetString("maxX"), bbox.TryToGetString("maxY")) + ", \"coordinate_system\" : \"4326\" }";
                }

            SDSInstance["DataSourceType"].StringValue = jtoken.TryToGetString("format");
            SEBInstance["DataSourceTypesAvailable"].StringValue = jtoken.TryToGetString("format");

            metadataInstance["RawMetadataFormat"].StringValue = "FGDC";

            if ( item.Date.HasValue )
                {
                SEBInstance["Date"].NativeValue = item.Date.Value;
                }
            SEBInstance["AccuracyResolutionDensity"].StringValue = item.Resolution;
            SEBInstance["ResolutionInMeters"].StringValue = item.ResolutionInMeters;

            SEBInstance["DataProvider"].StringValue = "USGS";
            SEBInstance["DataProviderName"].StringValue = "United States Geological Survey";

            SEBInstance["Classification"].StringValue = classification;

            if ( jtoken["sizeInBytes"] != null )
                {
                long size;
                if ( Int64.TryParse(jtoken.TryToGetString("sizeInBytes"), out size) )
                    {
                    SDSInstance["FileSize"].NativeValue = (long) (size / 1024);
                    }
                }

            SEBInstance.ExtendedDataValueSetter.Add("ParentDatasetIdStr", parentId);

            PrepareInstanceForCaching(SEBInstance);
            PrepareInstanceForCaching(metadataInstance);
            PrepareInstanceForCaching(SDSInstance);

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

        //This is an helper method we could put in another file. It belonged in the now unused USGSThumbnailRetrievalController
        private MemoryStream DownloadThumbnail (string thumbnailUri)
            {
            if ( thumbnailUri.StartsWith("ftp") )
                {
                using ( WebClient ftpClient = new WebClient() )
                    {
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

    internal class InstanceCritTuple
        {
        public IECInstance Instance
            {
            get;
            set;
            }
        public RelatedInstanceSelectCriteria ParentRelatedCriteria
            {
            get;
            set;
            }

        public InstanceCritTuple (IECInstance instance, RelatedInstanceSelectCriteria parentRelatedCriteria)
            {
            Instance = instance;
            ParentRelatedCriteria = parentRelatedCriteria;
            }
        }
    }
