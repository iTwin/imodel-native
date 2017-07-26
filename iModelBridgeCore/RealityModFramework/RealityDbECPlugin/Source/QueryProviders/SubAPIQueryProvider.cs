using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECSystem.Configuration;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace IndexECPlugin.Source.QueryProviders
    {

    /// <summary>
    /// Base class for sub indexes
    /// </summary>
    public abstract class SubAPIQueryProvider : IECQueryProvider
        {

        private DataSource m_dataSource;
        private ECQuery m_query;
        private ECQuerySettings m_querySettings;
        private IInstanceCacheManager m_instanceCacheManager;
        private IECSchema m_schema;
        private bool m_cacheIsActive = false;

        Dictionary<IECClass, List<IECInstance>> m_storageForCaching;

        /// <summary>
        /// The ECQuery associated to this object
        /// </summary>
        protected ECQuery Query
            {
            get
                {
                return m_query;
                }
            private set
                {
                this.m_query = value;
                }
            }

        /// <summary>
        /// The ECQuerySettings associated to this object
        /// </summary>
        protected ECQuerySettings QuerySettings
            {
            get
                {
                return m_querySettings;
                }
            private set
                {
                this.m_querySettings = value;
                }
            }

        /// <summary>
        /// The instance cache manager object used to access the cache tables in the database.
        /// </summary>
        protected IInstanceCacheManager InstanceCacheManager
            {
            get
                {
                return m_instanceCacheManager;
                }
            private set
                {
                this.m_instanceCacheManager = value;
                }
            }

        /// <summary>
        /// The Schema containing the necessary classes (SpatialEntity, Metadata, SpatialDataSource, Server, SpatialEntityWithDetailsView)
        /// </summary>
        protected IECSchema Schema
            {
            get
                {
                return m_schema;
                }
            private set
                {
                this.m_schema = value;
                }
            }


        /// <summary>
        /// Query a specific SpatialEntityWithDetailsView by its ID. This method has the responsibility to prepare the instances (SpatialEntity, Metadata and SpatialDataSource) for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialEntityWithDetailsView instance</returns>
        abstract protected IECInstance QuerySingleSpatialEntityWithDetailsView (string sourceID);
        
        /// <summary>
        /// Query a specific SpatialEntity by its ID. This method has the responsibility to prepare the instance for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialEntity instance</returns>
        abstract protected IECInstance QuerySingleSpatialEntity (string sourceID);

        /// <summary>
        /// Query a specific Metadata by its ID. This method has the responsibility to prepare the instance for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The Metadata instance</returns>
        abstract protected IECInstance QuerySingleMetadata (string sourceID);

        /// <summary>
        /// Query a specific SpatialDataSource by its ID. This method has the responsibility to prepare the instance for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The SpatialDataSource instance</returns>
        abstract protected IECInstance QuerySingleSpatialDataSource (string sourceID);

        /// <summary>
        /// Query a specific Server by its ID. This method has the responsibility to prepare the instance for caching.
        /// </summary>
        /// <param name="sourceID">The Id</param>
        /// <returns>The Server instance</returns>
        abstract protected IECInstance QuerySingleServer (string sourceID);

        /// <summary>
        /// Query SpatialEntityWithDetailsView instances in a given polygon. This method has the responsibility to prepare the instances (SpatialEntity, Metadata and SpatialDataSource) for caching.
        /// Also has the responsibility to query the cache for cached instances in the case the subAPI doesn't work.
        /// </summary>
        /// <param name="polygon">The polygon</param>
        /// <param name="criteriaList">List of criteria extracted from the query</param>
        /// <returns>The Server instance</returns>
        abstract protected List<IECInstance> QuerySpatialEntitiesWithDetailsViewByPolygon (string polygon, List<SingleWhereCriteriaHolder> criteriaList);

        /// <summary>
        /// Completes the fields that are not contained in the database, and are fixed by the sub api.
        /// </summary>
        /// <param name="cachedInstances">The instances to complete.</param>
        /// <param name="ecClass">The ECClass of the instances.</param>
        abstract protected void CompleteInstances (List<IECInstance> cachedInstances, IECClass ecClass);


        /// <summary>
        /// SubAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="cacheManager">The cache manager that will be used to access the cache in the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        /// <param name="dataSource">The data source representing the sub API</param>
        public SubAPIQueryProvider (ECQuery query, ECQuerySettings querySettings, IInstanceCacheManager cacheManager, IECSchema schema, DataSource dataSource)
            {
            Query = query;
            QuerySettings = querySettings;
            Schema = schema;
            m_dataSource = dataSource;

            if ( cacheManager != null )
                {
                m_cacheIsActive = true;
                m_storageForCaching = new Dictionary<IECClass, List<IECInstance>>();
                InstanceCacheManager = cacheManager;
                }
            }


        /// <summary>
        /// SubAPIQueryProvider constructor
        /// </summary>
        /// <param name="query">The ECQuery received by the plugin</param>
        /// <param name="querySettings">The ECQuerySettings received by the plugin</param>
        /// <param name="dbQuerier">The IDbQuerier object used to communicate with the database</param>
        /// <param name="schema">The schema of the ECPlugin</param>
        /// <param name="dataSource">The data source representing the sub API</param>
        /// <param name="activateCache">If the sub API uses the cache, set to true, otherwise, false.</param>
        public SubAPIQueryProvider (ECQuery query, ECQuerySettings querySettings, IDbQuerier dbQuerier, IECSchema schema, DataSource dataSource, bool activateCache)
            {
            Query = query;
            QuerySettings = querySettings;
            
            Schema = schema;
            m_dataSource = dataSource;

            if ( activateCache )
                {
                m_cacheIsActive = true;

                uint daysCacheIsValid;

                if ( !UInt32.TryParse(ConfigurationRoot.GetAppSetting("RECPDaysCacheIsValid"), out daysCacheIsValid) )
                    {
                    daysCacheIsValid = 10;
                    }

                m_storageForCaching = new Dictionary<IECClass, List<IECInstance>>();
                InstanceCacheManager = new InstanceCacheManager(dataSource, daysCacheIsValid, querySettings, dbQuerier);
                }

            }

        /// <summary>
        /// This method returns the instances according to the ECQuery received in the constructor.
        /// The instances are created from the results of a query of the sub API
        /// </summary>
        /// <returns>An IEnumerable containing the instances requested in the ECQuery received in the constructor</returns>
        public IEnumerable<IECInstance> CreateInstanceList ()
            {
            //This method will map to other methods, depending on the search class in the query

            Log.Logger.info("Fetching " + SourceStringMap.SourceToString(m_dataSource) + " results for query " + Query.ID);

            List<IECInstance> instanceList = null;
            //InstanceCacheManager instanceCacheManager = new InstanceCacheManager();

            IECClass ecClass = Query.SearchClasses.First().Class;

            int i = 0;
            while ( (i < Query.WhereClause.Count) && (instanceList == null) )
                {
                //We'll take the first ECInstanceIdExpression criterion. Normally, there is only one, non embedded criterion of this sort when queried by WSG.
                if ( Query.WhereClause[i] is ECInstanceIdExpression )
                    {
                    //instanceList = new List<IECInstance>();

                    ECInstanceIdExpression instanceIDExpression = (ECInstanceIdExpression) Query.WhereClause[i];

                    if ( instanceIDExpression.InstanceIdSet.Count() == 0 )
                        {
                        throw new UserFriendlyException("Please specify at least one ID in this type of where clause");
                        }

                    //We search in the cache for the instances of asked Ids

                    instanceList = CreateInstancesFromInstanceIdList(instanceIDExpression.InstanceIdSet, ecClass, Query.SelectClause);

                    //return instanceList;
                    }
                i++;
                }

            if ( instanceList == null )
                {
                // If we're here, that's because there was a polygon parameter in the extended data
                // We verify that this is true

                if ( !Query.ExtendedData.ContainsKey("polygon") )
                    {
                    throw new UserFriendlyException("Queries to this source should only be made using a spatial criterion or an Id criterion.");
                    }
                //We verify that the polygon has a valid format
                string polygonString = Query.ExtendedData["polygon"].ToString();
                try
                    {
                    JsonConvert.DeserializeObject<PolygonModel>(polygonString);
                    }
                catch ( JsonException )
                    {
                    throw new UserFriendlyException("The polygon format is not valid.");
                    }


                switch ( ecClass.Name )
                    {
                    case "SpatialEntityWithDetailsView":
                        instanceList = QuerySpatialEntitiesWithDetailsViewByPolygon(Query.ExtendedData["polygon"].ToString(), ExtractPropertyWhereClauses());
                        break;
                    default:
                        throw new UserFriendlyException("It is impossible to query instances of the class \"" + ecClass.Name + "\" in a sub API spatial request. The only class allowed is \"SpatialEntityWithDetailsView\".");
                    }
                }

            //Related instances of each instance found before
            //foreach ( var instance in instanceList )
            //    {
            //    CreateRelatedInstance(instance, Query.SelectClause.SelectedRelatedInstances);
            //    }

            PrepareCachingStatements();

            Thread cachingThread = new Thread(() => SendCachingStatements());
            cachingThread.Start();

            return instanceList;

            }

        private IECInstance CreateInstanceFromID (IECClass ecClass, string sourceID/*, bool allowSEWDV = false*/)
            {

            if ( !SourceStringMap.IsValidId(m_dataSource, sourceID) )
                {
                return null;
                }
            switch ( ecClass.Name )
                {
                case "SpatialEntity":
                    return QuerySingleSpatialEntity(sourceID);
                case "SpatialEntityWithDetailsView":
                    return QuerySingleSpatialEntityWithDetailsView(sourceID);
                case "Metadata":
                    return QuerySingleMetadata(sourceID);
                case "SpatialDataSource":
                    return QuerySingleSpatialDataSource(sourceID);
                case "Server":
                    return QuerySingleServer(sourceID);
                default:
                    throw new UserFriendlyException("It is impossible to query instances of the class \"" + ecClass.Name + "\" in a sub API request by ID.");
                }
            }

        /// <summary>
        /// Add the instance to the list of instances to insert in the cache.
        /// </summary>
        /// <param name="instance">The instance to insert in the cache</param>
        protected void PrepareInstanceForCaching (IECInstance instance)
            {
            if ( m_cacheIsActive )
                {
                IECClass baseClass = GetCacheBaseClass(instance.ClassDefinition);
                if ( !m_storageForCaching.Keys.Contains(baseClass) )
                    {
                    m_storageForCaching.Add(baseClass, new List<IECInstance>());
                    }
                m_storageForCaching[baseClass].Add(instance);
                }
            }

        private IECClass GetCacheBaseClass (IECClass ecClass)
            {
            switch ( ecClass.Name )
                {
                case "SpatialEntity":
                case "SpatialEntityWithDetailsView":
                case "Metadata":
                case "SpatialDataSource":
                case "Server":
                    return ecClass;
                case "OtherSource":
                    return ecClass.BaseClasses.First(c => c.Name == "SpatialDataSource");

                default:
                    throw new ProgrammerException("It is impossible to cache instances of the class \"" + ecClass.Name + "\"");
                }
            }

        private void PrepareCachingStatements ()
            {
            if ( m_cacheIsActive )
                {
                //Parallel.ForEach(m_storageForCaching, (instancesGroup) =>
                foreach ( var instancesGroup in m_storageForCaching )
                    {
                    InstanceCacheManager.PrepareCacheInsertStatement(instancesGroup.Value, instancesGroup.Key, null);
                    //});
                    }
                }
            }

        //The sending of the Statements has been separated from their creation, since we wanted to send the Db query on a separate thread,
        //while creating the statements on the main thread. This was done to make sure that the instances the statements are based on 
        //are not modified on the main thread while they are still used to create the statement.
        private void SendCachingStatements ()
            {
            if ( m_cacheIsActive )
                {
                InstanceCacheManager.SendAllPreparedCacheInsertStatements();
                }
            }

        private List<IECInstance> CreateInstancesFromInstanceIdList (IEnumerable<string> instanceIdSet, IECClass ecClass, SelectCriteria selectClause)
            {
            List<IECInstance> instanceList = new List<IECInstance>();
            List<IECInstance> cachedInstances = null;
            if ( m_cacheIsActive )
                {
                cachedInstances = InstanceCacheManager.QueryInstancesFromCache(instanceIdSet, ecClass, GetCacheBaseClass(ecClass), selectClause);
                CompleteInstances(cachedInstances, ecClass);
                CreateCacheRelatedInstances(cachedInstances, selectClause.SelectedRelatedInstances);
                }
            //We create the requested instances that were not in the cache
            foreach ( string sourceID in instanceIdSet )
                {
                IECInstance instance = cachedInstances == null ? null : cachedInstances.FirstOrDefault(inst => inst.InstanceId == sourceID);
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
                            //We had nothing in the cache and the sub API returned an error. We cannot continue
                            throw;
                            }
                        }
                    }
                if ( instance != null )
                    {
                    CreateRelatedInstance(instance, selectClause.SelectedRelatedInstances);
                    instanceList.Add(instance);
                    }

                }
            return instanceList;
            }

        private void CreateCacheRelatedInstances (List<IECInstance> cachedInstances, List<RelatedInstanceSelectCriteria> relatedCriteriaList)
            {
            foreach ( RelatedInstanceSelectCriteria crit in relatedCriteriaList )
                {
                IECClass relatedClass = crit.RelatedClassSpecifier.RelatedClass;

                IECRelationshipClass relationshipClass = crit.RelatedClassSpecifier.RelationshipClass;

                List<string> relInstIDs = new List<string>();

                foreach ( IECInstance instance in cachedInstances )
                    {
                    if ( !relInstIDs.Contains(instance.InstanceId) )
                        {
                        relInstIDs.Add(instance.InstanceId);
                        }
                    }

                List<IECInstance> relInstances = InstanceCacheManager.QueryInstancesFromCache(relInstIDs, relatedClass, GetCacheBaseClass(relatedClass), crit);
                CompleteInstances(relInstances, relatedClass);
                CreateCacheRelatedInstances(relInstances, crit.SelectedRelatedInstances);
                foreach ( IECInstance relInstance in relInstances )
                    {
                    IEnumerable<IECInstance> instances;

                    instances = cachedInstances.Where(i => i.InstanceId == relInstance.InstanceId);

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

        /// <summary>
        /// Creates the instances related to an instance, according to the RelatedInstanceSelectCriteria list. This method is recursive,
        /// meaning that this method is called on the related instances created before returning.
        /// </summary>
        /// <param name="instance">The instance for which we want the related instances ()</param>
        /// <param name="relatedCriteriaList"></param>
        /// <returns>null</returns>
        private void CreateRelatedInstance (IECInstance instance, List<RelatedInstanceSelectCriteria> relatedCriteriaList)
            {
            foreach ( RelatedInstanceSelectCriteria crit in relatedCriteriaList )
                {
                IECClass relatedClass = crit.RelatedClassSpecifier.RelatedClass;

                IECRelationshipClass relationshipClass = crit.RelatedClassSpecifier.RelationshipClass;

                //Tuple<string, JObject> oldJsonCache = jsonCache;

                string relInstID = instance.InstanceId;

                IECInstance relInst = null;
                bool mustAddRelation = false;

                relInst = instance.GetRelationshipInstances().GetRelatedInstances(relationshipClass, crit.RelatedClassSpecifier.RelatedDirection, relatedClass).FirstOrDefault();
                if ( relInst == null )
                    {
                    mustAddRelation = true;
                    }
                if ( relInst == null || (bool) relInst.ExtendedData["Complete"] == false )
                    {
                    try
                        {
                        relInst = CreateInstanceFromID(relatedClass, relInstID);
                        }
                    catch ( EnvironmentalException )
                        {
                        if ( relInst == null )
                            {
                            //We had nothing in the cache and the sub API returned an error. We cannot continue
                            throw;
                            }
                        }
                    ////We reset the json cache to the old one, since it is more likely to be useful than the old one (which may contain the json of the parent, which we don't need)
                    //jsonCache = oldJsonCache;
                    }
                if ( relInst != null && mustAddRelation )
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
                    }
                CreateRelatedInstance(relInst, crit.SelectedRelatedInstances);

                }
            }

        private List<SingleWhereCriteriaHolder> ExtractPropertyWhereClauses ()
            {
            List<SingleWhereCriteriaHolder> singleWhereCriteriaList = new List<SingleWhereCriteriaHolder>();
            var whereCriteria = Query.WhereClause;
            for ( int i = 0; i < whereCriteria.Count; i++ )
                {
                if ( i > 0 )
                    {
                    if ( whereCriteria.GetLogicalOperatorBefore(i) != LogicalOperator.AND )
                        {
                        throw new UserFriendlyException("Please use only AND operator to filter the SubAPI requests");
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
                    throw new UserFriendlyException("Related Criterions are not allowed in SubAPI requests");
                    }
                if ( criterion is WhereCriteria )
                    {
                    throw new UserFriendlyException("Complex expressions are not allowed in SubAPI requests");
                    }
                }

            return singleWhereCriteriaList;
            }
        }
    }
