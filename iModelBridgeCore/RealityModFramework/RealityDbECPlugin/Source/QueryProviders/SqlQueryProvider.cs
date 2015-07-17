using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Query;
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

        private int m_paramNumber = 0;
        private int m_tabNumber = 0;

        private bool m_instanceCount = false;

        private DbConnection m_dbConnection;

        private PolygonDescriptor m_polygonDescriptor;

        public SqlQueryProvider(ECQuery query, DbConnection dbConnection)
        {
            if (query == null || dbConnection == null)
            {
                throw new ArgumentNullException("Arguments should not be null");
            }

            m_query = query;
            m_dbConnection = dbConnection;
            m_polygonDescriptor = null;

            if (m_query.ExtendedData.ContainsKey("instanceCount"))
            {
                if (query.ExtendedData["instanceCount"].ToString().ToLower() == "true")
                {
                    m_instanceCount = true;
                }
            }

            if (m_query.ResultRangeOffset >= 0 && m_query.MaxResults > 0)
            {
                int lowerBound = m_query.ResultRangeOffset + 1;
                int upperBound = m_query.MaxResults + lowerBound - 1;

                m_sqlQueryBuilder = new PagedSQLQueryBuilder(lowerBound, upperBound);

                //This instruction is to prevent WSG from reusing the ResultRangeOffset (skip) a second time,
                //since we take care of the paging ourselves...
                m_query.ResultRangeOffset = 0;
            }
            else
            {
                m_sqlQueryBuilder = new StandardSQLQueryBuilder();
            }

            if (m_query.ExtendedData.ContainsKey("polygon"))
            {
                string polygonString = query.ExtendedData["polygon"].ToString();
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

                m_polygonDescriptor = new PolygonDescriptor
                {
                    WKT = polygonWKT,
                    SRID = polygonSRID
                };


            }




        }

        private string GetNewTableAlias()
        {
            return String.Format("tab{0}", m_tabNumber++);
        }

        private string GetNewParamName()
        {
            return String.Format("param{0}", m_paramNumber++);
        }

        public IEnumerable<IECInstance> CreateInstanceList()
        {
            //In the future, it might be a good idea to use a DbConnectionFactory instead of using this connection directly


            List<IECInstance> instanceList = new List<IECInstance>();

            //Note : In the future, we could create one command string per class
            //and iterate the lines below for each class. Of course with some changes
            //in the methods in order for all of this to work.

            //*****************************TEST
            //{
            //    using (DbCommand sqlCommandtest = new SqlCommand())
            //    {
            //        sqlCommandtest.CommandText = "SELECT dbo.SpatialEntities.ID from dbo.SpatialEntities WHERE dbo.SpatialEntities.ID <= @test";
            //        sqlCommandtest.CommandType = CommandType.Text;
            //        sqlCommandtest.Connection = sqlConnection;

            //        sqlConnection.Open();

            //        DbParameter param = sqlCommandtest.CreateParameter();
            //        param.DbType = DbType.String;
            //        param.Direction = ParameterDirection.Input;
                    
            //        param.ParameterName = "@test";
            //        param.Value = "2";

            //        sqlCommandtest.Parameters.Add(param);

            //        sqlCommandtest.Prepare();

            //        sqlCommandtest.Parameters[0].Value = "2";
            //        sqlCommandtest.ExecuteNonQuery();
            //        throw new UserFriendlyException("Nothing went wrong!");



            //        sqlCommandtest.Parameters[0].Value = param;
            //    }
                
            //}



            //*********************************

            string sqlCommandString;
            string sqlCountString;
            createSqlCommandStringFromQuery(out sqlCommandString, out sqlCountString);

            m_dbConnection.Open();

            using (DbCommand dbCommand = m_dbConnection.CreateCommand())
            {

                dbCommand.CommandText = sqlCommandString;
                //throw new UserFriendlyException(dbCommand.CommandText);
                dbCommand.CommandType = CommandType.Text;
                dbCommand.Connection = m_dbConnection;

                Dictionary<string, Tuple<string, DbType>> paramNameValueMap = m_sqlQueryBuilder.paramNameValueMap;

                foreach (KeyValuePair<string, Tuple<string, DbType>> paramNameValue in paramNameValueMap)
                {
                    DbParameter param = dbCommand.CreateParameter();
                    param.DbType = paramNameValue.Value.Item2;
                    param.ParameterName = paramNameValue.Key;
                    param.Value = paramNameValue.Value.Item1;
                    dbCommand.Parameters.Add(param);
                }

                using (DbDataReader reader = dbCommand.ExecuteReader())
                {
                    IECClass ecClass = m_query.SearchClasses.First().Class;

                    IEnumerable<IECProperty> propertyList;
                    if (m_query.SelectClause.SelectAllProperties)
                    {
                        propertyList = ecClass;
                    }
                    else
                    {
                        propertyList = m_query.SelectClause.SelectedProperties;
                    }
                    while (reader.Read())
                    {
                        int i = 0;
                        IECInstance instance = ecClass.CreateInstance();
                        foreach (IECProperty prop in propertyList)
                        {
                            if (ecClass.Contains(prop.Name))
                            {
                                IECPropertyValue instancePropertyValue = instance[prop.Name];

                                var isSpatial = prop.GetCustomAttributes("DBColumn")["IsSpatial"];
                                if (isSpatial.IsNull || isSpatial.StringValue == "false")
                                {
                                    
                                    if (!reader.IsDBNull(i))
                                    {
                                        ECToSQLMap.SQLReaderToECProperty(instancePropertyValue, reader, i);
                                    }
                                }
                                else
                                {
                                    if(!ECTypeHelper.IsString(instancePropertyValue.Type))
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

                    foreach (var instance in instanceList)
                    {
                        foreach (var crit in m_query.SelectClause.SelectedRelatedInstances)
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
                            var reverseCrit = new RelatedCriterion(new QueryRelatedClassSpecifier(relationshipClass, direction, ecClass), new WhereCriteria(new ECInstanceIdExpression(instance.InstanceId)));

                            relInstQuery.WhereClause = new WhereCriteria(reverseCrit);

                            var queryHelper = new SqlQueryProvider(relInstQuery, m_dbConnection);

                            var relInstList = queryHelper.CreateInstanceList();
                            foreach (var relInst in relInstList)
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

                }
                
            }
            if (m_instanceCount && sqlCountString != null)
            {
                instanceList.Add(CreateInstanceCountInstance(sqlCountString));
            }

            return instanceList;
        }

        private static RelatedInstanceDirection ReverseRelatedInstanceDirection(RelatedInstanceDirection direction)
        {
            if (direction == RelatedInstanceDirection.Backward)
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
        private IECInstance CreateInstanceCountInstance(string sqlCountString)
        {
            IECInstance instanceCountInstance = StandardClassesHelper.InstanceCountClass.CreateInstance();
            instanceCountInstance.InstanceId = "InstanceCount";

            using (DbCommand dbCommand = m_dbConnection.CreateCommand())
            {
                dbCommand.CommandText = sqlCountString;
                dbCommand.CommandType = CommandType.Text;
                dbCommand.Connection = m_dbConnection;

                using (DbDataReader reader = dbCommand.ExecuteReader())
                {
                    reader.Read();
                    instanceCountInstance["Count"].IntValue = reader.GetInt32(0);
                }
            }

            m_query.SearchClasses.Add(new SearchClass(StandardClassesHelper.InstanceCountClass));

            return instanceCountInstance;
        }




        private void createSqlCommandStringFromQuery(out string sqlCommandString, out string sqlCountString)
        {

            //For now, we only query on one class. To be completed one day
            SearchClass searchClass = m_query.SearchClasses.First();
            IECClass queriedClass = searchClass.Class;

            //We can only query SQLEntities. We verify that the custom attribute is set correctly.
            if(queriedClass.GetCustomAttributes("SQLEntity") == null)
            {
                throw new UserFriendlyException(String.Format("It is not permitted to query instances of the {0} class", queriedClass.Name));
            }

            TableDescriptor tableToJoin = extractFromSelectAndJoin(m_query.SelectClause, queriedClass);

            extractOrderByClause(queriedClass, tableToJoin);

            if (m_query.WhereClause.Count > 0)
            {
                extractWhereClause(m_query.WhereClause, queriedClass, tableToJoin);
            }

            if(m_polygonDescriptor != null)
            {
                extractSpatialWhereClause(m_query.WhereClause, queriedClass, tableToJoin);
            }


            //All extract* methods used before construct the query builder contained in m_sqlQueryBuilder.
            //This is why we call m_sqlQueryBuilder.BuildQuery after. It would be nice to refactor the code 
            //to make this more readable...
            sqlCommandString = m_sqlQueryBuilder.BuildQuery();

            if (m_instanceCount)
            {
                sqlCountString = m_sqlQueryBuilder.BuildCountQuery();
            }
            else
            {
                sqlCountString = null;
            }
            
        }

        private void extractOrderByClause(IECClass queriedClass, TableDescriptor primaryTable)
        {

            for(int i = 0; i < m_query.OrderBy.Count; i++)
            {
                m_query.OrderBy.MoveNext();
                IECProperty property = queriedClass[m_query.OrderBy.Current.PropertyAccessor];
                IECInstance dbColumn = property.GetCustomAttributes("DBColumn");
                string columnName = dbColumn["ColumnName"].StringValue;

                TableDescriptor propertyTableName = JoinInternalColumn(primaryTable, dbColumn);

                m_sqlQueryBuilder.AddOrderByClause(propertyTableName, columnName, m_query.OrderBy.Current.SortAscending);

            }
        }

        private TableDescriptor extractFromSelectAndJoin(SelectCriteria selectCriteria, IECClass queriedClass)
        {
            string primaryTableName = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue;

            TableDescriptor table = new TableDescriptor(primaryTableName, GetNewTableAlias());

            m_sqlQueryBuilder.SpecifyFromClause(table);
            
            try
            {
                //We add the instanceIdProperty to the selected properties. It is necessary to fetch this property to set the instance ID later
                string InstanceIDPropertyName = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                IECProperty InstanceIDProperty = queriedClass.FindProperty(InstanceIDPropertyName);

                if ((selectCriteria.SelectedProperties != null) && (!selectCriteria.SelectedProperties.Contains(InstanceIDProperty)))
                {
                    selectCriteria.SelectedProperties.Add(InstanceIDProperty);
                }
            }
            catch (Bentley.ECObjects.ECObjectsException.NullValue)
            {
                throw new ProgrammerException(String.Format("Error in class {0} of the ECSchema. The custom attribute InstanceIDProperty is not set", queriedClass.Name));
            }

            IEnumerable<IECProperty> propList = selectCriteria.SelectedProperties;
            if (propList == null)
            {
                propList = queriedClass;
            }

            foreach (IECProperty property in propList)
            {
                TableDescriptor tempTable1 = table;
                if (!queriedClass.Contains(property.Name))
                {
                    throw new UserFriendlyException(String.Format("The selected property {0} does not exist in class {1}", property.Name, queriedClass.Name));
                }

                if (queriedClass != property.ClassDefinition)
                {
                    tempTable1 = JoinBaseTables(queriedClass, tempTable1, property.ClassDefinition);
                }

                IECInstance dbColumn = property.GetCustomAttributes("DBColumn");
                TableDescriptor tempTable2 = JoinInternalColumn(tempTable1, dbColumn);
                string columnName = dbColumn["ColumnName"].StringValue;
                var isSpatialProperty = dbColumn["IsSpatial"];

                bool isSpatial = true;
                if (isSpatialProperty.IsNull || isSpatialProperty.StringValue == "false")
                {
                    isSpatial = false;
                }
                
                m_sqlQueryBuilder.AddSelectClause(tempTable2, columnName, isSpatial);


            }
            return table;
        }

        private TableDescriptor JoinBaseTables(IECClass queriedClass, TableDescriptor tempTable1, IECClass propertyClass)
        {
            if (queriedClass != propertyClass)
            {
                if (queriedClass.BaseClasses.Count() != 1)
                {
                    throw new ProgrammerException("IndexECPlugin only supports classes that have at most one base class.");
                }
                IECClass baseClass = queriedClass.BaseClasses[0];
                if (queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue !=
                    baseClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue)
                {
                    string baseClassKeyPropertyName;
                    string derivedClassKeyPropertyName;

                    if(queriedClass.GetCustomAttributes("DerivedClassLinker") != null)
                    {
                        baseClassKeyPropertyName = queriedClass.GetCustomAttributes("DerivedClassLinker").GetPropertyValue("BaseClassKeyProperty").StringValue;
                        derivedClassKeyPropertyName = queriedClass.GetCustomAttributes("DerivedClassLinker").GetPropertyValue("DerivedClassKeyProperty").StringValue;
                    }

                    else
                    {
                        baseClassKeyPropertyName = baseClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                        derivedClassKeyPropertyName = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                    }

                    //We have to join the two different tables
                    TableDescriptor newTable = new TableDescriptor(baseClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue, GetNewTableAlias());

                    string baseClassKeyColumn = baseClass[baseClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;
                    string derivedClassKeyColumn = queriedClass[derivedClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;

                    newTable.SetTableJoined(tempTable1, derivedClassKeyColumn, baseClassKeyColumn);

                    TableDescriptor similarTable;
                    bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(newTable, out similarTable);
                    if (!joinSuccessful)
                    {
                        //tempTable1 = similarTable;
                        return JoinBaseTables(baseClass, similarTable, propertyClass);
                    }
                    else
                    {
                        //tempTable1 = newTable;
                        return JoinBaseTables(baseClass, newTable, propertyClass);
                    }
                }

                return JoinBaseTables(baseClass, tempTable1, propertyClass);
            }

            else
            {
                return tempTable1;
            }
        }

        private TableDescriptor JoinInternalColumn(TableDescriptor firstTableDescriptor, IECInstance dbColumn)
        {
            var propertyTableAttribute = dbColumn["JoinTableName"];
            string propertyTableName = firstTableDescriptor.Name;
            TableDescriptor newPropertyTableDescriptor = firstTableDescriptor;
            if (!propertyTableAttribute.IsNull)
            {
                propertyTableName = propertyTableAttribute.StringValue;
            }

            if (propertyTableName != firstTableDescriptor.Name)
            {
                var firstTableKeyAttribute = dbColumn["FirstTableKey"];
                var newTableKeyAttribute = dbColumn["NewTableKey"];

                if (firstTableKeyAttribute.IsNull || newTableKeyAttribute.IsNull)
                {
                    throw new ProgrammerException("The ECSchema was badly written. If the JoinTableName property is not null, the FirstTableKey and NewTableKey must be specified.");
                }
                string firstTableKey = firstTableKeyAttribute.StringValue;
                string newTableKey = newTableKeyAttribute.StringValue;

                if (String.IsNullOrWhiteSpace(firstTableKey) || String.IsNullOrWhiteSpace(newTableKey))
                {
                    throw new ProgrammerException("The ECSchema was badly written. If the JoinTableName property is not null, the FirstTableKey and NewTableKey must be specified.");
                }

                newPropertyTableDescriptor = new TableDescriptor(propertyTableName, GetNewTableAlias());
                newPropertyTableDescriptor.SetTableJoined(firstTableDescriptor, firstTableKey, newTableKey);

                TableDescriptor similarTable;
                bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(newPropertyTableDescriptor, out similarTable);
                if (!joinSuccessful)
                {
                    newPropertyTableDescriptor = similarTable;
                }
            }
            return newPropertyTableDescriptor;
        }

        private void extractWhereClause(WhereCriteria whereCriteria, IECClass queriedClass, TableDescriptor table/*, bool forRelatedCriterion = false*/)
        {
            m_sqlQueryBuilder.StartOfInnerWhereClause();
            for (int i = 0; i < whereCriteria.Count; i++)
            {
                if (i > 0)
                {
                    m_sqlQueryBuilder.AddOperatorToWhereClause(whereCriteria.GetLogicalOperatorBefore(i));
                }
                WhereCriterion criterion = whereCriteria[i];

                if (criterion is PropertyExpression)
                {
                    PropertyExpression propertyExpression = (PropertyExpression)criterion;

                    string primaryTableName = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue;

                    IECProperty property = propertyExpression.LeftSideProperty;
                    IECInstance dbColumn = property.GetCustomAttributes("DBColumn");

                    TableDescriptor propertyTable = JoinInternalColumn(table, dbColumn);

                    string columnName = dbColumn.GetPropertyValue("ColumnName").StringValue;

                    //******KEEP THESE COMMENTED LINES******************************************************************************
                    //var isSpatial = dbColumn["IsSpatial"];
                    //if (isSpatial.IsNull || isSpatial.StringValue == "false")
                    //{
                        m_sqlQueryBuilder.AddWhereClause(propertyTable.Alias, columnName, propertyExpression.Operator, propertyExpression.RightSideString, ECToSQLMap.ECTypeToSQL(property.Type));
                    //}
                    //else
                    //{
                    //    //Spatial query special case
                    //    if (propertyExpression.Operator != RelationalOperator.IN)
                    //    {
                    //        throw new UserFriendlyException("Please use the operator \"IN\" for the \"intersects\" operation on two geometries.");
                    //    }

                    //    PolygonModel model;
                    //    try
                    //    {
                    //        model = JsonConvert.DeserializeObject<PolygonModel>(propertyExpression.RightSideString);
                    //    }
                    //    catch (JsonSerializationException)
                    //    {
                    //        throw new UserFriendlyException("The polygon format is not valid.");
                    //    }

                    //    int polygonSRID;
                    //    if(!int.TryParse(model.coordinate_system, out polygonSRID))
                    //    {
                    //        throw new UserFriendlyException("The polygon format is not valid.");
                    //    }

                    //    string polygonWKT = DbGeometryHelpers.CreateWktPolygonString(model.points);

                    //    //m_polygonDescriptor = new PolygonDescriptor
                    //    //{
                    //    //    WKT = polygonWKT,
                    //    //    SRID = polygonSRID
                    //    //};


                    //    m_sqlQueryBuilder.AddSpatialIntersectsWhereClause(propertyTable.Alias, columnName, polygonWKT, polygonSRID);
                    //}
                    //************************************************************************************************************
                    
                }
                else if (criterion is ECInstanceIdExpression) // TODO also add support for ECKeyExpression
                {

                    ECInstanceIdExpression instanceIDExpression = (ECInstanceIdExpression)criterion;

                    if (instanceIDExpression.InstanceIdSet.Count() == 0)
                    {
                        throw new ProgrammerException("The array of IDs in the ECInstanceIdExpression is empty.");
                    }

                    string idProperty = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                    IECProperty prop = queriedClass[idProperty];

                    string dbColumnName = prop.GetCustomAttributes("DBColumn")["ColumnName"].StringValue;
                    IECType ecType = prop.Type;

                    m_sqlQueryBuilder.StartOfInnerWhereClause();

                    for (int j = 0; j < instanceIDExpression.InstanceIdSet.Count(); j++)
                    {
                        if (j > 0)
                        {
                            m_sqlQueryBuilder.AddOperatorToWhereClause(LogicalOperator.OR);
                        }
                        m_sqlQueryBuilder.AddWhereClause(table.Alias, dbColumnName, RelationalOperator.EQ, instanceIDExpression.InstanceIdSet[j], ECToSQLMap.ECTypeToSQL(ecType));

                    }
                        m_sqlQueryBuilder.EndOfInnerWhereClause();


                    if (m_sqlQueryBuilder.OrderByListIsEmpty())
                    {
                        //This is because we need an order by clause to make the query work. WSG makes a paged request
                        //when creating an ECInstanceIdExpression and SQL needs an OrderBy to complete it.
                        m_sqlQueryBuilder.AddOrderByClause(table, dbColumnName, true);
                    }

                }
                else if (criterion is RelatedCriterion)
                {

                    var relatedCrit = criterion as RelatedCriterion;

                    var classSpecifier = relatedCrit.RelatedClassSpecifier;

                    IECRelationshipClass relationshipClass = classSpecifier.RelationshipClass;

                    IECInstance relationshipKeys = relationshipClass.GetCustomAttributes("RelationshipKeys");
                    
                    IECClass relatedClass = classSpecifier.RelatedClass;
                    
                    if (relationshipKeys != null)
                    {
                        //string containerKey = relationshipKeys["ContainerKey"].StringValue;
                        //string containedKey = relationshipKeys["ContainedKey"].StringValue;

                        string dbColumnOfQueriedClass;
                        string dbColumnOfRelatedClass;

                        if (classSpecifier.RelatedDirection == RelatedInstanceDirection.Forward)
                        {
                            dbColumnOfQueriedClass = relationshipKeys["ContainerKey"].StringValue;
                            dbColumnOfRelatedClass = relationshipKeys["ContainedKey"].StringValue;
                        }
                        else
                        {
                            dbColumnOfQueriedClass = relationshipKeys["ContainedKey"].StringValue;
                            dbColumnOfRelatedClass = relationshipKeys["ContainerKey"].StringValue;
                        }
                        string relatedTableName = relatedClass.GetCustomAttributes("SQLEntity")["FromTableName"].StringValue;

                        TableDescriptor relatedTableDescriptor = new TableDescriptor(relatedTableName, GetNewTableAlias());

                        relatedTableDescriptor.SetTableJoined(table, dbColumnOfQueriedClass, dbColumnOfRelatedClass);

                        TableDescriptor similarTable;

                        bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(relatedTableDescriptor, out similarTable);
                        if (!joinSuccessful)
                        {
                            relatedTableDescriptor = similarTable;
                        }
   
                        extractWhereClause(relatedCrit.RelatedWhereCriteria, relatedClass, relatedTableDescriptor);
                    }
                    else
                    {
                        //This remains to be tested

                        relationshipKeys = relationshipClass.GetCustomAttributes("ManyToManyRelationshipKeys");

                        string dbColumnOfQueriedClass;
                        string dbColumnIntermediateQueried;
                        string dbColumnIntermediateRelated;
                        string dbColumnOfRelatedClass;

                        if (classSpecifier.RelatedDirection == RelatedInstanceDirection.Forward)
                        {
                            //string queriedClassKey = relationshipKeys["ContainerKey"].StringValue;
                            dbColumnOfQueriedClass = relationshipKeys["ContainerKey"].StringValue;
                            
                            dbColumnIntermediateQueried = relationshipKeys["IntermediateContainerKey"].StringValue;

                            //string relatedClassKey = relationshipKeys["ContainedKey"].StringValue;
                            dbColumnOfRelatedClass = relationshipKeys["ContainedKey"].StringValue;

                            dbColumnIntermediateRelated = relationshipKeys["IntermediateContainedKey"].StringValue;
                        }
                        else
                        {
                            //string queriedClassKey = relationshipKeys["ContainedKey"].StringValue;
                            dbColumnOfQueriedClass = relationshipKeys["ContainedKey"].StringValue;
                            
                            dbColumnIntermediateQueried = relationshipKeys["IntermediateContainedKey"].StringValue;
                            
                            //string relatedClassKey = relationshipKeys["ContainerKey"].StringValue;
                            dbColumnOfRelatedClass = relationshipKeys["ContainerKey"].StringValue;

                            dbColumnIntermediateRelated = relationshipKeys["IntermediateContainerKey"].StringValue;
                        }
                        
                        
                        string intermediateTableName = relationshipKeys["IntermediateTableName"].StringValue;
                        string relatedTableName = relatedClass.GetCustomAttributes("SQLEntity")["FromTableName"].StringValue;

                        TableDescriptor intermediateTableDescriptor = new TableDescriptor(intermediateTableName, GetNewTableAlias());
                        intermediateTableDescriptor.SetTableJoined(table, dbColumnOfQueriedClass, dbColumnIntermediateQueried);

                        TableDescriptor similarTable;

                        bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(intermediateTableDescriptor, out similarTable);
                        if (!joinSuccessful)
                        {
                            intermediateTableDescriptor = similarTable;
                        }

                        TableDescriptor relatedTableDescriptor = new TableDescriptor(relatedTableName, GetNewTableAlias());
                        relatedTableDescriptor.SetTableJoined(intermediateTableDescriptor, dbColumnIntermediateRelated, dbColumnOfRelatedClass);

                        joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(relatedTableDescriptor, out similarTable);
                        if (!joinSuccessful)
                        {
                            relatedTableDescriptor = similarTable;
                        }

                        extractWhereClause(relatedCrit.RelatedWhereCriteria, relatedClass, relatedTableDescriptor);

                    }

                }
                else if (criterion is WhereCriteria)
                {

                    var whereCrit = criterion as WhereCriteria;

                    extractWhereClause(whereCrit, queriedClass, table/*, forRelatedCriterion*/);

                }
            }
            m_sqlQueryBuilder.EndOfInnerWhereClause();
        }

        private void extractSpatialWhereClause(WhereCriteria whereCriteria, IECClass queriedClass, TableDescriptor tableToJoin)
        {
            if (whereCriteria.Count > 0)
            {
                m_sqlQueryBuilder.AddOperatorToWhereClause(LogicalOperator.AND);
            }

            TableDescriptor propertyTable = tableToJoin;
            string columnName = null;
            foreach (var prop in queriedClass)
            {
                IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");
                var isSpatialProperty = dbColumn["IsSpatial"];
                if (!isSpatialProperty.IsNull && isSpatialProperty.StringValue.ToLower() == "true")
                {
                    columnName = dbColumn.GetPropertyValue("ColumnName").StringValue;
                    propertyTable = JoinInternalColumn(tableToJoin, dbColumn);
                    break;
                }
            }
            if(String.IsNullOrWhiteSpace(columnName))
            {
                throw new UserFriendlyException("The class for which you requested a spatial query does not have any spatial footprint.");
            }


            m_sqlQueryBuilder.AddSpatialIntersectsWhereClause(propertyTable.Alias, columnName, m_polygonDescriptor.WKT, m_polygonDescriptor.SRID);

        }


    }
}
