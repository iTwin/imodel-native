/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/QueryProviders/SqlQueryProvider.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Query;
using Bentley.EC.PluginBuilder.Modules;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECObjects.Standards;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
//using System.Data.Entity.Spatial;
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Text;
using System.Reflection;

namespace IndexECPlugin.Source.QueryProviders
    {
    internal class SqlQueryProvider : IECQueryProvider
        {
        private List<string> m_propertyLocationInSqlQuery = new List<string>();
        private Dictionary<string, object> m_paramNameToValue = new Dictionary<string, object>();

        private SQLQueryBuilder m_sqlQueryBuilder;

        private ECQuery m_query;
        private ECQuerySettings m_querySettings;

        private bool m_instanceCount = false;

        private IDbConnection m_dbConnection;

        private PolygonDescriptor m_polygonDescriptor;

        private IECSchema m_schema;

        public SqlQueryProvider (ECQuery query, ECQuerySettings querySettings, IDbConnection dbConnection, IECSchema schema)
            {
            if ( query == null )
                {
                throw new ArgumentNullException("query");
                }

            if ( dbConnection == null )
                {
                throw new ArgumentNullException("dbConnection");
                }

            if ( schema == null )
                {
                throw new ArgumentNullException("schema");
                }

            m_query = query;
            m_querySettings = querySettings;
            m_dbConnection = dbConnection;
            m_schema = schema;
            m_polygonDescriptor = null;

            //WE DEACTIVATE INSTANCE COUNT FOR NOW
            //if ( m_query.ExtendedData.ContainsKey("instanceCount") )
            //    {
            //    if ( query.ExtendedData["instanceCount"].ToString().ToLower() == "true" )
            //        {
            //        m_instanceCount = true;
            //        }
            //    }

            //WE DEACTIVATE PAGING FOR NOW
            //if (m_query.ResultRangeOffset >= 0 && m_query.MaxResults > 0)
            //{
            //    int lowerBound = m_query.ResultRangeOffset + 1;
            //    int upperBound = m_query.MaxResults + lowerBound - 1;

            //    m_sqlQueryBuilder = new PagedSQLQueryBuilder(lowerBound, upperBound);

            //    //This instruction is to prevent WSG from reusing the ResultRangeOffset (skip) a second time,
            //    //since we take care of the paging ourselves...
            //    m_query.ResultRangeOffset = 0;
            //}
            //else
            //{
            m_sqlQueryBuilder = new StandardSQLQueryBuilder();
            //}

            if ( m_query.ExtendedData.ContainsKey("polygon") )
                {
                string polygonString = query.ExtendedData["polygon"].ToString();
                PolygonModel model;
                try
                    {
                    model = JsonConvert.DeserializeObject<PolygonModel>(polygonString);
                    }
                catch ( JsonException )
                    {
                    throw new UserFriendlyException("The polygon format is not valid.");
                    }

                string polygonWKT = DbGeometryHelpers.CreateWktPolygonString(model.points);

                m_polygonDescriptor = new PolygonDescriptor
                {
                    WKT = polygonWKT,
                    SRID = model.coordinate_system
                };


                }
            }

        public IEnumerable<IECInstance> CreateInstanceList ()
            {
            Log.Logger.info("Fetching Index results for query " + m_query.ID);

            string sqlCommandString;
            string sqlCountString;

            ECQueryConverter ecQueryConverter = new ECQueryConverter(m_query, m_querySettings, m_sqlQueryBuilder, m_polygonDescriptor, m_schema, m_instanceCount);

            DataReadingHelper dataReadingHelper;

            IParamNameValueMap paramNameValueMap;

            IECClass ecClass = m_query.SearchClasses.First().Class;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommandString, out sqlCountString, out dataReadingHelper, out paramNameValueMap);

            IEnumerable<IECProperty> propertyList;
            if ( m_query.SelectClause.SelectAllProperties )
                {
                propertyList = ecClass;
                }
            else
                {
                propertyList = m_query.SelectClause.SelectedProperties;
                }

            List<IECInstance> instanceList = SqlQueryHelpers.QueryDbForInstances( sqlCommandString, dataReadingHelper, paramNameValueMap, ecClass, propertyList, m_dbConnection);
            

            //Creation of related instances
            //foreach ( var instance in instanceList )
            //    {
            if ( instanceList.Count != 0 )
                {
                foreach ( var crit in m_query.SelectClause.SelectedRelatedInstances )
                    {


                    //var reversedRelation = crit.RelatedClassSpecifier.RelationshipClass.Schema.GetClass(crit.RelatedClassSpecifier.RelationshipClass.Name);

                    IECRelationshipClass relationshipClass = crit.RelatedClassSpecifier.RelationshipClass;
                    RelatedInstanceDirection direction = (crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Backward ?
                        RelatedInstanceDirection.Forward : RelatedInstanceDirection.Backward);

                    ECQuery relInstQuery = new ECQuery(crit.RelatedClassSpecifier.RelatedClass);

                    relInstQuery.SelectClause.SelectedProperties = crit.SelectedProperties;
                    relInstQuery.SelectClause.SelectAllProperties = crit.SelectAllProperties;


                    //We probably should not take relatedInstances of relatedInstances, since this could be highly inefficient. To test eventually.
                    relInstQuery.SelectClause.SelectedRelatedInstances = crit.SelectedRelatedInstances;
                    //************************************************************************************

                    //We create the " reverse criterion", which we will use to search for every entity related to the instance by the same relationship used in the select.
                    //var reverseCrit = new RelatedCriterion(new QueryRelatedClassSpecifier(relationshipClass, direction, relatedCriterionClass), new WhereCriteria(new ECInstanceIdExpression(instance.InstanceId)));
                    ECInstanceIdExpression ex = new ECInstanceIdExpression(instanceList.Select(inst => inst.InstanceId).ToArray());

                    var reverseCrit = new RelatedCriterion(new QueryRelatedClassSpecifier(relationshipClass, direction, ecClass), new WhereCriteria(ex));
                    reverseCrit.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("RequestRelatedId", true));

                    relInstQuery.WhereClause = new WhereCriteria(reverseCrit);

                    var queryHelper = new SqlQueryProvider(relInstQuery, new ECQuerySettings(), m_dbConnection, m_schema);

                    var relInstList = queryHelper.CreateInstanceList();
                    foreach ( var relInst in relInstList )
                        {
                        string instanceId = relInst.ExtendedData["relInstID"] as string;

                        IECInstance instance = instanceList.First(inst => inst.InstanceId == instanceId);

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
                    }
                }

            //}

            if ( m_instanceCount && sqlCountString != null )
                {
                instanceList.Add(CreateInstanceCountInstance(sqlCountString));
                }

            return instanceList;
            }

        //This was the old way of searching for related instances. It was slow, but reliable. Left here just in case the other one is buggy.
        private void QueryAndInsertRelatedInstancesOneByOne(IEnumerable<IECInstance> instanceList)
            {
            foreach ( var instance in instanceList )
                {
                foreach ( var crit in m_query.SelectClause.SelectedRelatedInstances )
                    {


                    //var reversedRelation = crit.RelatedClassSpecifier.RelationshipClass.Schema.GetClass(crit.RelatedClassSpecifier.RelationshipClass.Name);

                    IECRelationshipClass relationshipClass = crit.RelatedClassSpecifier.RelationshipClass;
                    RelatedInstanceDirection direction = (crit.RelatedClassSpecifier.RelatedDirection == RelatedInstanceDirection.Backward ?
                        RelatedInstanceDirection.Forward : RelatedInstanceDirection.Backward);

                    ECQuery relInstQuery = new ECQuery(crit.RelatedClassSpecifier.RelatedClass);

                    relInstQuery.SelectClause.SelectedProperties = crit.SelectedProperties;
                    relInstQuery.SelectClause.SelectAllProperties = crit.SelectAllProperties;


                    //We probably should not take relatedInstances of relatedInstances, since this could be highly inefficient. To test eventually.
                    //relInstQuery.SelectClause.SelectedRelatedInstances = crit.SelectedRelatedInstances;
                    //************************************************************************************

                    //We create the " reverse criterion", which we will use to search for every entity related to the instance by the same relationship used in the select.
                    var reverseCrit = new RelatedCriterion(new QueryRelatedClassSpecifier(relationshipClass, direction, instance.ClassDefinition), new WhereCriteria(new ECInstanceIdExpression(instance.InstanceId)));

                    relInstQuery.WhereClause = new WhereCriteria(reverseCrit);

                    var queryHelper = new SqlQueryProvider(relInstQuery, new ECQuerySettings(), m_dbConnection, m_schema);



                    var relInstList = queryHelper.CreateInstanceList();
                    foreach ( var relInst in relInstList )
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
                    }
                }
            }

        //This is not to be used before having called createSqlCommandStringFromQuery
        private IECInstance CreateInstanceCountInstance (string sqlCountString)
            {
            IECInstance instanceCountInstance = StandardClassesHelper.InstanceCountClass.CreateInstance();
            instanceCountInstance.InstanceId = "InstanceCount";

            using ( IDbCommand dbCommand = m_dbConnection.CreateCommand() )
                {
                dbCommand.CommandText = sqlCountString;
                dbCommand.CommandType = CommandType.Text;
                dbCommand.Connection = m_dbConnection;

                using ( IDataReader reader = dbCommand.ExecuteReader() )
                    {
                    reader.Read();
                    instanceCountInstance["Count"].IntValue = reader.GetInt32(0);
                    }
                }

            m_query.SearchClasses.Add(new SearchClass(StandardClassesHelper.InstanceCountClass));

            return instanceCountInstance;
            }
        }
    }
