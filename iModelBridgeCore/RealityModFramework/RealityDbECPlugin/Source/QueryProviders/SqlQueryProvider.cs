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
        private int m_tabNumber = 0;

        private bool m_instanceCount = false;
        private bool m_streamColumnQueried = false;

        private DbConnection m_dbConnection;

        private PolygonDescriptor m_polygonDescriptor;

        private SchemaHelper m_schemaHelper;

        public SqlQueryProvider(ECQuery query, ECQuerySettings querySettings, DbConnection dbConnection, SchemaHelper schemaHelper)
        {
            if (query == null)
            {
                throw new ArgumentNullException("query");
            }

            if (dbConnection == null)
            {
                throw new ArgumentNullException("dbConnection");
            }

            if (schemaHelper == null)
            {
                throw new ArgumentNullException("schemaHelper");
            }

            m_query = query;
            m_querySettings = querySettings;
            m_dbConnection = dbConnection;
            m_schemaHelper = schemaHelper;
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
                    Log.Logger.error("Query aborted : The polygon format is not valid.");
                    throw new UserFriendlyException("The polygon format is not valid.");
                }

                int polygonSRID;
                if (!int.TryParse(model.coordinate_system, out polygonSRID))
                {
                    Log.Logger.error("Query aborted : The polygon format is not valid.");
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

            string sqlCommandString;
            string sqlCountString;
            createSqlCommandStringFromQuery(out sqlCommandString, out sqlCountString);

            m_dbConnection.Open();

            using (DbCommand dbCommand = m_dbConnection.CreateCommand())
            {

                dbCommand.CommandText = sqlCommandString;
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
                        if (m_streamColumnQueried)
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

                            Byte[] byteArray = (byte[])reader[i];

                            MemoryStream mStream = new MemoryStream(byteArray);

                            StreamBackedDescriptor desc = new StreamBackedDescriptor(mStream, "Thumbnail", mStream.Length, DateTime.Now);
                            StreamBackedDescriptorAccessor.SetIn(instance, desc);

                            i++;

                        }
                        foreach (IECProperty prop in propertyList)
                        {
                            if (ecClass.Contains(prop.Name))
                            {
                                IECPropertyValue instancePropertyValue = instance[prop.Name];
                                IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");

                                if(dbColumn == null)
                                {
                                    //This is not a column in the sql table. We skip it...
                                    continue;
                                }

                                var isSpatial = dbColumn["IsSpatial"];
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
                                        Log.Logger.error(String.Format("The property {0} tagged as spatial must be declared as a string in the ECSchema.", prop.Name));
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
                            //var reverseCrit = new RelatedCriterion(new QueryRelatedClassSpecifier(relationshipClass, direction, relatedCriterionClass), new WhereCriteria(new ECInstanceIdExpression(instance.InstanceId)));
                            var reverseCrit = new RelatedCriterion(new QueryRelatedClassSpecifier(relationshipClass, direction, ecClass), new WhereCriteria(new ECInstanceIdExpression(instance.InstanceId)));

                            relInstQuery.WhereClause = new WhereCriteria(reverseCrit);

                            var queryHelper = new SqlQueryProvider(relInstQuery, new ECQuerySettings(), m_dbConnection, m_schemaHelper);

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
                Log.Logger.error(String.Format("Query aborted: It is not permitted to query instances of the {0} class", queriedClass.Name));
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

                if(dbColumn == null)
                {
                    //This is not a column in the sql table. We skip it...
                    continue;
                }

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
                Log.Logger.error(String.Format("Query aborted: Error in class {0} of the ECSchema. The custom attribute InstanceIDProperty is not set", queriedClass.Name));
                throw new ProgrammerException(String.Format("Error in class {0} of the ECSchema. The custom attribute InstanceIDProperty is not set", queriedClass.Name));
            }

            IEnumerable<IECProperty> propList = selectCriteria.SelectedProperties;
            if (propList == null)
            {
                propList = queriedClass;
            }

            if((m_querySettings != null) && ((m_querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None))
            {
                m_streamColumnQueried = true;
                IECInstance fileHolderAttribute = queriedClass.GetCustomAttributes("FileHolder");
                if ((fileHolderAttribute != null) && (fileHolderAttribute["Type"].StringValue == "SQLThumbnail"))
                {
                    string streamableColumnName = fileHolderAttribute["LocationHoldingColumn"].StringValue;
                    m_sqlQueryBuilder.AddSelectClause(table, streamableColumnName, false);
                }
            }

            foreach (IECProperty property in propList)
            {
                TableDescriptor tempTable1 = table;
                if (!queriedClass.Contains(property.Name))
                {
                    Log.Logger.error(String.Format("Query aborted: The selected property {0} does not exist in class {1}", property.Name, queriedClass.Name));
                    throw new UserFriendlyException(String.Format("The selected property {0} does not exist in class {1}", property.Name, queriedClass.Name));
                }

                IECInstance dbColumn = property.GetCustomAttributes("DBColumn");

                if (dbColumn == null)
                {
                    //This is not a column in the sql table. We skip it...
                    continue;
                }

                if (queriedClass != property.ClassDefinition)
                {
                    tempTable1 = JoinBaseTables(queriedClass, tempTable1, property.ClassDefinition);
                }

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

        /// <summary>
        /// This method's purpose is to join recursively all tables in the hierarchy of a class, up to a specified base class
        /// </summary>
        /// <param name="queriedClass">The derived class of which we need the base class table joined</param>
        /// <param name="tempTable1">The table descriptor of the queried class</param>
        /// <param name="finalBaseClass">The specified final base class to have its table joined</param>
        /// <returns>The final class' table descriptor</returns>
        private TableDescriptor JoinBaseTables(IECClass queriedClass, TableDescriptor tempTable1, IECClass finalBaseClass)
        {
            if (queriedClass != finalBaseClass)
            {
                if (queriedClass.BaseClasses.Count() != 1)
                {
                    Log.Logger.error("Query aborted: IndexECPlugin only supports classes that have at most one base class.");
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
                        return JoinBaseTables(baseClass, similarTable, finalBaseClass);
                    }
                    else
                    {
                        //tempTable1 = newTable;
                        return JoinBaseTables(baseClass, newTable, finalBaseClass);
                    }
                }

                return JoinBaseTables(baseClass, tempTable1, finalBaseClass);
            }

            return tempTable1;
            
        }

        /// <summary>
        /// This method's purpose is to join recursively all tables in the hierarchy of a class, down to a specified derived class
        /// </summary>
        /// <param name="queriedClass">The base class of which we need the derived class table joined</param>
        /// <param name="tempTable1">The table descriptor of the queried class</param>
        /// <param name="finalDerivedClass">The specified derived base class to have its table joined</param>
        /// <returns>The final class' table descriptor</returns>
        private TableDescriptor JoinDerivedTables(IECClass queriedClass, TableDescriptor tempTable1, IECClass finalDerivedClass)
        {
            if (queriedClass != finalDerivedClass)
            {

                //if (queriedClass.DerivedClasses.Count() != 1)
                //{
                //    throw new ProgrammerException("IndexECPlugin only supports classes that have at most one derived class.");
                //}
                IEnumerable<IECClass> derivedClasses = queriedClass.DerivedClasses.Where(x => firstIsDerivedFromSecond(finalDerivedClass, x));

                IECClass derivedClass = derivedClasses.First();

                if (queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue !=
                    derivedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue)
                {
                    string baseClassKeyPropertyName;
                    string derivedClassKeyPropertyName;

                    if (queriedClass.GetCustomAttributes("DerivedClassLinker") != null)
                    {
                        baseClassKeyPropertyName = derivedClass.GetCustomAttributes("DerivedClassLinker").GetPropertyValue("BaseClassKeyProperty").StringValue;
                        derivedClassKeyPropertyName = derivedClass.GetCustomAttributes("DerivedClassLinker").GetPropertyValue("DerivedClassKeyProperty").StringValue;
                    }

                    else
                    {
                        baseClassKeyPropertyName = derivedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                        derivedClassKeyPropertyName = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                    }

                    //We have to join the two different tables
                    TableDescriptor newTable = new TableDescriptor(derivedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue, GetNewTableAlias());

                    string baseClassKeyColumn = queriedClass[baseClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;
                    string derivedClassKeyColumn = derivedClass[derivedClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;

                    newTable.SetTableJoined(tempTable1, baseClassKeyColumn, derivedClassKeyColumn);

                    TableDescriptor similarTable;
                    bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(newTable, out similarTable);
                    if (!joinSuccessful)
                    {
                        //tempTable1 = similarTable;
                        return JoinDerivedTables(derivedClass, similarTable, finalDerivedClass);
                    }
                    else
                    {
                        //tempTable1 = newTable;
                        return JoinDerivedTables(derivedClass, newTable, finalDerivedClass);
                    }
                }

                return JoinDerivedTables(derivedClass, tempTable1, finalDerivedClass);
            }

            return tempTable1;
            
        }

        //This method is to be used solely in the JoinDerivedTables method as an utilitary method...
        private bool firstIsDerivedFromSecond(IECClass firstClass, IECClass secondClass)
        {
            //If firstClass is the second class, we consider
            if (firstClass.Name == secondClass.Name)
            {
                return true;
            }

            //Is the classes are not the same and there is no base class to the first, the we are 
            if (firstClass.BaseClasses == null)
            {
                return false;
            }

            int baseClassCount = firstClass.BaseClasses.Count();
            if (baseClassCount > 1)
            {
                Log.Logger.error("Query aborted: IndexECPlugin only supports classes that have at most one derived class.");
                throw new ProgrammerException("IndexECPlugin only supports classes that have at most one derived class.");
            }


            if(firstClass.BaseClasses.First().Name == secondClass.Name)
            {
                return true;
            }
            else
            {
                return firstIsDerivedFromSecond(firstClass.BaseClasses.First(), secondClass);
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
                    Log.Logger.error("Query aborted: The ECSchema was badly written. If the JoinTableName property is not null, the FirstTableKey and NewTableKey must be specified.");
                    throw new ProgrammerException("The ECSchema was badly written. If the JoinTableName property is not null, the FirstTableKey and NewTableKey must be specified.");
                }
                string firstTableKey = firstTableKeyAttribute.StringValue;
                string newTableKey = newTableKeyAttribute.StringValue;

                if (String.IsNullOrWhiteSpace(firstTableKey) || String.IsNullOrWhiteSpace(newTableKey))
                {
                    Log.Logger.error("Query aborted: The ECSchema was badly written. If the JoinTableName property is not null, the FirstTableKey and NewTableKey must be specified.");
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
                    if (dbColumn == null)
                    {
                        //This is not a column in the sql table. We skip it...
                        continue;
                    }

                    TableDescriptor tempTable1 = table;

                    if (queriedClass != property.ClassDefinition)
                    {
                        tempTable1 = JoinBaseTables(queriedClass, tempTable1, property.ClassDefinition);
                    }

                    TableDescriptor propertyTable = JoinInternalColumn(tempTable1, dbColumn);

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
                        Log.Logger.error("Query aborted: The array of IDs in the ECInstanceIdExpression is empty.");
                        throw new ProgrammerException("The array of IDs in the ECInstanceIdExpression is empty.");
                    }

                    string idProperty = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                    IECProperty prop = queriedClass[idProperty];
                    IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");
                    if (dbColumn == null)
                    {
                        //This is not a column in the sql table. We skip it...
                        continue;
                    }
                    string dbColumnName = dbColumn["ColumnName"].StringValue;

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

                        string dbColumnOfBaseQueriedClass;
                        string dbColumnOfBaseRelatedClass;

                        string baseQueriedClassName;
                        string baseRelatedClassName;

                        IECClass baseQueriedClass;
                        IECClass baseRelatedClass;

                        

                        if (classSpecifier.RelatedDirection == RelatedInstanceDirection.Forward)
                        {
                            baseQueriedClassName = relationshipKeys["ContainerClassName"].StringValue;
                            dbColumnOfBaseQueriedClass = relationshipKeys["ContainerKey"].StringValue;

                            baseRelatedClassName = relationshipKeys["ContainedClassName"].StringValue;
                            dbColumnOfBaseRelatedClass = relationshipKeys["ContainedKey"].StringValue;
                        }
                        else
                        {
                            baseQueriedClassName = relationshipKeys["ContainedClassName"].StringValue;
                            dbColumnOfBaseQueriedClass = relationshipKeys["ContainedKey"].StringValue;

                            baseRelatedClassName = relationshipKeys["ContainerClassName"].StringValue;
                            dbColumnOfBaseRelatedClass = relationshipKeys["ContainerKey"].StringValue;
                        }

                        baseQueriedClass = m_schemaHelper.GetClass(baseQueriedClassName);
                        baseRelatedClass = m_schemaHelper.GetClass(baseRelatedClassName);

                        //We go up the hierarchy of the base queried class
                        TableDescriptor baseQueriedTableDescriptor = table;

                        baseQueriedTableDescriptor = JoinBaseTables(queriedClass, baseQueriedTableDescriptor, baseQueriedClass);

                        //We link both base classes
                        string relatedBaseTableName = baseRelatedClass.GetCustomAttributes("SQLEntity")["FromTableName"].StringValue;
                        TableDescriptor relatedBaseTableDescriptor = new TableDescriptor(relatedBaseTableName, GetNewTableAlias());

                        relatedBaseTableDescriptor.SetTableJoined(baseQueriedTableDescriptor, dbColumnOfBaseQueriedClass, dbColumnOfBaseRelatedClass);

                        TableDescriptor similarTable;

                        bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(relatedBaseTableDescriptor, out similarTable);
                        if (!joinSuccessful)
                        {
                            relatedBaseTableDescriptor = similarTable;
                        }

                        //We go up in the hierarchy of the table
                        TableDescriptor finalTableDescriptor = JoinDerivedTables(baseRelatedClass, relatedBaseTableDescriptor, relatedClass);

                        //string relatedTableName = relatedClass.GetCustomAttributes("SQLEntity")["FromTableName"].StringValue;

                        //TableDescriptor relatedTableDescriptor = new TableDescriptor(relatedTableName, GetNewTableAlias());

                        //relatedTableDescriptor.SetTableJoined(table, dbColumnOfQueriedClass, dbColumnOfRelatedClass);

                        //TableDescriptor similarTable;

                        //bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(relatedTableDescriptor, out similarTable);
                        //if (!joinSuccessful)
                        //{
                        //    relatedTableDescriptor = similarTable;
                        //}

                        extractWhereClause(relatedCrit.RelatedWhereCriteria, relatedClass, finalTableDescriptor);
                    }
                    else
                    {
                        relationshipKeys = relationshipClass.GetCustomAttributes("ManyToManyRelationshipKeys");

                        string dbColumnOfBaseQueriedClass;
                        string dbColumnIntermediateQueried;
                        string dbColumnIntermediateRelated;
                        string dbColumnOfBaseRelatedClass;

                        string baseQueriedClassName;
                        string baseRelatedClassName;

                        IECClass baseQueriedClass;
                        IECClass baseRelatedClass;


                        if (classSpecifier.RelatedDirection == RelatedInstanceDirection.Forward)
                        {
                            baseQueriedClassName = relationshipKeys["ContainerClassName"].StringValue;
                            dbColumnOfBaseQueriedClass = relationshipKeys["ContainerKey"].StringValue;
                            
                            dbColumnIntermediateQueried = relationshipKeys["IntermediateContainerKey"].StringValue;

                            baseRelatedClassName = relationshipKeys["ContainedClassName"].StringValue;
                            dbColumnOfBaseRelatedClass = relationshipKeys["ContainedKey"].StringValue;

                            dbColumnIntermediateRelated = relationshipKeys["IntermediateContainedKey"].StringValue;
                        }
                        else
                        {
                            baseQueriedClassName = relationshipKeys["ContainedClassName"].StringValue;
                            dbColumnOfBaseQueriedClass = relationshipKeys["ContainedKey"].StringValue;
                            
                            dbColumnIntermediateQueried = relationshipKeys["IntermediateContainedKey"].StringValue;

                            baseRelatedClassName = relationshipKeys["ContainerClassName"].StringValue; 
                            dbColumnOfBaseRelatedClass = relationshipKeys["ContainerKey"].StringValue;

                            dbColumnIntermediateRelated = relationshipKeys["IntermediateContainerKey"].StringValue;
                        }

                        baseQueriedClass = m_schemaHelper.GetClass(baseQueriedClassName);
                        baseRelatedClass = m_schemaHelper.GetClass(baseRelatedClassName);

                        string intermediateTableName = relationshipKeys["IntermediateTableName"].StringValue;
                        string relatedBaseTableName = baseRelatedClass.GetCustomAttributes("SQLEntity")["FromTableName"].StringValue;

                        //We go up the hierarchy of the base queried class
                        TableDescriptor baseQueriedTableDescriptor = table;

                        baseQueriedTableDescriptor = JoinBaseTables(queriedClass, baseQueriedTableDescriptor, baseQueriedClass);

                        TableDescriptor intermediateTableDescriptor = new TableDescriptor(intermediateTableName, GetNewTableAlias());
                        intermediateTableDescriptor.SetTableJoined(baseQueriedTableDescriptor, dbColumnOfBaseQueriedClass, dbColumnIntermediateQueried);

                        TableDescriptor similarTable;

                        bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(intermediateTableDescriptor, out similarTable);
                        if (!joinSuccessful)
                        {
                            intermediateTableDescriptor = similarTable;
                        }

                        TableDescriptor relatedBaseTableDescriptor = new TableDescriptor(relatedBaseTableName, GetNewTableAlias());
                        relatedBaseTableDescriptor.SetTableJoined(intermediateTableDescriptor, dbColumnIntermediateRelated, dbColumnOfBaseRelatedClass);

                        joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(relatedBaseTableDescriptor, out similarTable);
                        if (!joinSuccessful)
                        {
                            relatedBaseTableDescriptor = similarTable;
                        }

                        TableDescriptor finalTableDescriptor = JoinDerivedTables(baseRelatedClass, relatedBaseTableDescriptor, relatedClass);

                        extractWhereClause(relatedCrit.RelatedWhereCriteria, relatedClass, finalTableDescriptor);

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
                if (dbColumn == null)
                {
                    //This is not a column in the sql table. We skip it...
                    continue;
                }
                var isSpatialProperty = dbColumn["IsSpatial"];
                if (!isSpatialProperty.IsNull && isSpatialProperty.StringValue.ToLower() == "true")
                {
                    columnName = dbColumn.GetPropertyValue("ColumnName").StringValue;

                    if (queriedClass != prop.ClassDefinition)
                    {
                        propertyTable = JoinBaseTables(queriedClass, propertyTable, prop.ClassDefinition);
                    }

                    propertyTable = JoinInternalColumn(propertyTable, dbColumn);
                    break;
                }
            }
            if(String.IsNullOrWhiteSpace(columnName))
            {
                Log.Logger.error("Query aborted: The class for which the user requested a spatial query does not have any spatial property.");
                throw new UserFriendlyException("The class for which you requested a spatial query does not have any spatial property.");
            }

            m_sqlQueryBuilder.AddSpatialIntersectsWhereClause(propertyTable.Alias, columnName, m_polygonDescriptor.WKT, m_polygonDescriptor.SRID);

        }


    }
}
