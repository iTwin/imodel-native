using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;

namespace IndexECPlugin.Source.Helpers
    {

    /// <summary>
    /// Interface of an instance cache manager, whose purpose is to fetch instances from the cache as well as insert them.
    /// </summary>
    public interface IInstanceCacheManager
        {
        /// <summary>
        /// Inserts instances in cache
        /// </summary>
        /// <param name="instanceList">The list of instances to insert in the cache</param>
        /// <param name="ecClass">The ecClass of the instances to insert.</param>
        /// <param name="additionalColumns">Additional data to add to columns that are not related to any ECProperty</param>
        void InsertInstancesInCache (IEnumerable<IECInstance> instanceList, IECClass ecClass, IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>> additionalColumns = null);

        /// <summary>
        /// Queries instances from the cache, using a list of ids. Only instances of ids that are present in the cache will be returned.
        /// </summary>
        /// <param name="instanceIdsList">The list of ids of the instances to fetch in the cache.</param>
        /// <param name="actualECClass">The class of the instances to create from the data fetched in the database.</param>
        /// <param name="baseECClass">The class that is persisted in the database. The class must have a property in the SQLEntity custom parameter to indicate the database table to use.</param>
        /// <param name="selectCriteria">The criteria indicating which properties to fetch</param>
        /// <returns>The cached instances that were present in the database and were in the ids list.</returns>
        List<IECInstance> QueryInstancesFromCache (IEnumerable<string> instanceIdsList, IECClass actualECClass, IECClass baseECClass, SelectCriteria selectCriteria);

        /// <summary>
        /// Queries instances from the cache, using a polygon an a where criteria.
        /// </summary>
        /// <param name="polygonDescriptor">The polygon used as a delimiter for the query</param>
        /// <param name="actualECClass">The class of the instances to create from the data fetched in the database.</param>
        /// <param name="baseECClass">The class that is persisted in the database. The class must have a property in the SQLEntity custom parameter to indicate the database table to use.</param>
        /// <param name="selectCriteria">The criteria indicating which properties to fetch</param>
        /// <param name="whereCriteria">The where criteria list used as a filter in the query.</param>
        /// <returns>The cached instances inside the polygon and fulfilling the criteria list.</returns>
        List<IECInstance> QuerySpatialInstancesFromCache (PolygonDescriptor polygonDescriptor, IECClass actualECClass, IECClass baseECClass, SelectCriteria selectCriteria, List<SingleWhereCriteriaHolder> whereCriteria);
        }

    /// <summary>
    /// Class used to manage the cache data of the SubAPI sources.
    /// </summary>
    public class InstanceCacheManager : /*MimicTableWriter,*/ IInstanceCacheManager
        {
        DataSource m_source;
        int m_daysCacheIsValid;
        IDbConnection m_dbConnection;
        ECQuerySettings m_querySettings;

        MimicTableWriter m_mimicTableWriter;
        /// <summary>
        /// 
        /// </summary>
        /// <param name="source">The SubAPI source</param>
        /// <param name="daysCacheIsValid">The maximum age allowed for the cached information (in days)</param>
        /// <param name="dbConnection">The dbConnection used to communicate to the appropriate database</param>
        /// <param name="querySettings">The ecquery settings of the present query</param>
        public InstanceCacheManager(DataSource source, int daysCacheIsValid, IDbConnection dbConnection, ECQuerySettings querySettings)
            //: base(true, "CacheTableName", "CacheColumnName", "CacheJoinTableName", null)
            {
            m_source = source;
            m_daysCacheIsValid = daysCacheIsValid;
            m_dbConnection = dbConnection;
            m_querySettings = querySettings;

            m_mimicTableWriter = new MimicTableWriter(true, "CacheTableName", "CacheColumnName", "CacheJoinTableName", null);
            }

        /// <summary>
        /// Gets instances from the cache tables in the database.
        /// </summary>
        /// <param name="instanceIdsList">The list of ids to query.</param>
        /// <param name="actualECClass">The class of the instances to query. The instances returned will be of this ecclass</param>
        /// <param name="baseECClass">The base class of the instances to query. This is necessary because only the base properties are kept in the database</param>
        /// <param name="selectCriteria">The select criteria, for selecting the desired properties.</param>
        /// <returns>The cached instances</returns>
        public List<IECInstance> QueryInstancesFromCache(IEnumerable<string> instanceIdsList, IECClass actualECClass, IECClass baseECClass, SelectCriteria selectCriteria)
            {
            DataReadingHelper drh;
            IParamNameValueMap paramNameValueMap;
            List<string> additionalColumns = new List<string>();

            additionalColumns.Add("Complete");
            additionalColumns.Add("DateCacheCreated");

            if ( baseECClass.Name == "SpatialEntityBase" || baseECClass.Name == "SpatialEntityWithDetailsView" )
                {
                additionalColumns.Add("ParentDatasetIdStr");
                }

            if((m_querySettings != null) && ((m_querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None))
                {
                m_mimicTableWriter.GetStream = true;
                }

            //It's necessary to take the base class properties while creating the query using the base ECClass, since there are conflicts with the Id properties names.
            IEnumerable<IECProperty> propertiesSelected = selectCriteria.SelectedProperties;
            if(propertiesSelected == null)
                {
                propertiesSelected = actualECClass;
                }
            List<IECProperty> basePropertiesSelected;
            if ( actualECClass.Name != baseECClass.Name )
                {
                basePropertiesSelected = new List<IECProperty>();
                foreach ( IECProperty prop in propertiesSelected )
                    {
                    if ( baseECClass.Contains(prop.Name) )
                        {
                        basePropertiesSelected.Add(baseECClass[prop.Name]);
                        }
                    }
                }
            else
                {
                basePropertiesSelected = propertiesSelected.ToList();
                }

            try
                {
                //We add the instanceIdProperty to the selected properties. It is necessary to fetch this property to set the instance ID later
                string InstanceIDPropertyName = baseECClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                IECProperty InstanceIDProperty = baseECClass.FindProperty(InstanceIDPropertyName);

                if ( !basePropertiesSelected.Contains(InstanceIDProperty) )
                    {
                    basePropertiesSelected.Add(InstanceIDProperty);
                    }
                }
            catch ( Bentley.ECObjects.ECObjectsException.NullValue )
                {
                throw new ProgrammerException(String.Format("Error in class {0} of the ECSchema. The custom attribute InstanceIDProperty is not set", baseECClass.Name));
                }

            string sqlQueryString = m_mimicTableWriter.CreateMimicSQLQuery(m_source, instanceIdsList, baseECClass, basePropertiesSelected, out drh, out paramNameValueMap, additionalColumns);

            List<IECInstance> cachedInstances = SqlQueryHelpers.QueryDbForInstances(sqlQueryString, drh, paramNameValueMap, actualECClass, basePropertiesSelected, m_dbConnection, additionalColumns);
            //List<string> instancesToDelete;
            foreach (IECInstance oldInstance in cachedInstances.Where(inst => (DateTime.UtcNow - (DateTime)inst.ExtendedData["DateCacheCreated"]).Days > m_daysCacheIsValid).ToList())
                {
                cachedInstances.Remove(oldInstance);
                }

            return cachedInstances;
            }

        /// <summary>
        /// Gets instances from the cache tables in the database using a spatial criteria.
        /// </summary>
        /// <param name="polygonDescriptor">The polygon in which we want to get instances</param>
        /// <param name="actualECClass">The class of the instances to query. The instances returned will be of this ecclass</param>
        /// <param name="baseECClass">The base class of the instances to query. This is necessary because only the base properties are kept in the database</param>
        /// <param name="selectCriteria">The select criteria, for selecting the desired properties.</param>
        /// <param name="whereCriteria">The where criteria List, in the form of a list of criterias linked by AND logical operators</param>>
        /// <returns></returns>
        public List<IECInstance> QuerySpatialInstancesFromCache (PolygonDescriptor polygonDescriptor, IECClass actualECClass, IECClass baseECClass, SelectCriteria selectCriteria, List<SingleWhereCriteriaHolder> whereCriteria)
            {
            DataReadingHelper drh;
            IParamNameValueMap paramNameValueMap;
            List<string> additionalColumns = new List<string>();

            if ( !additionalColumns.Contains("Complete") )
                {
                additionalColumns.Add("Complete");
                }
            if ( !additionalColumns.Contains("DateCacheCreated") )
                {
                additionalColumns.Add("DateCacheCreated");
                }

            if ( !additionalColumns.Contains("ParentDatasetIdStr") && (baseECClass.Name == "SpatialEntityBase" || baseECClass.Name == "SpatialEntityWithDetailsView") )
                {
                additionalColumns.Add("ParentDatasetIdStr");
                }

            if ( baseECClass.Name == "SpatialEntityWithDetailsView" )
                {
                if ( !additionalColumns.Contains("MetadataComplete") )
                    {
                    additionalColumns.Add("MetadataComplete");
                    }
                if ( !additionalColumns.Contains("SDSComplete") )
                    {
                    additionalColumns.Add("SDSComplete");
                    }
                if ( !additionalColumns.Contains("MetadataDateCreated") )
                    {
                    additionalColumns.Add("MetadataDateCreated");
                    }
                if ( !additionalColumns.Contains("SDSDateCreated") )
                    {
                    additionalColumns.Add("SDSDateCreated");
                    }
                }

            if ( (m_querySettings != null) && ((m_querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None) )
                {
                m_mimicTableWriter.GetStream = true;
                }

            //It's necessary to take the base class properties while creating the query using the base ECClass, since there are conflicts with the Id properties names.
            IEnumerable<IECProperty> propertiesSelected = selectCriteria.SelectedProperties;
            if ( propertiesSelected == null )
                {
                propertiesSelected = actualECClass;
                }
            List<IECProperty> basePropertiesSelected;
            if ( actualECClass.Name != baseECClass.Name )
                {
                basePropertiesSelected = new List<IECProperty>();
                foreach ( IECProperty prop in propertiesSelected )
                    {
                    if ( baseECClass.Contains(prop.Name) )
                        {
                        basePropertiesSelected.Add(baseECClass[prop.Name]);
                        }
                    }
                }
            else
                {
                basePropertiesSelected = propertiesSelected.ToList();
                }

            try
                {
                //We add the instanceIdProperty to the selected properties. It is necessary to fetch this property to set the instance ID later
                string InstanceIDPropertyName = baseECClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                IECProperty InstanceIDProperty = baseECClass.FindProperty(InstanceIDPropertyName);

                if ( !basePropertiesSelected.Contains(InstanceIDProperty) )
                    {
                    basePropertiesSelected.Add(InstanceIDProperty);
                    }
                }
            catch ( Bentley.ECObjects.ECObjectsException.NullValue )
                {
                throw new ProgrammerException(String.Format("Error in class {0} of the ECSchema. The custom attribute InstanceIDProperty is not set", baseECClass.Name));
                }

            string sqlQueryString = m_mimicTableWriter.CreateMimicSQLSpatialQuery(m_source, polygonDescriptor, baseECClass, basePropertiesSelected, out drh, out paramNameValueMap, additionalColumns, whereCriteria);

            List<IECInstance> cachedInstances = SqlQueryHelpers.QueryDbForInstances(sqlQueryString, drh, paramNameValueMap, actualECClass, basePropertiesSelected, m_dbConnection, additionalColumns);
            //List<string> instancesToDelete;
            foreach ( IECInstance oldInstance in cachedInstances.Where(inst => (DateTime.UtcNow - (DateTime) inst.ExtendedData["DateCacheCreated"]).Days > m_daysCacheIsValid).ToList() )
                    {
                    cachedInstances.Remove(oldInstance);
                    }

            if ( baseECClass.Name == "SpatialEntityWithDetailsView" )
                {
                foreach ( IECInstance oldInstance in cachedInstances.Where(inst => (DateTime.UtcNow - (DateTime) inst.ExtendedData["MetadataDateCreated"]).Days > m_daysCacheIsValid ||
                                                                                   (DateTime.UtcNow - (DateTime) inst.ExtendedData["SDSDateCreated"]).Days > m_daysCacheIsValid).ToList() )
                    {
                    cachedInstances.Remove(oldInstance);
                    }
                foreach ( IECInstance instance in cachedInstances )
                    {
                    if ( (!(bool) instance.ExtendedData["MetadataComplete"]) || (!(bool) instance.ExtendedData["SDSComplete"]) )
                        {
                        instance.ExtendedData.Remove("Complete");
                        instance.ExtendedDataValueSetter.Add("Complete", false);
                        }
                    //instance.ExtendedData.Remove("MetadataComplete");
                    //instance.ExtendedData.Remove("SDSComplete");
                    }
                }
            return cachedInstances;
            }

        /// <summary>
        /// Inserts instances in the cache tables in the database
        /// </summary>
        /// <param name="instanceList">The list of instances to insert. The instances must have the extended data "Complete" set with a boolean</param>
        /// <param name="ecClass">The class of the instances to insert. It is necessary to give an ECClass that is persisted in the database</param>
        /// <param name="additionalColumns">Additional columns to insert and that are not represented by any property in the class</param>
        public void InsertInstancesInCache (IEnumerable<IECInstance> instanceList, IECClass ecClass, IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>> additionalColumns = null)
            {

            ISQLInsertStatementBuilder sqlQueryBuilder = DbConnectionHelper.GetSqlInsertStatementBuilder(m_dbConnection);
            IParamNameValueMap paramNameValueMap;

            string sqlInsertQueryString = CreateCacheSQLStatement(instanceList, ecClass, sqlQueryBuilder, out paramNameValueMap, additionalColumns);

            SqlQueryHelpers.ExecuteNonQueryInDb(sqlInsertQueryString, paramNameValueMap, m_dbConnection);
            }

        /// <summary>
        /// Creates the statement that inserts instances in the cache tables on the database
        /// </summary>
        /// <param name="instanceList">The list of instances to insert. The instances must have the extended data "Complete" set with a boolean</param>
        /// <param name="ecClass">The class of the instances to insert. It is necessary to give an ECClass that is persisted in the database</param>
        /// <param name="sqlQueryBuilder">The ISQLInsertStatementBuilder used to write the SQL statement.</param>
        /// <param name="paramNameValueMap">The IParamNameValueMap object used to complete the query</param>
        /// <param name="additionalColumns">Additional columns to fill in the database and that are not contained in the class properties.</param>
        /// <returns>The sql statement for inserting the instances in the cache.</returns>
        private string CreateCacheSQLStatement (IEnumerable<IECInstance> instanceList, 
                                               IECClass ecClass, ISQLInsertStatementBuilder sqlQueryBuilder, 
                                               out IParamNameValueMap paramNameValueMap, IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>> additionalColumns = null)
            {
            List<Tuple<string, IECType, Func<IECInstance, string>>> additionalColumnsList = new List<Tuple<string, IECType, Func<IECInstance, string>>>();

            additionalColumnsList.Add(new Tuple<string, IECType, Func<IECInstance, string>>("SubAPI", Bentley.ECObjects.ECObjects.StringType, inst => SourceStringMap.SourceToString(m_source)));
            additionalColumnsList.Add(new Tuple<string, IECType, Func<IECInstance, string>>("DateCacheCreated", Bentley.ECObjects.ECObjects.StringType, inst => DateTime.UtcNow.ToString()));
            additionalColumnsList.Add(new Tuple<string, IECType, Func<IECInstance, string>>("Complete", Bentley.ECObjects.ECObjects.BooleanType, inst => ((bool)inst.ExtendedData["Complete"]).ToString()));

            String idColumnName = ecClass["Id"].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;

            Func<IECInstance, WhereStatementManager> deleteStatementConstructor;

            deleteStatementConstructor = inst =>
            {
                WhereStatementManager deleteStatementManager;
            if ( (bool) inst.ExtendedData["Complete"] )
                {
                deleteStatementManager = new WhereStatementManager();
                deleteStatementManager.WhereStatement = idColumnName + " = @instId@ AND SubAPI = @subAPI@; ";
                deleteStatementManager.AddParameter("@instId@", Bentley.ECObjects.ECObjects.StringType, inst.InstanceId);
                deleteStatementManager.AddParameter("@subAPI@", Bentley.ECObjects.ECObjects.StringType, SourceStringMap.SourceToString(m_source));
                }
            else
                {
                deleteStatementManager = new WhereStatementManager();
                deleteStatementManager.WhereStatement = idColumnName + " = @instId@ AND SubAPI = @subAPI@ AND (Complete = @isComplete@ OR DateCacheCreated <= @dateCacheCreated@); ";
                deleteStatementManager.AddParameter("@instId@", Bentley.ECObjects.ECObjects.StringType, inst.InstanceId);
                deleteStatementManager.AddParameter("@subAPI@", Bentley.ECObjects.ECObjects.StringType, SourceStringMap.SourceToString(m_source));
                deleteStatementManager.AddParameter("@isComplete@", Bentley.ECObjects.ECObjects.BooleanType, false);
                deleteStatementManager.AddParameter("@dateCacheCreated@", Bentley.ECObjects.ECObjects.DateTimeType, DateTime.UtcNow.AddDays(m_daysCacheIsValid * -1));
                }
            return deleteStatementManager;
            };
                

            if ( additionalColumns != null )
                {
            foreach ( var column in additionalColumns )
                {
                additionalColumnsList.Add(column);
                }
                }
            return m_mimicTableWriter.CreateMimicSQLInsert(instanceList, ecClass, sqlQueryBuilder, out paramNameValueMap, additionalColumnsList, deleteStatementConstructor);
            }
        }
    }
