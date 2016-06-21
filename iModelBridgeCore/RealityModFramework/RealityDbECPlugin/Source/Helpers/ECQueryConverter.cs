/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/ECQueryConverter.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Converts an ECQuery to a SQL query
    /// </summary>
    public class ECQueryConverter
        {
        ECQuery m_query;
        ECQuerySettings m_querySettings;
        SQLQueryBuilder m_sqlQueryBuilder;
        PolygonDescriptor m_polygonDescriptor;
        IECSchema m_schema;
        bool m_instanceCount;

        //int m_tabNumber = 0;
        TableAliasCreator m_tac = new TableAliasCreator();

        /// <summary>
        /// 
        /// </summary>
        /// <param name="query">The ECQuery</param>
        /// <param name="querySettings">The ECQuerySettings</param>
        /// <param name="sqlQueryBuilder">The SqlQueryBuilder used to generate the wanted type of SQL query</param>
        /// <param name="polygonDescriptor">The polygon used for spatial queries</param>
        /// <param name="schema">The RealityModeling schema</param>
        /// <param name="instanceCount">Indicates if an instance count is requested</param>
        public ECQueryConverter (ECQuery query, ECQuerySettings querySettings, SQLQueryBuilder sqlQueryBuilder, PolygonDescriptor polygonDescriptor, IECSchema schema, bool instanceCount)
            {
            m_query = query;
            m_querySettings = querySettings;
            m_sqlQueryBuilder = sqlQueryBuilder;
            m_polygonDescriptor = polygonDescriptor;
            m_schema = schema;
            m_instanceCount = instanceCount;
            }

        /// <summary>
        /// Creates the requested SQL query command string and SQL count query command string
        /// </summary>
        /// <param name="sqlCommandString">The requested SQL query command string</param>
        /// <param name="sqlCountString">The SQL count query command string if requested</param>
        /// <param name="dataReadingHelper">The helper for reading the data from the DataReader</param>
        /// <param name="paramNameValueMap">The mapping of the parameters names and values for the parameterized query</param>
        public void CreateSqlCommandStringFromQuery (out string sqlCommandString, out string sqlCountString, out DataReadingHelper dataReadingHelper, 
                                                     out IParamNameValueMap paramNameValueMap)
            {
            //For now, we only query on one class. To be completed one day
            SearchClass searchClass = m_query.SearchClasses.First();
            IECClass queriedClass = searchClass.Class;

            //We can only query SQLEntities. We verify that the custom attribute is set correctly.
            if ( queriedClass.GetCustomAttributes("SQLEntity") == null )
                {
                throw new UserFriendlyException(String.Format("It is not permitted to query instances of the {0} class", queriedClass.Name));
                }

            TableDescriptor tableToJoin = extractFromSelectAndJoin(m_query.SelectClause, queriedClass);

            extractOrderByClause(queriedClass, tableToJoin);

            if ( m_query.WhereClause.Count > 0 )
                {
                extractWhereClause(m_query.WhereClause, queriedClass, tableToJoin);
                }

            if ( m_polygonDescriptor != null )
                {
                extractSpatialWhereClause(m_query.WhereClause, queriedClass, tableToJoin);
                }


            //All extract* methods used before construct the query builder contained in m_sqlQueryBuilder.
            //This is why we call m_sqlQueryBuilder.BuildQuery after. It would be nice to refactor the code 
            //to make this more readable...
            sqlCommandString = m_sqlQueryBuilder.BuildQuery(out dataReadingHelper);

            if ( m_instanceCount )
                {
                sqlCountString = m_sqlQueryBuilder.BuildCountQuery();
                }
            else
                {
                sqlCountString = null;
                }
            paramNameValueMap = m_sqlQueryBuilder.paramNameValueMap;
            }

        private TableDescriptor extractFromSelectAndJoin (SelectCriteria selectCriteria, IECClass queriedClass)
            {
            string primaryTableName = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue;

            TableDescriptor table = new TableDescriptor(primaryTableName, m_tac.GetNewTableAlias());

            m_sqlQueryBuilder.SpecifyFromClause(table);

            try
                {
                //We add the instanceIdProperty to the selected properties. It is necessary to fetch this property to set the instance ID later
                string InstanceIDPropertyName = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                IECProperty InstanceIDProperty = queriedClass.FindProperty(InstanceIDPropertyName);

                if ( (selectCriteria.SelectedProperties != null) && (!selectCriteria.SelectedProperties.Contains(InstanceIDProperty)) )
                    {
                    selectCriteria.SelectedProperties.Add(InstanceIDProperty);
                    }
                }
            catch ( Bentley.ECObjects.ECObjectsException.NullValue )
                {
                throw new ProgrammerException(String.Format("Error in class {0} of the ECSchema. The custom attribute InstanceIDProperty is not set", queriedClass.Name));
                }

            IEnumerable<IECProperty> propList = selectCriteria.SelectedProperties;
            if ( propList == null )
                {
                propList = queriedClass;
                }

            //Special case for thumbnails and file 
            if ( (m_querySettings != null) && ((m_querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None) && queriedClass.Name == "Thumbnail" )
                {
                IECInstance fileHolderAttribute = queriedClass.GetCustomAttributes("FileHolder");
                if ( (fileHolderAttribute != null) && (fileHolderAttribute["Type"].StringValue == "SQLThumbnail") )
                    {
                    string streamableColumnName = fileHolderAttribute["LocationHoldingColumn"].StringValue;
                    m_sqlQueryBuilder.AddSelectClause(table, streamableColumnName, ColumnCategory.streamData, null);
                    }
                }

            foreach ( IECProperty property in propList )
                {
                if ( !queriedClass.Contains(property.Name) )
                    {
                    throw new UserFriendlyException(String.Format("The selected property {0} does not exist in class {1}", property.Name, queriedClass.Name));
                    }

                IECInstance dbColumn = property.GetCustomAttributes("DBColumn");

                if ( dbColumn == null )
                    {
                    //This is not a column in the sql table. We skip it...
                    continue;
                    }
                TableDescriptor tempTable1 = table;
                if ( queriedClass != property.ClassDefinition )
                    {
                    tempTable1 = SqlQueryHelpers.JoinBaseTables(queriedClass, tempTable1, property.ClassDefinition, m_sqlQueryBuilder, m_tac);
                    }

                TableDescriptor tempTable2 = JoinInternalColumn(tempTable1, dbColumn);
                string columnName = dbColumn["ColumnName"].StringValue;
                var isSpatialProperty = dbColumn["IsSpatial"];

                ColumnCategory columnCategory = ColumnCategory.spatialInstanceData;

                if ( isSpatialProperty.IsNull || isSpatialProperty.StringValue.ToLower() == "false" )
                    {
                    columnCategory = ColumnCategory.instanceData;
                    }

                m_sqlQueryBuilder.AddSelectClause(tempTable2, columnName, columnCategory, property);


                }
            return table;
            }

        private void extractOrderByClause (IECClass queriedClass, TableDescriptor primaryTable)
            {

            for ( int i = 0; i < m_query.OrderBy.Count; i++ )
                {
                m_query.OrderBy.MoveNext();
                IECProperty property = queriedClass[m_query.OrderBy.Current.PropertyAccessor];
                IECInstance dbColumn = property.GetCustomAttributes("DBColumn");

                if ( dbColumn == null )
                    {
                    //This is not a column in the sql table. We skip it...
                    continue;
                    }

                string columnName = dbColumn["ColumnName"].StringValue;

                TableDescriptor propertyTableName = JoinInternalColumn(primaryTable, dbColumn);

                m_sqlQueryBuilder.AddOrderByClause(propertyTableName, columnName, m_query.OrderBy.Current.SortAscending);

                }
            }

        /// <summary>
        /// This method's purpose is to join recursively all tables in the hierarchy of a class, down to a specified derived class
        /// </summary>
        /// <param name="queriedClass">The base class of which we need the derived class table joined</param>
        /// <param name="tempTable1">The table descriptor of the queried class</param>
        /// <param name="finalDerivedClass">The specified derived base class to have its table joined</param>
        /// <returns>The final class' table descriptor</returns>
        private TableDescriptor JoinDerivedTables (IECClass queriedClass, TableDescriptor tempTable1, IECClass finalDerivedClass)
            {
            if ( queriedClass != finalDerivedClass )
                {

                //if (queriedClass.DerivedClasses.Count() != 1)
                //{
                //    throw new ProgrammerException("IndexECPlugin only supports classes that have at most one derived class.");
                //}
                IEnumerable<IECClass> derivedClasses = queriedClass.DerivedClasses.Where(x => firstIsDerivedFromSecond(finalDerivedClass, x));

                IECClass derivedClass = derivedClasses.First();

                if ( queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue !=
                    derivedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue )
                    {
                    string baseClassKeyPropertyName;
                    string derivedClassKeyPropertyName;

                    if ( queriedClass.GetCustomAttributes("DerivedClassLinker") != null )
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
                    TableDescriptor newTable = new TableDescriptor(derivedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue, m_tac.GetNewTableAlias());

                    string baseClassKeyColumn = queriedClass[baseClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;
                    string derivedClassKeyColumn = derivedClass[derivedClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;

                    newTable.SetTableJoined(tempTable1, baseClassKeyColumn, derivedClassKeyColumn);

                    TableDescriptor similarTable;
                    bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(newTable, out similarTable);
                    if ( !joinSuccessful )
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
        private bool firstIsDerivedFromSecond (IECClass firstClass, IECClass secondClass)
            {
            //If firstClass is the second class, we consider
            if ( firstClass.Name == secondClass.Name )
                {
                return true;
                }

            //Is the classes are not the same and there is no base class to the first, the we are 
            if ( firstClass.BaseClasses == null )
                {
                return false;
                }

            int baseClassCount = firstClass.BaseClasses.Count();
            if ( baseClassCount > 1 )
                {
                throw new ProgrammerException("IndexECPlugin only supports classes that have at most one derived class.");
                }


            if ( firstClass.BaseClasses.First().Name == secondClass.Name )
                {
                return true;
                }
            else
                {
                return firstIsDerivedFromSecond(firstClass.BaseClasses.First(), secondClass);
                }
            }


        private TableDescriptor JoinInternalColumn (TableDescriptor firstTableDescriptor, IECInstance dbColumn)
            {
            var propertyTableAttribute = dbColumn["JoinTableName"];
            string propertyTableName = firstTableDescriptor.Name;
            TableDescriptor newPropertyTableDescriptor = firstTableDescriptor;
            if ( !propertyTableAttribute.IsNull )
                {
                propertyTableName = propertyTableAttribute.StringValue;
                }

            if ( propertyTableName != firstTableDescriptor.Name )
                {
                var firstTableKeyAttribute = dbColumn["FirstTableKey"];
                var newTableKeyAttribute = dbColumn["NewTableKey"];

                if ( firstTableKeyAttribute.IsNull || newTableKeyAttribute.IsNull )
                    {
                    throw new ProgrammerException("The ECSchema was incorrectly written. If the JoinTableName property is not null, the FirstTableKey and NewTableKey must be specified.");
                    }
                string firstTableKey = firstTableKeyAttribute.StringValue;
                string newTableKey = newTableKeyAttribute.StringValue;

                if ( String.IsNullOrWhiteSpace(firstTableKey) || String.IsNullOrWhiteSpace(newTableKey) )
                    {
                    throw new ProgrammerException("The ECSchema was incorrectly written. If the JoinTableName property is not null, the FirstTableKey and NewTableKey must be specified.");
                    }

                newPropertyTableDescriptor = new TableDescriptor(propertyTableName, m_tac.GetNewTableAlias());
                newPropertyTableDescriptor.SetTableJoined(firstTableDescriptor, firstTableKey, newTableKey);

                TableDescriptor similarTable;
                bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(newPropertyTableDescriptor, out similarTable);
                if ( !joinSuccessful )
                    {
                    newPropertyTableDescriptor = similarTable;
                    }
                }
            return newPropertyTableDescriptor;
            }

        private void extractWhereClause (WhereCriteria whereCriteria, IECClass queriedClass, TableDescriptor table/*, bool forRelatedCriterion = false*/)
            {
            m_sqlQueryBuilder.StartOfInnerWhereClause();
            for ( int i = 0; i < whereCriteria.Count; i++ )
                {
                if ( i > 0 )
                    {
                    m_sqlQueryBuilder.AddOperatorToWhereClause(whereCriteria.GetLogicalOperatorBefore(i));
                    }
                WhereCriterion criterion = whereCriteria[i];

                if ( criterion is PropertyExpression )
                    {
                    PropertyExpression propertyExpression = (PropertyExpression) criterion;

                    string primaryTableName = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue;

                    IECProperty property = propertyExpression.LeftSideProperty;
                    IECInstance dbColumn = property.GetCustomAttributes("DBColumn");
                    if ( dbColumn == null )
                        {
                        //This is not a column in the sql table. We skip it...
                        continue;
                        }

                    TableDescriptor tempTable1 = table;

                    if ( queriedClass != property.ClassDefinition )
                        {
                        tempTable1 = SqlQueryHelpers.JoinBaseTables(queriedClass, tempTable1, property.ClassDefinition, m_sqlQueryBuilder, m_tac);
                        }

                    TableDescriptor propertyTable = JoinInternalColumn(tempTable1, dbColumn);

                    string columnName = dbColumn.GetPropertyValue("ColumnName").StringValue;

                    //******KEEP THESE COMMENTED LINES******************************************************************************
                    //var isSpatial = dbColumn["IsSpatial"];
                    //if (isSpatial.IsNull || isSpatial.StringValue == "false")
                    //{
                    m_sqlQueryBuilder.AddWhereClause(propertyTable.Alias, columnName, propertyExpression.Operator, propertyExpression.RightSideString, ECToSQLMap.ECTypeToDbType(property.Type));
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
                else if ( criterion is ECInstanceIdExpression ) // TODO also add support for ECKeyExpression
                    {

                    ECInstanceIdExpression instanceIDExpression = (ECInstanceIdExpression) criterion;

                    if ( instanceIDExpression.InstanceIdSet.Count() == 0 )
                        {
                        throw new ProgrammerException("The array of IDs in the ECInstanceIdExpression is empty.");
                        }

                    string idProperty = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                    IECProperty prop = queriedClass[idProperty];
                    IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");
                    //if ( dbColumn == null )
                    //    {
                    //    //This is not a column in the sql table. We skip it...
                    //    continue;
                    //    }
                    string dbColumnName = dbColumn["ColumnName"].StringValue;

                    IECType ecType = prop.Type;

                    m_sqlQueryBuilder.StartOfInnerWhereClause();

                    for ( int j = 0; j < instanceIDExpression.InstanceIdSet.Count(); j++ )
                        {
                        if ( j > 0 )
                            {
                            m_sqlQueryBuilder.AddOperatorToWhereClause(LogicalOperator.OR);
                            }
                        m_sqlQueryBuilder.AddWhereClause(table.Alias, dbColumnName, RelationalOperator.EQ, instanceIDExpression.InstanceIdSet[j], ECToSQLMap.ECTypeToDbType(ecType));

                        }
                    m_sqlQueryBuilder.EndOfInnerWhereClause();

                    if ( m_sqlQueryBuilder.OrderByListIsEmpty() )
                        {
                        //This is because we need an order by clause to make the query work. WSG makes a paged request
                        //when creating an ECInstanceIdExpression and SQL needs an OrderBy to complete it.
                        m_sqlQueryBuilder.AddOrderByClause(table, dbColumnName, true);
                        }

                    if ( criterion.ExtendedData.ContainsKey("RequestRelatedId") && ( (bool) criterion.ExtendedData["RequestRelatedId"] ) )
                        {
                        m_sqlQueryBuilder.AddSelectClause(table, dbColumnName, ColumnCategory.relatedInstanceId, null);
                        }
                    }
                else if ( criterion is RelatedCriterion )
                    {

                    var relatedCrit = criterion as RelatedCriterion;

                    var classSpecifier = relatedCrit.RelatedClassSpecifier;

                    IECRelationshipClass relationshipClass = classSpecifier.RelationshipClass;

                    IECInstance relationshipKeys = relationshipClass.GetCustomAttributes("RelationshipKeys");

                    IECClass relatedClass = classSpecifier.RelatedClass;

                    TableDescriptor finalTableDescriptor;

                    if ( relationshipKeys != null )
                        {
                        //string containerKey = relationshipKeys["ContainerKey"].StringValue;
                        //string containedKey = relationshipKeys["ContainedKey"].StringValue;

                        string dbColumnOfBaseQueriedClass;
                        string dbColumnOfBaseRelatedClass;

                        string baseQueriedClassName;
                        string baseRelatedClassName;

                        IECClass baseQueriedClass;
                        IECClass baseRelatedClass;

                        if ( classSpecifier.RelatedDirection == RelatedInstanceDirection.Forward )
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

                        baseQueriedClass = m_schema.GetClass(baseQueriedClassName);
                        baseRelatedClass = m_schema.GetClass(baseRelatedClassName);

                        //We go up the hierarchy of the base queried class
                        TableDescriptor baseQueriedTableDescriptor = table;

                        baseQueriedTableDescriptor = SqlQueryHelpers.JoinBaseTables(queriedClass, baseQueriedTableDescriptor, baseQueriedClass, m_sqlQueryBuilder, m_tac);

                        //We link both base classes
                        string relatedBaseTableName = baseRelatedClass.GetCustomAttributes("SQLEntity")["FromTableName"].StringValue;
                        TableDescriptor relatedBaseTableDescriptor = new TableDescriptor(relatedBaseTableName, m_tac.GetNewTableAlias());

                        relatedBaseTableDescriptor.SetTableJoined(baseQueriedTableDescriptor, dbColumnOfBaseQueriedClass, dbColumnOfBaseRelatedClass);

                        TableDescriptor similarTable;

                        bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(relatedBaseTableDescriptor, out similarTable);
                        if ( !joinSuccessful )
                            {
                            relatedBaseTableDescriptor = similarTable;
                            }

                        //We go up in the hierarchy of the table
                        finalTableDescriptor = JoinDerivedTables(baseRelatedClass, relatedBaseTableDescriptor, relatedClass);

                        //string relatedTableName = relatedClass.GetCustomAttributes("SQLEntity")["FromTableName"].StringValue;

                        //TableDescriptor relatedTableDescriptor = new TableDescriptor(relatedTableName, GetNewTableAlias());

                        //relatedTableDescriptor.SetTableJoined(table, dbColumnOfQueriedClass, dbColumnOfRelatedClass);

                        //TableDescriptor similarTable;

                        //bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(relatedTableDescriptor, out similarTable);
                        //if (!joinSuccessful)
                        //{
                        //    relatedTableDescriptor = similarTable;
                        //}

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


                        if ( classSpecifier.RelatedDirection == RelatedInstanceDirection.Forward )
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

                        baseQueriedClass = m_schema.GetClass(baseQueriedClassName);
                        baseRelatedClass = m_schema.GetClass(baseRelatedClassName);

                        string intermediateTableName = relationshipKeys["IntermediateTableName"].StringValue;
                        string relatedBaseTableName = baseRelatedClass.GetCustomAttributes("SQLEntity")["FromTableName"].StringValue;

                        //We go up the hierarchy of the base queried class
                        TableDescriptor baseQueriedTableDescriptor = table;

                        baseQueriedTableDescriptor = SqlQueryHelpers.JoinBaseTables(queriedClass, baseQueriedTableDescriptor, baseQueriedClass, m_sqlQueryBuilder, m_tac);

                        TableDescriptor intermediateTableDescriptor = new TableDescriptor(intermediateTableName, m_tac.GetNewTableAlias());
                        intermediateTableDescriptor.SetTableJoined(baseQueriedTableDescriptor, dbColumnOfBaseQueriedClass, dbColumnIntermediateQueried);

                        TableDescriptor similarTable;

                        bool joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(intermediateTableDescriptor, out similarTable);
                        if ( !joinSuccessful )
                            {
                            intermediateTableDescriptor = similarTable;
                            }

                        TableDescriptor relatedBaseTableDescriptor = new TableDescriptor(relatedBaseTableName, m_tac.GetNewTableAlias());
                        relatedBaseTableDescriptor.SetTableJoined(intermediateTableDescriptor, dbColumnIntermediateRelated, dbColumnOfBaseRelatedClass);

                        joinSuccessful = m_sqlQueryBuilder.AddLeftJoinClause(relatedBaseTableDescriptor, out similarTable);
                        if ( !joinSuccessful )
                            {
                            relatedBaseTableDescriptor = similarTable;
                            }

                        finalTableDescriptor = JoinDerivedTables(baseRelatedClass, relatedBaseTableDescriptor, relatedClass);

                        }

                    if ( criterion.ExtendedData.ContainsKey("RequestRelatedId") && ((bool) criterion.ExtendedData["RequestRelatedId"]) )
                        {
                        string idProperty = relatedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                        IECProperty prop = relatedClass[idProperty];
                        IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");

                        string dbColumnName = dbColumn["ColumnName"].StringValue;

                        m_sqlQueryBuilder.AddSelectClause(finalTableDescriptor, dbColumnName, ColumnCategory.relatedInstanceId, null);
                        }

                    extractWhereClause(relatedCrit.RelatedWhereCriteria, relatedClass, finalTableDescriptor);

                    }
                else if ( criterion is WhereCriteria )
                    {

                    var whereCrit = criterion as WhereCriteria;

                    extractWhereClause(whereCrit, queriedClass, table/*, forRelatedCriterion*/);

                    }
                }
            m_sqlQueryBuilder.EndOfInnerWhereClause();
            }

        private void extractSpatialWhereClause (WhereCriteria whereCriteria, IECClass queriedClass, TableDescriptor tableToJoin)
            {
            if ( whereCriteria.Count > 0 )
                {
                m_sqlQueryBuilder.AddOperatorToWhereClause(LogicalOperator.AND);
                }

            TableDescriptor propertyTable = tableToJoin;
            string columnName = null;
            foreach ( var prop in queriedClass )
                {
                IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");
                if ( dbColumn == null )
                    {
                    //This is not a column in the sql table. We skip it...
                    continue;
                    }
                var isSpatialProperty = dbColumn["IsSpatial"];
                if ( !isSpatialProperty.IsNull && isSpatialProperty.StringValue.ToLower() == "true" )
                    {
                    columnName = dbColumn.GetPropertyValue("ColumnName").StringValue;

                    if ( queriedClass != prop.ClassDefinition )
                        {
                        propertyTable = SqlQueryHelpers.JoinBaseTables(queriedClass, propertyTable, prop.ClassDefinition, m_sqlQueryBuilder, m_tac);
                        }

                    propertyTable = JoinInternalColumn(propertyTable, dbColumn);
                    break;
                    }
                }
            if ( String.IsNullOrWhiteSpace(columnName) )
                {
                throw new UserFriendlyException("The class for which you requested a spatial query does not have any spatial property.");
                }

            m_sqlQueryBuilder.AddSpatialIntersectsWhereClause(propertyTable.Alias, columnName, m_polygonDescriptor.WKT, m_polygonDescriptor.SRID);

            }

        //private string GetNewTableAlias ()
        //    {
        //    return String.Format("tab{0}", m_tabNumber++);
        //    }
        }
    }
