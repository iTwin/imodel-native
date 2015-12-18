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

namespace IndexECPlugin.Source.QueryProviders
    {
    internal class SqlQueryProvider : IECQueryProvider
        {
        private List<string> m_propertyLocationInSqlQuery = new List<string>();
        private Dictionary<string, object> m_paramNameToValue = new Dictionary<string, object>();

        private SQLQueryBuilder m_sqlQueryBuilder;

        private ECQuery m_query;
        private ECQuerySettings m_querySettings;

        private int m_paramNumber = 0;

        private bool m_instanceCount = false;

        private DbConnection m_dbConnection;

        private PolygonDescriptor m_polygonDescriptor;

        private IECSchema m_schema;

        public SqlQueryProvider (ECQuery query, ECQuerySettings querySettings, DbConnection dbConnection, IECSchema schema)
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

            if ( m_query.ExtendedData.ContainsKey("instanceCount") )
                {
                if ( query.ExtendedData["instanceCount"].ToString().ToLower() == "true" )
                    {
                    m_instanceCount = true;
                    }
                }

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
                int polygonSRID;
                if ( !int.TryParse(model.coordinate_system, out polygonSRID) )
                    {
                    throw new UserFriendlyException("The polygon format is not valid.");
                    }
                string polygonWKT = DbGeometryHelpers.CreateWktPolygonString(model.points);

                m_polygonDescriptor = new PolygonDescriptor
                {
                    WKT = polygonWKT,
                    SRID = polygonSRID
                };


                }
            }



        private string GetNewParamName ()
            {
            return String.Format("param{0}", m_paramNumber++);
            }

        public IEnumerable<IECInstance> CreateInstanceList ()
            {
            //In the future, it might be a good idea to use a DbConnectionFactory instead of using this connection directly

            Log.Logger.info("Fetching Index results for query " + m_query.ID);

            List<IECInstance> instanceList = new List<IECInstance>();

            string sqlCommandString;
            string sqlCountString;

            ECQueryConverter ecQueryConverter = new ECQueryConverter(m_query, m_querySettings, m_sqlQueryBuilder, m_polygonDescriptor, m_schema, m_instanceCount);

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommandString, out sqlCountString);

            m_dbConnection.Open();

            using ( DbCommand dbCommand = m_dbConnection.CreateCommand() )
                {

                dbCommand.CommandText = sqlCommandString;
                dbCommand.CommandType = CommandType.Text;
                dbCommand.Connection = m_dbConnection;

                //TODO : find a more elegant, less obscure way of obtaining the parameter Name-Value map. Maybe return it from CreateSqlCommandStringFromQuery...
                Dictionary<string, Tuple<string, DbType>> paramNameValueMap = m_sqlQueryBuilder.paramNameValueMap;

                foreach ( KeyValuePair<string, Tuple<string, DbType>> paramNameValue in paramNameValueMap )
                    {
                    DbParameter param = dbCommand.CreateParameter();
                    param.DbType = paramNameValue.Value.Item2;
                    param.ParameterName = paramNameValue.Key;
                    param.Value = paramNameValue.Value.Item1;
                    dbCommand.Parameters.Add(param);
                    }

                using ( DbDataReader reader = dbCommand.ExecuteReader() )
                    {
                    IECClass ecClass = m_query.SearchClasses.First().Class;

                    IEnumerable<IECProperty> propertyList;
                    if ( m_query.SelectClause.SelectAllProperties )
                        {
                        propertyList = ecClass;
                        }
                    else
                        {
                        propertyList = m_query.SelectClause.SelectedProperties;
                        }
                    while ( reader.Read() )
                        {
                        int i = 0;
                        IECInstance instance = ecClass.CreateInstance();
                        if ( (m_querySettings != null) && ((m_querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None) && ecClass.Name == "Thumbnail" )
                            {
                            //The purpose of this "If" statement is to enable the thumbnail instance to send its data through a stream.
                            //The fact that the data is always the first row has been hard coded for this to work, and it is quite probably 
                            //the most obscure and ugly piece of this code. To be rewritten...

                            //Since I think getStream only works with sqlServer, we'll use the GetBytes method

                            //Byte[] byteArray = new byte[8040];
                            //long offset = 0;
                            //long bytesread;
                            //while((bytesread = reader.GetBytes(i, offset, byteArray, 0, byteArray.Length)) > 0)
                            //{
                            //     offset += reader.GetBytes(i, offset, byteArray, 0, byteArray.Length);
                            //}
                            //i++;

                            Byte[] byteArray = (byte[]) reader[i];

                            MemoryStream mStream = new MemoryStream(byteArray);

                            StreamBackedDescriptor desc = new StreamBackedDescriptor(mStream, "Thumbnail", mStream.Length, DateTime.UtcNow);
                            StreamBackedDescriptorAccessor.SetIn(instance, desc);

                            i++;

                            }
                        foreach ( IECProperty prop in propertyList )
                            {
                            if ( ecClass.Contains(prop.Name) )
                                {
                                IECPropertyValue instancePropertyValue = instance[prop.Name];
                                IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");

                                if ( dbColumn == null )
                                    {
                                    //This is not a column in the sql table. We skip it...
                                    continue;
                                    }

                                var isSpatial = dbColumn["IsSpatial"];
                                if ( isSpatial.IsNull || isSpatial.StringValue == "false" )
                                    {

                                    if ( !reader.IsDBNull(i) )
                                        {
                                        ECToSQLMap.SQLReaderToECProperty(instancePropertyValue, reader, i);
                                        }
                                    }
                                else
                                    {
                                    if ( !ECTypeHelper.IsString(instancePropertyValue.Type) )
                                        {
                                        throw new ProgrammerException(String.Format("The property {0} tagged as spatial must be declared as a string in the ECSchema.", prop.Name));
                                        }
                                    else
                                        {
                                        string WKTString = reader.GetString(i);

                                        //DbGeometry geom = DbGeometry.FromText(WKTString);


                                        i++;

                                        //int SRID = reader.GetInt32(i);

                                        //instancePropertyValue.StringValue = "{ \"points\" : " + DbGeometryHelpers.ExtractPointsLongLat(geom) + ", \"coordinate_system\" : \"" + reader.GetInt32(i) + "\" }";
                                        instancePropertyValue.StringValue = "{ \"points\" : " + DbGeometryHelpers.ExtractOuterShellPointsFromWKTPolygon(WKTString) + ", \"coordinate_system\" : \"" + reader.GetInt32(i) + "\" }";
                                        }
                                    }
                                i++;
                                }
                            }
                        string IDProperty = ecClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                        instance.InstanceId = instance.GetInstanceStringValue(IDProperty, "");

                        instanceList.Add(instance);

                        }

                    m_dbConnection.Close();

                    //Creation of related instances
                    foreach ( var instance in instanceList )
                        {
                        foreach ( var crit in m_query.SelectClause.SelectedRelatedInstances )
                            {


                            //var reversedRelation = crit.RelatedClassSpecifier.RelationshipClass.Schema.GetClass(crit.RelatedClassSpecifier.RelationshipClass.Name);

                            IECRelationshipClass relationshipClass = crit.RelatedClassSpecifier.RelationshipClass;
                            RelatedInstanceDirection direction = ReverseRelatedInstanceDirection(crit.RelatedClassSpecifier.RelatedDirection);

                            ECQuery relInstQuery = new ECQuery(crit.RelatedClassSpecifier.RelatedClass);

                            relInstQuery.SelectClause.SelectedProperties = crit.SelectedProperties;
                            relInstQuery.SelectClause.SelectAllProperties = crit.SelectAllProperties;


                            //We probably should not take relatedInstances of relatedInstances, since this could be highly inefficient. To test eventually.
                            //relInstQuery.SelectClause.SelectedRelatedInstances = crit.SelectedRelatedInstances;
                            //************************************************************************************

                            //We create the " reverse criterion", which we will use to search for every entity related to the instance by the same relationship used in the select.
                            //var reverseCrit = new RelatedCriterion(new QueryRelatedClassSpecifier(relationshipClass, direction, relatedCriterionClass), new WhereCriteria(new ECInstanceIdExpression(instance.InstanceId)));
                            var reverseCrit = new RelatedCriterion(new QueryRelatedClassSpecifier(relationshipClass, direction, ecClass), new WhereCriteria(new ECInstanceIdExpression(instance.InstanceId)));

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

                }
            if ( m_instanceCount && sqlCountString != null )
                {
                instanceList.Add(CreateInstanceCountInstance(sqlCountString));
                }

            return instanceList;
            }

        private static RelatedInstanceDirection ReverseRelatedInstanceDirection (RelatedInstanceDirection direction)
            {
            if ( direction == RelatedInstanceDirection.Backward )
                {
                direction = RelatedInstanceDirection.Forward;
                }
            else
                {
                direction = RelatedInstanceDirection.Backward;
                }
            return direction;
            }

        //This is not to be used before having called createSqlCommandStringFromQuery
        private IECInstance CreateInstanceCountInstance (string sqlCountString)
            {
            IECInstance instanceCountInstance = StandardClassesHelper.InstanceCountClass.CreateInstance();
            instanceCountInstance.InstanceId = "InstanceCount";

            using ( DbCommand dbCommand = m_dbConnection.CreateCommand() )
                {
                dbCommand.CommandText = sqlCountString;
                dbCommand.CommandType = CommandType.Text;
                dbCommand.Connection = m_dbConnection;

                using ( DbDataReader reader = dbCommand.ExecuteReader() )
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
